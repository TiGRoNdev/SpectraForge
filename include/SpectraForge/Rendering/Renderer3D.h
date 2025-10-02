
#pragma once

#include <memory>
#include <vector>
#include "../Math/Matrix4.h"
#include "../Math/Vector3.h"
#include "Camera3D.h"
#include "Mesh3D.h"
#include "Shader3D.h"

namespace SpectraForge {
namespace Rendering {

/**
 * @brief Основной класс рендерера для 3D
 */
class Renderer3D {
  public:
    // Получение singleton экземпляра
    static Renderer3D& getInstance();

    // Инициализация и очистка
    bool initialize(int width, int height);
    void cleanup();
    bool isInitialized() const { return initialized; }

    // Управление кадром
    void beginFrame();
    void endFrame();
    void clear();

    // Настройки рендеринга
    void setClearColor(float r, float g, float b, float a = 1.0f);
    void setViewport(int x, int y, int width, int height);
    void enableDepthTest(bool enable);
    void enableBlending(bool enable);
    void enableWireframe(bool enable);
    void enableBackfaceCulling(bool enable);

    // Управление камерой
    void setMainCamera(std::shared_ptr<Camera3D> camera);
    std::shared_ptr<Camera3D> getMainCamera() const { return mainCamera; }

    // Рендеринг объектов
    void renderMesh(const Mesh3D& mesh, const Math::Matrix4& transform, const Shader3D& shader);
    void renderMesh(std::shared_ptr<Mesh3D> mesh,
                    const Math::Matrix4& transform,
                    std::shared_ptr<Shader3D> shader);
    void renderWireframe(const Mesh3D& mesh,
                         const Math::Matrix4& transform,
                         const Shader3D& shader);

    // Система освещения
    struct Light {
        enum Type { DIRECTIONAL, POINT, SPOT };

        Type type;
        Math::Vector3 position;  // Для точечного и направленного света
        Math::Vector3 direction;  // Для направленного и прожекторного света
        Math::Vector3 color;
        float intensity;
        float range;      // Для точечного и прожекторного света
        float innerCone;  // Для прожекторного света
        float outerCone;  // Для прожекторного света
        bool enabled;

        Light()
            : type(DIRECTIONAL),
              position(0, 5, 0),
              direction(0, -1, 0),
              color(1, 1, 1),
              intensity(1.0f),
              range(10.0f),
              innerCone(30.0f),
              outerCone(45.0f),
              enabled(true) {}
    };

    void addLight(const Light& light);
    void removeLight(size_t index);
    void clearLights();
    const std::vector<Light>& getLights() const { return lights; }
    void setAmbientLight(const Math::Vector3& color) { ambientLight = color; }
    const Math::Vector3& getAmbientLight() const { return ambientLight; }

    // Статистика рендеринга
    struct RenderStats {
        size_t trianglesRendered;
        size_t drawCalls;
        size_t verticesProcessed;
        float frameTime;

        void reset() {
            trianglesRendered = drawCalls = verticesProcessed = 0;
            frameTime = 0.0f;
        }
    };
    const RenderStats& getRenderStats() const { return renderStats; }
    void resetRenderStats() { renderStats.reset(); }

    // Получение размеров экрана
    int getScreenWidth() const { return screenWidth; }
    int getScreenHeight() const { return screenHeight; }

    // Обработка изменения размера окна
    void onWindowResize(int width, int height);

  protected:
    // Singleton
    Renderer3D();
    ~Renderer3D();
    Renderer3D(const Renderer3D&) = delete;
    Renderer3D& operator=(const Renderer3D&) = delete;

    // Состояние рендерера
    bool initialized = false;
    int screenWidth = 0, screenHeight = 0;

    // Камера
    std::shared_ptr<Camera3D> mainCamera;

    // Освещение
    std::vector<Light> lights;
    Math::Vector3 ambientLight{0.2f, 0.2f, 0.2f};

    // Настройки рендеринга
    Math::Vector3 clearColor{0.1f, 0.1f, 0.2f};
    bool depthTestEnabled = true;
    bool blendingEnabled = false;
    bool wireframeEnabled = false;
    bool backfaceCullingEnabled = true;

    // Статистика
    RenderStats renderStats;

    // Вспомогательные методы
    virtual void setupRenderState();
    virtual void setupLighting(const Shader3D& shader);
    virtual void setupMatrices(const Shader3D& shader, const Math::Matrix4& modelMatrix);
    void updateRenderStats(const Mesh3D& mesh);
};

}  // namespace Rendering
}  // namespace SpectraForge
