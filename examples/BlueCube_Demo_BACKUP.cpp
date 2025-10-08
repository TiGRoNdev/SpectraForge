//
// BlueCube_Demo_FINAL_FIXED.cpp — ОКОНЧАТЕЛЬНОЕ исправление
// ✅ РЕШАЕТ: camera position, triangle size, coordinate system
//

#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <thread>

#include "SpectraForge/App/Config.h"
#include "SpectraForge/App/Engine.h"
#include "SpectraForge/Core/Console.h"
#include "SpectraForge/Core/Logger.h"
#include "SpectraForge/Core/SafeConsole.h"
#include "SpectraForge/Rendering/Common/IRenderer.h"
#include "SpectraForge/Rendering/Camera3D.h"
#include "SpectraForge/Vulkan/SceneManager.h"
#include "SpectraForge/Rendering/HybridFreGSRenderer.h"
#include "SpectraForge/Rendering/RenderPass/TriangleSplattingPass.h"

using namespace SpectraForge;

class BlueCubeDemoFinalFixed {
public:
    bool initialize() {
        SpectraForge::Core::Console::initialize();

        SpectraForge::App::AppConfig cfg;
        cfg.window_width = 960;
        cfg.window_height = 540;
        cfg.window_title = "Blue Cube Demo - FINAL FIXED";
        cfg.vsync = true;

        cfg.backgroundColor[0] = 0.02f;
        cfg.backgroundColor[1] = 0.05f;
        cfg.backgroundColor[2] = 0.12f;
        cfg.backgroundColor[3] = 1.0f;

        logger_ = std::make_shared<SpectraForge::Core::Logger>(
            "", SpectraForge::Core::LogLevel::INFO_LEVEL);

        engine_ = std::make_unique<SpectraForge::App::Engine>(cfg, logger_);
        if (!engine_->init()) {
            SAFE_ERROR("Не удалось инициализировать Engine");
            return false;
        }

        // ✅ КРИТИЧЕСКИЙ ФИКС: Внешнее управление камерой
        engine_->setExternalCameraControl(true);
        std::cout << "[BlueCubeFINAL] ✅ External camera control enabled" << std::endl;

        // Пустая сцена
        SpectraForge::Vulkan::SceneData scene{};
        scene.scenePath = "";
        scene.triangleStep = 1;
        if (!engine_->load_scene(scene)) {
            SAFE_ERROR("Не удалось загрузить пустую сцену");
            return false;
        }

        // ✅ ОКОНЧАТЕЛЬНЫЙ ФИКС КАМЕРЫ: Позиция гарантирующая видимость куба
        auto camera = engine_->getCamera();
        if (camera) {
            // Куб размером 4x4x4 units в центре, камера должна быть намного дальше
            camera->setPosition(SpectraForge::Math::Vector3(3.0f, 3.0f, 12.0f));
            camera->setTarget(SpectraForge::Math::Vector3(0.0f, 0.0f, 0.0f));
            camera->setPerspective(50.0f,  // Narrower FOV для лучшего focus
                                   static_cast<float>(cfg.window_width) / cfg.window_height,
                                   0.1f, 100.0f);
            
            std::cout << "[BlueCubeFINAL] 📷 Camera positioned at (3, 3, 12) for GUARANTEED cube visibility" << std::endl;
        }

        if (auto r = engine_->getRenderer()) {
            r->setBackgroundColor(cfg.backgroundColor[0], cfg.backgroundColor[1],
                                  cfg.backgroundColor[2], cfg.backgroundColor[3]);
            r->enableDepthTest(true);
            r->enableBackfaceCulling(false);
        }

        // ✅ ФИКС: Debug mode 2 для визуализации проекций
        auto h = std::dynamic_pointer_cast<Rendering::HybridFreGSRenderer>(engine_->getRenderer());
        if (h) {
            auto triangleSplattingPass = h->getTriangleSplattingPass();
            if (triangleSplattingPass) {
                triangleSplattingPass->setDebugMode(2);  // Показывает проекции треугольников
                std::cout << "[BlueCubeFINAL] ✅ Debug mode 2: triangle projection visualization" << std::endl;
            }
        }

        initCubeGeometry();
        SAFE_PRINT_LINE("BlueCube_Demo_FINAL_FIXED инициализировано");
        return true;
    }

    void run() {
        constexpr float TARGET_FPS = 30.0f;  // Понижаем FPS для стабильности
        constexpr float FRAME_TIME_MS = 1000.0f / TARGET_FPS;

        auto last = std::chrono::high_resolution_clock::now();
        float angle = 0.0f;

        std::cout << "[BlueCubeFINAL] 🎮 Starting with camera at (3,3,12) looking at cube at origin" << std::endl;

        while (!engine_->should_close()) {
            auto frame_start = std::chrono::high_resolution_clock::now();

            auto now = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(now - last).count();
            last = now;

            // Очень медленное вращение для четкого наблюдения
            angle += dt * 0.3f;

            uploadRotatedCube(angle);
            engine_->update(dt);
            engine_->render();

            auto frame_end = std::chrono::high_resolution_clock::now();
            auto frame_ms = std::chrono::duration<float, std::milli>(frame_end - frame_start).count();
            float remain = FRAME_TIME_MS - frame_ms;
            if (remain > 0.0f) {
                std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(remain * 1000.0f)));
            }
        }
    }

    void shutdown() {
        if (engine_) {
            engine_->shutdown();
            engine_.reset();
        }
        SAFE_PRINT_LINE("BlueCube_Demo_FINAL_FIXED завершено");
    }

private:
    std::unique_ptr<SpectraForge::App::Engine> engine_;
    std::shared_ptr<SpectraForge::Core::ILogger> logger_;
    std::vector<glm::vec3> cube_vertices_;
    std::vector<unsigned> cube_indices_;

    void initCubeGeometry() {
        // ✅ ОКОНЧАТЕЛЬНЫЙ ФИКС: БОЛЬШОЙ куб для гарантированной видимости
        const float s = 2.0f; // Куб 4x4x4 units
        cube_vertices_ = {
            {-s, -s,  s}, { s, -s,  s}, { s,  s,  s}, {-s,  s,  s}, // front
            {-s, -s, -s}, {-s,  s, -s}, { s,  s, -s}, { s, -s, -s}  // back
        };

        cube_indices_ = {
            // front (CCW)
            0, 1, 2, 2, 3, 0,
            // back  
            4, 6, 5, 4, 7, 6,
            // left
            4, 0, 3, 4, 3, 5,
            // right
            1, 7, 2, 7, 6, 2,
            // bottom
            4, 1, 0, 4, 7, 1,
            // top
            3, 2, 6, 3, 6, 5
        };

        std::cout << "[BlueCubeFINAL] ✅ HUGE cube geometry: " << (s*2) << "x" << (s*2) << "x" << (s*2) 
                  << " units, " << (cube_indices_.size()/3) << " triangles" << std::endl;
    }

    static glm::vec3 rotateY(const glm::vec3& v, float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        return glm::vec3(c * v.x + s * v.z, v.y, -s * v.x + c * v.z);
    }

    void uploadRotatedCube(float angle) {
        auto renderer = engine_->getRenderer();
        auto h = std::dynamic_pointer_cast<SpectraForge::Rendering::HybridFreGSRenderer>(renderer);
        if (!h) return;

        std::vector<spectraforge::rendering::spectraforge::rendering::Triangle> tris;
        tris.reserve(cube_indices_.size() / 3);

        // ✅ ОЧЕНЬ яркий синий для maximum contrast
        const glm::vec3 brightBlue(0.3f, 0.8f, 1.0f);

        for (size_t i = 0; i < cube_indices_.size(); i += 3) {
            glm::vec3 v0 = rotateY(cube_vertices_[cube_indices_[i + 0]], angle);
            glm::vec3 v1 = rotateY(cube_vertices_[cube_indices_[i + 1]], angle);
            glm::vec3 v2 = rotateY(cube_vertices_[cube_indices_[i + 2]], angle);

            spectraforge::rendering::spectraforge::rendering::Triangle t;
            t.v0 = v0; t.v1 = v1; t.v2 = v2;
            t.color = brightBlue; 
            t.opacity = 1.0f;
            
            glm::vec3 e1 = v1 - v0;
            glm::vec3 e2 = v2 - v0;
            glm::vec3 n = glm::cross(e1, e2);
            float area2 = glm::length(n);
            t.normal = (area2 > 1e-6f) ? (n / area2) : glm::vec3(0, 0, 1);
            
            t.sigma = 1.0f;  // Средняя четкость
            t.materialId = 0;
            
            // Текстурные координаты
            t.texCoord0 = glm::vec2(0.0f, 0.0f);
            t.texCoord1 = glm::vec2(1.0f, 0.0f);
            t.texCoord2 = glm::vec2(0.5f, 1.0f);
            
            tris.push_back(t);
        }

        // Диагностика каждые 60 кадров
        static int frameCount = 0;
        if (frameCount++ % 60 == 0 && !tris.empty()) {
            const auto& t0 = tris[0];
            std::cout << "[BlueCubeFINAL] 🔺 Current triangle: v0=(" << t0.v0.x << "," 
                      << t0.v0.y << "," << t0.v0.z << ") angle=" << angle << std::endl;
        }

        h->setRenderMode(SpectraForge::Rendering::HybridFreGSRenderer::RenderMode::TriangleSplatting);
        h->uploadTriangles(tris);
    }
};

int main() {
    try {
        BlueCubeDemoFinalFixed demo;
        if (!demo.initialize()) return -1;
        demo.run();
        demo.shutdown();
    } catch (const std::exception& e) {
        std::cerr << "💥 Критическая ошибка: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}