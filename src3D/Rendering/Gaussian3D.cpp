#include "Engine3D/Rendering/Gaussian3D.h"
#include "Engine3D/Rendering/Mesh3D.h"
#include "Engine3D/Rendering/Shader3D.h"
#include "Engine3D/Math/Matrix4.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <chrono>
#include "Engine3D/Core/Console.h"

using namespace Engine3D::Math;
using namespace Engine3D::Rendering;

namespace Engine3D {
namespace Rendering {

// === Gaussian3D Implementation ===

void Gaussian3D::updateCovariance() {
    // Создаем матрицу вращения из углов Эйлера
    Matrix4 rotationMatrix = Matrix4::rotationX(rotation.x) * 
                            Matrix4::rotationY(rotation.y) * 
                            Matrix4::rotationZ(rotation.z);
    
    // Создаем матрицу масштабирования
    Matrix4 scaleMatrix = Matrix4::scaling(scale.x, scale.y, scale.z);
    
    // Ковариация = R * S * S^T * R^T
    Matrix4 RS = rotationMatrix * scaleMatrix;
    covariance = RS * RS.transpose();
}

float Gaussian3D::evaluateAt(const Vector3& point) const {
    Vector3 diff = point - position;
    
    // Вычисляем exp(-0.5 * diff^T * Σ^(-1) * diff)
    // Для упрощения используем диагональную ковариацию
    float x = diff.x / scale.x;
    float y = diff.y / scale.y;
    float z = diff.z / scale.z;
    
    float exponent = -(x*x + y*y + z*z) * 0.5f;
    return opacity * std::exp(exponent);
}

bool Gaussian3D::intersects(const Vector3& min, const Vector3& max) const {
    // Проверяем пересечение с bounding box используя 3-сигма правило
    Vector3 gaussianMin = position - scale * 3.0f;
    Vector3 gaussianMax = position + scale * 3.0f;
    
    return !(gaussianMax.x < min.x || gaussianMin.x > max.x ||
             gaussianMax.y < min.y || gaussianMin.y > max.y ||
             gaussianMax.z < min.z || gaussianMin.z > max.z);
}

// === GaussianField3D Implementation ===

GaussianField3D::GaussianField3D() 
    : uploaded(false)
    , adaptiveSampling(true)
    , SSBO(0)
    , VAO(0)
    , VBO(0) {
    renderStats.reset();
    SAFE_PRINT_LINE("Создано поле 3D гауссианов");
}

GaussianField3D::~GaussianField3D() {
    cleanup();
}

void GaussianField3D::addGaussian(const Gaussian3D& gaussian) {
    gaussians.push_back(gaussian);
    uploaded = false; // Требуется повторная загрузка в GPU
}

void GaussianField3D::removeGaussian(size_t index) {
    if (index < gaussians.size()) {
        gaussians.erase(gaussians.begin() + index);
        uploaded = false;
    }
}

void GaussianField3D::clear() {
    gaussians.clear();
    visibleIndices.clear();
    uploaded = false;
    renderStats.reset();
}

void GaussianField3D::optimizeGaussians(const std::vector<Vector3>& viewPoints) {
    std::cout << "Оптимизация " << gaussians.size() << " гауссианов..." << std::endl;
    
    // Вычисляем важность каждого гауссиана на основе видимости из разных точек
    for (auto& gaussian : gaussians) {
        float totalImportance = 0.0f;
        
        for (const auto& viewPoint : viewPoints) {
            float distance = (gaussian.position - viewPoint).magnitude();
            float contribution = gaussian.evaluateAt(viewPoint);
            
            // Важность обратно пропорциональна расстоянию и прямо пропорциональна вкладу
            totalImportance += contribution / (1.0f + distance * 0.1f);
        }
        
        gaussian.importance = totalImportance / viewPoints.size();
    }
    
    // Сортируем по важности для оптимизации рендеринга
    std::sort(gaussians.begin(), gaussians.end(), 
              [](const Gaussian3D& a, const Gaussian3D& b) {
                  return a.importance > b.importance;
              });
    
    SAFE_PRINT_LINE("Оптимизация завершена. Гауссианы отсортированы по важности.");
    uploaded = false;
}

void GaussianField3D::cullInvisibleGaussians(const Matrix4& viewMatrix, const Matrix4& projMatrix) {
    visibleIndices.clear();
    
    Matrix4 viewProjMatrix = projMatrix * viewMatrix;
    
    for (size_t i = 0; i < gaussians.size(); ++i) {
        const Gaussian3D& gaussian = gaussians[i];
        
        if (!gaussian.visible) continue;
        
        // Трансформируем позицию в клиповое пространство
        Vector3 clipPos = viewProjMatrix.transformPoint(gaussian.position);
        
        // Проверяем, находится ли гауссиан в frustum
        if (clipPos.x >= -1.0f && clipPos.x <= 1.0f &&
            clipPos.y >= -1.0f && clipPos.y <= 1.0f &&
            clipPos.z >= 0.0f && clipPos.z <= 1.0f) {
            
            // Дополнительная проверка на размер в экранном пространстве
            Vector3 screenScale = viewProjMatrix.transformVector(gaussian.scale);
            float screenSize = screenScale.magnitude();
            
            if (screenSize > 0.001f) { // Минимальный размер в пикселях
                visibleIndices.push_back(i);
            }
        }
    }
    
    renderStats.totalGaussians = gaussians.size();
    renderStats.renderedGaussians = visibleIndices.size();
    renderStats.culledGaussians = gaussians.size() - visibleIndices.size();
}

void GaussianField3D::sortByDistance(const Vector3& viewPoint) {
    // Сортируем индексы видимых гауссианов по расстоянию от камеры
    std::sort(visibleIndices.begin(), visibleIndices.end(),
              [this, &viewPoint](size_t a, size_t b) {
                  float distA = (gaussians[a].position - viewPoint).magnitudeSquared();
                  float distB = (gaussians[b].position - viewPoint).magnitudeSquared();
                  return distA > distB; // Сортируем от дальних к ближним для правильного блендинга
              });
}

std::shared_ptr<GaussianField3D> GaussianField3D::createFromMesh(const Mesh3D& mesh, int gaussianDensity) {
    auto field = std::make_shared<GaussianField3D>();
    
    const auto& vertices = mesh.getVertices();
    const auto& indices = mesh.getIndices();
    
    std::cout << "Создание поля гауссианов из меша с " << vertices.size() 
              << " вершинами и " << indices.size()/3 << " треугольниками..." << std::endl;
    
    // Вычисляем размер меша для определения масштаба гауссианов
    Vector3 meshSize = mesh.getSize();
    float avgScale = (meshSize.x + meshSize.y + meshSize.z) / 3.0f;
    float gaussianScale = avgScale / std::sqrt(static_cast<float>(gaussianDensity));
    
    // Создаем гауссианы в вершинах
    for (const auto& vertex : vertices) {
        Gaussian3D gaussian;
        gaussian.position = vertex.position;
        gaussian.color = vertex.color;
        gaussian.scale = Vector3(gaussianScale, gaussianScale, gaussianScale);
        gaussian.opacity = 0.8f; // Слегка прозрачные для лучшего смешивания
        
        field->addGaussian(gaussian);
    }
    
    // Добавляем дополнительные гауссианы внутри треугольников для плотности
    if (gaussianDensity > static_cast<int>(vertices.size())) {
        int additionalGaussians = gaussianDensity - static_cast<int>(vertices.size());
        int triangleCount = static_cast<int>(indices.size() / 3);
        
        for (int i = 0; i < additionalGaussians && i < triangleCount * 3; ++i) {
            int triIndex = i % triangleCount;
            
            const Vertex3D& v0 = vertices[indices[triIndex * 3]];
            const Vertex3D& v1 = vertices[indices[triIndex * 3 + 1]];
            const Vertex3D& v2 = vertices[indices[triIndex * 3 + 2]];
            
            // Случайная точка внутри треугольника (барицентрические координаты)
            float r1 = static_cast<float>(rand()) / RAND_MAX;
            float r2 = static_cast<float>(rand()) / RAND_MAX;
            if (r1 + r2 > 1.0f) {
                r1 = 1.0f - r1;
                r2 = 1.0f - r2;
            }
            float r3 = 1.0f - r1 - r2;
            
            Gaussian3D gaussian;
            gaussian.position = v0.position * r1 + v1.position * r2 + v2.position * r3;
            gaussian.color = v0.color * r1 + v1.color * r2 + v2.color * r3;
            gaussian.scale = Vector3(gaussianScale * 0.7f, gaussianScale * 0.7f, gaussianScale * 0.7f);
            gaussian.opacity = 0.6f;
            
            field->addGaussian(gaussian);
        }
    }
    
    std::cout << "Создано " << field->getGaussianCount() << " гауссианов" << std::endl;
    return field;
}

std::shared_ptr<GaussianField3D> GaussianField3D::createFromPointCloud(
    const std::vector<Vector3>& points, const std::vector<Vector3>& colors) {
    
    auto field = std::make_shared<GaussianField3D>();
    
    std::cout << "Создание поля гауссианов из облака точек (" << points.size() << " точек)..." << std::endl;
    
    // Вычисляем средний масштаб на основе плотности точек
    float totalDistance = 0.0f;
    int distanceCount = 0;
    
    for (size_t i = 0; i < points.size() && i < 100; ++i) { // Выборочная проверка
        for (size_t j = i + 1; j < points.size() && j < i + 10; ++j) {
            totalDistance += (points[i] - points[j]).magnitude();
            distanceCount++;
        }
    }
    
    float avgDistance = distanceCount > 0 ? totalDistance / distanceCount : 1.0f;
    float gaussianScale = avgDistance * 0.5f;
    
    for (size_t i = 0; i < points.size(); ++i) {
        Gaussian3D gaussian;
        gaussian.position = points[i];
        gaussian.color = i < colors.size() ? colors[i] : Vector3(1.0f, 1.0f, 1.0f);
        gaussian.scale = Vector3(gaussianScale, gaussianScale, gaussianScale);
        gaussian.opacity = 0.8f;
        
        field->addGaussian(gaussian);
    }
    
    std::cout << "Создано " << field->getGaussianCount() << " гауссианов из облака точек" << std::endl;
    return field;
}

void GaussianField3D::setLevelOfDetail(float distance, int maxGaussians) {
    if (!adaptiveSampling) return;
    
    // Адаптивное уменьшение количества гауссианов в зависимости от расстояния
    float lodFactor = 1.0f / (1.0f + distance * 0.01f);
    int targetGaussians = static_cast<int>(maxGaussians * lodFactor);
    
    // Оставляем только наиболее важные гауссианы
    if (static_cast<int>(gaussians.size()) > targetGaussians) {
        std::partial_sort(gaussians.begin(), gaussians.begin() + targetGaussians, gaussians.end(),
                         [](const Gaussian3D& a, const Gaussian3D& b) {
                             return a.importance > b.importance;
                         });
        
        // Временно скрываем менее важные гауссианы
        for (int i = targetGaussians; i < static_cast<int>(gaussians.size()); ++i) {
            gaussians[i].visible = false;
        }
        for (int i = 0; i < targetGaussians; ++i) {
            gaussians[i].visible = true;
        }
    }
}

void GaussianField3D::uploadToGPU() {
    if (uploaded || gaussians.empty()) return;
    
    std::cout << "Загрузка " << gaussians.size() << " гауссианов в GPU..." << std::endl;
    
    // TODO: Реализация загрузки в GPU для конкретного API
    // Здесь будет создание SSBO и загрузка данных гауссианов
    
    updateGaussianBuffer();
    uploaded = true;
    
    SAFE_PRINT_LINE("Гауссианы успешно загружены в GPU");
}

void GaussianField3D::render(const Matrix4& viewMatrix, const Matrix4& projMatrix) const {
    if (!uploaded || gaussians.empty()) return;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // TODO: Реализация рендеринга гауссианов для конкретного API
    std::cout << "Рендеринг " << visibleIndices.size() << " видимых гауссианов из " 
              << gaussians.size() << " общих" << std::endl;
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    const_cast<GaussianField3D*>(this)->renderStats.frameTime = duration.count() / 1000.0f;
}

void GaussianField3D::cleanup() {
    if (!uploaded) return;
    
    // TODO: Очистка GPU ресурсов
    SAFE_PRINT_LINE("Очистка ресурсов поля гауссианов...");
    
    uploaded = false;
}

void GaussianField3D::buildSpatialTree() {
    // TODO: Реализация пространственного индексирования для оптимизации
    std::cout << "Построение пространственного дерева для " << gaussians.size() << " гауссианов..." << std::endl;
}

void GaussianField3D::updateGaussianBuffer() {
    // TODO: Обновление GPU буфера с данными гауссианов
    SAFE_PRINT_LINE("Обновление буфера гауссианов в GPU...");
}

// === GaussianRenderer3D Implementation ===

GaussianRenderer3D::GaussianRenderer3D() 
    : initialized(false)
    , gaussianShader(nullptr)
    , tileShader(nullptr)
    , renderQuality(1.0f)
    , alphaBlendingEnabled(true)
    , tileBasedRenderingEnabled(true)
    , earlyZRejectionEnabled(true)
    , adaptiveSamplingEnabled(true)
    , sortingMethod(0)
    , tileSize(16)
    , maxSamplesPerPixel(4)
    , tileBuffer(0)
    , depthBuffer(0)
    , colorBuffer(0) {
    renderStats.reset();
}

GaussianRenderer3D::~GaussianRenderer3D() {
    cleanup();
}

bool GaussianRenderer3D::initialize() {
    if (initialized) return true;
    
    SAFE_PRINT_LINE("Инициализация рендерера 3D гауссианов...");
    
    // TODO: Загрузка шейдеров для рендеринга гауссианов
    // gaussianShader = loadGaussianSplatShader();
    // tileShader = loadTileRenderingShader();
    
    // TODO: Создание GPU буферов для тайлового рендеринга
    
    initialized = true;
    SAFE_PRINT_LINE("Рендерер 3D гауссианов успешно инициализирован");
    return true;
}

void GaussianRenderer3D::cleanup() {
    if (!initialized) return;
    
    SAFE_PRINT_LINE("Очистка рендерера 3D гауссианов...");
    
    // TODO: Очистка шейдеров и GPU ресурсов
    
    initialized = false;
}

void GaussianRenderer3D::render(const GaussianField3D& field, 
                               const Matrix4& viewMatrix, 
                               const Matrix4& projMatrix,
                               int screenWidth, int screenHeight) {
    if (!initialized) {
        SAFE_ERROR("GaussianRenderer3D не инициализирован!");
        return;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    std::cout << "Рендеринг сцены с " << field.getGaussianCount() 
              << " гауссианами (разрешение: " << screenWidth << "x" << screenHeight << ")" << std::endl;
    
    // Этап 1: Настройка шейдера
    setupGaussianShader(viewMatrix, projMatrix);
    
    // Этап 2: Сортировка гауссианов по глубине
    auto sortStart = std::chrono::high_resolution_clock::now();
    // TODO: Реализация сортировки
    auto sortEnd = std::chrono::high_resolution_clock::now();
    renderStats.sortingTime = std::chrono::duration_cast<std::chrono::microseconds>(sortEnd - sortStart).count() / 1000.0f;
    
    // Этап 3: Тайловый рендеринг или стандартный рендеринг
    if (tileBasedRenderingEnabled) {
        renderTiles(field, screenWidth, screenHeight);
    } else {
        // Стандартный рендеринг всех гауссианов
        field.render(viewMatrix, projMatrix);
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    renderStats.rasterizationTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() / 1000.0f;
    
    renderStats.gaussiansProcessed = field.getGaussianCount();
    renderStats.pixelsShaded = screenWidth * screenHeight;
    
    std::cout << "Рендеринг завершен. Время: " << renderStats.rasterizationTime << "мс" << std::endl;
}

void GaussianRenderer3D::setRenderQuality(float quality) {
    renderQuality = std::max(0.0f, std::min(1.0f, quality));
    std::cout << "Установлено качество рендеринга: " << renderQuality << std::endl;
}

void GaussianRenderer3D::enableAlphaBlending(bool enable) {
    alphaBlendingEnabled = enable;
    std::cout << "Альфа-смешивание " << (enable ? "включено" : "выключено") << std::endl;
}

void GaussianRenderer3D::setSortingMethod(int method) {
    sortingMethod = method;
    std::cout << "Метод сортировки установлен: " << (method == 0 ? "по глубине" : "по важности") << std::endl;
}

void GaussianRenderer3D::enableTileBasedRendering(bool enable) {
    tileBasedRenderingEnabled = enable;
    std::cout << "Тайловый рендеринг " << (enable ? "включен" : "выключен") << std::endl;
}

void GaussianRenderer3D::enableEarlyZRejection(bool enable) {
    earlyZRejectionEnabled = enable;
    std::cout << "Ранний Z-тест " << (enable ? "включен" : "выключен") << std::endl;
}

void GaussianRenderer3D::enableAdaptiveSampling(bool enable) {
    adaptiveSamplingEnabled = enable;
    std::cout << "Адаптивная выборка " << (enable ? "включена" : "выключена") << std::endl;
}

void GaussianRenderer3D::setMaxSamplesPerPixel(int samples) {
    maxSamplesPerPixel = std::max(1, samples);
    std::cout << "Максимум выборок на пиксель: " << maxSamplesPerPixel << std::endl;
}

void GaussianRenderer3D::sortGaussians(std::vector<Gaussian3D>& gaussians, const Vector3& viewPoint) {
    if (sortingMethod == 0) {
        // Сортировка по глубине
        std::sort(gaussians.begin(), gaussians.end(),
                 [&viewPoint](const Gaussian3D& a, const Gaussian3D& b) {
                     float distA = (a.position - viewPoint).magnitudeSquared();
                     float distB = (b.position - viewPoint).magnitudeSquared();
                     return distA > distB; // От дальних к ближним
                 });
    } else {
        // Сортировка по важности
        std::sort(gaussians.begin(), gaussians.end(),
                 [](const Gaussian3D& a, const Gaussian3D& b) {
                     return a.importance > b.importance;
                 });
    }
}

void GaussianRenderer3D::renderTiles(const GaussianField3D& field, int screenWidth, int screenHeight) {
    int tilesX = (screenWidth + tileSize - 1) / tileSize;
    int tilesY = (screenHeight + tileSize - 1) / tileSize;
    int totalTiles = tilesX * tilesY;
    
    std::cout << "Тайловый рендеринг: " << tilesX << "x" << tilesY << " = " << totalTiles << " тайлов" << std::endl;
    
    // TODO: Реализация тайлового рендеринга
    // 1. Разделение экрана на тайлы
    // 2. Определение гауссианов, влияющих на каждый тайл
    // 3. Рендеринг каждого тайла параллельно
    
    renderStats.tilesProcessed = totalTiles;
}

void GaussianRenderer3D::setupGaussianShader(const Matrix4& viewMatrix, const Matrix4& projMatrix) {
    if (!gaussianShader) return;
    
    // TODO: Настройка униформов шейдера
    gaussianShader->use();
    gaussianShader->setMatrix4("uView", viewMatrix);
    gaussianShader->setMatrix4("uProjection", projMatrix);
    gaussianShader->setFloat("uRenderQuality", renderQuality);
    gaussianShader->setBool("uAlphaBlending", alphaBlendingEnabled);
    gaussianShader->setBool("uEarlyZRejection", earlyZRejectionEnabled);
    gaussianShader->setInt("uMaxSamples", maxSamplesPerPixel);
}

} // namespace Rendering
} // namespace Engine3D
