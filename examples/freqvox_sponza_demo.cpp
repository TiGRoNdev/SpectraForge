/**
 * @file freqvox_sponza_demo.cpp
 * @brief Полнофункциональное демо FreqVox Renderer с рендерингом сцены Sponza
 * 
 * Демонстрирует:
 * - Загрузку OBJ модели (Sponza)
 * - Вокселизацию сцены
 * - Полный FreqVox пайплайн (Foveation, Frequency Shading, Temporal Reprojection, Upscaling)
 * - Интерактивную камеру (WASD + мышь)
 * - Статистику производительности в реальном времени
 */

// КРИТИЧЕСКИ ВАЖНО: Порядок включения заголовков для Vulkan/GLFW
// Проблема: vulkan.hpp переопределяет макросы, ломая GLFW функции

// Решение: Включаем GLFW с Vulkan ПЕРВЫМ
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifdef VULKAN_RENDERER_BUILD
// Затем C++ wrapper БЕЗ повторного включения базовых определений
// vulkan.hpp автоматически подтянет нужные типы из уже включенного vulkan.h
#include <vulkan/vulkan.hpp>

// ПРИМЕЧАНИЕ: HardwareDetector.h включает vulkan.hpp внутри,
// но это безопасно т.к. include guards предотвращают повторное определение
#include "SpectraForge/Vulkan/HardwareDetector.h"
#endif

// 4. Остальные библиотеки
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "SpectraForge/Core/Console.h"
#include "SpectraForge/Core/SafeConsole.h"
#include "SpectraForge/Math/Matrix4.h"
#include "SpectraForge/Math/Vector3.h"
#include "SpectraForge/Rendering/FreqVox/BackendFactory.h"
#include "SpectraForge/Rendering/FreqVox/FoveatedSelector.h"
#include "SpectraForge/Rendering/FreqVox/FrequencyShading.h"
#include "SpectraForge/Rendering/FreqVox/FreqVoxTypes.h"
#include "SpectraForge/Rendering/FreqVox/TemporalReprojection.h"
#include "SpectraForge/Rendering/FreqVox/NeuralUpscaling.h"

using namespace SpectraForge;
using namespace SpectraForge::Core;
using namespace SpectraForge::Rendering::FreqVox;
using namespace SpectraForge::Math;

// ============================================================================
// OBJ Loader - Простой загрузчик OBJ файлов
// ============================================================================

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

struct Material {
    std::string name;
    glm::vec3 ambient{0.2f, 0.2f, 0.2f};
    glm::vec3 diffuse{0.8f, 0.8f, 0.8f};
    glm::vec3 specular{1.0f, 1.0f, 1.0f};
    float shininess{32.0f};
    std::string texture_diffuse;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    Material material;
    glm::vec3 boundsMin{FLT_MAX, FLT_MAX, FLT_MAX};
    glm::vec3 boundsMax{-FLT_MAX, -FLT_MAX, -FLT_MAX};
};

class OBJLoader {
  public:
    static bool loadOBJ(const std::string& filepath,
                        std::vector<Mesh>& meshes,
                        std::unordered_map<std::string, Material>& materials) {
        std::cout << "[OBJLoader] Загрузка OBJ: " << filepath << std::endl;

        std::ifstream file(filepath);
        if (!file.is_open()) {
            SAFE_ERROR("Не удалось открыть файл: " + filepath);
            return false;
        }

        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> texCoords;

        std::vector<Vertex> currentVertices;
        std::vector<uint32_t> currentIndices;
        Material currentMaterial;
        std::string currentMaterialName = "default";

        std::string line;
        std::string mtlFile;
        size_t lineNumber = 0;

        // Первый проход: подсчет вершин для резервирования памяти
        while (std::getline(file, line)) {
            if (line.substr(0, 2) == "v ") {
                positions.reserve(positions.size() + 10000);
                break;
            }
        }
        file.clear();
        file.seekg(0);

        while (std::getline(file, line)) {
            ++lineNumber;
            
            if (line.empty() || line[0] == '#') {
                continue;
            }

            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;

            if (prefix == "mtllib") {
                iss >> mtlFile;
                // Загрузка MTL файла
                std::string mtlPath = filepath.substr(0, filepath.find_last_of("/\\") + 1) + mtlFile;
                loadMTL(mtlPath, materials);
            } else if (prefix == "v") {
                glm::vec3 pos;
                iss >> pos.x >> pos.y >> pos.z;
                positions.push_back(pos);
            } else if (prefix == "vn") {
                glm::vec3 normal;
                iss >> normal.x >> normal.y >> normal.z;
                normals.push_back(normal);
            } else if (prefix == "vt") {
                glm::vec2 texCoord;
                iss >> texCoord.x >> texCoord.y;
                texCoords.push_back(texCoord);
            } else if (prefix == "usemtl") {
                // Сохраняем предыдущий меш, если есть вершины
                if (!currentVertices.empty()) {
                    Mesh mesh;
                    mesh.vertices = std::move(currentVertices);
                    mesh.indices = std::move(currentIndices);
                    mesh.material = currentMaterial;
                    calculateBounds(mesh);
                    meshes.push_back(std::move(mesh));

                    currentVertices.clear();
                    currentIndices.clear();
                }

                iss >> currentMaterialName;
                if (materials.find(currentMaterialName) != materials.end()) {
                    currentMaterial = materials[currentMaterialName];
                }
            } else if (prefix == "f") {
                // Парсинг граней
                std::vector<Vertex> faceVertices;
                std::string vertexData;

                while (iss >> vertexData) {
                    Vertex vertex;
                    parseVertexData(vertexData, positions, normals, texCoords, vertex);
                    faceVertices.push_back(vertex);
                }

                // Триангуляция (для квадов и полигонов)
                if (faceVertices.size() >= 3) {
                    uint32_t baseIndex = static_cast<uint32_t>(currentVertices.size());
                    
                    for (size_t i = 0; i < faceVertices.size(); ++i) {
                        currentVertices.push_back(faceVertices[i]);
                    }

                    // Триангуляция веером
                    for (size_t i = 1; i + 1 < faceVertices.size(); ++i) {
                        currentIndices.push_back(baseIndex);
                        currentIndices.push_back(baseIndex + i);
                        currentIndices.push_back(baseIndex + i + 1);
                    }
                }
            }

            // Прогресс каждые 10000 строк
            if (lineNumber % 10000 == 0) {
                std::cout << "[OBJLoader] Обработано " << lineNumber << " строк..." << std::endl;
            }
        }

        // Сохраняем последний меш
        if (!currentVertices.empty()) {
            Mesh mesh;
            mesh.vertices = std::move(currentVertices);
            mesh.indices = std::move(currentIndices);
            mesh.material = currentMaterial;
            calculateBounds(mesh);
            meshes.push_back(std::move(mesh));
        }

        file.close();

        std::cout << "[OBJLoader] Загружено:" << std::endl;
        std::cout << "  - Мешей: " << meshes.size() << std::endl;
        std::cout << "  - Вершин: " << positions.size() << std::endl;
        std::cout << "  - Материалов: " << materials.size() << std::endl;

        return !meshes.empty();
    }

  private:
    static void parseVertexData(const std::string& data,
                                 const std::vector<glm::vec3>& positions,
                                 const std::vector<glm::vec3>& normals,
                                 const std::vector<glm::vec2>& texCoords,
                                 Vertex& vertex) {
        std::istringstream iss(data);
        std::string token;
        int indices[3] = {0, 0, 0};
        int idx = 0;

        while (std::getline(iss, token, '/') && idx < 3) {
            if (!token.empty()) {
                indices[idx] = std::stoi(token);
            }
            ++idx;
        }

        // OBJ индексы начинаются с 1
        if (indices[0] > 0 && static_cast<size_t>(indices[0]) <= positions.size()) {
            vertex.position = positions[indices[0] - 1];
        }
        if (indices[1] > 0 && static_cast<size_t>(indices[1]) <= texCoords.size()) {
            vertex.texCoord = texCoords[indices[1] - 1];
        }
        if (indices[2] > 0 && static_cast<size_t>(indices[2]) <= normals.size()) {
            vertex.normal = normals[indices[2] - 1];
        } else {
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);  // Дефолтная нормаль
        }
    }

    static bool loadMTL(const std::string& filepath,
                        std::unordered_map<std::string, Material>& materials) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            SAFE_WARNING("Не удалось загрузить MTL: " + filepath);
            return false;
        }

        Material currentMaterial;
        std::string currentName;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') {
                continue;
            }

            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;

            if (prefix == "newmtl") {
                if (!currentName.empty()) {
                    materials[currentName] = currentMaterial;
                }
                iss >> currentName;
                currentMaterial = Material();
                currentMaterial.name = currentName;
            } else if (prefix == "Ka") {
                iss >> currentMaterial.ambient.r >> currentMaterial.ambient.g >>
                    currentMaterial.ambient.b;
            } else if (prefix == "Kd") {
                iss >> currentMaterial.diffuse.r >> currentMaterial.diffuse.g >>
                    currentMaterial.diffuse.b;
            } else if (prefix == "Ks") {
                iss >> currentMaterial.specular.r >> currentMaterial.specular.g >>
                    currentMaterial.specular.b;
            } else if (prefix == "Ns") {
                iss >> currentMaterial.shininess;
            } else if (prefix == "map_Kd") {
                iss >> currentMaterial.texture_diffuse;
            }
        }

        if (!currentName.empty()) {
            materials[currentName] = currentMaterial;
        }

        file.close();
        return true;
    }

    static void calculateBounds(Mesh& mesh) {
        for (const auto& vertex : mesh.vertices) {
            mesh.boundsMin = glm::min(mesh.boundsMin, vertex.position);
            mesh.boundsMax = glm::max(mesh.boundsMax, vertex.position);
        }
    }
};

// ============================================================================
// Voxelizer - Конвертация мешей в воксели
// ============================================================================

class Voxelizer {
  public:
    static std::vector<Voxel> voxelizeMeshes(const std::vector<Mesh>& meshes,
                                             float voxelSize = 0.5f) {
        std::cout << "[Voxelizer] Вокселизация " << meshes.size() << " мешей..." << std::endl;
        std::cout << "[Voxelizer] Размер вокселя: " << voxelSize << " м" << std::endl;

        std::vector<Voxel> voxels;

        // Подсчет общих границ сцены
        glm::vec3 sceneMin(FLT_MAX, FLT_MAX, FLT_MAX);
        glm::vec3 sceneMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

        for (const auto& mesh : meshes) {
            sceneMin = glm::min(sceneMin, mesh.boundsMin);
            sceneMax = glm::max(sceneMax, mesh.boundsMax);
        }

        glm::vec3 sceneSize = sceneMax - sceneMin;
        std::cout << "[Voxelizer] Границы сцены: " << std::endl;
        std::cout << "  Min: (" << sceneMin.x << ", " << sceneMin.y << ", " << sceneMin.z << ")"
                  << std::endl;
        std::cout << "  Max: (" << sceneMax.x << ", " << sceneMax.y << ", " << sceneMax.z << ")"
                  << std::endl;
        std::cout << "  Size: (" << sceneSize.x << ", " << sceneSize.y << ", " << sceneSize.z
                  << ")" << std::endl;

        // Вокселизация через семплинг
        int voxelsX = static_cast<int>(sceneSize.x / voxelSize) + 1;
        int voxelsY = static_cast<int>(sceneSize.y / voxelSize) + 1;
        int voxelsZ = static_cast<int>(sceneSize.z / voxelSize) + 1;

        int totalVoxels = voxelsX * voxelsY * voxelsZ;
        std::cout << "[Voxelizer] Размеры вокселной сетки: " << voxelsX << "x" << voxelsY << "x"
                  << voxelsZ << " = " << totalVoxels << " вокселей" << std::endl;

        // Упрощенная вокселизация: создаем воксели в центрах треугольников
        std::cout << "[Voxelizer] Создание вокселей из треугольников..." << std::endl;
        int progress = 0;
        int totalTriangles = 0;

        for (const auto& mesh : meshes) {
            totalTriangles += mesh.indices.size() / 3;
        }

        for (const auto& mesh : meshes) {
            for (size_t i = 0; i < mesh.indices.size(); i += 3) {
                const Vertex& v0 = mesh.vertices[mesh.indices[i]];
                const Vertex& v1 = mesh.vertices[mesh.indices[i + 1]];
                const Vertex& v2 = mesh.vertices[mesh.indices[i + 2]];

                // Центр треугольника
                glm::vec3 center = (v0.position + v1.position + v2.position) / 3.0f;
                glm::vec3 normal = glm::normalize(v0.normal + v1.normal + v2.normal);

                // Создаем воксель
                Voxel voxel;
                voxel.position = Vector3(center.x, center.y, center.z);
                voxel.lod_bias = 0.0f;

                // Инициализируем SH коэффициенты на основе цвета материала
                glm::vec3 color = mesh.material.diffuse;
                initializeSH(voxel.radiance_sh, color, normal);

                voxels.push_back(voxel);

                // Прогресс
                ++progress;
                if (progress % 10000 == 0) {
                    float percent = (float)progress / totalTriangles * 100.0f;
                    std::cout << "[Voxelizer] Прогресс: " << percent << "% (" << progress << "/"
                              << totalTriangles << " треугольников)" << std::endl;
                }
            }
        }

        std::cout << "[Voxelizer] Создано " << voxels.size() << " вокселей" << std::endl;
        return voxels;
    }

  private:
    static void initializeSH(SH9& sh, const glm::vec3& color, const glm::vec3& normal) {
        // Упрощенная инициализация SH коэффициентов
        // L=0: константа
        float Y00 = 0.282095f;  // sqrt(1/(4*pi))

        // L=1: линейные компоненты
        float Y1m1 = 0.488603f;  // sqrt(3/(4*pi))
        float Y10 = 0.488603f;
        float Y11 = 0.488603f;

        // Базовые коэффициенты (упрощенно)
        sh.r[0] = color.r * Y00;
        sh.g[0] = color.g * Y00;
        sh.b[0] = color.b * Y00;

        // Направленная компонента от нормали
        sh.r[1] = color.r * Y1m1 * normal.x * 0.5f;
        sh.g[1] = color.g * Y1m1 * normal.x * 0.5f;
        sh.b[1] = color.b * Y1m1 * normal.x * 0.5f;

        sh.r[2] = color.r * Y10 * normal.y * 0.5f;
        sh.g[2] = color.g * Y10 * normal.y * 0.5f;
        sh.b[2] = color.b * Y10 * normal.y * 0.5f;

        sh.r[3] = color.r * Y11 * normal.z * 0.5f;
        sh.g[3] = color.g * Y11 * normal.z * 0.5f;
        sh.b[3] = color.b * Y11 * normal.z * 0.5f;

        // Остальные коэффициенты (L=2) - нули для упрощения
        for (int i = 4; i < 9; ++i) {
            sh.r[i] = 0.0f;
            sh.g[i] = 0.0f;
            sh.b[i] = 0.0f;
        }
    }
};

// ============================================================================
// FreqVoxDemo - Главный класс демо-приложения
// ============================================================================

class FreqVoxDemo {
  public:
    FreqVoxDemo() = default;
    ~FreqVoxDemo() = default;

    bool initialize() {
        SAFE_PRINT_LINE("=== FreqVox Sponza Demo ===");
        SAFE_PRINT_LINE("Полнофункциональное демо FreqVox Renderer");
        SAFE_PRINT_LINE("");

        // Инициализация GLFW
        if (!glfwInit()) {
            SAFE_ERROR("Ошибка инициализации GLFW");
            return false;
        }

#ifdef VULKAN_RENDERER_BUILD
        // Проверка Vulkan support в GLFW (ПОСЛЕ glfwInit!)
        if (!glfwVulkanSupported()) {
            SAFE_ERROR("GLFW: Vulkan не поддерживается!");
            SAFE_WARNING("Убедитесь что:");
            SAFE_WARNING("  1. Vulkan драйверы установлены (vulkan-tools, vulkan-loader)");
            SAFE_WARNING("  2. GLFW скомпилирована с Vulkan support");
            SAFE_WARNING("  3. VK_ICD_FILENAMES указывает на правильный ICD");
            glfwTerminate();
            return false;
        }
        std::cout << "[Vulkan] GLFW Vulkan support: ДА" << std::endl;
#endif

        // Настройка для Vulkan (без OpenGL контекста)
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        // Попытка создать окно (может вернуть nullptr в headless режиме)
        window_ = glfwCreateWindow(1920, 1080, "FreqVox Sponza Demo", nullptr, nullptr);
        if (!window_) {
            SAFE_WARNING("Ошибка создания окна - работа в headless режиме (без визуального отображения)");
        } else {
            // Настройка callbacks только если окно создано
            glfwSetWindowUserPointer(window_, this);
            glfwSetCursorPosCallback(window_, mouseCallback);
            glfwSetScrollCallback(window_, scrollCallback);
            glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

#ifdef VULKAN_RENDERER_BUILD
        // Инициализация Vulkan ПОСЛЕ создания окна (требуется для получения расширений)
        if (!initializeVulkan()) {
            SAFE_WARNING("Не удалось инициализировать Vulkan, используется compile-time выбор backend'а");
        }
#endif

        // Загрузка сцены Sponza
        if (!loadScene()) {
            return false;
        }

        // Инициализация FreqVox компонентов
        if (!initializeFreqVox()) {
            return false;
        }

        // Инициализация буферов для рендеринга
        initializeBuffers();

        SAFE_PRINT_LINE("");
        SAFE_PRINT_LINE("✅ Инициализация завершена успешно!");
        SAFE_PRINT_LINE("");
        SAFE_PRINT_LINE("Управление:");
        SAFE_PRINT_LINE("  WASD - движение камеры");
        SAFE_PRINT_LINE("  Мышь - поворот камеры");
        SAFE_PRINT_LINE("  Scroll - изменение скорости");
        SAFE_PRINT_LINE("  ESC - выход");
        SAFE_PRINT_LINE("  F - переключение фовеации");
        SAFE_PRINT_LINE("  T - переключение темпоральной репроекции");
        SAFE_PRINT_LINE("  U - переключение апскейлинга");
        SAFE_PRINT_LINE("");

        return true;
    }

    void run() {
        auto lastTime = std::chrono::high_resolution_clock::now();
        uint32_t frameCount = 0;
        float totalTime = 0.0f;

        while (!glfwWindowShouldClose(window_)) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            deltaTime_ =
                std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime)
                    .count();
            lastTime = currentTime;

            processInput();
            updateCamera();
            renderFrame();

            // Статистика FPS
            frameCount++;
            totalTime += deltaTime_;

            if (totalTime >= 1.0f) {
                fps_ = frameCount / totalTime;
                updateWindowTitle();
                frameCount = 0;
                totalTime = 0.0f;
            }

            // ❌ НЕ glfwSwapBuffers для Vulkan! Presentation делается через vkQueuePresentKHR в displayFrame()
            glfwPollEvents();
        }
    }

    void shutdown() {
        SAFE_PRINT_LINE("Завершение работы...");

        // Сначала освобождаем компоненты в обратном порядке создания
        if (upscaler_) {
            upscaler_->shutdown();
            upscaler_.reset();
        }
        if (temporalReprojection_) {
            temporalReprojection_->shutdown();
            temporalReprojection_.reset();
        }
        
        // ❗ ВАЖНО: НЕ освобождаем fftBackend_ напрямую, т.к. он был перемещен
        // в FrequencyShadingPipeline через std::move и будет освобожден автоматически
        // при уничтожении frequencyPipeline_
        if (frequencyPipeline_) {
            frequencyPipeline_->shutdown();
            frequencyPipeline_.reset();
        }
        
        if (foveatedSelector_) {
            foveatedSelector_.reset();
        }

#ifdef VULKAN_RENDERER_BUILD
        // Ожидание завершения всех операций
        if (vkDevice_) {
            vkDevice_.waitIdle();
        }
        
        // Очистка Vulkan Presentation ресурсов
        if (vulkanPresentationReady_) {
            // Cleanup compute pipeline
            if (computeDescriptorPool_) vkDevice_.destroyDescriptorPool(computeDescriptorPool_);
            if (computePipeline_) vkDevice_.destroyPipeline(computePipeline_);
            if (computePipelineLayout_) vkDevice_.destroyPipelineLayout(computePipelineLayout_);
            if (computeDescriptorSetLayout_) vkDevice_.destroyDescriptorSetLayout(computeDescriptorSetLayout_);
            if (computeShaderModule_) vkDevice_.destroyShaderModule(computeShaderModule_);
            
            if (inFlightFence_) vkDevice_.destroyFence(inFlightFence_);
            if (renderFinishedSemaphore_) vkDevice_.destroySemaphore(renderFinishedSemaphore_);
            if (imageAvailableSemaphore_) vkDevice_.destroySemaphore(imageAvailableSemaphore_);
            if (commandPool_) vkDevice_.destroyCommandPool(commandPool_);
            if (stagingBufferMemory_) vkDevice_.freeMemory(stagingBufferMemory_);
            if (stagingBuffer_) vkDevice_.destroyBuffer(stagingBuffer_);
            
            for (auto imageView : swapchainImageViews_) {
                vkDevice_.destroyImageView(imageView);
            }
            swapchainImageViews_.clear();
            
            if (vkSwapchain_) vkDevice_.destroySwapchainKHR(vkSwapchain_);
            if (vkDevice_) vkDevice_.destroy();
        }
        
        if (vkSurface_) vkInstance_.destroySurfaceKHR(vkSurface_);
        
        if (hardwareDetector_) {
            hardwareDetector_.reset();
        }
        if (vkInstance_) {
            vkInstance_.destroy();
        }
#endif

        if (window_) {
            glfwDestroyWindow(window_);
        }
        glfwTerminate();

        SAFE_PRINT_LINE("✅ Завершено успешно!");
    }

  private:
    // GLFW
    GLFWwindow* window_ = nullptr;
    float deltaTime_ = 0.0f;
    float fps_ = 0.0f;

    // Camera
    glm::vec3 cameraPos_ = glm::vec3(0.0f, 2.0f, 5.0f);
    glm::vec3 cameraFront_ = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp_ = glm::vec3(0.0f, 1.0f, 0.0f);
    float cameraSpeed_ = 5.0f;
    float yaw_ = -90.0f;
    float pitch_ = 0.0f;
    float lastX_ = 960.0f;
    float lastY_ = 540.0f;
    bool firstMouse_ = true;

    // Scene
    std::vector<Mesh> meshes_;
    std::unordered_map<std::string, Material> materials_;
    std::vector<Voxel> voxels_;

    // FreqVox Components
    std::unique_ptr<FoveatedSelector> foveatedSelector_;
    std::unique_ptr<IFrequencyBackend> fftBackend_;
    std::unique_ptr<FrequencyShadingPipeline> frequencyPipeline_;
    std::unique_ptr<TemporalReprojection> temporalReprojection_;
    std::unique_ptr<NeuralUpscaler> upscaler_;
    FoveatedParams foveatedParams_;
    std::vector<float> material_freq_;  ///< Precomputed material в частотной области

#ifdef VULKAN_RENDERER_BUILD
    // Vulkan для Hardware Detection
    vk::Instance vkInstance_;
    std::unique_ptr<SpectraForge::Vulkan::HardwareDetector> hardwareDetector_;
    
    // Vulkan Presentation
    vk::SurfaceKHR vkSurface_;
    vk::PhysicalDevice vkPhysicalDevice_;
    vk::Device vkDevice_;
    vk::Queue vkGraphicsQueue_;
    vk::Queue vkPresentQueue_;
    uint32_t graphicsQueueFamily_ = 0;
    uint32_t presentQueueFamily_ = 0;
    
    vk::SwapchainKHR vkSwapchain_;
    std::vector<vk::Image> swapchainImages_;
    std::vector<vk::ImageView> swapchainImageViews_;
    vk::Format swapchainImageFormat_;
    vk::Extent2D swapchainExtent_;
    
    // Staging buffer для копирования CPU данных на GPU
    vk::Buffer stagingBuffer_;
    vk::DeviceMemory stagingBufferMemory_;
    vk::CommandPool commandPool_;
    vk::CommandBuffer commandBuffer_;
    vk::Semaphore imageAvailableSemaphore_;
    vk::Semaphore renderFinishedSemaphore_;
    vk::Fence inFlightFence_;
    
    // Compute pipeline для float → uint8 конвертации
    vk::ShaderModule computeShaderModule_;
    vk::DescriptorSetLayout computeDescriptorSetLayout_;
    vk::PipelineLayout computePipelineLayout_;
    vk::Pipeline computePipeline_;
    vk::DescriptorPool computeDescriptorPool_;
    std::vector<vk::DescriptorSet> computeDescriptorSets_; // По одному на каждый swapchain image
    
    bool vulkanPresentationReady_ = false;
#endif

    // Render buffers
    std::vector<float> renderBuffer_;
    std::vector<Vector3> motionVectors_;
    std::vector<float> depthBuffer_;
    uint32_t renderWidth_ = 1920 / 2;   // Render at half resolution for upscaling
    uint32_t renderHeight_ = 1080 / 2;
    uint32_t displayWidth_ = 1920;
    uint32_t displayHeight_ = 1080;

    // Settings
    bool enableFoveation_ = true;
    bool enableTemporal_ = true;   // ✅ Включено после исправления
    bool enableUpscaling_ = true;  // ✅ Включено после исправления buffer overflow

    // Statistics
    float voxelSelectionTime_ = 0.0f;
    float frequencyShadingTime_ = 0.0f;
    float temporalTime_ = 0.0f;
    float upscalingTime_ = 0.0f;
    size_t selectedVoxelCount_ = 0;

    bool initializeVulkan() {
#ifdef VULKAN_RENDERER_BUILD
        try {
            SAFE_PRINT_LINE("[Vulkan] Инициализация для Hardware Detection...");

            // Создание Vulkan instance
            vk::ApplicationInfo appInfo{};
            appInfo.pApplicationName = "FreqVox Sponza Demo";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "SpectraForge";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_3;

            std::vector<const char*> extensions;

            // Получение расширений GLFW (только если окно создано)
            if (window_ != nullptr) {
                uint32_t glfwExtensionCount = 0;
                const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
                
                if (glfwExtensions && glfwExtensionCount > 0) {
                    extensions.assign(glfwExtensions, glfwExtensions + glfwExtensionCount);
                    
                    std::cout << "[Vulkan] GLFW расширения для поддержки окна:" << std::endl;
                    for (uint32_t i = 0; i < glfwExtensionCount; ++i) {
                        std::cout << "  - " << glfwExtensions[i] << std::endl;
                    }
                } else {
                    SAFE_WARNING("[Vulkan] Не удалось получить GLFW расширения (окно создано, но расширения недоступны)");
                }
            } else {
                std::cout << "[Vulkan] Headless режим - создаём instance без расширений surface" << std::endl;
            }

            vk::InstanceCreateInfo createInfo{};
            createInfo.pApplicationInfo = &appInfo;
            createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
            createInfo.ppEnabledExtensionNames = extensions.empty() ? nullptr : extensions.data();

            vkInstance_ = vk::createInstance(createInfo);

            // Получение физического устройства (берем первый доступный GPU)
            auto physicalDevices = vkInstance_.enumeratePhysicalDevices();
            if (physicalDevices.empty()) {
                SAFE_ERROR("[Vulkan] Физические устройства не найдены");
                return false;
            }
            vkPhysicalDevice_ = physicalDevices[0];

            // Создание HardwareDetector
            hardwareDetector_ = std::make_unique<SpectraForge::Vulkan::HardwareDetector>();
            if (!hardwareDetector_->init(vkPhysicalDevice_)) {
                SAFE_ERROR("[Vulkan] Ошибка инициализации HardwareDetector");
                return false;
            }

            // Вывод информации о железе
            std::cout << "[Vulkan] GPU обнаружен: " << hardwareDetector_->getDeviceName() << std::endl;
            
            auto vendor = hardwareDetector_->detectVendor();
            std::string vendorName;
            switch (vendor) {
                case SpectraForge::Vulkan::VendorType::NVIDIA:
                    vendorName = "NVIDIA";
                    break;
                case SpectraForge::Vulkan::VendorType::AMD:
                    vendorName = "AMD";
                    break;
                case SpectraForge::Vulkan::VendorType::INTEL:
                    vendorName = "Intel";
                    break;
                default:
                    vendorName = "Other";
                    break;
            }
            std::cout << "[Vulkan] Вендор: " << vendorName << std::endl;
            std::cout << "[Vulkan] VRAM: " << hardwareDetector_->getVRAMSize() / 1024 / 1024 << " MB" << std::endl;
            std::cout << "[Vulkan] CUDA поддержка: " << (hardwareDetector_->supportsCUDA() ? "Да" : "Нет") << std::endl;

            // Создаём минимальный Vulkan logical device для VkFFT backend
            // (простая версия без swapchain - только для compute)
            auto queueFamilies = vkPhysicalDevice_.getQueueFamilyProperties();
            uint32_t computeQueueFamily = UINT32_MAX;
            
            for (uint32_t i = 0; i < queueFamilies.size(); ++i) {
                if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute) {
                    computeQueueFamily = i;
                    break;
                }
            }
            
            if (computeQueueFamily != UINT32_MAX) {
                float queuePriority = 1.0f;
                vk::DeviceQueueCreateInfo queueCreateInfo{};
                queueCreateInfo.queueFamilyIndex = computeQueueFamily;
                queueCreateInfo.queueCount = 1;
                queueCreateInfo.pQueuePriorities = &queuePriority;
                
                vk::PhysicalDeviceFeatures deviceFeatures{};
                
                // Добавляем минимальные extensions для compute
                std::vector<const char*> deviceExtensions;
                // Проверяем доступные extensions
                auto availableExtensions = vkPhysicalDevice_.enumerateDeviceExtensionProperties();
                std::cout << "[Vulkan] Доступно " << availableExtensions.size() << " device extensions" << std::endl;
                
                vk::DeviceCreateInfo deviceCreateInfo{};
                deviceCreateInfo.queueCreateInfoCount = 1;
                deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
                deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
                deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
                deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.empty() ? nullptr : deviceExtensions.data();
                
                vkDevice_ = vkPhysicalDevice_.createDevice(deviceCreateInfo);
                std::cout << "[Vulkan] Logical device создан для VkFFT" << std::endl;
                std::cout << "[Vulkan] Physical Device Handle: " << static_cast<VkPhysicalDevice>(vkPhysicalDevice_) << std::endl;
                std::cout << "[Vulkan] Logical Device Handle: " << static_cast<VkDevice>(vkDevice_) << std::endl;
            } else {
                SAFE_WARNING("[Vulkan] Compute queue family не найдена, device не создан");
            }

            return true;

        } catch (const vk::SystemError& err) {
            std::cerr << "[Vulkan] Ошибка: " << err.what() << std::endl;
            return false;
        }
#else
        return false;
#endif
    }

    bool loadScene() {
        SAFE_PRINT_LINE("[1/4] Загрузка сцены Sponza...");

        // Путь относительно build/ каталога
        std::string scenePath = "../examples/scenes/sponza/sponza.obj";

        if (!OBJLoader::loadOBJ(scenePath, meshes_, materials_)) {
            SAFE_ERROR("Ошибка загрузки сцены Sponza");
            return false;
        }

        SAFE_PRINT_LINE("[2/4] Вокселизация сцены...");
        voxels_ = Voxelizer::voxelizeMeshes(meshes_, 0.3f);

        if (voxels_.empty()) {
            SAFE_ERROR("Ошибка вокселизации");
            return false;
        }

        return true;
    }

    bool initializeFreqVox() {
        SAFE_PRINT_LINE("[3/4] Инициализация FreqVox компонентов...");

        // 1. Фовеированный селектор
        foveatedSelector_ = std::make_unique<FoveatedSelector>();
        foveatedParams_.foveal_sigma_deg = 15.0f;  // Используем новый параметр
        foveatedParams_.far_plane = 200.0f;
        
        std::cout << "  ✓ FoveatedSelector создан" << std::endl;

        // 2. FFT Backend с Hardware Detection
#ifdef VULKAN_RENDERER_BUILD
        if (hardwareDetector_ && vkInstance_ && vkDevice_) {
            std::cout << "  [FFT Backend] Используется hardware-aware выбор с Vulkan контекстом" << std::endl;
            
            // Передаём Vulkan контекст для VkFFT backend
            fftBackend_ = BackendFactory::createWithHardwareDetection(
                BackendFactory::BackendType::Auto,
                hardwareDetector_.get(),
                vkInstance_,
                hardwareDetector_->getPhysicalDevice(),
                vkDevice_
            );
        } else if (hardwareDetector_) {
            std::cout << "  [FFT Backend] Hardware Detection без Vulkan контекста" << std::endl;
            fftBackend_ = BackendFactory::createWithHardwareDetection(
                BackendFactory::BackendType::Auto,
                hardwareDetector_.get()
            );
        } else {
            std::cout << "  [FFT Backend] Headless режим - используется SimpleDct" << std::endl;
            // В headless режиме без Hardware Detection используем SimpleDct (работает везде)
            fftBackend_ = BackendFactory::create(BackendFactory::BackendType::Simple);
        }
#else
        std::cout << "  [FFT Backend] Используется SimpleDct (Vulkan недоступен)" << std::endl;
        fftBackend_ = BackendFactory::create(BackendFactory::BackendType::Simple);
#endif

        if (!fftBackend_) {
            SAFE_ERROR("Ошибка создания FFT backend");
            return false;
        }

        DctBlockConfig dctConfig;
        dctConfig.blockSize = 8;
        // Уменьшаем batch для отладки VkFFT
        dctConfig.batchCount = 1;  // Минимальный batch для тестирования
        // TODO: После успешной работы увеличить до: std::min<uint32_t>(128, voxels_.size() / 64);

        if (!fftBackend_->initialize(dctConfig)) {
            SAFE_ERROR("Ошибка инициализации FFT backend");
            return false;
        }
        std::cout << "  ✓ FFT Backend инициализирован успешно" << std::endl;

        // Вывод информации о выбранном backend'е
        std::cout << "\n  💡 Информация о FFT backend:" << std::endl;
#ifdef VULKAN_RENDERER_BUILD
        if (hardwareDetector_) {
            std::cout << "     Режим: Hardware-Aware Selection" << std::endl;
            std::cout << "     GPU: " << hardwareDetector_->getDeviceName() << std::endl;
            if (hardwareDetector_->supportsCUDA()) {
                std::cout << "     Ожидаемый backend: cuFFT (NVIDIA CUDA)" << std::endl;
            } else {
                std::cout << "     Ожидаемый backend: VkFFT (Vulkan)" << std::endl;
            }
        } else {
            std::cout << "     Режим: Compile-Time Selection" << std::endl;
#ifdef HYPERENGINE_CUDA_AVAILABLE
            std::cout << "     Доступные: cuFFT, VkFFT, Simple" << std::endl;
#else
            std::cout << "     Доступные: VkFFT, Simple" << std::endl;
#endif
        }
#else
        std::cout << "     Режим: Compile-Time Selection (Vulkan недоступен)" << std::endl;
#ifdef HYPERENGINE_CUDA_AVAILABLE
        std::cout << "     Доступные: cuFFT, Simple" << std::endl;
#else
        std::cout << "     Доступные: Simple" << std::endl;
#endif
#endif
        std::cout << std::endl;

        // 3. Frequency Shading Pipeline
        frequencyPipeline_ = std::make_unique<FrequencyShadingPipeline>(std::move(fftBackend_));
        
        // Переиспользуем dctConfig из FFT backend (строка 868)
        // Обновляем только batchCount (будет динамически рассчитано в shade_image())
        dctConfig.batchCount = 1;
        
        if (!frequencyPipeline_->initialize(dctConfig)) {
            SAFE_ERROR("Ошибка инициализации FrequencyShadingPipeline");
            return false;
        }
        std::cout << "  ✓ FrequencyShadingPipeline инициализирован (блоки " 
                  << dctConfig.blockSize << "x" << dctConfig.blockSize << ")" << std::endl;
        
        // Precompute default material BRDF в частотной области
        // Для упрощения: Lambert diffuse material (константа 1/π)
        std::vector<float> material_spatial(64, 1.0f / 3.14159f);
        material_freq_.resize(64);
        if (!frequencyPipeline_->precompute_material(material_spatial, material_freq_)) {
            SAFE_ERROR("Ошибка precompute material");
            return false;
        }
        std::cout << "  ✓ Material BRDF precomputed" << std::endl;

        // 4. Temporal Reprojection
        temporalReprojection_ = std::make_unique<TemporalReprojection>();
        TemporalReprojectionParams temporalParams;
        temporalParams.blendFactor = 0.1f;
        temporalParams.motionVectorThreshold = 5.0f;
        temporalParams.depthChangeThreshold = 0.05f;

        if (!temporalReprojection_->initialize(renderWidth_, renderHeight_, temporalParams)) {
            SAFE_ERROR("Ошибка инициализации TemporalReprojection");
            return false;
        }
        std::cout << "  ✓ TemporalReprojection инициализирован" << std::endl;

        // 5. Neural Upscaler
        upscaler_ = std::make_unique<NeuralUpscaler>();
        NeuralUpscalingParams upscaleParams;
        upscaleParams.upscaleFactor = 2.0f;

        if (!upscaler_->initialize(UpscalerType::Bilinear, renderWidth_,
                                    renderHeight_, upscaleParams)) {
            SAFE_WARNING(
                "Ошибка инициализации Neural Upscaler, продолжаем без апскейлинга");
            enableUpscaling_ = false;
        } else {
            std::cout << "  ✓ NeuralUpscaler инициализирован" << std::endl;
        }

        return true;
    }

    bool initializeVulkanPresentation() {
#ifndef VULKAN_RENDERER_BUILD
        SAFE_WARNING("[Vulkan] Сборка без Vulkan - визуальное отображение недоступно");
        return false;
#else
        try {
            SAFE_PRINT_LINE("  [Vulkan Presentation] Инициализация...");
            
            // 1. Создать Surface
            VkSurfaceKHR rawSurface;
            VkResult result = glfwCreateWindowSurface(VkInstance(vkInstance_), window_, nullptr, &rawSurface);
            if (result != VK_SUCCESS) {
                std::cerr << "[Vulkan] Ошибка создания surface: VkResult = " << result << std::endl;
                std::cerr << "[Vulkan] vkInstance = " << VkInstance(vkInstance_) << std::endl;
                std::cerr << "[Vulkan] window = " << window_ << std::endl;
                
                // Проверка включенных расширений instance
                std::cerr << "[Vulkan] Проверка расширений не выполнена (недоступно в runtime)" << std::endl;
                
                SAFE_ERROR("[Vulkan] Ошибка создания surface - см. детали выше");
                return false;
            }
            vkSurface_ = vk::SurfaceKHR(rawSurface);
            std::cout << "    ✓ VkSurfaceKHR создан" << std::endl;
            
            // 2. Выбрать Physical Device (уже есть из Hardware Detection)
            if (!vkPhysicalDevice_) {
                auto physicalDevices = vkInstance_.enumeratePhysicalDevices();
                if (physicalDevices.empty()) {
                    SAFE_ERROR("[Vulkan] Физические устройства не найдены");
                    return false;
                }
                vkPhysicalDevice_ = physicalDevices[0];
            }
            
            // 3. Найти Queue Families
            if (!findQueueFamilies()) {
                SAFE_ERROR("[Vulkan] Не удалось найти подходящие queue families");
                return false;
            }
            std::cout << "    ✓ Queue families найдены (Graphics: " << graphicsQueueFamily_ 
                      << ", Present: " << presentQueueFamily_ << ")" << std::endl;
            
            // 4. Создать Logical Device
            if (!createLogicalDevice()) {
                SAFE_ERROR("[Vulkan] Ошибка создания logical device");
                return false;
            }
            std::cout << "    ✓ VkDevice создан" << std::endl;
            
            // 5. Создать Swapchain
            if (!createSwapchain()) {
                SAFE_ERROR("[Vulkan] Ошибка создания swapchain");
                return false;
            }
            std::cout << "    ✓ VkSwapchainKHR создан (" << swapchainExtent_.width << "x" 
                      << swapchainExtent_.height << ")" << std::endl;
            
            // 6. Создать Image Views
            if (!createImageViews()) {
                SAFE_ERROR("[Vulkan] Ошибка создания image views");
                return false;
            }
            std::cout << "    ✓ Image views созданы (" << swapchainImageViews_.size() << ")" << std::endl;
            
            // 7. Создать Staging Buffer для копирования данных
            if (!createStagingBuffer()) {
                SAFE_ERROR("[Vulkan] Ошибка создания staging buffer");
                return false;
            }
            std::cout << "    ✓ Staging buffer создан" << std::endl;
            
            // 8. Создать Command Pool и Buffer
            if (!createCommandResources()) {
                SAFE_ERROR("[Vulkan] Ошибка создания command resources");
                return false;
            }
            std::cout << "    ✓ Command pool и buffers созданы" << std::endl;
            
            // 9. Создать Synchronization Objects
            if (!createSyncObjects()) {
                SAFE_ERROR("[Vulkan] Ошибка создания sync objects");
                return false;
            }
            std::cout << "    ✓ Sync objects созданы" << std::endl;
            
            // 10. Создать Compute Pipeline для конвертации форматов
            if (!createComputePipeline()) {
                SAFE_ERROR("[Vulkan] Ошибка создания compute pipeline");
                return false;
            }
            std::cout << "    ✓ Compute pipeline создан" << std::endl;
            
            vulkanPresentationReady_ = true;
            SAFE_PRINT_LINE("  ✅ Vulkan Presentation готов к работе!");
            return true;
            
        } catch (const vk::SystemError& err) {
            SAFE_ERROR(std::string("[Vulkan] Исключение: ") + err.what());
            return false;
        }
#endif
    }

#ifdef VULKAN_RENDERER_BUILD
    bool findQueueFamilies() {
        auto queueFamilyProperties = vkPhysicalDevice_.getQueueFamilyProperties();
        
        bool graphicsFound = false;
        bool presentFound = false;
        
        for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i) {
            if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) {
                graphicsQueueFamily_ = i;
                graphicsFound = true;
            }
            
            VkBool32 presentSupport = vkPhysicalDevice_.getSurfaceSupportKHR(i, vkSurface_);
            if (presentSupport) {
                presentQueueFamily_ = i;
                presentFound = true;
            }
            
            if (graphicsFound && presentFound) {
                return true;
            }
        }
        
        return graphicsFound && presentFound;
    }
    
    bool createLogicalDevice() {
        std::set<uint32_t> uniqueQueueFamilies = {graphicsQueueFamily_, presentQueueFamily_};
        
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        float queuePriority = 1.0f;
        
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            vk::DeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }
        
        vk::PhysicalDeviceFeatures deviceFeatures{};
        
        vk::DeviceCreateInfo createInfo{};
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        
        // Extensions
        std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();
        
        vkDevice_ = vkPhysicalDevice_.createDevice(createInfo);
        vkGraphicsQueue_ = vkDevice_.getQueue(graphicsQueueFamily_, 0);
        vkPresentQueue_ = vkDevice_.getQueue(presentQueueFamily_, 0);
        
        return true;
    }
    
    bool createSwapchain() {
        auto surfaceCapabilities = vkPhysicalDevice_.getSurfaceCapabilitiesKHR(vkSurface_);
        auto surfaceFormats = vkPhysicalDevice_.getSurfaceFormatsKHR(vkSurface_);
        auto presentModes = vkPhysicalDevice_.getSurfacePresentModesKHR(vkSurface_);
        
        // Выбрать формат
        vk::SurfaceFormatKHR chosenFormat = surfaceFormats[0];
        for (const auto& format : surfaceFormats) {
            if (format.format == vk::Format::eB8G8R8A8Srgb &&
                format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                chosenFormat = format;
                break;
            }
        }
        swapchainImageFormat_ = chosenFormat.format;
        
        // Выбрать present mode
        vk::PresentModeKHR chosenPresentMode = vk::PresentModeKHR::eFifo; // VSync
        for (const auto& mode : presentModes) {
            if (mode == vk::PresentModeKHR::eMailbox) {
                chosenPresentMode = mode; // Triple buffering
                break;
            }
        }
        
        // Выбрать extent
        if (surfaceCapabilities.currentExtent.width != UINT32_MAX) {
            swapchainExtent_ = surfaceCapabilities.currentExtent;
        } else {
            swapchainExtent_ = vk::Extent2D{displayWidth_, displayHeight_};
        }
        
        // Количество изображений
        uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
        if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) {
            imageCount = surfaceCapabilities.maxImageCount;
        }
        
        vk::SwapchainCreateInfoKHR createInfo{};
        createInfo.surface = vkSurface_;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = swapchainImageFormat_;
        createInfo.imageColorSpace = chosenFormat.colorSpace;
        createInfo.imageExtent = swapchainExtent_;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
        
        uint32_t queueFamilyIndices[] = {graphicsQueueFamily_, presentQueueFamily_};
        if (graphicsQueueFamily_ != presentQueueFamily_) {
            createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        }
        
        createInfo.preTransform = surfaceCapabilities.currentTransform;
        createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        createInfo.presentMode = chosenPresentMode;
        createInfo.clipped = VK_TRUE;
        
        vkSwapchain_ = vkDevice_.createSwapchainKHR(createInfo);
        swapchainImages_ = vkDevice_.getSwapchainImagesKHR(vkSwapchain_);
        
        return true;
    }
    
    bool createImageViews() {
        swapchainImageViews_.resize(swapchainImages_.size());
        
        for (size_t i = 0; i < swapchainImages_.size(); ++i) {
            vk::ImageViewCreateInfo createInfo{};
            createInfo.image = swapchainImages_[i];
            createInfo.viewType = vk::ImageViewType::e2D;
            createInfo.format = swapchainImageFormat_;
            createInfo.components.r = vk::ComponentSwizzle::eIdentity;
            createInfo.components.g = vk::ComponentSwizzle::eIdentity;
            createInfo.components.b = vk::ComponentSwizzle::eIdentity;
            createInfo.components.a = vk::ComponentSwizzle::eIdentity;
            createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;
            
            swapchainImageViews_[i] = vkDevice_.createImageView(createInfo);
        }
        
        return true;
    }
    
    bool createStagingBuffer() {
        // Создаем staging buffer для CPU -> GPU трансфера
        // ВАЖНО: Размер по МАКСИМАЛЬНОМУ разрешению (display, не render!)
        // т.к. после upscaling буфер может быть displayWidth×displayHeight
        // GPU compute shader сделает конвертацию RGB→RGBA (DRY!)
        vk::DeviceSize bufferSize = static_cast<vk::DeviceSize>(displayWidth_) * displayHeight_ * 3 * sizeof(float);
        
        vk::BufferCreateInfo bufferInfo{};
        bufferInfo.size = bufferSize;
        bufferInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer; // Для compute shader чтения
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;
        
        stagingBuffer_ = vkDevice_.createBuffer(bufferInfo);
        
        // Выделить память (host-visible для CPU записи)
        vk::MemoryRequirements memRequirements = vkDevice_.getBufferMemoryRequirements(stagingBuffer_);
        
        vk::MemoryAllocateInfo allocInfo{};
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        
        stagingBufferMemory_ = vkDevice_.allocateMemory(allocInfo);
        vkDevice_.bindBufferMemory(stagingBuffer_, stagingBufferMemory_, 0);
        
        return true;
    }
    
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
        vk::PhysicalDeviceMemoryProperties memProperties = vkPhysicalDevice_.getMemoryProperties();
        
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        
        throw std::runtime_error("Failed to find suitable memory type!");
    }
    
    bool createCommandResources() {
        vk::CommandPoolCreateInfo poolInfo{};
        poolInfo.queueFamilyIndex = graphicsQueueFamily_;
        poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        
        commandPool_ = vkDevice_.createCommandPool(poolInfo);
        
        vk::CommandBufferAllocateInfo allocInfo{};
        allocInfo.commandPool = commandPool_;
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandBufferCount = 1;
        
        auto commandBuffers = vkDevice_.allocateCommandBuffers(allocInfo);
        commandBuffer_ = commandBuffers[0];
        
        return true;
    }
    
    bool createSyncObjects() {
        vk::SemaphoreCreateInfo semaphoreInfo{};
        vk::FenceCreateInfo fenceInfo{};
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
        
        imageAvailableSemaphore_ = vkDevice_.createSemaphore(semaphoreInfo);
        renderFinishedSemaphore_ = vkDevice_.createSemaphore(semaphoreInfo);
        inFlightFence_ = vkDevice_.createFence(fenceInfo);
        
        return true;
    }
    
    std::vector<char> readShaderFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        
        if (!file.is_open()) {
            throw std::runtime_error("Не удалось открыть shader file: " + filename);
        }
        
        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);
        
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        
        return buffer;
    }
    
    bool createComputePipeline() {
        try {
            // 1. Загрузить compute shader
            auto shaderCode = readShaderFile("../shaders/float_to_rgba8.comp.spv");
            
            vk::ShaderModuleCreateInfo shaderModuleInfo{};
            shaderModuleInfo.codeSize = shaderCode.size();
            shaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());
            
            computeShaderModule_ = vkDevice_.createShaderModule(shaderModuleInfo);
            
            // 2. Создать descriptor set layout
            // Binding 0: Storage Buffer (input float data)
            // Binding 1: Storage Image (output rgba8 image)
            std::array<vk::DescriptorSetLayoutBinding, 2> bindings{};
            
            bindings[0].binding = 0;
            bindings[0].descriptorType = vk::DescriptorType::eStorageBuffer;
            bindings[0].descriptorCount = 1;
            bindings[0].stageFlags = vk::ShaderStageFlagBits::eCompute;
            
            bindings[1].binding = 1;
            bindings[1].descriptorType = vk::DescriptorType::eStorageImage;
            bindings[1].descriptorCount = 1;
            bindings[1].stageFlags = vk::ShaderStageFlagBits::eCompute;
            
            vk::DescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.bindingCount = bindings.size();
            layoutInfo.pBindings = bindings.data();
            
            computeDescriptorSetLayout_ = vkDevice_.createDescriptorSetLayout(layoutInfo);
            
            // 3. Создать pipeline layout с push constants
            vk::PushConstantRange pushConstantRange{};
            pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eCompute;
            pushConstantRange.offset = 0;
            pushConstantRange.size = 4 * sizeof(uint32_t); // 4 uint32: srcWidth, srcHeight, dstWidth, dstHeight
            
            vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &computeDescriptorSetLayout_;
            pipelineLayoutInfo.pushConstantRangeCount = 1;
            pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
            
            computePipelineLayout_ = vkDevice_.createPipelineLayout(pipelineLayoutInfo);
            
            // 4. Создать compute pipeline
            vk::PipelineShaderStageCreateInfo shaderStageInfo{};
            shaderStageInfo.stage = vk::ShaderStageFlagBits::eCompute;
            shaderStageInfo.module = computeShaderModule_;
            shaderStageInfo.pName = "main";
            
            vk::ComputePipelineCreateInfo pipelineInfo{};
            pipelineInfo.stage = shaderStageInfo;
            pipelineInfo.layout = computePipelineLayout_;
            
            auto result = vkDevice_.createComputePipeline(nullptr, pipelineInfo);
            if (result.result != vk::Result::eSuccess) {
                throw std::runtime_error("Не удалось создать compute pipeline");
            }
            computePipeline_ = result.value;
            
            // 5. Создать descriptor pool
            std::array<vk::DescriptorPoolSize, 2> poolSizes{};
            poolSizes[0].type = vk::DescriptorType::eStorageBuffer;
            poolSizes[0].descriptorCount = static_cast<uint32_t>(swapchainImages_.size());
            
            poolSizes[1].type = vk::DescriptorType::eStorageImage;
            poolSizes[1].descriptorCount = static_cast<uint32_t>(swapchainImages_.size());
            
            vk::DescriptorPoolCreateInfo poolInfo{};
            poolInfo.poolSizeCount = poolSizes.size();
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = static_cast<uint32_t>(swapchainImages_.size());
            
            computeDescriptorPool_ = vkDevice_.createDescriptorPool(poolInfo);
            
            // 6. Создать descriptor sets (по одному на каждый swapchain image)
            std::vector<vk::DescriptorSetLayout> layouts(swapchainImages_.size(), computeDescriptorSetLayout_);
            
            vk::DescriptorSetAllocateInfo allocInfo{};
            allocInfo.descriptorPool = computeDescriptorPool_;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
            allocInfo.pSetLayouts = layouts.data();
            
            computeDescriptorSets_ = vkDevice_.allocateDescriptorSets(allocInfo);
            
            // 7. Обновить descriptor sets (binding 0 = staging buffer для всех)
            for (size_t i = 0; i < computeDescriptorSets_.size(); ++i) {
                vk::DescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = stagingBuffer_;
                bufferInfo.offset = 0;
                bufferInfo.range = VK_WHOLE_SIZE;
                
                vk::DescriptorImageInfo imageInfo{};
                imageInfo.imageView = swapchainImageViews_[i];
                imageInfo.imageLayout = vk::ImageLayout::eGeneral;
                
                std::array<vk::WriteDescriptorSet, 2> descriptorWrites{};
                
                descriptorWrites[0].dstSet = computeDescriptorSets_[i];
                descriptorWrites[0].dstBinding = 0;
                descriptorWrites[0].dstArrayElement = 0;
                descriptorWrites[0].descriptorType = vk::DescriptorType::eStorageBuffer;
                descriptorWrites[0].descriptorCount = 1;
                descriptorWrites[0].pBufferInfo = &bufferInfo;
                
                descriptorWrites[1].dstSet = computeDescriptorSets_[i];
                descriptorWrites[1].dstBinding = 1;
                descriptorWrites[1].dstArrayElement = 0;
                descriptorWrites[1].descriptorType = vk::DescriptorType::eStorageImage;
                descriptorWrites[1].descriptorCount = 1;
                descriptorWrites[1].pImageInfo = &imageInfo;
                
                vkDevice_.updateDescriptorSets(descriptorWrites, {});
            }
            
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "[Vulkan] Ошибка создания compute pipeline: " << e.what() << std::endl;
            return false;
        }
    }
#endif

    void initializeBuffers() {
        SAFE_PRINT_LINE("[4/4] Инициализация буферов рендеринга...");

        size_t pixelCount = renderWidth_ * renderHeight_;
        renderBuffer_.resize(pixelCount * 3, 0.0f);  // RGB
        motionVectors_.resize(pixelCount, Vector3(0.0f, 0.0f, 0.0f));
        depthBuffer_.resize(pixelCount, 1.0f);

        std::cout << "  Разрешение рендера: " << renderWidth_ << "x" << renderHeight_
                  << std::endl;
        std::cout << "  Разрешение дисплея: " << displayWidth_ << "x" << displayHeight_
                  << std::endl;
        std::cout << "  Размер буфера: "
                  << (renderBuffer_.size() * sizeof(float) / 1024 / 1024) << " MB" << std::endl;
        
        // Инициализация Vulkan презентации (только если окно создано)
        if (window_ != nullptr) {
            if (!initializeVulkanPresentation()) {
                SAFE_WARNING("Ошибка инициализации Vulkan презентации, визуальное отображение недоступно");
            }
        } else {
            SAFE_PRINT_LINE("  [Headless Mode] Окно не создано, визуальное отображение недоступно");
        }
    }

    void renderFrame() {
        auto frameStart = std::chrono::high_resolution_clock::now();

        // Получение параметров камеры
        Vector3 cameraPosition(cameraPos_.x, cameraPos_.y, cameraPos_.z);
        Vector3 cameraDirection(cameraFront_.x, cameraFront_.y, cameraFront_.z);
        // float fov = 60.0f;  // Не используется в текущей реализации

        // 1. Фовеированная выборка вокселей
        auto t1 = std::chrono::high_resolution_clock::now();
        std::vector<Voxel> selectedVoxels;
        float effectiveCount = 0.0f;
        if (enableFoveation_) {
            foveatedSelector_->select(voxels_, foveatedParams_, selectedVoxels, effectiveCount);
            selectedVoxelCount_ = selectedVoxels.size();
        } else {
            selectedVoxels = voxels_;
            selectedVoxelCount_ = voxels_.size();
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        voxelSelectionTime_ =
            std::chrono::duration<float, std::milli>(t2 - t1).count();

        // 2. Frequency-Domain Shading
        t1 = std::chrono::high_resolution_clock::now();
        rasterizeVoxels(selectedVoxels, renderBuffer_);
        
        // Применяем частотное преобразование к полному изображению
        // Используем новый высокоуровневый API shade_image()
        if (frequencyPipeline_) {
            // Автоматически разбивает изображение на блоки 8×8 и обрабатывает
            if (!frequencyPipeline_->shade_image(renderBuffer_, 
                                                  renderWidth_, 
                                                  renderHeight_,
                                                  material_freq_)) {
                SAFE_ERROR("Ошибка frequency shading");
            }
        }
        t2 = std::chrono::high_resolution_clock::now();
        frequencyShadingTime_ = std::chrono::duration<float, std::milli>(t2 - t1).count();

        // 3. Temporal Reprojection
        t1 = std::chrono::high_resolution_clock::now();
        if (enableTemporal_ && temporalReprojection_) {
            std::vector<float> temporalOutput;
            temporalReprojection_->reproject(renderBuffer_, motionVectors_, depthBuffer_,
                                            temporalOutput);
            renderBuffer_ = std::move(temporalOutput);
        }
        t2 = std::chrono::high_resolution_clock::now();
        temporalTime_ = std::chrono::duration<float, std::milli>(t2 - t1).count();

        // 4. Neural Upscaling
        t1 = std::chrono::high_resolution_clock::now();
        std::vector<float> finalBuffer;
        if (enableUpscaling_ && upscaler_) {
            upscaler_->upscale(renderBuffer_, finalBuffer, nullptr);
        } else {
            finalBuffer = renderBuffer_;
        }
        t2 = std::chrono::high_resolution_clock::now();
        upscalingTime_ = std::chrono::duration<float, std::milli>(t2 - t1).count();

        // 5. Отображение через Vulkan
        displayFrame(finalBuffer);
        auto frameEnd = std::chrono::high_resolution_clock::now();
        float frameTotalTime =
            std::chrono::duration<float, std::milli>(frameEnd - frameStart).count();

        // Вывод статистики каждые 60 кадров
        static int frameCounter = 0;
        if (++frameCounter >= 60) {
            printStatistics(frameTotalTime);
            frameCounter = 0;
        }
    }

    void rasterizeVoxels(const std::vector<Voxel>& voxels, std::vector<float>& buffer) {
        // Упрощенная растеризация вокселей в буфер
        // В реальной реализации здесь был бы Gaussian Splatting или ray marching
        
        std::fill(buffer.begin(), buffer.end(), 0.0f);

        glm::mat4 view = glm::lookAt(cameraPos_, cameraPos_ + cameraFront_, cameraUp_);
        glm::mat4 proj = glm::perspective(glm::radians(60.0f),
                                          (float)renderWidth_ / renderHeight_, 0.1f, 100.0f);
        glm::mat4 viewProj = proj * view;

        for (const auto& voxel : voxels) {
            glm::vec4 pos(voxel.position.x, voxel.position.y, voxel.position.z, 1.0f);
            glm::vec4 clipPos = viewProj * pos;

            if (clipPos.w > 0.0f) {
                glm::vec3 ndcPos = glm::vec3(clipPos) / clipPos.w;

                if (ndcPos.x >= -1.0f && ndcPos.x <= 1.0f && ndcPos.y >= -1.0f &&
                    ndcPos.y <= 1.0f) {
                    int x = static_cast<int>((ndcPos.x * 0.5f + 0.5f) * renderWidth_);
                    int y = static_cast<int>((ndcPos.y * 0.5f + 0.5f) * renderHeight_);

                    if (x >= 0 && x < (int)renderWidth_ && y >= 0 && y < (int)renderHeight_) {
                        size_t idx = (y * renderWidth_ + x) * 3;
                        if (idx + 2 < buffer.size()) {
                            // Используем SH коэффициенты L=0 как цвет
                            buffer[idx + 0] = voxel.radiance_sh.r[0];
                            buffer[idx + 1] = voxel.radiance_sh.g[0];
                            buffer[idx + 2] = voxel.radiance_sh.b[0];
                        }
                    }
                }
            }
        }
    }

    void displayFrame(const std::vector<float>& buffer) {
#ifndef VULKAN_RENDERER_BUILD
        // Без Vulkan просто пропускаем отображение
        (void)buffer;
#else
        if (!vulkanPresentationReady_) {
            return; // Презентация не готова
        }
        
        try {
            // 1. Ожидание предыдущего кадра
            auto result = vkDevice_.waitForFences(1, &inFlightFence_, VK_TRUE, UINT64_MAX);
            if (result != vk::Result::eSuccess) return;
            
            result = vkDevice_.resetFences(1, &inFlightFence_);
            if (result != vk::Result::eSuccess) return;
            
            // 2. Получить следующее изображение из swapchain
            uint32_t imageIndex;
            result = vkDevice_.acquireNextImageKHR(vkSwapchain_, UINT64_MAX,
                                                   imageAvailableSemaphore_, nullptr, &imageIndex);
            
            if (result == vk::Result::eErrorOutOfDateKHR) {
                // Нужно пересоздать swapchain (при изменении размера окна)
                return;
            }
            
            // 3. Скопировать RGB float данные напрямую в staging buffer
            // Конвертация RGB→RGBA + масштабирование будет на GPU через compute shader (DRY!)
            void* mappedData = vkDevice_.mapMemory(stagingBufferMemory_, 0, VK_WHOLE_SIZE);
            memcpy(mappedData, buffer.data(), buffer.size() * sizeof(float));
            vkDevice_.unmapMemory(stagingBufferMemory_);
            
            // 4. Записать команды compute shader для конвертации и копирования
            commandBuffer_.reset();
            
            vk::CommandBufferBeginInfo beginInfo{};
            commandBuffer_.begin(beginInfo);
            
            // Transition swapchain image to General layout (требуется для storage image)
            vk::ImageMemoryBarrier barrier{};
            barrier.oldLayout = vk::ImageLayout::eUndefined;
            barrier.newLayout = vk::ImageLayout::eGeneral;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = swapchainImages_[imageIndex];
            barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.srcAccessMask = vk::AccessFlagBits::eNone;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderWrite;
            
            commandBuffer_.pipelineBarrier(
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eComputeShader,
                {}, 0, nullptr, 0, nullptr, 1, &barrier);
            
            // Bind compute pipeline
            commandBuffer_.bindPipeline(vk::PipelineBindPoint::eCompute, computePipeline_);
            
            // Bind descriptor set для текущего swapchain image
            commandBuffer_.bindDescriptorSets(
                vk::PipelineBindPoint::eCompute,
                computePipelineLayout_,
                0, 1, &computeDescriptorSets_[imageIndex],
                0, nullptr
            );
            
            // Push constants (размеры)
            struct PushConstants {
                uint32_t srcWidth;
                uint32_t srcHeight;
                uint32_t dstWidth;
                uint32_t dstHeight;
            } pushConstants;
            
            pushConstants.srcWidth = renderWidth_;
            pushConstants.srcHeight = renderHeight_;
            pushConstants.dstWidth = swapchainExtent_.width;
            pushConstants.dstHeight = swapchainExtent_.height;
            
            commandBuffer_.pushConstants(
                computePipelineLayout_,
                vk::ShaderStageFlagBits::eCompute,
                0, sizeof(PushConstants), &pushConstants
            );
            
            // Dispatch compute shader
            // Work groups: divideRoundUp(width, 16) x divideRoundUp(height, 16)
            uint32_t groupsX = (swapchainExtent_.width + 15) / 16;
            uint32_t groupsY = (swapchainExtent_.height + 15) / 16;
            commandBuffer_.dispatch(groupsX, groupsY, 1);
            
            // Barrier для синхронизации compute -> present
            barrier.oldLayout = vk::ImageLayout::eGeneral;
            barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
            barrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eNone;
            
            commandBuffer_.pipelineBarrier(
                vk::PipelineStageFlagBits::eComputeShader,
                vk::PipelineStageFlagBits::eBottomOfPipe,
                {}, 0, nullptr, 0, nullptr, 1, &barrier);
            
            commandBuffer_.end();
            
            // 5. Submit command buffer
            vk::SubmitInfo submitInfo{};
            vk::Semaphore waitSemaphores[] = {imageAvailableSemaphore_};
            vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer_;
            vk::Semaphore signalSemaphores[] = {renderFinishedSemaphore_};
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signalSemaphores;
            
            result = vkGraphicsQueue_.submit(1, &submitInfo, inFlightFence_);
            if (result != vk::Result::eSuccess) return;
            
            // 6. Present
            vk::PresentInfoKHR presentInfo{};
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = signalSemaphores;
            vk::SwapchainKHR swapchains[] = {vkSwapchain_};
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = swapchains;
            presentInfo.pImageIndices = &imageIndex;
            
            result = vkPresentQueue_.presentKHR(&presentInfo);
            
        } catch (const vk::SystemError& err) {
            // Ошибка презентации - не критично, просто пропускаем кадр
            static int errorCount = 0;
            if (++errorCount < 5) {
                std::cerr << "[Vulkan] Ошибка презентации: " << err.what() << std::endl;
            }
        }
#endif
    }

    void processInput() {
        if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window_, true);
        }

        float velocity = cameraSpeed_ * deltaTime_;

        if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) {
            cameraPos_ += velocity * cameraFront_;
        }
        if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) {
            cameraPos_ -= velocity * cameraFront_;
        }
        if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) {
            cameraPos_ -= glm::normalize(glm::cross(cameraFront_, cameraUp_)) * velocity;
        }
        if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) {
            cameraPos_ += glm::normalize(glm::cross(cameraFront_, cameraUp_)) * velocity;
        }
        if (glfwGetKey(window_, GLFW_KEY_SPACE) == GLFW_PRESS) {
            cameraPos_ += velocity * cameraUp_;
        }
        if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            cameraPos_ -= velocity * cameraUp_;
        }

        // Переключение настроек
        static bool fKeyPressed = false;
        if (glfwGetKey(window_, GLFW_KEY_F) == GLFW_PRESS && !fKeyPressed) {
            enableFoveation_ = !enableFoveation_;
            std::cout << "Фовеация: " << (enableFoveation_ ? "ВКЛ" : "ВЫКЛ") << std::endl;
            fKeyPressed = true;
        }
        if (glfwGetKey(window_, GLFW_KEY_F) == GLFW_RELEASE) {
            fKeyPressed = false;
        }

        static bool tKeyPressed = false;
        if (glfwGetKey(window_, GLFW_KEY_T) == GLFW_PRESS && !tKeyPressed) {
            enableTemporal_ = !enableTemporal_;
            if (!enableTemporal_ && temporalReprojection_) {
                temporalReprojection_->resetHistory();
            }
            std::cout << "Temporal Reprojection: " << (enableTemporal_ ? "ВКЛ" : "ВЫКЛ")
                      << std::endl;
            tKeyPressed = true;
        }
        if (glfwGetKey(window_, GLFW_KEY_T) == GLFW_RELEASE) {
            tKeyPressed = false;
        }

        static bool uKeyPressed = false;
        if (glfwGetKey(window_, GLFW_KEY_U) == GLFW_PRESS && !uKeyPressed) {
            enableUpscaling_ = !enableUpscaling_;
            std::cout << "Апскейлинг: " << (enableUpscaling_ ? "ВКЛ" : "ВЫКЛ") << std::endl;
            uKeyPressed = true;
        }
        if (glfwGetKey(window_, GLFW_KEY_U) == GLFW_RELEASE) {
            uKeyPressed = false;
        }
    }

    void updateCamera() {
        // Camera update is handled in mouse callback
    }

    void updateWindowTitle() {
        std::string title = "FreqVox Sponza Demo - FPS: " + std::to_string(static_cast<int>(fps_));
        title += " | Voxels: " + std::to_string(selectedVoxelCount_) + "/" +
                 std::to_string(voxels_.size());
        glfwSetWindowTitle(window_, title.c_str());
    }

    void printStatistics(float frameTotalTime) {
        std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
        std::cout << "📊 Статистика FreqVox Pipeline" << std::endl;
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
        std::cout << "FPS: " << static_cast<int>(fps_) << " (" << frameTotalTime << " ms)"
                  << std::endl;
        std::cout << "Камера: (" << cameraPos_.x << ", " << cameraPos_.y << ", " << cameraPos_.z
                  << ")" << std::endl;
        std::cout << "\nЭтапы рендеринга:" << std::endl;
        std::cout << "  1. Voxel Selection:     " << voxelSelectionTime_ << " ms ("
                  << selectedVoxelCount_ << "/" << voxels_.size() << " вокселей, "
                  << (float)selectedVoxelCount_ / voxels_.size() * 100.0f << "%)" << std::endl;
        std::cout << "  2. Frequency Shading:   " << frequencyShadingTime_ << " ms" << std::endl;
        std::cout << "  3. Temporal Reprojection: " << temporalTime_ << " ms" << std::endl;
        std::cout << "  4. Neural Upscaling:    " << upscalingTime_ << " ms" << std::endl;
        std::cout << "\nНастройки:" << std::endl;
        std::cout << "  Фовеация: " << (enableFoveation_ ? "✅ ВКЛ" : "❌ ВЫКЛ") << std::endl;
        std::cout << "  Temporal: " << (enableTemporal_ ? "✅ ВКЛ" : "❌ ВЫКЛ") << std::endl;
        std::cout << "  Upscaling: " << (enableUpscaling_ ? "✅ ВКЛ" : "❌ ВЫКЛ") << std::endl;
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    }

    static void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
        FreqVoxDemo* demo = static_cast<FreqVoxDemo*>(glfwGetWindowUserPointer(window));
        if (!demo) return;

        if (demo->firstMouse_) {
            demo->lastX_ = static_cast<float>(xpos);
            demo->lastY_ = static_cast<float>(ypos);
            demo->firstMouse_ = false;
        }

        float xoffset = static_cast<float>(xpos) - demo->lastX_;
        float yoffset = demo->lastY_ - static_cast<float>(ypos);
        demo->lastX_ = static_cast<float>(xpos);
        demo->lastY_ = static_cast<float>(ypos);

        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        demo->yaw_ += xoffset;
        demo->pitch_ += yoffset;

        if (demo->pitch_ > 89.0f) demo->pitch_ = 89.0f;
        if (demo->pitch_ < -89.0f) demo->pitch_ = -89.0f;

        glm::vec3 direction;
        direction.x = cos(glm::radians(demo->yaw_)) * cos(glm::radians(demo->pitch_));
        direction.y = sin(glm::radians(demo->pitch_));
        direction.z = sin(glm::radians(demo->yaw_)) * cos(glm::radians(demo->pitch_));
        demo->cameraFront_ = glm::normalize(direction);
    }

    static void scrollCallback(GLFWwindow* window, double /* xoffset */, double yoffset) {
        FreqVoxDemo* demo = static_cast<FreqVoxDemo*>(glfwGetWindowUserPointer(window));
        if (!demo) return;

        demo->cameraSpeed_ += static_cast<float>(yoffset) * 0.5f;
        demo->cameraSpeed_ = std::max(0.5f, std::min(demo->cameraSpeed_, 20.0f));
        std::cout << "Скорость камеры: " << demo->cameraSpeed_ << std::endl;
    }
};

// ============================================================================
// Main
// ============================================================================

int main() {
    Console::initialize();
    Console::setTitle("🎨 FreqVox Sponza Demo - Hardware-Aware Rendering");

    FreqVoxDemo demo;

    try {
        if (!demo.initialize()) {
            SAFE_ERROR("Ошибка инициализации демо");
            return -1;
        }

        demo.run();
        demo.shutdown();

    } catch (const std::exception& e) {
        std::cerr << "💥 Критическая ошибка: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}

