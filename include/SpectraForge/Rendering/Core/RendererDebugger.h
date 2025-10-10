/**
 * @file RendererDebugger.h
 * @brief Debug functionality and settings (P0.2 Refactoring)
 */

#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include <functional>
#include <memory>
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
class IRendererDebugger {
  public:
    virtual ~IRendererDebugger() = default;

    virtual void setDebugMode(int mode) = 0;
    virtual int getDebugMode() const = 0;
    virtual void enableWireframe(bool enable) = 0;
    virtual bool isWireframeEnabled() const = 0;
    virtual void enableBackfaceCulling(bool enable) = 0;
    virtual bool isBackfaceCullingEnabled() const = 0;
    virtual void enableDepthTest(bool enable) = 0;
    virtual bool isDepthTestEnabled() const = 0;
    virtual void enableAlphaBlending(bool enable) = 0;
    virtual bool isAlphaBlendingEnabled() const = 0;
    virtual void enableEarlyTermination(bool enable) = 0;
    virtual bool isEarlyTerminationEnabled() const = 0;
    virtual void setBackgroundColor(float r, float g, float b, float a = 1.0f) = 0;
    virtual glm::vec4 getBackgroundColor() const = 0;
    virtual void setViewport(int x, int y, int width, int height) = 0;
    virtual vk::Viewport getViewport() const = 0;
    virtual void setDebugCallback(std::function<void(const std::string&)> callback) = 0;
    virtual void log(const std::string& message) const = 0;
};

class RendererDebugger : public IRendererDebugger {
public:
    RendererDebugger() = default;
    ~RendererDebugger() = default;

    void setDebugMode(int mode) override { currentDebugMode_ = mode; }
    int getDebugMode() const override { return currentDebugMode_; }

    void enableWireframe(bool enable) override { wireframeEnabled_ = enable; }
    bool isWireframeEnabled() const override { return wireframeEnabled_; }

    void enableBackfaceCulling(bool enable) override { backfaceCullingEnabled_ = enable; }
    bool isBackfaceCullingEnabled() const override { return backfaceCullingEnabled_; }

    void enableDepthTest(bool enable) override { depthTestEnabled_ = enable; }
    bool isDepthTestEnabled() const override { return depthTestEnabled_; }

    void enableAlphaBlending(bool enable) override { alphaBlendingEnabled_ = enable; }
    bool isAlphaBlendingEnabled() const override { return alphaBlendingEnabled_; }

    void enableEarlyTermination(bool enable) override { earlyTerminationEnabled_ = enable; }
    bool isEarlyTerminationEnabled() const override { return earlyTerminationEnabled_; }

    void setBackgroundColor(float r, float g, float b, float a = 1.0f) override {
        backgroundColor_ = glm::vec4(r, g, b, a);
    }
    glm::vec4 getBackgroundColor() const override { return backgroundColor_; }

    void setViewport(int x, int y, int width, int height) override;
    vk::Viewport getViewport() const override { return currentViewport_; }

    void setDebugCallback(std::function<void(const std::string&)> callback) override {
        debugCallback_ = std::move(callback);
    }

    void log(const std::string& message) const override;

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

