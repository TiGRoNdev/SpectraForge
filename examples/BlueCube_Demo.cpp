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

        // ВАЖНО: позиционируем камеру ПОСЛЕ load_scene, чтобы переопределить дефолтные значения
        // ВРЕМЕННО: Прямая камера для отладки - смотрим прямо на куб
        std::cout << "🎯 [BlueCube_Demo] ABOUT TO CALL setCameraPosition(0, 0, 0.3)" << std::endl;
        engine_->setCameraPosition({0.0f, 0.0f, 0.3f});
        std::cout << "🎯 [BlueCube_Demo] ABOUT TO CALL setCameraTarget(0, 0, 0)" << std::endl;
        engine_->setCameraTarget({0.0f, 0.0f, 0.0f});
        std::cout << "🎯 [BlueCube_Demo] Camera setup COMPLETED" << std::endl;

        // Цвет фона через рендерер
        if (auto r = engine_->getRenderer()) {
            r->setBackgroundColor(cfg.backgroundColor[0], cfg.backgroundColor[1],
                                  cfg.backgroundColor[2], cfg.backgroundColor[3]);
            r->enableDepthTest(true);
            r->enableBackfaceCulling(false);  // ВРЕМЕННО отключено для отладки
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
    std::vector<glm::vec3> cube_face_normals_; // Оригинальные нормали для каждого треугольника

    void initCubeGeometry() {
        const float s = 0.5f;  // Нормальный размер куба (1.0 единица)
        cube_vertices_ = {
            {-s, -s,  s}, { s, -s,  s}, { s,  s,  s}, {-s,  s,  s}, // front  0..3
            {-s, -s, -s}, {-s,  s, -s}, { s,  s, -s}, { s, -s, -s}  // back   4..7
        };

        // ИСПРАВЛЕНО: Правильный порядок вершин для Vulkan (CCW в screen space)
        // Vulkan использует правостороннюю систему с Y вниз
        cube_indices_ = {
            // front (z+)
            0, 1, 2,  0, 2, 3,
            // back (z-)
            7, 6, 5,  7, 5, 4,
            // left (x-)
            4, 5, 3,  4, 3, 0,
            // right (x+)
            1, 7, 2,  7, 6, 2,
            // bottom (y-)
            4, 0, 1,  4, 1, 7,
            // top (y+)
            3, 2, 6,  3, 6, 5
        };
        
        // Оригинальные нормали для каждого треугольника (для стабильного цвета)
        // Порядок соответствует cube_indices_ (2 треугольника на грань)
        cube_face_normals_ = {
            // front (z+) - 2 треугольника
            {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f},
            // back (z-) - 2 треугольника
            {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f},
            // left (x-) - 2 треугольника
            {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
            // right (x+) - 2 треугольника
            {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
            // bottom (y-) - 2 треугольника
            {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f},
            // top (y+) - 2 треугольника
            {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}
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

        // Создаем куб из 12 треугольников (2 на грань, 6 граней)
        std::cout << "🔍 [BlueCube_Demo] sizeof(Triangle) = " 
                  << sizeof(spectraforge::rendering::TriangleSplattingPass::Triangle) 
                  << " bytes (expected: 160)\n";
        std::vector<spectraforge::rendering::TriangleSplattingPass::Triangle> tris;
        tris.reserve(cube_indices_.size() / 3);

        const glm::vec3 blue(0.0f, 0.3f, 1.0f);
        
        static int frameCount = 0;
        bool printDebug = (frameCount++ < 3);

        for (size_t i = 0; i < cube_indices_.size(); i += 3) {
            glm::vec3 v0 = rotateY(cube_vertices_[cube_indices_[i + 0]], angle);
            glm::vec3 v1 = rotateY(cube_vertices_[cube_indices_[i + 1]], angle);
            glm::vec3 v2 = rotateY(cube_vertices_[cube_indices_[i + 2]], angle);

            // ИСПРАВЛЕНО: Размещаем куб близко к камере для видимой 3D геометрии
            // Камера на Z=0.3, куб на Z=-2, размер куба 1.0
            // Расстояние = 2.3 единицы - куб будет занимать значительную часть экрана
            v0.z -= 2.0f; v1.z -= 2.0f; v2.z -= 2.0f;
            
            // Выводим информацию о треугольниках для отладки
            if (printDebug && (i/3) < 3) {
                std::cout << "[BlueCube] Triangle " << (i/3) << " vertices:" << std::endl;
                std::cout << "  v0: (" << v0.x << ", " << v0.y << ", " << v0.z << ")" << std::endl;
                std::cout << "  v1: (" << v1.x << ", " << v1.y << ", " << v1.z << ")" << std::endl;
                std::cout << "  v2: (" << v2.x << ", " << v2.y << ", " << v2.z << ")" << std::endl;
                
                // Проверяем площадь треугольника
                glm::vec3 e1 = v1 - v0;
                glm::vec3 e2 = v2 - v0;
                glm::vec3 cross = glm::cross(e1, e2);
                float area = glm::length(cross) * 0.5f;
                std::cout << "  Area: " << area << " (0 = degenerate)" << std::endl;
                
                // Проверяем расстояние от камеры (0, 0, 0.3)
                glm::vec3 camPos(0.0f, 0.0f, 0.3f);
                float dist = glm::length(v0 - camPos);
                std::cout << "  Distance from camera: " << dist << std::endl;
            }
            

            spectraforge::rendering::TriangleSplattingPass::Triangle t{};  // Инициализация по умолчанию
            t.v0 = glm::vec4(v0, 1.0f); 
            t.v1 = glm::vec4(v1, 1.0f); 
            t.v2 = glm::vec4(v2, 1.0f);
            
            // Вычисляем нормаль
            glm::vec3 e1 = v1 - v0;
            glm::vec3 e2 = v2 - v0;
            glm::vec3 n = glm::cross(e1, e2);
            float area = glm::length(n);
            
            // Нормализуем нормаль
            glm::vec3 normal;
            if (area > 1e-8f) {
                normal = n / area;
            } else {
                normal = glm::vec3(0, 0, 1); // Fallback
            }
            
            // Разные цвета для разных граней куба (по ОРИГИНАЛЬНОЙ нормали)
            // Используем cube_face_normals_ для стабильного цвета независимо от вращения
            glm::vec3 originalNormal = cube_face_normals_[i / 3];
            glm::vec3 faceColor;
            if (std::abs(originalNormal.x) > 0.9f) {
                faceColor = glm::vec3(1.0f, 0.0f, 0.0f); // Красный для X (left/right)
            } else if (std::abs(originalNormal.y) > 0.9f) {
                faceColor = glm::vec3(0.0f, 1.0f, 0.0f); // Зеленый для Y (top/bottom)
            } else {
                faceColor = glm::vec3(0.0f, 0.3f, 1.0f); // Синий для Z (front/back)
            }
            
            t.color = glm::vec4(faceColor, 1.0f);
            t.params = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f); // opacity=1.0, sigma=1.0
            t.normal = glm::vec4(normal, 0.0f);
            t.material = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
            tris.push_back(t);
        }
        
        // DEBUG: Выводим первый треугольник в hex для сравнения с GPU
        if (!tris.empty()) {
            const auto& t0 = tris[0];
            unsigned char* bytes = (unsigned char*)&t0;
            std::cout << "\n🔬 [BlueCube_Demo] First triangle RAW BYTES (first 160 bytes):\n";
            for (int i = 0; i < 160; i += 16) {
                printf("  %3d: ", i);
                for (int j = 0; j < 16 && (i+j) < 160; j++) {
                    printf("%02X ", bytes[i+j]);
                }
                printf("\n");
            }
            std::cout << "  Offset  96 (color):   expected 00 00 00 00 | 9A 99 99 3E | 00 00 80 3F | 00 00 80 3F\n";
            std::cout << "  Offset 112 (opacity): expected 00 00 80 3F (float 1.0)\n";
            std::cout << "  Offset 116 (sigma):   expected 00 00 80 3F (float 1.0)\n\n";
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


