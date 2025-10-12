#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

#include "SpectraForge/Headless/AppConfig.h"
#include "SpectraForge/Headless/FrameReport.h"
#include "SpectraForge/Headless/Logger.h"
#include "SpectraForge/Headless/Scene.h"

namespace SpectraForge::Headless {

class Engine {
  public:
    Engine();

    bool initialize(const AppConfig& config, std::shared_ptr<Logger> logger = nullptr);
    void shutdown();

    [[nodiscard]] bool isRunning() const { return running_; }

    void setExternalCameraControl(bool enabled);
    [[nodiscard]] bool externalCameraControl() const { return externalCameraControl_; }

    void setDebugMode(int mode);
    [[nodiscard]] int debugMode() const { return debugMode_; }

    void setCameraPose(const std::array<float, 3>& position, const std::array<float, 3>& target);
    [[nodiscard]] std::array<float, 3> cameraPosition() const { return cameraPosition_; }
    [[nodiscard]] std::array<float, 3> cameraTarget() const { return cameraTarget_; }

    void setRotationSpeed(float degreesPerSecond);

    void loadScene(const Scene& scene);
    [[nodiscard]] std::optional<Scene> activeScene() const { return scene_; }

    void update(float deltaSeconds);
    FrameReport renderFrame();

    [[nodiscard]] const AppConfig& config() const { return config_; }
    [[nodiscard]] const std::vector<FrameReport>& frameHistory() const { return frameHistory_; }

  private:
    AppConfig config_{};
    std::shared_ptr<Logger> logger_;
    bool running_ = false;
    bool externalCameraControl_ = false;
    int debugMode_ = 0;
    float rotationSpeed_ = 45.0f;  // degrees per second
    float accumulatedTime_ = 0.0f;
    std::size_t frameCounter_ = 0;
    std::array<float, 3> cameraPosition_{0.0f, 0.0f, 5.0f};
    std::array<float, 3> cameraTarget_{0.0f, 0.0f, 0.0f};
    std::optional<Scene> scene_;
    std::vector<FrameReport> frameHistory_;

    void logInfo(const std::string& message) const;
};

}  // namespace SpectraForge::Headless
