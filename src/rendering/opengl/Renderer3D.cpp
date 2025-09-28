#include "HyperEngine/Rendering/Renderer3D.h"
#include "HyperEngine/Math/Matrix4.h"
#include <iostream>
#include <chrono>
#include "HyperEngine/Core/Console.h"
#include "HyperEngine/Core/SafeConsole.h"

using namespace HyperEngine::Math;
using namespace HyperEngine::Rendering;
using namespace HyperEngine::Core;

namespace HyperEngine {
namespace Rendering {

// Получение singleton экземпляра
Renderer3D& Renderer3D::getInstance() {
    static Renderer3D instance;
    return instance;
}

// Инициализация и очистка
bool Renderer3D::initialize(int width, int height) {
    if (initialized) {
        SAFE_PRINT_LINE("Renderer3D already initialized");
        return true;
    }
    
    screenWidth = width;
    screenHeight = height;
    
    // Инициализация графического API (заглушка)
    SAFE_PRINT_LINE("Initializing Renderer3D with resolution " + SAFE_TO_STRING(width) + "x" + SAFE_TO_STRING(height));
    
    // Настройки по умолчанию
    setClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    enableDepthTest(true);
    enableBlending(false);
    enableWireframe(false);
    enableBackfaceCulling(true);
    
    setAmbientLight(Vector3(0.2f, 0.2f, 0.2f));
    
    // Создаем камеру по умолчанию
    mainCamera = std::make_shared<Camera3D>();
    mainCamera->setPerspective(60.0f, (float)width / height, 0.1f, 1000.0f);
    mainCamera->setPosition(Vector3(0, 0, 5));
    mainCamera->lookAt(Vector3(0, 0, 5), Vector3(0, 0, 0), Vector3(0, 1, 0));
    
    initialized = true;
    resetRenderStats();
    
    SAFE_PRINT_LINE("Renderer3D initialized successfully");
    return true;
}

void Renderer3D::cleanup() {
    if (!initialized) {
        return;
    }
    
    SAFE_PRINT_LINE("Cleaning up Renderer3D...");
    
    clearLights();
    mainCamera.reset();
    
    // TODO: Очистка ресурсов графического API
    
    initialized = false;
    SAFE_PRINT_LINE("Renderer3D cleaned up");
}

// Управление кадром
void Renderer3D::beginFrame() {
    if (!initialized) {
        SAFE_ERROR("Renderer3D not initialized!");
        return;
    }
    
    resetRenderStats();
    
    // TODO: Начало кадра для конкретного API
    SAFE_PRINT_LINE("Beginning frame...");
    
    clear();
    setupRenderState();
}

void Renderer3D::endFrame() {
    if (!initialized) {
        return;
    }
    
    // TODO: Завершение кадра для конкретного API
    SAFE_PRINT_LINE("Ending frame. Stats: " + 
                     SAFE_TO_STRING(renderStats.drawCalls) + " draw calls, " +
                     SAFE_TO_STRING(renderStats.trianglesRendered) + " triangles, " +
                     SAFE_TO_STRING(renderStats.verticesProcessed) + " vertices");
}

void Renderer3D::clear() {
    // TODO: Очистка буферов для конкретного API
    SAFE_PRINT_LINE("Clearing buffers with color (" + 
                     SAFE_TO_STRING(clearColor.x) + ", " + 
                     SAFE_TO_STRING(clearColor.y) + ", " + 
                     SAFE_TO_STRING(clearColor.z) + ")");
}

// Настройки рендеринга
void Renderer3D::setClearColor(float r, float g, float b, float a) {
    clearColor = Vector3(r, g, b);
    SAFE_PRINT_LINE("Set clear color to (" + SAFE_TO_STRING(r) + ", " + SAFE_TO_STRING(g) + ", " + SAFE_TO_STRING(b) + ", " + SAFE_TO_STRING(a) + ")");
}

void Renderer3D::setViewport(int x, int y, int width, int height) {
    // TODO: Установка viewport для конкретного API
    SAFE_PRINT_LINE("Set viewport to (" + SAFE_TO_STRING(x) + ", " + SAFE_TO_STRING(y) + ", " + SAFE_TO_STRING(width) + ", " + SAFE_TO_STRING(height) + ")");
}

void Renderer3D::enableDepthTest(bool enable) {
    depthTestEnabled = enable;
    SAFE_PRINT_LINE("Depth test " + std::string(enable ? "enabled" : "disabled"));
}

void Renderer3D::enableBlending(bool enable) {
    blendingEnabled = enable;
    SAFE_PRINT_LINE("Blending " + std::string(enable ? "enabled" : "disabled"));
}

void Renderer3D::enableWireframe(bool enable) {
    wireframeEnabled = enable;
    SAFE_PRINT_LINE("Wireframe mode " + std::string(enable ? "enabled" : "disabled"));
}

void Renderer3D::enableBackfaceCulling(bool enable) {
    backfaceCullingEnabled = enable;
    SAFE_PRINT_LINE("Backface culling " + std::string(enable ? "enabled" : "disabled"));
}

// Управление камерой
void Renderer3D::setMainCamera(std::shared_ptr<Camera3D> camera) {
    mainCamera = camera;
    if (camera) {
        camera->setAspectRatio((float)screenWidth / screenHeight);
        SAFE_PRINT_LINE("Set main camera");
    }
}

// Рендеринг объектов
void Renderer3D::renderMesh(const Mesh3D& mesh, const Math::Matrix4& transform, const Shader3D& shader) {
    if (!initialized || !mainCamera) {
        SAFE_ERROR("Renderer3D not properly initialized!");
        return;
    }
    
    SAFE_PRINT_LINE("Rendering mesh with " + SAFE_TO_STRING(mesh.getTriangleCount()) + " triangles");
    
    // Используем шейдер
    shader.use();
    
    // Устанавливаем матрицы
    setupMatrices(shader, transform);
    
    // Устанавливаем освещение
    setupLighting(shader);
    
    // Рендерим меш
    mesh.render();
    
    // Обновляем статистику
    updateRenderStats(mesh);
}

void Renderer3D::renderMesh(std::shared_ptr<Mesh3D> mesh, const Math::Matrix4& transform, std::shared_ptr<Shader3D> shader) {
    if (mesh && shader) {
        renderMesh(*mesh, transform, *shader);
    }
}

void Renderer3D::renderWireframe(const Mesh3D& mesh, const Math::Matrix4& transform, const Shader3D& shader) {
    bool wasWireframe = wireframeEnabled;
    this->enableWireframe(true);
    
    renderMesh(mesh, transform, shader);
    
    this->enableWireframe(wasWireframe);
}

// Система освещения
void Renderer3D::addLight(const Light& light) {
    lights.push_back(light);
    SAFE_PRINT_LINE("Added light. Total lights: " + SAFE_TO_STRING(lights.size()));
}

void Renderer3D::removeLight(size_t index) {
    if (index < lights.size()) {
        lights.erase(lights.begin() + index);
        SAFE_PRINT_LINE("Removed light. Remaining lights: " + SAFE_TO_STRING(lights.size()));
    }
}

void Renderer3D::clearLights() {
    lights.clear();
    SAFE_PRINT_LINE("Cleared all lights");
}

// Обработка изменения размера окна
void Renderer3D::onWindowResize(int width, int height) {
    screenWidth = width;
    screenHeight = height;
    
    if (mainCamera) {
        mainCamera->setAspectRatio((float)width / height);
    }
    
    setViewport(0, 0, width, height);
    SAFE_PRINT_LINE("Window resized to " + SAFE_TO_STRING(width) + "x" + SAFE_TO_STRING(height));
}

// Вспомогательные методы
void Renderer3D::setupRenderState() {
    // TODO: Настройка состояния рендеринга для конкретного API
    SAFE_PRINT_LINE("Setting up render state...");
}

void Renderer3D::setupLighting(const Shader3D& shader) {
    // Устанавливаем ambient освещение
    shader.setVector3("uAmbientLight", ambientLight);
    
    // Устанавливаем количество источников света
    shader.setInt("uNumLights", static_cast<int>(lights.size()));
    
    // Устанавливаем параметры каждого источника света
    for (size_t i = 0; i < lights.size() && i < 8; ++i) { // Ограничиваем до 8 источников
        const Light& light = lights[i];
        std::string prefix = "uLights[" + SAFE_TO_STRING(i) + "].";
        
        shader.setInt(prefix + "type", static_cast<int>(light.type));
        shader.setVector3(prefix + "position", light.position);
        shader.setVector3(prefix + "direction", light.direction);
        shader.setVector3(prefix + "color", light.color);
        shader.setFloat(prefix + "intensity", light.intensity);
        shader.setFloat(prefix + "range", light.range);
        shader.setFloat(prefix + "innerCone", light.innerCone);
        shader.setFloat(prefix + "outerCone", light.outerCone);
        shader.setBool(prefix + "enabled", light.enabled);
    }
    
    // Устанавливаем позицию камеры для specular освещения
    if (mainCamera) {
        shader.setVector3("uViewPos", mainCamera->getPosition());
    }
}

void Renderer3D::setupMatrices(const Shader3D& shader, const Math::Matrix4& modelMatrix) {
    if (!mainCamera) {
        return;
    }
    
    Math::Matrix4 viewMatrix = mainCamera->getViewMatrix();
    Math::Matrix4 projectionMatrix = mainCamera->getProjectionMatrix();
    
    shader.setMatrix4("uModel", modelMatrix);
    shader.setMatrix4("uView", viewMatrix);
    shader.setMatrix4("uProjection", projectionMatrix);
    
    // MVP матрица для оптимизации
    Math::Matrix4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;
    shader.setMatrix4("uMVP", mvpMatrix);
    
    // Normal matrix для правильного преобразования нормалей
    Math::Matrix4 normalMatrix = modelMatrix.inverse().transpose();
    shader.setMatrix4("uNormal", normalMatrix);
}

void Renderer3D::updateRenderStats(const Mesh3D& mesh) {
    renderStats.drawCalls++;
    renderStats.trianglesRendered += mesh.getTriangleCount();
    renderStats.verticesProcessed += mesh.getVertexCount();
}

} // namespace Rendering
} // namespace HyperEngine

