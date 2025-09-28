#pragma once

#include <memory>
#include <vector>
#include "../Math/Matrix4.h"
#include "../Math/Vector3.h"

namespace HyperEngine {
namespace Rendering {

/**
 * @brief 3D Гауссиан для представления сцены методом 3D Gaussian Splatting
 *
 * Каждый гауссиан определяется позицией, ковариацией, прозрачностью и цветом,
 * что позволяет эффективно представлять сложные геометрические формы.
 */
struct Gaussian3D {
    Math::Vector3 position;  // Позиция центра гауссиана
    Math::Matrix4 covariance;  // Матрица ковариации (определяет форму)
    Math::Vector3 color;       // RGB цвет
    float opacity;             // Прозрачность (0.0 - 1.0)
    Math::Vector3 scale;       // Масштаб по осям
    Math::Vector3 rotation;  // Углы поворота (Эйлера)

    // Дополнительные параметры для оптимизации
    float importance;  // Важность гауссиана для LOD
    bool visible;      // Флаг видимости

    Gaussian3D()
        : position(0.0f, 0.0f, 0.0f),
          covariance(Math::Matrix4::identity()),
          color(1.0f, 1.0f, 1.0f),
          opacity(1.0f),
          scale(1.0f, 1.0f, 1.0f),
          rotation(0.0f, 0.0f, 0.0f),
          importance(1.0f),
          visible(true) {}

    Gaussian3D(const Math::Vector3& pos, const Math::Vector3& col, float alpha = 1.0f)
        : position(pos),
          covariance(Math::Matrix4::identity()),
          color(col),
          opacity(alpha),
          scale(1.0f, 1.0f, 1.0f),
          rotation(0.0f, 0.0f, 0.0f),
          importance(1.0f),
          visible(true) {}

    /**
     * @brief Вычисляет матрицу ковариации из параметров масштаба и поворота
     */
    void updateCovariance();

    /**
     * @brief Вычисляет вклад гауссиана в точке пространства
     */
    float evaluateAt(const Math::Vector3& point) const;

    /**
     * @brief Проверяет, влияет ли гауссиан на заданную область
     */
    bool intersects(const Math::Vector3& min, const Math::Vector3& max) const;
};

/**
 * @brief Коллекция 3D гауссианов для представления сцены
 *
 * Реализует эффективное хранение и обработку гауссианов
 * с поддержкой пространственного индексирования и LOD.
 */
class GaussianField3D {
  public:
    GaussianField3D();
    virtual ~GaussianField3D();

    // Управление гауссианами
    void addGaussian(const Gaussian3D& gaussian);
    void removeGaussian(size_t index);
    void clear();

    // Геттеры
    const std::vector<Gaussian3D>& getGaussians() const { return gaussians; }
    size_t getGaussianCount() const { return gaussians.size(); }

    // Оптимизация сцены
    void optimizeGaussians(const std::vector<Math::Vector3>& viewPoints);
    void cullInvisibleGaussians(const Math::Matrix4& viewMatrix, const Math::Matrix4& projMatrix);
    void sortByDistance(const Math::Vector3& viewPoint);

    // Создание из геометрии
    static std::shared_ptr<GaussianField3D> createFromMesh(const class Mesh3D& mesh,
                                                           int gaussianDensity = 1000);
    static std::shared_ptr<GaussianField3D> createFromPointCloud(
        const std::vector<Math::Vector3>& points,
        const std::vector<Math::Vector3>& colors);

    // Адаптивная выборка для производительности
    void setLevelOfDetail(float distance, int maxGaussians = 10000);
    void enableAdaptiveSampling(bool enable) { adaptiveSampling = enable; }

    // GPU операции
    virtual void uploadToGPU();
    virtual void render(const Math::Matrix4& viewMatrix, const Math::Matrix4& projMatrix) const;
    virtual void cleanup();
    bool isUploaded() const { return uploaded; }

    // Статистика
    struct RenderStats {
        size_t totalGaussians;
        size_t renderedGaussians;
        size_t culledGaussians;
        float frameTime;

        void reset() {
            totalGaussians = renderedGaussians = culledGaussians = 0;
            frameTime = 0.0f;
        }
    };
    const RenderStats& getRenderStats() const { return renderStats; }

  protected:
    std::vector<Gaussian3D> gaussians;
    bool uploaded;
    bool adaptiveSampling;

    // GPU ресурсы
    unsigned int SSBO;      // Shader Storage Buffer Object для гауссианов
    unsigned int VAO, VBO;  // Vertex Array Object для рендеринга

    // Оптимизация
    std::vector<size_t> visibleIndices;  // Индексы видимых гауссианов
    RenderStats renderStats;

    // Пространственное индексирование (для будущей оптимизации)
    struct SpatialNode {
        Math::Vector3 min, max;
        std::vector<size_t> gaussianIndices;
        std::vector<std::unique_ptr<SpatialNode>> children;
    };
    std::unique_ptr<SpatialNode> spatialTree;

    void buildSpatialTree();
    void updateGaussianBuffer();
};

/**
 * @brief Рендерер для 3D Gaussian Splatting
 *
 * Реализует дифференцируемую растеризацию гауссианов
 * с поддержкой real-time рендеринга.
 */
class GaussianRenderer3D {
  public:
    GaussianRenderer3D();
    virtual ~GaussianRenderer3D();

    // Инициализация
    bool initialize();
    void cleanup();

    // Основной рендеринг
    void render(const GaussianField3D& field,
                const Math::Matrix4& viewMatrix,
                const Math::Matrix4& projMatrix,
                int screenWidth,
                int screenHeight);

    // Настройки качества
    void setRenderQuality(float quality);  // 0.0 - 1.0
    void enableAlphaBlending(bool enable);
    void setSortingMethod(int method);  // 0 - по глубине, 1 - по важности

    // Оптимизации
    void enableTileBasedRendering(bool enable);  // Тайловый рендеринг для больших сцен
    void setTileSize(int size) { tileSize = size; }
    void enableEarlyZRejection(bool enable);  // Ранний Z-тест

    // Адаптивная выборка
    void enableAdaptiveSampling(bool enable);
    void setMaxSamplesPerPixel(int samples);

    // Статистика
    struct GaussianRenderStats {
        size_t gaussiansProcessed;
        size_t pixelsShaded;
        size_t tilesProcessed;
        float rasterizationTime;
        float sortingTime;

        void reset() {
            gaussiansProcessed = pixelsShaded = tilesProcessed = 0;
            rasterizationTime = sortingTime = 0.0f;
        }
    };
    const GaussianRenderStats& getRenderStats() const { return renderStats; }

  protected:
    bool initialized;
    class Shader3D* gaussianShader;
    class Shader3D* tileShader;

    // Параметры рендеринга
    float renderQuality;
    bool alphaBlendingEnabled;
    bool tileBasedRenderingEnabled;
    bool earlyZRejectionEnabled;
    bool adaptiveSamplingEnabled;
    int sortingMethod;
    int tileSize;
    int maxSamplesPerPixel;

    // Буферы для тайлового рендеринга
    unsigned int tileBuffer;
    unsigned int depthBuffer;
    unsigned int colorBuffer;

    GaussianRenderStats renderStats;

    // Внутренние методы
    void sortGaussians(std::vector<Gaussian3D>& gaussians, const Math::Vector3& viewPoint);
    void renderTiles(const GaussianField3D& field, int screenWidth, int screenHeight);
    void setupGaussianShader(const Math::Matrix4& viewMatrix, const Math::Matrix4& projMatrix);
};

}  // namespace Rendering
}  // namespace HyperEngine

