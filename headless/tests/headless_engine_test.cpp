#include <array>
#include <cmath>
#include <iostream>
#include <stdexcept>

#include "SpectraForge/Headless/Engine.h"

using namespace SpectraForge::Headless;

namespace {
void expect(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error("Test failure: " + message);
    }
}

void testInitializeAndShutdown() {
    Engine engine;
    AppConfig config;
    config.width = 800;
    config.height = 600;

    expect(engine.initialize(config), "Engine should initialise successfully");
    expect(engine.isRunning(), "Engine should be running after initialise");
    expect(engine.config().width == 800, "Width should equal configured value");
    expect(engine.config().height == 600, "Height should equal configured value");

    engine.shutdown();
    expect(!engine.isRunning(), "Engine should not be running after shutdown");
}

void testSceneLoadingAndRendering() {
    Engine engine;
    expect(engine.initialize(AppConfig{}), "Engine initialises with default config");

    Scene scene{"TestScene", 42, "Testing scene"};
    engine.loadScene(scene);
    engine.setRotationSpeed(90.0f);
    engine.setDebugMode(2);
    engine.setExternalCameraControl(true);

    engine.update(0.5f);
    auto report = engine.renderFrame();

    expect(report.frameIndex == 1u, "First frame index should be 1");
    expect(report.sceneName == "TestScene", "Scene name should match");
    expect(report.trianglesRendered == 42u, "Triangle count should match");
    expect(std::abs(report.rotationDegrees - 45.0f) < 1e-3, "Rotation should be 45 degrees");
    expect(report.debugMode == 2, "Debug mode should equal configured value");
    expect(report.externalCameraControl, "External camera control should be enabled");

    engine.update(0.5f);
    report = engine.renderFrame();
    expect(report.frameIndex == 2u, "Second frame index should be 2");
    expect(std::abs(report.rotationDegrees - 90.0f) < 1e-3, "Rotation should advance to 90 degrees");
    expect(engine.frameHistory().size() == 2u, "Frame history should contain two entries");
}

void testCameraPose() {
    Engine engine;
    expect(engine.initialize(AppConfig{}), "Engine initialises");

    Scene scene{"CameraScene", 12, "Camera tracking"};
    engine.loadScene(scene);

    std::array<float, 3> position{3.0f, 3.0f, 12.0f};
    std::array<float, 3> target{0.0f, 0.0f, 0.0f};
    engine.setCameraPose(position, target);

    engine.update(0.1f);
    const auto report = engine.renderFrame();

    expect(report.cameraPosition == position, "Camera position should match configured pose");
    expect(report.cameraTarget == target, "Camera target should match configured pose");
}
}  // namespace

int main() {
    try {
        testInitializeAndShutdown();
        testSceneLoadingAndRendering();
        testCameraPose();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    std::cout << "All headless engine tests passed" << std::endl;
    return 0;
}
