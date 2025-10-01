
#pragma once

#include <memory>
#include <vector>
#include "../Math/Matrix4.h"
#include "../Math/Vector3.h"
#include "Camera3D.h"
#include "Mesh3D.h"
#include "Shader3D.h"

namespace HyperEngine {
namespace Rendering {

/**
 * @brief Перечисление доступных графических API
 */
enum class RenderBackend {
    OPENGL,  ///< OpenGL рендеринг (существующий код)
    VULKAN,  ///< Vulkan рендеринг (новая архитектура)
    AUTO  ///< Автоматический выбор на основе аппаратного обеспечения
};

/**
 * @brief Абстрактный интерфейс для адаптеров рендеринга
 *
 * Обеспечивает единый интерфейс для работы с различными графическими API.
 * Позволяет переключаться между OpenGL и Vulkan без изменения клиентского кода.
 */
class IRendererAdapter {
  public:
    virtual ~IRendererAdapter() = default;

    // Инициализация и очистка
    virtual bool initialize(int width, int height) = 0;
    virtual void cleanup() = 0;
    virtual bool isInitialized() const = 0;

    // Управление кадром
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    virtual void clear() = 0;

    // Настройки рендеринга
    virtual void setClearColor(float r, float g, float b, float a = 1.0f) = 0;
    virtual void setViewport(int x, int y, int width, int height) = 0;
    virtual void enableDepthTest(bool enable) = 0;
    virtual void enableBlending(bool enable) = 0;
    virtual void enableWireframe(bool enable) = 0;
    virtual void enableBackfaceCulling(bool enable) = 0;

    // Управление камерой
    virtual void setMainCamera(std::shared_ptr<Camera3D> camera) = 0;
    virtual std::shared_ptr<Camera3D> getMainCamera() const = 0;

    // Рендеринг объектов
    virtual void renderMesh(const Mesh3D& mesh,
                            const Math::Matrix4& transform,
                            const Shader3D& shader) = 0;
    virtual void renderMesh(std::shared_ptr<Mesh3D> mesh,
                            const Math::Matrix4& transform,
                            std::shared_ptr<Shader3D> shader) = 0;
    virtual void renderWireframe(const Mesh3D& mesh,
                                 const Math::Matrix4& transform,
                                 const Shader3D& shader) = 0;

    // Информация о backend
    virtual RenderBackend getBackendType() const = 0;
    virtual const char* getBackendName() const = 0;
    virtual bool supportsFeature(const std::string& feature) const = 0;
};

/**
 * @brief Основной класс адаптера рендеринга
 *
 * Управляет переключением между различными backend'ами рендеринга
 * и обеспечивает обратную совместимость с существующим кодом.
 */
class RendererAdapter {
  public:
    // Получение singleton экземпляра
    static RendererAdapter& getInstance();

    // Управление backend
    bool setBackend(RenderBackend backend);
    RenderBackend getCurrentBackend() const;
    bool isBackendAvailable(RenderBackend backend) const;

    // Автоматический выбор оптимального backend
    RenderBackend selectOptimalBackend() const;

    // Делегирование к текущему адаптеру
    bool initialize(int width, int height);
    void cleanup();
    bool isInitialized() const;

    void beginFrame();
    void endFrame();
    void clear();

    void setClearColor(float r, float g, float b, float a = 1.0f);
    void setViewport(int x, int y, int width, int height);
    void enableDepthTest(bool enable);
    void enableBlending(bool enable);
    void enableWireframe(bool enable);
    void enableBackfaceCulling(bool enable);

    void setMainCamera(std::shared_ptr<Camera3D> camera);
    std::shared_ptr<Camera3D> getMainCamera() const;

    void renderMesh(const Mesh3D& mesh, const Math::Matrix4& transform, const Shader3D& shader);
    void renderMesh(std::shared_ptr<Mesh3D> mesh,
                    const Math::Matrix4& transform,
                    std::shared_ptr<Shader3D> shader);
    void renderWireframe(const Mesh3D& mesh,
                         const Math::Matrix4& transform,
                         const Shader3D& shader);

    // Информация о текущем backend
    const char* getBackendName() const;
    bool supportsFeature(const std::string& feature) const;

    // Статистика и отладка
    void printBackendInfo() const;
    std::vector<RenderBackend> getAvailableBackends() const;

  private:
    RendererAdapter() = default;
    ~RendererAdapter() = default;
    RendererAdapter(const RendererAdapter&) = delete;
    RendererAdapter& operator=(const RendererAdapter&) = delete;

    // Создание адаптеров
    std::unique_ptr<IRendererAdapter> createOpenGLAdapter();
    std::unique_ptr<IRendererAdapter> createVulkanAdapter();

    // Проверка доступности backend'ов
    bool checkOpenGLAvailability() const;
    bool checkVulkanAvailability() const;

    std::unique_ptr<IRendererAdapter> currentAdapter;
    RenderBackend currentBackend = RenderBackend::OPENGL;
};

/**
 * @brief Адаптер для OpenGL рендеринга
 *
 * Обертка над существующим Renderer3D для обеспечения совместимости
 * с новым интерфейсом адаптера.
 */
class OpenGLRendererAdapter : public IRendererAdapter {
  public:
    OpenGLRendererAdapter();
    ~OpenGLRendererAdapter() override;

    // Реализация IRendererAdapter
    bool initialize(int width, int height) override;
    void cleanup() override;
    bool isInitialized() const override;

    void beginFrame() override;
    void endFrame() override;
    void clear() override;

    void setClearColor(float r, float g, float b, float a = 1.0f) override;
    void setViewport(int x, int y, int width, int height) override;
    void enableDepthTest(bool enable) override;
    void enableBlending(bool enable) override;
    void enableWireframe(bool enable) override;
    void enableBackfaceCulling(bool enable) override;

    void setMainCamera(std::shared_ptr<Camera3D> camera) override;
    std::shared_ptr<Camera3D> getMainCamera() const override;

    void renderMesh(const Mesh3D& mesh,
                    const Math::Matrix4& transform,
                    const Shader3D& shader) override;
    void renderMesh(std::shared_ptr<Mesh3D> mesh,
                    const Math::Matrix4& transform,
                    std::shared_ptr<Shader3D> shader) override;
    void renderWireframe(const Mesh3D& mesh,
                         const Math::Matrix4& transform,
                         const Shader3D& shader) override;

    RenderBackend getBackendType() const override { return RenderBackend::OPENGL; }
    const char* getBackendName() const override { return "OpenGL"; }
    bool supportsFeature(const std::string& feature) const override;

  private:
    // Нет необходимости хранить указатель, используем singleton напрямую
};

/**
 * @brief Адаптер для Vulkan рендеринга
 *
 * Интеграция с новой Vulkan архитектурой из FEATURE_PLAN.
 * Обеспечивает совместимость интерфейса с существующим кодом.
 */
class VulkanRendererAdapter : public IRendererAdapter {
  public:
    VulkanRendererAdapter();
    ~VulkanRendererAdapter() override;

    // Реализация IRendererAdapter
    bool initialize(int width, int height) override;
    void cleanup() override;
    bool isInitialized() const override;

    void beginFrame() override;
    void endFrame() override;
    void clear() override;

    void setClearColor(float r, float g, float b, float a = 1.0f) override;
    void setViewport(int x, int y, int width, int height) override;
    void enableDepthTest(bool enable) override;
    void enableBlending(bool enable) override;
    void enableWireframe(bool enable) override;
    void enableBackfaceCulling(bool enable) override;

    void setMainCamera(std::shared_ptr<Camera3D> camera) override;
    std::shared_ptr<Camera3D> getMainCamera() const override;

    void renderMesh(const Mesh3D& mesh,
                    const Math::Matrix4& transform,
                    const Shader3D& shader) override;
    void renderMesh(std::shared_ptr<Mesh3D> mesh,
                    const Math::Matrix4& transform,
                    std::shared_ptr<Shader3D> shader) override;
    void renderWireframe(const Mesh3D& mesh,
                         const Math::Matrix4& transform,
                         const Shader3D& shader) override;

    RenderBackend getBackendType() const override { return RenderBackend::VULKAN; }
    const char* getBackendName() const override { return "Vulkan"; }
    bool supportsFeature(const std::string& feature) const override;

  private:
    void* vulkanEngine;  // Указатель на VulkanEngine (избегаем forward declaration)
    std::shared_ptr<Camera3D> mainCamera;
    bool initialized = false;

    // Настройки рендеринга
    struct RenderSettings {
        float clearColor[4] = {0.1f, 0.1f, 0.2f, 1.0f};
        int viewport[4] = {0, 0, 800, 600};
        bool depthTest = true;
        bool blending = false;
        bool wireframe = false;
        bool backfaceCulling = true;
    } settings;
};

}  // namespace Rendering
}  // namespace HyperEngine
