#include "HyperEngine/Rendering/RendererAdapter.h"
#include "HyperEngine/Rendering/Renderer3D.h"
#include "HyperEngine/Core/Console.h"
#include "HyperEngine/Core/SafeConsole.h"
#include <iostream>
#include <stdexcept>

// Условная компиляция для Vulkan
#ifdef ENGINE3D_ENABLE_VULKAN
// #include "HyperEngine/Vulkan/VulkanEngine.h"
// #include "HyperEngine/Vulkan/HardwareDetector.h"
// Пока не включаем, так как Vulkan компоненты не собираются с Engine3D
#endif

// Определение дефолтного backend на основе макросов компиляции
#ifdef ENGINE3D_DEFAULT_BACKEND_VULKAN
    #define DEFAULT_RENDER_BACKEND RenderBackend::VULKAN
#else
    #define DEFAULT_RENDER_BACKEND RenderBackend::OPENGL
#endif

using namespace HyperEngine::Rendering;
using namespace HyperEngine::Core;
using namespace HyperEngine::Math;

namespace HyperEngine {
namespace Rendering {

// ============================================================================
// RendererAdapter Implementation
// ============================================================================

RendererAdapter& RendererAdapter::getInstance() {
    static RendererAdapter instance;
    return instance;
}

bool RendererAdapter::setBackend(RenderBackend backend) {
    if (backend == currentBackend && currentAdapter && currentAdapter->isInitialized()) {
        std::cout << "[RendererAdapter] Backend уже установлен: " << getBackendName() << std::endl;
        return true;
    }
    
    // Очищаем текущий адаптер
    if (currentAdapter) {
        currentAdapter->cleanup();
        currentAdapter.reset();
    }
    
    // Создаем новый адаптер
    try {
        switch (backend) {
            case RenderBackend::OPENGL:
                if (!checkOpenGLAvailability()) {
                    SAFE_ERROR("[RendererAdapter] OpenGL недоступен");
                    return false;
                }
                currentAdapter = createOpenGLAdapter();
                break;
                
            case RenderBackend::VULKAN:
                if (!checkVulkanAvailability()) {
                    SAFE_ERROR("[RendererAdapter] Vulkan недоступен");
                    return false;
                }
                currentAdapter = createVulkanAdapter();
                break;
                
            case RenderBackend::AUTO:
                backend = selectOptimalBackend();
                return setBackend(backend);  // Рекурсивный вызов с конкретным backend
        }
        
        if (currentAdapter) {
            currentBackend = backend;
            std::cout << "[RendererAdapter] Переключен на backend: " << getBackendName() << std::endl;
            return true;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[RendererAdapter] Ошибка создания адаптера: " << e.what() << std::endl;
    }
    
    return false;
}

RenderBackend RendererAdapter::getCurrentBackend() const {
    return currentBackend;
}

bool RendererAdapter::isBackendAvailable(RenderBackend backend) const {
    switch (backend) {
        case RenderBackend::OPENGL:
            return checkOpenGLAvailability();
        case RenderBackend::VULKAN:
            return checkVulkanAvailability();
        case RenderBackend::AUTO:
            return true;  // AUTO всегда доступен
    }
    return false;
}

RenderBackend RendererAdapter::selectOptimalBackend() const {
    // Сначала пробуем дефолтный backend из настроек компиляции
    RenderBackend defaultBackend = DEFAULT_RENDER_BACKEND;
    
    if (defaultBackend == RenderBackend::VULKAN && checkVulkanAvailability()) {
        SAFE_PRINT_LINE("[RendererAdapter] Выбран дефолтный backend: Vulkan");
        return RenderBackend::VULKAN;
    }
    
    if (defaultBackend == RenderBackend::OPENGL && checkOpenGLAvailability()) {
        SAFE_PRINT_LINE("[RendererAdapter] Выбран дефолтный backend: OpenGL");
        return RenderBackend::OPENGL;
    }
    
    // Fallback: пробуем альтернативные backend'ы
    if (checkVulkanAvailability()) {
        SAFE_PRINT_LINE("[RendererAdapter] Fallback на Vulkan backend");
        return RenderBackend::VULKAN;
    }
    
    if (checkOpenGLAvailability()) {
        SAFE_PRINT_LINE("[RendererAdapter] Fallback на OpenGL backend");
        return RenderBackend::OPENGL;
    }
    
    throw std::runtime_error("Ни один графический API недоступен");
}

// Делегирование методов к текущему адаптеру
bool RendererAdapter::initialize(int width, int height) {
    if (!currentAdapter) {
        // Автоматически выбираем backend при первой инициализации
        if (!setBackend(RenderBackend::AUTO)) {
            return false;
        }
    }
    return currentAdapter->initialize(width, height);
}

void RendererAdapter::cleanup() {
    if (currentAdapter) {
        currentAdapter->cleanup();
    }
}

bool RendererAdapter::isInitialized() const {
    return currentAdapter && currentAdapter->isInitialized();
}

void RendererAdapter::beginFrame() {
    if (currentAdapter) currentAdapter->beginFrame();
}

void RendererAdapter::endFrame() {
    if (currentAdapter) currentAdapter->endFrame();
}

void RendererAdapter::clear() {
    if (currentAdapter) currentAdapter->clear();
}

void RendererAdapter::setClearColor(float r, float g, float b, float a) {
    if (currentAdapter) currentAdapter->setClearColor(r, g, b, a);
}

void RendererAdapter::setViewport(int x, int y, int width, int height) {
    if (currentAdapter) currentAdapter->setViewport(x, y, width, height);
}

void RendererAdapter::enableDepthTest(bool enable) {
    if (currentAdapter) currentAdapter->enableDepthTest(enable);
}

void RendererAdapter::enableBlending(bool enable) {
    if (currentAdapter) currentAdapter->enableBlending(enable);
}

void RendererAdapter::enableWireframe(bool enable) {
    if (currentAdapter) currentAdapter->enableWireframe(enable);
}

void RendererAdapter::enableBackfaceCulling(bool enable) {
    if (currentAdapter) currentAdapter->enableBackfaceCulling(enable);
}

void RendererAdapter::setMainCamera(std::shared_ptr<Camera3D> camera) {
    if (currentAdapter) currentAdapter->setMainCamera(camera);
}

std::shared_ptr<Camera3D> RendererAdapter::getMainCamera() const {
    return currentAdapter ? currentAdapter->getMainCamera() : nullptr;
}

void RendererAdapter::renderMesh(const Mesh3D& mesh, const Matrix4& transform, const Shader3D& shader) {
    if (currentAdapter) currentAdapter->renderMesh(mesh, transform, shader);
}

void RendererAdapter::renderMesh(std::shared_ptr<Mesh3D> mesh, const Matrix4& transform, std::shared_ptr<Shader3D> shader) {
    if (currentAdapter) currentAdapter->renderMesh(mesh, transform, shader);
}

void RendererAdapter::renderWireframe(const Mesh3D& mesh, const Matrix4& transform, const Shader3D& shader) {
    if (currentAdapter) currentAdapter->renderWireframe(mesh, transform, shader);
}

const char* RendererAdapter::getBackendName() const {
    return currentAdapter ? currentAdapter->getBackendName() : "None";
}

bool RendererAdapter::supportsFeature(const std::string& feature) const {
    return currentAdapter ? currentAdapter->supportsFeature(feature) : false;
}

void RendererAdapter::printBackendInfo() const {
    SAFE_PRINT_LINE("\n=== Информация о рендерере ===");
    std::cout << "Текущий backend: " << getBackendName() << std::endl;
    std::cout << "Инициализирован: " << (isInitialized() ? "Да" : "Нет") << std::endl;
    
    SAFE_PRINT_LINE("\nДоступные backend'ы:");
    for (auto backend : getAvailableBackends()) {
        const char* name = "";
        switch (backend) {
            case RenderBackend::OPENGL: name = "OpenGL"; break;
            case RenderBackend::VULKAN: name = "Vulkan"; break;
            case RenderBackend::AUTO: name = "AUTO"; break;
        }
        std::cout << "  - " << name << std::endl;
    }
    
    if (currentAdapter) {
        SAFE_PRINT_LINE("\nПоддерживаемые функции:");
        std::vector<std::string> features = {
            "ray_tracing", "gaussian_splatting", "compute_shaders", 
            "tessellation", "geometry_shaders", "multi_draw_indirect"
        };
        
        for (const auto& feature : features) {
            std::cout << "  - " << feature << ": " 
                      << (supportsFeature(feature) ? "Да" : "Нет") << std::endl;
        }
    }
    SAFE_PRINT_LINE("==============================\n");
}

std::vector<RenderBackend> RendererAdapter::getAvailableBackends() const {
    std::vector<RenderBackend> backends;
    
    if (checkOpenGLAvailability()) {
        backends.push_back(RenderBackend::OPENGL);
    }
    
    if (checkVulkanAvailability()) {
        backends.push_back(RenderBackend::VULKAN);
    }
    
    if (!backends.empty()) {
        backends.push_back(RenderBackend::AUTO);
    }
    
    return backends;
}

// Приватные методы создания адаптеров
std::unique_ptr<IRendererAdapter> RendererAdapter::createOpenGLAdapter() {
    return std::make_unique<OpenGLRendererAdapter>();
}

std::unique_ptr<IRendererAdapter> RendererAdapter::createVulkanAdapter() {
#ifdef ENGINE3D_ENABLE_VULKAN
    return std::make_unique<VulkanRendererAdapter>();
#else
    throw std::runtime_error("Vulkan поддержка не скомпилирована");
#endif
}

bool RendererAdapter::checkOpenGLAvailability() const {
    // OpenGL всегда доступен в текущей реализации
    return true;
}

bool RendererAdapter::checkVulkanAvailability() const {
#ifdef ENGINE3D_ENABLE_VULKAN
    // Пока используем простую проверку компиляции
    // В будущем здесь будет проверка через HardwareDetector
    return true;  // Предполагаем, что Vulkan доступен если скомпилирован
#else
    return false;
#endif
}

// ============================================================================
// OpenGLRendererAdapter Implementation
// ============================================================================

OpenGLRendererAdapter::OpenGLRendererAdapter() {
}

OpenGLRendererAdapter::~OpenGLRendererAdapter() {
    cleanup();
}

bool OpenGLRendererAdapter::initialize(int width, int height) {
    return Renderer3D::getInstance().initialize(width, height);
}

void OpenGLRendererAdapter::cleanup() {
    Renderer3D::getInstance().cleanup();
}

bool OpenGLRendererAdapter::isInitialized() const {
    return Renderer3D::getInstance().isInitialized();
}

void OpenGLRendererAdapter::beginFrame() {
    Renderer3D::getInstance().beginFrame();
}

void OpenGLRendererAdapter::endFrame() {
    Renderer3D::getInstance().endFrame();
}

void OpenGLRendererAdapter::clear() {
    Renderer3D::getInstance().clear();
}

void OpenGLRendererAdapter::setClearColor(float r, float g, float b, float a) {
    Renderer3D::getInstance().setClearColor(r, g, b, a);
}

void OpenGLRendererAdapter::setViewport(int x, int y, int width, int height) {
    Renderer3D::getInstance().setViewport(x, y, width, height);
}

void OpenGLRendererAdapter::enableDepthTest(bool enable) {
    Renderer3D::getInstance().enableDepthTest(enable);
}

void OpenGLRendererAdapter::enableBlending(bool enable) {
    Renderer3D::getInstance().enableBlending(enable);
}

void OpenGLRendererAdapter::enableWireframe(bool enable) {
    Renderer3D::getInstance().enableWireframe(enable);
}

void OpenGLRendererAdapter::enableBackfaceCulling(bool enable) {
    Renderer3D::getInstance().enableBackfaceCulling(enable);
}

void OpenGLRendererAdapter::setMainCamera(std::shared_ptr<Camera3D> camera) {
    Renderer3D::getInstance().setMainCamera(camera);
}

std::shared_ptr<Camera3D> OpenGLRendererAdapter::getMainCamera() const {
    return Renderer3D::getInstance().getMainCamera();
}

void OpenGLRendererAdapter::renderMesh(const Mesh3D& mesh, const Matrix4& transform, const Shader3D& shader) {
    Renderer3D::getInstance().renderMesh(mesh, transform, shader);
}

void OpenGLRendererAdapter::renderMesh(std::shared_ptr<Mesh3D> mesh, const Matrix4& transform, std::shared_ptr<Shader3D> shader) {
    Renderer3D::getInstance().renderMesh(mesh, transform, shader);
}

void OpenGLRendererAdapter::renderWireframe(const Mesh3D& mesh, const Matrix4& transform, const Shader3D& shader) {
    Renderer3D::getInstance().renderWireframe(mesh, transform, shader);
}

bool OpenGLRendererAdapter::supportsFeature(const std::string& feature) const {
    // OpenGL поддерживает базовые функции
    if (feature == "basic_rendering" || feature == "wireframe" || 
        feature == "depth_test" || feature == "blending") {
        return true;
    }
    
    // Продвинутые функции не поддерживаются в базовой OpenGL реализации
    if (feature == "ray_tracing" || feature == "gaussian_splatting" || 
        feature == "ai_denoising" || feature == "dlss" || feature == "fsr") {
        return false;
    }
    
    // Некоторые функции могут поддерживаться в зависимости от расширений
    if (feature == "compute_shaders" || feature == "tessellation" || 
        feature == "geometry_shaders") {
        return false;  // Пока не реализовано
    }
    
    return false;
}

// ============================================================================
// VulkanRendererAdapter Implementation
// ============================================================================

VulkanRendererAdapter::VulkanRendererAdapter() 
    : vulkanEngine(nullptr) {
}

VulkanRendererAdapter::~VulkanRendererAdapter() {
    cleanup();
}

bool VulkanRendererAdapter::initialize(int width, int height) {
#ifdef ENGINE3D_ENABLE_VULKAN
    try {
        // TODO: Создание и инициализация VulkanEngine
        // vulkanEngine = new HyperEngine::Vulkan::VulkanEngine();
        // return vulkanEngine->init(instance);
        
        // Пока заглушка
        settings.viewport[2] = width;
        settings.viewport[3] = height;
        
        // Создаем камеру по умолчанию
        mainCamera = std::make_shared<Camera3D>();
        mainCamera->setPerspective(60.0f, (float)width / height, 0.1f, 1000.0f);
        mainCamera->setPosition(Vector3(0, 0, 5));
        mainCamera->lookAt(Vector3(0, 0, 5), Vector3(0, 0, 0), Vector3(0, 1, 0));
        
        initialized = true;
        SAFE_PRINT_LINE("[VulkanRendererAdapter] Инициализация завершена (заглушка)");
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[VulkanRendererAdapter] Ошибка инициализации: " << e.what() << std::endl;
        return false;
    }
#else
    SAFE_ERROR("[VulkanRendererAdapter] Vulkan поддержка не скомпилирована");
    return false;
#endif
}

void VulkanRendererAdapter::cleanup() {
#ifdef ENGINE3D_ENABLE_VULKAN
    if (vulkanEngine) {
        // vulkanEngine->shutdown();
        // delete vulkanEngine;
        vulkanEngine = nullptr;
    }
#endif
    initialized = false;
    mainCamera.reset();
}

bool VulkanRendererAdapter::isInitialized() const {
    return initialized;
}

void VulkanRendererAdapter::beginFrame() {
    if (!initialized) return;
    
    // TODO: Вызов vulkanEngine->beginFrame()
    SAFE_PRINT_LINE("[VulkanRendererAdapter] Begin frame (заглушка)");
}

void VulkanRendererAdapter::endFrame() {
    if (!initialized) return;
    
    // TODO: Вызов vulkanEngine->endFrame()
    SAFE_PRINT_LINE("[VulkanRendererAdapter] End frame (заглушка)");
}

void VulkanRendererAdapter::clear() {
    if (!initialized) return;
    
    // TODO: Очистка Vulkan framebuffer
    SAFE_PRINT_LINE("[VulkanRendererAdapter] Clear (заглушка)");
}

void VulkanRendererAdapter::setClearColor(float r, float g, float b, float a) {
    settings.clearColor[0] = r;
    settings.clearColor[1] = g;
    settings.clearColor[2] = b;
    settings.clearColor[3] = a;
}

void VulkanRendererAdapter::setViewport(int x, int y, int width, int height) {
    settings.viewport[0] = x;
    settings.viewport[1] = y;
    settings.viewport[2] = width;
    settings.viewport[3] = height;
}

void VulkanRendererAdapter::enableDepthTest(bool enable) {
    settings.depthTest = enable;
}

void VulkanRendererAdapter::enableBlending(bool enable) {
    settings.blending = enable;
}

void VulkanRendererAdapter::enableWireframe(bool enable) {
    settings.wireframe = enable;
}

void VulkanRendererAdapter::enableBackfaceCulling(bool enable) {
    settings.backfaceCulling = enable;
}

void VulkanRendererAdapter::setMainCamera(std::shared_ptr<Camera3D> camera) {
    mainCamera = camera;
    if (camera && settings.viewport[2] > 0 && settings.viewport[3] > 0) {
        camera->setAspectRatio((float)settings.viewport[2] / settings.viewport[3]);
    }
}

std::shared_ptr<Camera3D> VulkanRendererAdapter::getMainCamera() const {
    return mainCamera;
}

void VulkanRendererAdapter::renderMesh(const Mesh3D& mesh, const Matrix4& transform, const Shader3D& shader) {
    if (!initialized) return;
    
    // TODO: Рендеринг через Vulkan
    std::cout << "[VulkanRendererAdapter] Render mesh with " << mesh.getTriangleCount() 
              << " triangles (заглушка)" << std::endl;
}

void VulkanRendererAdapter::renderMesh(std::shared_ptr<Mesh3D> mesh, const Matrix4& transform, std::shared_ptr<Shader3D> shader) {
    if (mesh && shader) {
        renderMesh(*mesh, transform, *shader);
    }
}

void VulkanRendererAdapter::renderWireframe(const Mesh3D& mesh, const Matrix4& transform, const Shader3D& shader) {
    if (!initialized) return;
    
    bool wasWireframe = settings.wireframe;
    settings.wireframe = true;
    
    renderMesh(mesh, transform, shader);
    
    settings.wireframe = wasWireframe;
}

bool VulkanRendererAdapter::supportsFeature(const std::string& feature) const {
    // Vulkan поддерживает все современные функции
    if (feature == "basic_rendering" || feature == "wireframe" || 
        feature == "depth_test" || feature == "blending" ||
        feature == "compute_shaders" || feature == "tessellation" || 
        feature == "geometry_shaders" || feature == "multi_draw_indirect") {
        return true;
    }
    
    // Продвинутые функции (будут реализованы в следующих этапах)
    if (feature == "ray_tracing" || feature == "gaussian_splatting" || 
        feature == "ai_denoising" || feature == "dlss" || feature == "fsr") {
        return true;  // Планируется поддержка
    }
    
    return false;
}

} // namespace Rendering
} // namespace HyperEngine

