#include "SpectraForge/Headless/Engine.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace SpectraForge::Headless {

namespace {
std::string formatVec(const std::array<float, 3>& vec) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2)
        << "(" << vec[0] << ", " << vec[1] << ", " << vec[2] << ")";
    return oss.str();
}
}  // namespace

Engine::Engine() = default;

bool Engine::initialize(const AppConfig& config, std::shared_ptr<Logger> logger) {
    if (running_) {
        throw std::runtime_error("Engine is already running");
    }

    config_ = config;
    logger_ = logger ? std::move(logger) : std::make_shared<Logger>("SpectraForgeHeadless");
    running_ = true;
    accumulatedTime_ = 0.0f;
    frameCounter_ = 0;
    frameHistory_.clear();

    std::ostringstream oss;
    oss << "Engine initialised with window " << config.width << "x" << config.height
        << ", vsync=" << (config.vsync ? "on" : "off") << ", bg="
        << std::fixed << std::setprecision(2) << "(" << config.backgroundColor[0] << ", "
        << config.backgroundColor[1] << ", " << config.backgroundColor[2] << ", "
        << config.backgroundColor[3] << ")";
    logInfo(oss.str());

    return true;
}

void Engine::shutdown() {
    if (!running_) {
        return;
    }

    logInfo("Shutting down headless engine");
    running_ = false;
    scene_.reset();
    frameHistory_.clear();
    accumulatedTime_ = 0.0f;
}

void Engine::setExternalCameraControl(bool enabled) {
    externalCameraControl_ = enabled;
    logInfo(std::string("External camera control ") + (enabled ? "enabled" : "disabled"));
}

void Engine::setDebugMode(int mode) {
    debugMode_ = mode;
    logInfo("Debug mode set to " + std::to_string(mode));
}

void Engine::setCameraPose(const std::array<float, 3>& position,
                           const std::array<float, 3>& target) {
    cameraPosition_ = position;
    cameraTarget_ = target;
    logInfo("Camera pose -> position " + formatVec(position) + ", target " + formatVec(target));
}

void Engine::setRotationSpeed(float degreesPerSecond) {
    rotationSpeed_ = degreesPerSecond;
    logInfo("Rotation speed set to " + std::to_string(degreesPerSecond) + " deg/s");
}

void Engine::loadScene(const Scene& scene) {
    scene_ = scene;
    logInfo("Scene loaded: " + scene.name + " with " + std::to_string(scene.triangleCount) +
            " triangles");
}

void Engine::update(float deltaSeconds) {
    if (!running_) {
        throw std::runtime_error("Cannot update stopped engine");
    }

    accumulatedTime_ += deltaSeconds;
    std::ostringstream oss;
    oss << "Update dt=" << std::fixed << std::setprecision(3) << deltaSeconds
        << "s, total=" << accumulatedTime_ << "s";
    logInfo(oss.str());
}

FrameReport Engine::renderFrame() {
    if (!running_) {
        throw std::runtime_error("Cannot render frame on stopped engine");
    }

    if (!scene_) {
        throw std::runtime_error("No scene loaded");
    }

    FrameReport report{};
    report.frameIndex = ++frameCounter_;
    report.sceneName = scene_->name;
    report.trianglesRendered = scene_->triangleCount;
    report.rotationDegrees = std::fmod(accumulatedTime_ * rotationSpeed_, 360.0f);
    report.debugMode = debugMode_;
    report.externalCameraControl = externalCameraControl_;
    report.cameraPosition = cameraPosition_;
    report.cameraTarget = cameraTarget_;

    frameHistory_.push_back(report);

    std::ostringstream oss;
    oss << "Frame " << report.frameIndex << " -> scene='" << report.sceneName
        << "', triangles=" << report.trianglesRendered << ", rotation="
        << std::fixed << std::setprecision(1) << report.rotationDegrees << "°, debug="
        << report.debugMode;
    logInfo(oss.str());

    return report;
}

void Engine::logInfo(const std::string& message) const {
    if (logger_) {
        logger_->info(message);
    } else {
        std::cout << message << std::endl;
    }
}

}  // namespace SpectraForge::Headless
