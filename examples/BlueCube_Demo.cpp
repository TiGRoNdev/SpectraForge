//
// BlueCube_Demo.cpp — минимальное демо: вращающийся синий куб перед камерой
// Встраивается в текущий фасад `SpectraForge::App::Engine`.
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
#include "SpectraForge/rendering/TriangleSplattingPass.h"

using namespace SpectraForge;

class BlueCubeDemo {
public:
    bool initialize() {
        SpectraForge::Core::Console::initialize();

        SpectraForge::App::AppConfig cfg;
        cfg.window_width = 960;
        cfg.window_height = 540;
        cfg.window_title = "Blue Cube Demo";
        cfg.vsync = true;

        // Темно-синий фон — помогает контрасту
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

        // Минимальная сцена: без внешних моделей, достаточно пустой сцены
        SpectraForge::Vulkan::SceneData scene{};
        scene.scenePath = ""; // пусто — рендерер поднимется, фон и камера будут активны
        scene.triangleStep = 1;
        if (!engine_->load_scene(scene)) {
            SAFE_ERROR("Не удалось загрузить пустую сцену");
            return false;
        }

        // Позиционируем камеру чтобы видеть центр (0,0,0)
        auto camera = engine_->getCamera();
        if (camera) {
            camera->setPosition({0.0f, 0.0f, 3.5f});
            camera->setTarget({0.0f, 0.0f, 0.0f});
            camera->setPerspective(60.0f,
                                   static_cast<float>(cfg.window_width) / cfg.window_height,
                                   0.1f,
                                   100.0f);
        }

        // Цвет фона через рендерер
        if (auto r = engine_->getRenderer()) {
            r->setBackgroundColor(cfg.backgroundColor[0], cfg.backgroundColor[1],
                                  cfg.backgroundColor[2], cfg.backgroundColor[3]);
            r->enableDepthTest(true);
            r->enableBackfaceCulling(true);
        }

        // Подготовим базовую геометрию куба (вершины и индексы)
        initCubeGeometry();

        SAFE_PRINT_LINE("BlueCube_Demo инициализировано");
        return true;
    }

    void run() {
        // Простой render loop с ограничением FPS
        constexpr float TARGET_FPS = 60.0f;
        constexpr float FRAME_TIME_MS = 1000.0f / TARGET_FPS;

        auto last = std::chrono::high_resolution_clock::now();
        float angle = 0.0f;

        while (!engine_->should_close()) {
            auto frame_start = std::chrono::high_resolution_clock::now();

            // dt
            auto now = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(now - last).count();
            last = now;

            // Вращение куба
            angle += dt * 1.0f; // рад/сек

            // Обновляем треугольники куба под текущий угол и загружаем в рендерер
            uploadRotatedCube(angle);

            engine_->update(dt);
            engine_->render();

            // Ограничение FPS
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
        SAFE_PRINT_LINE("BlueCube_Demo завершено");
    }

private:
    std::unique_ptr<SpectraForge::App::Engine> engine_;
    std::shared_ptr<SpectraForge::Core::ILogger> logger_;

    // Базовая геометрия куба в локальных координатах (1.0f размер ребра)
    std::vector<glm::vec3> cube_vertices_;
    std::vector<unsigned> cube_indices_; // 12 треугольников * 3 индекса

    void initCubeGeometry() {
        const float s = 0.5f;
        cube_vertices_ = {
            {-s, -s,  s}, { s, -s,  s}, { s,  s,  s}, {-s,  s,  s}, // front  0..3
            {-s, -s, -s}, {-s,  s, -s}, { s,  s, -s}, { s, -s, -s}  // back   4..7
        };

        // Две триады на грань (всего 12 треугольников)
        cube_indices_ = {
            // front
            0, 1, 2, 2, 3, 0,
            // back
            4, 5, 6, 6, 7, 4,
            // left
            4, 0, 3, 3, 5, 4,
            // right
            1, 7, 6, 6, 2, 1,
            // bottom
            4, 7, 1, 1, 0, 4,
            // top
            3, 2, 6, 6, 5, 3
        };
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

        // Формируем треугольники с синим цветом
        std::vector<spectraforge::rendering::TriangleSplattingPass::Triangle> tris;
        tris.reserve(cube_indices_.size() / 3);

        const glm::vec3 blue(0.0f, 0.3f, 1.0f);

        for (size_t i = 0; i < cube_indices_.size(); i += 3) {
            glm::vec3 v0 = rotateY(cube_vertices_[cube_indices_[i + 0]], angle);
            glm::vec3 v1 = rotateY(cube_vertices_[cube_indices_[i + 1]], angle);
            glm::vec3 v2 = rotateY(cube_vertices_[cube_indices_[i + 2]], angle);

            // Сдвигаем куб немного вперёд по Z, чтобы он оказался перед камерой
            v0.z += 0.0f; v1.z += 0.0f; v2.z += 0.0f;

            spectraforge::rendering::TriangleSplattingPass::Triangle t;
            t.v0 = v0; t.v1 = v1; t.v2 = v2;
            t.color = blue; t.opacity = 1.0f;
            // Нормаль
            glm::vec3 e1 = v1 - v0;
            glm::vec3 e2 = v2 - v0;
            glm::vec3 n = glm::cross(e1, e2);
            float area2 = glm::length(n);
            t.normal = (area2 > 1e-6f) ? (n / area2) : glm::vec3(0, 0, 1);
            // Sigma чуть меньше для чётких граней
            t.sigma = 0.15f;
            t.materialId = 0;
            tris.push_back(t);
        }

        h->setRenderMode(SpectraForge::Rendering::HybridFreGSRenderer::RenderMode::TriangleSplatting);
        h->uploadTriangles(tris);
    }
};

int main() {
    try {
        BlueCubeDemo demo;
        if (!demo.initialize()) return -1;
        demo.run();
        demo.shutdown();
    } catch (const std::exception& e) {
        std::cerr << "💥 Критическая ошибка: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
