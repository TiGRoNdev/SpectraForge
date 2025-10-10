/**
 * @file HybridFreGSRenderer.h
 * @brief Реализация IRenderer для Hybrid DWT + FreGS пайплайна
 */

#pragma once

#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include "SpectraForge/Rendering/Common/IRenderer.h"
#include "SpectraForge/Rendering/RenderPass/TriangleSplattingPass.h"
#include "SpectraForge/Rendering/RenderPass/FreGSPass.h"
#include "SpectraForge/Rendering/Mesh3D.h"
#include "SpectraForge/Rendering/Camera3D.h"

// P0.2 REFACTORING: Новые SOLID-компоненты
#include "SpectraForge/Rendering/Common/IWindowBinder.h"
#include "SpectraForge/Rendering/Core/FrameManager.h"
#include "SpectraForge/Rendering/Core/PipelineManager.h"
#include "SpectraForge/Rendering/Core/RendererCore.h"
#include "SpectraForge/Rendering/Core/RendererDebugger.h"
#include "SpectraForge/Rendering/Core/RendererStatistics.h"
#include "SpectraForge/Rendering/Core/SwapchainManager.h"

namespace SpectraForge {
namespace Rendering {

/**
 * @brief Рендерер, объединяющий WaveletPass, FreGSPass и TriangleSplattingPass
 * 
 * Dual-path rendering:
 * - Gaussian Splatting (FreGS) → Point clouds (.ply)
 * - Triangle Splatting → Triangle meshes (.obj)
 */
class HybridFreGSRenderer final : public IRenderer, public IWindowBinder {
  public:
    /**
     * @brief Rendering mode selector
     */
    enum class RenderMode {
        GaussianSplatting,  ///< For point clouds (FreGS)
        TriangleSplatting   ///< For triangle meshes (Triangle Splatting)
    };

    HybridFreGSRenderer();
    HybridFreGSRenderer(std::shared_ptr<Core::IRendererCore> core,
                        std::shared_ptr<Core::IRendererDebugger> debugger,
                        std::shared_ptr<Core::ISwapchainManagerFactory> swapchainFactory,
                        std::shared_ptr<Core::IPipelineManagerFactory> pipelineFactory,
                        std::shared_ptr<Core::IFrameManagerFactory> frameFactory,
                        std::shared_ptr<Core::IRendererStatisticsFactory> statisticsFactory);
    ~HybridFreGSRenderer() override;

    // IRenderer
    bool initialize() override;
    /**
     * @brief Привязать окно рендереру (создание поверхности/свапчейна)
     */
    bool attachWindow(void* x11Display, void* x11Window, uint32_t width, uint32_t height) override;
    void renderFrame(const FrameData& frameData) override;
    void shutdown() override;
    RendererType getType() const override { return RendererType::Vulkan; }
    bool supportsFeature(RenderingFeature feature) const override;
    std::string getName() const override { return "HybridFreGSRenderer"; }
    std::string getApiVersion() const override { return "Vulkan 1.3 (compute)"; }
    bool isReady() const override { return initialized_; }
    bool isInitialized() const override { return initialized_; }
    void beginFrame() override;
    void endFrame() override;
    RenderingStats getStats() const override;

    /**
     * @brief Загрузка гауссианов для FreGS пасса (point clouds)
     */
    void uploadGaussians(const std::vector<spectraforge::rendering::GaussianSplat>& gaussians);

    /**
     * @brief Загрузка треугольников для Triangle Splatting (meshes)
     */
    void uploadTriangles(const std::vector<spectraforge::rendering::Triangle>& triangles);
    
    /**
     * @brief Получить указатель на Triangle Splatting Pass для отладки
     */
    spectraforge::rendering::TriangleSplattingPass* getTriangleSplattingPass() { 
        return triangleSplattingPass_.get(); 
    }

    /**
     * @brief Установить режим рендеринга
     */
    void setRenderMode(RenderMode mode) { renderMode_ = mode; }

    /**
     * @brief Признак потери устройства (VK_ERROR_DEVICE_LOST)
     * @return true если устройство потеряно и рендер следует остановить
     */
    bool isDeviceLost() const;

    /**
     * @brief Загрузить меш для рендеринга через Triangle Splatting
     */
    void uploadMesh(const std::shared_ptr<Mesh3D>& mesh, const Math::Matrix4& transform = Math::Matrix4::identity());

    /**
     * @brief Установить камеру для рендеринга
     */
    void setCamera(Camera3D* camera) { camera_ = camera; }

    // НОВЫЕ DEBUG API МЕТОДЫ (реализуют расширенный IRenderer интерфейс):
    
    /**
     * @brief Установить режим отладки
     * @param mode 0=normal, 1=SDF, 2=barycentric, 3=depth, 4=wireframe
     */
     void setDebugMode(int mode) override;
    
     /**
      * @brief Получить текущий режим отладки
      */
     int getDebugMode() const override;
     
     /**
      * @brief Включить/выключить wireframe режим
      */
     void enableWireframe(bool enable) override;
     
     /**
      * @brief Включить/выключить backface culling
      */
     void enableBackfaceCulling(bool enable) override;
     
     /**
      * @brief Включить/выключить depth test
      */
     void enableDepthTest(bool enable) override;
     
     /**
      * @brief Установить цвет фона
      */
     void setBackgroundColor(float r, float g, float b, float a = 1.0f) override;
     
     /**
      * @brief Получить цвет фона
      */
     glm::vec4 getBackgroundColor() const override;
     
     /**
      * @brief Установить viewport
      */
     void setViewport(int x, int y, int width, int height) override;
     
     /**
      * @brief Включить/выключить alpha blending
      */
     void enableAlphaBlending(bool enable) override;
     
     /**
      * @brief Установить triangle budget для performance tuning
      */
     void setTriangleBudget(uint32_t maxTriangles) override;
     
     /**
      * @brief Включить/выключить early termination в alpha blending
      */
     void enableEarlyTermination(bool enable) override;
     
     /**
      * @brief Получить подробную статистику производительности
      */
     DetailedRenderingStats getDetailedStats() const override;
     
     /**
      * @brief Сохранить скриншот в файл
      */
     bool saveScreenshot(const std::string& filename) const override;
     
     /**
      * @brief Получить данные framebuffer для анализа
      */
     std::vector<uint8_t> getFramebufferData() const override;
     
     /**
      * @brief Установить debug callback для логирования
      */
     void setDebugCallback(std::function<void(const std::string&)> callback) override;
     
     /**
      * @brief Принудительно обновить все uniform буферы
      */
     void flushUniforms() override;
     
     /**
      * @brief Получить информацию о GPU
      */
     GPUInfo getGPUInfo() const override;

  private:
    bool initialized_ = false;
    bool device_lost_ = false;
    RenderMode renderMode_ = RenderMode::TriangleSplatting;
    Camera3D* camera_ = nullptr;
    RenderingStats stats_{};

    std::shared_ptr<Core::IRendererCore> core_;
    std::shared_ptr<Core::IRendererDebugger> debugger_;
    std::shared_ptr<Core::ISwapchainManagerFactory> swapchainFactory_;
    std::shared_ptr<Core::IPipelineManagerFactory> pipelineFactory_;
    std::shared_ptr<Core::IFrameManagerFactory> frameFactory_;
    std::shared_ptr<Core::IRendererStatisticsFactory> statisticsFactory_;

    std::shared_ptr<Core::ISwapchainManager> swapchain_;
    std::shared_ptr<Core::IFrameManager> frame_;
    std::shared_ptr<Core::IPipelineManager> pipeline_;
    std::shared_ptr<Core::IRendererStatistics> statistics_;
    
    // Rendering passes (не изменились)
    std::unique_ptr<spectraforge::rendering::TriangleSplattingPass> triangleSplattingPass_;
    std::unique_ptr<spectraforge::rendering::FreGSPass> fregsPass_;

    // Helpers (упрощены после рефакторинга)
    std::vector<spectraforge::rendering::Triangle> convertMeshToTriangles(
        const Mesh3D& mesh, const Math::Matrix4& transform);
    bool initializeTriangleSplatting();
    void recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
    glm::mat4 getCorrectedViewProjMatrix() const;
};

}  // namespace Rendering
}  // namespace SpectraForge


