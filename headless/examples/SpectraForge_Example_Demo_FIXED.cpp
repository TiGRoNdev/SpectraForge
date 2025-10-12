#include <chrono>
#include <iostream>
#include <thread>

#include "SpectraForge/Headless/Engine.h"

using namespace SpectraForge::Headless;

int main() {
    AppConfig config;
    config.width = 1280;
    config.height = 720;
    config.title = "SpectraForge Example Demo - Headless";
    config.backgroundColor = {0.08f, 0.08f, 0.12f, 1.0f};

    Engine engine;
    if (!engine.initialize(config)) {
        std::cerr << "[Example_HEADLESS] ❌ Failed to initialise engine" << std::endl;
        return 1;
    }

    engine.setExternalCameraControl(false);
    engine.setCameraPose({0.0f, 1.5f, 4.5f}, {0.0f, 1.2f, 0.0f});
    engine.setRotationSpeed(12.0f);
    engine.setDebugMode(1);

    Scene scene{"SponzaHeadless", 128, "Colourful splatting triangles"};
    engine.loadScene(scene);

    std::cout << "[Example_HEADLESS] 🌈 Running colourful triangle showcase" << std::endl;

    constexpr int frameCount = 4;
    for (int i = 0; i < frameCount; ++i) {
        engine.update(0.25f);
        const auto report = engine.renderFrame();
        std::cout << "[Example_HEADLESS] Frame " << report.frameIndex
                  << ": rotation=" << report.rotationDegrees
                  << "°, triangles=" << report.trianglesRendered
                  << ", debug=" << report.debugMode << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }

    engine.shutdown();
    std::cout << "[Example_HEADLESS] ✅ Demo completed" << std::endl;
    return 0;
}
