/**
 * @file Engine.cpp
 * @brief Реализация фасада App::Engine
 */

#include "SpectraForge/App/Engine.h"
#include <stdexcept>
#include "SpectraForge/Core/SafeConsole.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include "SpectraForge/Rendering/RendererFactory.h"
#include "SpectraForge/Rendering/Common/IResourceManager.h"
#include "SpectraForge/Rendering/HybridFreGSRenderer.h"
#include "SpectraForge/rendering/TriangleSplattingPass.h"
#include "SpectraForge/rendering/FreGSPass.h"
#include "SpectraForge/Core/Logger.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

namespace {
// Простой дефолтный ResourceManager-адаптер до полноценной интеграции VMA-слоя в IResourceManager
class NullResourceManager final : public SpectraForge::Rendering::IResourceManager {
  public:
    bool initialize() override { return true; }
    void shutdown() override {}
    SpectraForge::Rendering::BufferHandle createBuffer(const SpectraForge::Rendering::BufferDesc&) override { return SpectraForge::Rendering::INVALID_HANDLE; }
    void updateBuffer(SpectraForge::Rendering::BufferHandle, const void*, size_t, size_t) override {}
    void readBuffer(SpectraForge::Rendering::BufferHandle, void*, size_t, size_t) override {}
    SpectraForge::Rendering::TextureHandle createTexture(const SpectraForge::Rendering::TextureDesc&) override { return SpectraForge::Rendering::INVALID_HANDLE; }
    void updateTexture(SpectraForge::Rendering::TextureHandle, const void*, uint32_t, uint32_t) override {}
    SpectraForge::Rendering::ShaderHandle createShader(const std::string&, SpectraForge::Rendering::ShaderType) override { return SpectraForge::Rendering::INVALID_HANDLE; }
    SpectraForge::Rendering::ShaderHandle createShaderFromFile(const std::string&, SpectraForge::Rendering::ShaderType) override { return SpectraForge::Rendering::INVALID_HANDLE; }
    void releaseResource(SpectraForge::Rendering::ResourceHandle) override {}
    void releaseAllResources() override {}
    bool isValid(SpectraForge::Rendering::ResourceHandle) const override { return false; }
    size_t getResourceSize(SpectraForge::Rendering::ResourceHandle) const override { return 0; }
    SpectraForge::Rendering::MemoryStats getMemoryStats() const override { return {}; }
    void waitForCompletion() override {}
    void flush() override {}
};
}

using namespace SpectraForge;
using namespace SpectraForge::App;

Engine::Engine(const AppConfig &config, std::shared_ptr<Core::ILogger> logger)
    : config_(config), logger_(std::move(logger))
{
    // Дефолт: Жёстко выбираем HybridFreGSRenderer для соответствия требованиям
    std::shared_ptr<Rendering::IRenderer> rendererShared =
        std::make_shared<Rendering::HybridFreGSRenderer>();
    std::shared_ptr<Rendering::IResourceManager> rm = std::make_shared<NullResourceManager>();

    renderer_ = std::move(rendererShared);
    resource_manager_ = std::move(rm);

    if (!logger_ || !renderer_ || !resource_manager_) {
        throw std::invalid_argument("App::Engine: зависимости не могут быть nullptr");
    }
}

Engine::Engine(const AppConfig &config,
               std::shared_ptr<Core::ILogger> logger,
               std::shared_ptr<Rendering::IRenderer> renderer,
               std::shared_ptr<Rendering::IResourceManager> resource_manager)
    : config_(config),
      logger_(std::move(logger)),
      renderer_(std::move(renderer)),
      resource_manager_(std::move(resource_manager)) {
    if (!logger_ || !renderer_ || !resource_manager_) {
        throw std::invalid_argument("App::Engine: зависимости не могут быть nullptr");
    }
}

Engine::~Engine() { shutdown(); }

bool Engine::init() {
    // Инициализируем GLFW ДО всего остального
    logger_->logInfo("[App::Engine] Инициализация GLFW системы...");
    if (!Core::Window::initializeSystem()) {
        SAFE_ERROR("[App::Engine] Ошибка инициализации GLFW");
        return false;
    }
    logger_->logInfo("[App::Engine] GLFW инициализирована успешно");

    // Окно - создаем ДО инициализации EngineCore (Vulkan нужно окно для получения расширений WSI)
    logger_->logInfo("[App::Engine] Создание окна...");
    Core::Window::Config wc;
    wc.title = config_.window_title;
    wc.width = config_.window_width;
    wc.height = config_.window_height;
    wc.vSync = config_.vsync;
    // Подсказка для окна: использовать Vulkan presentation, без GL контекста
    setenv("SPECTRAFORGE_PRESENT_VULKAN", "1", 1);
    window_ = std::make_unique<Core::Window>(wc);
    if (!window_->initialize()) {
        SAFE_ERROR("[App::Engine] Ошибка инициализации окна (возможно, Vulkan не поддерживается GLFW)");
        return false;
    }
    logger_->logInfo("[App::Engine] Окно создано успешно");
    
    // Настройка обработки мыши для камеры
    GLFWwindow* win = window_->getNativeWindow();
    if (win) {
        glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetWindowUserPointer(win, this);
        glfwSetCursorPosCallback(win, [](GLFWwindow* w, double x, double y) {
            auto* engine = static_cast<Engine*>(glfwGetWindowUserPointer(w));
            if (engine->firstMouse) {
                engine->lastMouseX = x;
                engine->lastMouseY = y;
                engine->firstMouse = false;
            }
            float dx = static_cast<float>(x - engine->lastMouseX);
            float dy = static_cast<float>(engine->lastMouseY - y);
            engine->lastMouseX = x;
            engine->lastMouseY = y;
            
            const float sens = 0.1f;
            engine->yaw += dx * sens;
            engine->pitch += dy * sens;
            if (engine->pitch > 89.0f) engine->pitch = 89.0f;
            if (engine->pitch < -89.0f) engine->pitch = -89.0f;
            
            glm::vec3 front;
            front.x = cos(glm::radians(engine->yaw)) * cos(glm::radians(engine->pitch));
            front.y = sin(glm::radians(engine->pitch));
            front.z = sin(glm::radians(engine->yaw)) * cos(glm::radians(engine->pitch));
            engine->cameraFront = glm::normalize(front);
        });
    }

    // !!!ВАЖНО!!! glfwGetRequiredInstanceExtensions работает ТОЛЬКО после создания окна
    
    // Ядро - инициализируем ПОСЛЕ создания окна
    logger_->logInfo("[App::Engine] Инициализация EngineCore...");
    core_ = std::make_unique<Core::EngineCore>(renderer_, resource_manager_, logger_);
    if (!core_->initialize()) {
        SAFE_ERROR("[App::Engine] Ошибка инициализации EngineCore");
        return false;
    }
    logger_->logInfo("[App::Engine] EngineCore инициализирован успешно");

    // Привязываем окно к рендереру (для Vulkan swapchain и TriangleSplatting)
    auto* hybridRenderer = dynamic_cast<Rendering::HybridFreGSRenderer*>(renderer_.get());
    if (hybridRenderer && win) {
        logger_->logInfo("[App::Engine] Привязка окна к рендереру (Vulkan swapchain + Triangle Splatting)...");
        Display* display = glfwGetX11Display();
        Window x11Window = glfwGetX11Window(win);
        if (!hybridRenderer->attachWindow(display, reinterpret_cast<void*>(x11Window), config_.window_width, config_.window_height)) {
            SAFE_ERROR("[App::Engine] Ошибка привязки окна к рендереру");
            return false;
        }
        logger_->logInfo("[App::Engine] Окно успешно привязано к рендереру (Triangle Splatting готов)");
    }
    
    // Создаём камеру для рендеринга
    renderCamera_ = std::make_unique<Rendering::Camera3D>();
    float aspectRatio = static_cast<float>(config_.window_width) / static_cast<float>(config_.window_height);
    renderCamera_->setPerspective(60.0f, aspectRatio, 0.1f, 1000.0f);
    renderCamera_->setPosition(Math::Vector3(cameraPos.x, cameraPos.y, cameraPos.z));
    renderCamera_->lookAt(
        Math::Vector3(cameraPos.x, cameraPos.y, cameraPos.z),
        Math::Vector3(cameraPos.x + cameraFront.x, cameraPos.y + cameraFront.y, cameraPos.z + cameraFront.z),
        Math::Vector3(0.0f, 1.0f, 0.0f)  // Up vector
    );

    std::cout << "[Engine] 🎥 Камера инициализирована в позиции: (" << cameraPos.x << ", "
              << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
    std::cout << "[Engine] 👁️ Камера смотрит в направлении: (" << cameraFront.x << ", "
              << cameraFront.y << ", " << cameraFront.z << ")" << std::endl;
    
    // Передаём камеру в рендерер
    if (hybridRenderer) {
        hybridRenderer->setCamera(renderCamera_.get());
        logger_->logInfo("[App::Engine] Камера создана и передана в рендерер");
    }

    // Сцена
    scene_manager_ = std::make_unique<Vulkan::SceneManager>();
    if (!scene_manager_->init()) {
        SAFE_ERROR("[App::Engine] Ошибка инициализации SceneManager");
        return false;
    }

    logger_->logInfo("App::Engine инициализирован");
    return true;
}

bool Engine::load_scene(const Vulkan::SceneData &data) {
    if (!scene_manager_) return false;
    bool ok = scene_manager_->loadScene(data);
    if (ok && renderer_) {
        // Простейший OBJ-парсер для вершин и текстурных координат sponza
        std::vector<glm::vec3> verts;
        std::vector<glm::vec2> texCoords;
        verts.reserve(50000);
        texCoords.reserve(50000);
        try {
            // Разрешаем путь относительно нескольких баз: CWD, каталог exe, его родители
            auto resolvePathFn = [](const std::string& rel) -> std::filesystem::path {
                std::filesystem::path input(rel);
                if (std::filesystem::exists(input)) return std::filesystem::canonical(input);
                std::vector<std::filesystem::path> roots;
                roots.push_back(std::filesystem::current_path());
                std::error_code ec;
                auto exe = std::filesystem::read_symlink("/proc/self/exe", ec);
                if (!ec) {
                    auto exeDir = exe.parent_path();
                    roots.push_back(exeDir);
                    if (exeDir.has_parent_path()) roots.push_back(exeDir.parent_path());
                    if (exeDir.has_parent_path() && exeDir.parent_path().has_parent_path())
                        roots.push_back(exeDir.parent_path().parent_path());
                }
                for (const auto& r : roots) {
                    auto cand = r / input;
                    if (std::filesystem::exists(cand)) return std::filesystem::canonical(cand);
                }
                return input; // как есть
            };

            std::filesystem::path p = resolvePathFn(data.scenePath);
            std::ifstream in(p);
            if (in) {
                std::string line;
                while (std::getline(in, line)) {
                    if (line.size() > 2 && line[0] == 'v' && line[1] == ' ') {
                        // Парсинг вершин
                        std::istringstream iss(line.substr(2));
                        float x, y, z;
                        if (iss >> x >> y >> z) {
                            verts.emplace_back(x, y, z);
                        }
                    } else if (line.size() > 3 && line[0] == 'v' && line[1] == 't' && line[2] == ' ') {
                        // Парсинг текстурных координат
                        std::istringstream iss(line.substr(3));
                        float u, v;
                        if (iss >> u >> v) {
                            texCoords.emplace_back(u, v);
                        }
                    }
                }
                in.close();
            } else {
                logger_->logError("[App::Engine] Не удалось открыть OBJ: " + p.string());
            }
        } catch (...) {
            // игнорируем, используем fallback
        }

        // Функция разрешения пути (нужна для материалов тоже)
        auto resolvePathFn = [](const std::string& rel) -> std::filesystem::path {
            std::filesystem::path input(rel);
            if (std::filesystem::exists(input)) return std::filesystem::canonical(input);
            std::vector<std::filesystem::path> roots;
            roots.push_back(std::filesystem::current_path());
            std::error_code ec;
            auto exe = std::filesystem::read_symlink("/proc/self/exe", ec);
            if (!ec) {
                auto exeDir = exe.parent_path();
                roots.push_back(exeDir);
                if (exeDir.has_parent_path()) roots.push_back(exeDir.parent_path());
                if (exeDir.has_parent_path() && exeDir.parent_path().has_parent_path())
                    roots.push_back(exeDir.parent_path().parent_path());
            }
            for (const auto& r : roots) {
                auto cand = r / input;
                if (std::filesystem::exists(cand)) return std::filesystem::canonical(cand);
            }
            return input; // как есть
        };

        // Загружаем материалы из MTL файла
        struct Material {
            std::string name;
            std::string diffuseTexture;
            std::string normalTexture;
            glm::vec3 diffuseColor{1.0f};
            int id = 0;
        };
        std::vector<Material> materials;
        std::unordered_map<std::string, int> materialMap;

        // Парсим MTL файл если он существует
        std::filesystem::path objPath = resolvePathFn(data.scenePath);
        std::filesystem::path mtlPath = objPath.parent_path() / (objPath.stem().string() + ".mtl");
        if (std::filesystem::exists(mtlPath)) {
            std::ifstream mtlFile(mtlPath);
            if (mtlFile) {
                std::string line;
                std::string currentMaterialName;
                Material currentMaterial;
                while (std::getline(mtlFile, line)) {
                    std::istringstream iss(line);
                    std::string token;
                    iss >> token;

                    if (token == "newmtl") {
                        if (!currentMaterialName.empty()) {
                            currentMaterial.id = materials.size();
                            materials.push_back(currentMaterial);
                            materialMap[currentMaterialName] = currentMaterial.id;
                        }
                        iss >> currentMaterialName;
                        currentMaterial = Material{};
                        currentMaterial.name = currentMaterialName;
                    } else if (token == "Kd") {
                        iss >> currentMaterial.diffuseColor.r >> currentMaterial.diffuseColor.g >> currentMaterial.diffuseColor.b;
                    } else if (token == "map_Kd") {
                        std::string texturePath;
                        iss >> texturePath;
                        currentMaterial.diffuseTexture = texturePath;
                    } else if (token == "map_bump" || token == "bump") {
                        std::string texturePath;
                        iss >> texturePath;
                        currentMaterial.normalTexture = texturePath;
                    }
                }
                if (!currentMaterialName.empty()) {
                    currentMaterial.id = materials.size();
                    materials.push_back(currentMaterial);
                    materialMap[currentMaterialName] = currentMaterial.id;
                }
                mtlFile.close();
                std::cout << "[App::Engine] Загружено " << materials.size() << " материалов из " << mtlPath.string() << std::endl;
            }
        }

        // Собираем треугольники из OBJ (грани с текстурными координатами)
        struct Tri { int v1,v2,v3; int vt1,vt2,vt3; int material_id; };
        std::vector<Tri> tris;
        try {
            std::filesystem::path p = resolvePathFn(data.scenePath);
            std::ifstream in2(p);
            if (in2) {
                std::string line;
                std::string currentMaterialName = "default";
                int currentMaterialId = 0;
                while (std::getline(in2, line)) {
                    std::istringstream iss(line);
                    std::string token;
                    iss >> token;

                    // Отслеживаем текущий материал
                    if (token == "usemtl") {
                        iss >> currentMaterialName;
                        auto it = materialMap.find(currentMaterialName);
                        if (it != materialMap.end()) {
                            currentMaterialId = it->second;
                        } else {
                            currentMaterialId = 0;
                        }
                    } else if (token == "mtllib") {
                        // Материалы уже загружены выше
                    } else if (line.size() > 2 && line[0]=='f' && line[1]==' ') {
                        // Парсим грань - может быть треугольником или четырехугольником
                        std::istringstream iss(line.substr(2));
                        std::vector<std::string> vertices;
                        std::string token;
                        while (iss >> token) {
                            vertices.push_back(token);
                        }

                        if (vertices.size() >= 3) {
                            auto parseIndices=[&](const std::string& t, int& v, int& vt){
                                size_t s1 = t.find('/');
                                size_t s2 = t.find('/', s1 + 1);
                                v = std::stoi(s1==std::string::npos? t : t.substr(0,s1)) - 1;
                                vt = (s2 != std::string::npos) ? std::stoi(t.substr(s1+1, s2-s1-1)) - 1 : -1;
                            };

                            // Для треугольника: v1, v2, v3
                            if (vertices.size() == 3) {
                                int v1, vt1, v2, vt2, v3, vt3;
                                parseIndices(vertices[0], v1, vt1);
                                parseIndices(vertices[1], v2, vt2);
                                parseIndices(vertices[2], v3, vt3);
                                tris.push_back({v1, v2, v3, vt1, vt2, vt3, currentMaterialId});
                            }
                            // Для четырехугольника: разбиваем на два треугольника v1,v2,v3 и v1,v3,v4
                            else if (vertices.size() == 4) {
                                int v1, vt1, v2, vt2, v3, vt3, v4, vt4;
                                parseIndices(vertices[0], v1, vt1);
                                parseIndices(vertices[1], v2, vt2);
                                parseIndices(vertices[2], v3, vt3);
                                parseIndices(vertices[3], v4, vt4);

                                // Первый треугольник: v1, v2, v3
                                tris.push_back({v1, v2, v3, vt1, vt2, vt3, currentMaterialId});
                                // Второй треугольник: v1, v3, v4
                                tris.push_back({v1, v3, v4, vt1, vt3, vt4, currentMaterialId});
                            }
                        }
                    }
                }
                in2.close();
            }
        } catch (...) {}

        // Храним гауссианы в world-space 3D координатах (без проекции)
        std::vector<spectraforge::rendering::GaussianSplat> gs;

        // DEBUG: Добавляем тестовый треугольник прямо перед камерой для проверки алгоритма
        // Камера в (0, 3, 0) смотрит вперед (положительный Z), так что треугольник должен быть в Z > 3

        // Создаем только тестовый треугольник для изоляции проблемы
        std::vector<spectraforge::rendering::TriangleSplattingPass::Triangle> triangles;

        // DEBUG: Минимальный тест Triangle Splatting с известными координатами
        spectraforge::rendering::TriangleSplattingPass::Triangle testTriangle;
        // Треугольник в координатах, которые должны работать независимо от системы координат
        testTriangle.v0 = glm::vec3(-0.5f, -0.5f, 0.5f);  // Близко к камере
        testTriangle.v1 = glm::vec3(0.5f, -0.5f, 0.5f);
        testTriangle.v2 = glm::vec3(0.0f, 0.5f, 0.5f);
        testTriangle.color = glm::vec3(1.0f, 0.0f, 0.0f); // Красный цвет
        testTriangle.opacity = 1.0f;
        testTriangle.sigma = 0.1f;

        // Вычисляем нормаль (ориентирована на камеру)
        glm::vec3 edge1 = testTriangle.v1 - testTriangle.v0;
        glm::vec3 edge2 = testTriangle.v2 - testTriangle.v0;
        testTriangle.normal = glm::normalize(glm::cross(edge1, edge2));

        triangles.push_back(testTriangle);
        std::cout << "[App::Engine] DEBUG: Минимальный тест - треугольник координаты: ("
                  << testTriangle.v0.x << "," << testTriangle.v0.y << "," << testTriangle.v0.z << ") ("
                  << testTriangle.v1.x << "," << testTriangle.v1.y << "," << testTriangle.v1.z << ") ("
                  << testTriangle.v2.x << "," << testTriangle.v2.y << "," << testTriangle.v2.z << ")\n";

        if (!verts.empty() && !tris.empty()) {
            // ЗНАЧИТЕЛЬНО увеличиваем количество треугольников для плотного покрытия
            const size_t step = std::max<size_t>(1, tris.size() / 50000); // было 15000 → 50000
            gs.reserve((tris.size() / step) * 6); // резервируем с учётом subdivision до 6

            // Генерируем палитру цветов для разных материалов
            std::vector<glm::vec3> materialColors;
            materialColors.push_back(glm::vec3(0.85f, 0.75f, 0.65f)); // бежевый (стены)
            materialColors.push_back(glm::vec3(0.5f, 0.3f, 0.2f));    // коричневый (дерево)
            materialColors.push_back(glm::vec3(0.7f, 0.7f, 0.7f));    // серый (камень)
            materialColors.push_back(glm::vec3(0.8f, 0.6f, 0.4f));    // песочный
            materialColors.push_back(glm::vec3(0.4f, 0.3f, 0.25f));   // тёмно-коричневый
            materialColors.push_back(glm::vec3(0.9f, 0.85f, 0.7f));    // светлый
            materialColors.push_back(glm::vec3(0.6f, 0.4f, 0.3f));    // терракота

            // Старый код гауссианов удален - теперь используем TriangleSplatting
        } else {
            // Fallback: процедурная сетка гауссианов
            const int gx = 48, gy = 24;
            gs.reserve(gx * gy);
            for (int iy = 0; iy < gy; ++iy) {
                for (int ix = 0; ix < gx; ++ix) {
                    float sx = (ix / float(gx - 1)) * 1.8f - 0.9f;
                    float sy = (iy / float(gy - 1)) * 1.8f - 0.9f;
                    spectraforge::rendering::GaussianSplat s{};
                    s.positionAndScale = { sx, sy, 0.0f, 0.015f };
                    s.colorAndWeight = { 0.9f, 0.9f, 0.9f, 1.0f };
                    gs.push_back(s);
                }
            }
        }

        // === DUAL-PATH: Triangle Splatting для .obj, Gaussian для .ply ===
        auto h = std::dynamic_pointer_cast<Rendering::HybridFreGSRenderer>(renderer_);
        if (!h) return ok;

        // Определяем формат файла
        std::string ext = data.scenePath.substr(data.scenePath.find_last_of(".") + 1);

        if (ext == "obj" && !verts.empty() && !tris.empty()) {
            // ✅ OBJ → Triangle Splatting
            triangles.reserve(tris.size() + 1); // +1 для тестового треугольника
            
            // Реалистичная палитра цветов для Sponza (камень, штукатурка, дерево)
            std::vector<glm::vec3> materialColors;
            materialColors.push_back(glm::vec3(0.82f, 0.75f, 0.68f)); // светлая штукатурка (стены)
            materialColors.push_back(glm::vec3(0.65f, 0.55f, 0.45f)); // бежевый камень
            materialColors.push_back(glm::vec3(0.55f, 0.45f, 0.38f)); // тёмный камень
            materialColors.push_back(glm::vec3(0.48f, 0.35f, 0.25f)); // тёмное дерево (балки)
            materialColors.push_back(glm::vec3(0.72f, 0.62f, 0.52f)); // светлое дерево
            materialColors.push_back(glm::vec3(0.58f, 0.48f, 0.42f)); // терракота (пол)
            materialColors.push_back(glm::vec3(0.68f, 0.68f, 0.68f)); // серый камень (колонны)
            
            // DEBUG: Выводим диапазон координат для диагностики
            glm::vec3 minCoord(FLT_MAX), maxCoord(-FLT_MAX);
            for (const auto& v : verts) {
                minCoord.x = std::min(minCoord.x, v.x);
                minCoord.y = std::min(minCoord.y, v.y);
                minCoord.z = std::min(minCoord.z, v.z);
                maxCoord.x = std::max(maxCoord.x, v.x);
                maxCoord.y = std::max(maxCoord.y, v.y);
                maxCoord.z = std::max(maxCoord.z, v.z);
            }
            std::cout << "[Engine] 📐 Sponza AABB: min=(" << minCoord.x << ", " << minCoord.y << ", " << minCoord.z << ") max=(" << maxCoord.x << ", " << maxCoord.y << ", " << maxCoord.z << ")\n";
            std::cout << "[Engine] 📷 Камера находится в: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ") смотрит в: (" << cameraFront.x << ", " << cameraFront.y << ", " << cameraFront.z << ")\n";
            
            // Применяем triangleStep для уменьшения плотности (оптимизация производительности)
            size_t step = data.triangleStep > 0 ? data.triangleStep : 1;
            for (size_t i = 0; i < tris.size(); i += step) {
                const auto& tri = tris[i];

                if (tri.v1 < 0 || tri.v1 >= (int)verts.size() ||
                    tri.v2 < 0 || tri.v2 >= (int)verts.size() ||
                    tri.v3 < 0 || tri.v3 >= (int)verts.size()) {
                    continue;
                }

                spectraforge::rendering::TriangleSplattingPass::Triangle t;
                t.v0 = verts[tri.v1];
                t.v1 = verts[tri.v2];
                t.v2 = verts[tri.v3];

                // Текстурные координаты (если есть)
                if (tri.vt1 >= 0 && tri.vt1 < (int)texCoords.size()) {
                    t.texCoord0 = texCoords[tri.vt1];
                } else {
                    t.texCoord0 = glm::vec2(0.0f);
                }
                if (tri.vt2 >= 0 && tri.vt2 < (int)texCoords.size()) {
                    t.texCoord1 = texCoords[tri.vt2];
                } else {
                    t.texCoord1 = glm::vec2(0.0f);
                }
                if (tri.vt3 >= 0 && tri.vt3 < (int)texCoords.size()) {
                    t.texCoord2 = texCoords[tri.vt3];
                } else {
                    t.texCoord2 = glm::vec2(0.0f);
                }

                // Цвет из материала или дефолтный
                glm::vec3 color = materialColors[tri.material_id % materialColors.size()];
                if (tri.material_id < (int)materials.size()) {
                    color = materials[tri.material_id].diffuseColor;
                }
                // Повышаем яркость материалов для лучшей видимости в демо (временная калибровка)
                t.color = glm::min(color * 1.5f, glm::vec3(1.0f));
                t.opacity = 1.0f;

                // Вычисляем нормаль треугольника для освещения
                glm::vec3 edge1 = t.v1 - t.v0;
                glm::vec3 edge2 = t.v2 - t.v0;
                glm::vec3 faceNormal = glm::cross(edge1, edge2);
                float area = glm::length(faceNormal) * 0.5f;
                t.normal = (area > 0.0001f) ? glm::normalize(faceNormal) : glm::vec3(0, 1, 0);

                // Material ID для текстур
                t.materialId = tri.material_id;

                // Sigma зависит от размера треугольника
                t.sigma = std::sqrt(area) * 0.5f;  // Smoothness parameter
                t.sigma = glm::clamp(t.sigma, 0.10f, 2.0f);

                triangles.push_back(t);
            }
            
            std::cout << "[App::Engine] ⚡ TriangleStep=" << step << " применён: " 
                      << tris.size() << " → " << triangles.size() << " треугольников\n";
            
            // DEBUG: Первый треугольник
            if (!triangles.empty()) {
                const auto& t0 = triangles[0];
                std::cout << "🔺 DEBUG: Первый треугольник:\n";
                std::cout << "  v0=(" << t0.v0.x << "," << t0.v0.y << "," << t0.v0.z << ")\n";
                std::cout << "  v1=(" << t0.v1.x << "," << t0.v1.y << "," << t0.v1.z << ")\n";
                std::cout << "  v2=(" << t0.v2.x << "," << t0.v2.y << "," << t0.v2.z << ")\n";
                std::cout << "  color=(" << t0.color.r << "," << t0.color.g << "," << t0.color.b << ")\n";
                std::cout << "  normal=(" << t0.normal.x << "," << t0.normal.y << "," << t0.normal.z << ")\n";
                std::cout << "  sigma=" << t0.sigma << " opacity=" << t0.opacity << "\n";
            }
            
            std::cout << "[App::Engine] 🔺 Загружено " << triangles.size() 
                      << " треугольников для Triangle Splatting (" 
                      << (triangles.size() * sizeof(spectraforge::rendering::TriangleSplattingPass::Triangle) / 1024) 
                      << " KB)\n";
            h->uploadTriangles(triangles);
            
        } else {
            // ✅ PLY или fallback → Gaussian Splatting
            std::cout << "[App::Engine] ⚪ Uploaded " << gs.size() << " Gaussians (" 
                      << (gs.size() * sizeof(spectraforge::rendering::GaussianSplat) / 1024) << " KB)\n";
            h->uploadGaussians(gs);
        }
    }
    return ok;
}

void Engine::update(float delta_time) {
    if (core_) {
        // Пока только обновляем подсистемы через ядро
        // В текущем ядре updateSubsystems вызывается внутри run(),
        // поэтому здесь достаточно подготовить данные. Заглушка.
        (void)delta_time;
    }
    if (window_) {
        // Обработка событий каждый кадр, чтобы не зависала навигация/закрытие
        window_->pollEvents();
        
        // Управление камерой (WASD + мышь)
        GLFWwindow* win = window_->getNativeWindow();
        if (win) {
            float speed = 2.5f * delta_time;

            // Вычисляем направление взгляда камеры из yaw и pitch
            glm::vec3 front;
            front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            front.y = sin(glm::radians(pitch));
            front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            cameraFront = glm::normalize(front);

            // Вычисляем правую ось
            glm::vec3 right = glm::normalize(glm::cross(cameraFront, glm::vec3(0,1,0)));

            // Обработка клавиш WASD
            if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) cameraPos += cameraFront * speed;
            if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= cameraFront * speed;
            if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= right * speed;
            if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) cameraPos += right * speed;
            if (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS) cameraPos.y += speed;
            if (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) cameraPos.y -= speed;

            // Обновляем рендер-камеру после движения
            if (renderCamera_) {
                renderCamera_->setPosition(Math::Vector3(cameraPos.x, cameraPos.y, cameraPos.z));
                renderCamera_->lookAt(
                    Math::Vector3(cameraPos.x, cameraPos.y, cameraPos.z),
                    Math::Vector3(cameraPos.x + cameraFront.x, cameraPos.y + cameraFront.y, cameraPos.z + cameraFront.z),
                    Math::Vector3(0.0f, 1.0f, 0.0f)  // Up vector
                );
            }

            // Вывод отладочной информации о камере
            static int frameCount = 0;
            if (frameCount++ % 300 == 0) {  // Каждые 5 секунд при 60 FPS
                std::cout << "[Camera] Position: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
                std::cout << "[Camera] Direction: (" << cameraFront.x << ", " << cameraFront.y << ", " << cameraFront.z << ")" << std::endl;
                std::cout << "[Camera] Yaw: " << yaw << "°, Pitch: " << pitch << "°" << std::endl;
            }
        }
    }
}

void Engine::render() {
    if (!renderer_ || !renderer_->isInitialized()) return;
    
    // КРИТИЧЕСКИ ВАЖНО: Проверяем размер окна перед рендерингом
    // Пропускаем рендеринг если окно минимизировано (framebuffer 0x0)
    if (window_) {
        auto fb = window_->getFramebufferSize();
        if (fb.x <= 0.0f || fb.y <= 0.0f) {
            // Окно минимизировано или скрыто - пропускаем кадр
            static bool logged = false;
            if (!logged) {
                std::cout << "[Engine] ⏸️  Окно минимизировано, рендеринг приостановлен" << std::endl;
                logged = true;
            }
            return;
        }
    }
    
    renderer_->beginFrame();
    // Формируем данные кадра с камерой
    Rendering::FrameData frame{};
    if (window_) {
        auto fb = window_->getFramebufferSize();
        frame.renderTargetSize.width = static_cast<int>(fb.x);
        frame.renderTargetSize.height = static_cast<int>(fb.y);
        
        // Заполняем камеру
        float aspect = fb.x / std::max(1.0f, fb.y);
        frame.camera.position = {cameraPos.x, cameraPos.y, cameraPos.z};
        glm::vec3 center = cameraPos + cameraFront;
        frame.camera.target = {center.x, center.y, center.z};
        frame.camera.up = {0.0f, 1.0f, 0.0f};
        frame.camera.fov = 60.0f;
        frame.camera.nearPlane = 0.1f;
        frame.camera.farPlane = 100.0f;
        
        // Вычисляем матрицы View и Projection
        glm::vec3 upGlm(0.0f, 1.0f, 0.0f);
        frame.camera.viewMatrix = glm::lookAt(cameraPos, center, upGlm);
        frame.camera.projectionMatrix = glm::perspective(glm::radians(frame.camera.fov), aspect, frame.camera.nearPlane, frame.camera.farPlane);
    }
    // Обработка VK_ERROR_DEVICE_LOST: если рендерер сообщает о потере устройства — закрываем окно
    if (auto h = std::dynamic_pointer_cast<Rendering::HybridFreGSRenderer>(renderer_)) {
        if (h->isDeviceLost()) {
            std::cerr << "[App::Engine] ❌ Device lost — закрываем окно" << std::endl;
            if (window_) window_->close();
            return;
        }
    }

    renderer_->renderFrame(frame);

    if (auto h2 = std::dynamic_pointer_cast<Rendering::HybridFreGSRenderer>(renderer_)) {
        if (h2->isDeviceLost()) {
            std::cerr << "[App::Engine] ❌ Device lost обнаружен после renderFrame — закрываем окно" << std::endl;
            if (window_) window_->close();
            return;
        }
    }
    renderer_->endFrame();
}

void Engine::shutdown() {
    if (scene_manager_) {
        scene_manager_->shutdown();
        scene_manager_.reset();
    }
    if (core_) {
        core_->shutdown();
        core_.reset();
    }
    if (window_) {
        window_->close();
        window_.reset();
    }
    // Гарантируем корректное завершение GLFW
    SpectraForge::Core::Window::terminateSystem();
}


