#include <chrono>
#include <iostream>
#include <thread>

#include "SpectraForge/Headless/Engine.h"

using namespace SpectraForge::Headless;

int main() {
    AppConfig config;
    config.width = 960;
    config.height = 540;
    config.title = "Blue Cube Demo - Headless";
    config.backgroundColor = {0.02f, 0.05f, 0.12f, 1.0f};

    Engine engine;
    if (!engine.initialize(config)) {
        std::cerr << "[BlueCube_HEADLESS] ❌ Failed to initialise engine" << std::endl;
        return 1;
    }

    engine.setExternalCameraControl(true);
    engine.setCameraPose({3.0f, 3.0f, 12.0f}, {0.0f, 0.0f, 0.0f});
    engine.setRotationSpeed(18.0f);
    engine.setDebugMode(2);

    Scene scene{"BlueCube_Final", 12, "Bright blue rotating cube"};
    engine.loadScene(scene);

    std::cout << "[BlueCube_HEADLESS] 🎬 Starting headless simulation" << std::endl;

    constexpr int frameCount = 6;
    for (int i = 0; i < frameCount; ++i) {
        engine.update(0.16f);
        const auto report = engine.renderFrame();
        std::cout << "[BlueCube_HEADLESS] Frame " << report.frameIndex
                  << ": camera=" << report.cameraPosition[0] << "," << report.cameraPosition[1]
                  << "," << report.cameraPosition[2]
                  << " rotation=" << report.rotationDegrees
                  << "°, triangles=" << report.trianglesRendered << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
    }

    engine.shutdown();
    std::cout << "[BlueCube_HEADLESS] ✅ Simulation finished" << std::endl;
    return 0;
}
