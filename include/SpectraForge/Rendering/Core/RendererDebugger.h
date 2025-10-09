/**
 * @file RendererDebugger.h
 * @brief Debug functionality and settings (P0.2 Refactoring)
 */

#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include <functional>
#include <string>

namespace SpectraForge {
namespace Rendering {
namespace Core {

/**
 * @brief Управление debug функциями renderer
 * 
 * SOLID:
 * - SRP ✅: Только debug настройки и логирование
 * - ISP ✅: Не зависит от рендеринга
 */
class RendererDebugger {
public:
    RendererDebugger() = default;
    ~RendererDebugger() = default;
    
    void setDebugMode(int mode) { currentDebugMode_ = mode; }
    int getDebugMode() const { return currentDebugMode_; }
    
    void enableWireframe(bool enable) { wireframeEnabled_ = enable; }
    bool isWireframeEnabled() const { return wireframeEnabled_; }
    
    void enableBackfaceCulling(bool enable) { backfaceCullingEnabled_ = enable; }
    bool isBackfaceCullingEnabled() const { return backfaceCullingEnabled_; }
    
    void enableDepthTest(bool enable) { depthTestEnabled_ = enable; }
    bool isDepthTestEnabled() const { return depthTestEnabled_; }
    
    void enableAlphaBlending(bool enable) { alphaBlendingEnabled_ = enable; }
    bool isAlphaBlendingEnabled() const { return alphaBlendingEnabled_; }
    
    void enableEarlyTermination(bool enable) { earlyTerminationEnabled_ = enable; }
    bool isEarlyTerminationEnabled() const { return earlyTerminationEnabled_; }
    
    void setBackgroundColor(float r, float g, float b, float a = 1.0f) {
        backgroundColor_ = glm::vec4(r, g, b, a);
    }
    glm::vec4 getBackgroundColor() const { return backgroundColor_; }
    
    void setViewport(int x, int y, int width, int height);
    vk::Viewport getViewport() const { return currentViewport_; }
    
    void setDebugCallback(std::function<void(const std::string&)> callback) {
        debugCallback_ = callback;
    }
    
    void log(const std::string& message) const;

private:
    int currentDebugMode_ = 0;
    glm::vec4 backgroundColor_{0.1f, 0.2f, 0.3f, 1.0f};
    bool wireframeEnabled_ = false;
    bool depthTestEnabled_ = true;
    bool backfaceCullingEnabled_ = true;
    bool alphaBlendingEnabled_ = true;
    bool earlyTerminationEnabled_ = true;
    vk::Viewport currentViewport_{};
    std::function<void(const std::string&)> debugCallback_;
};

}}}

