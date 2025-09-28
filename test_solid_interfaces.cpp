/**
 * @brief Тестовый файл для проверки компиляции SOLID интерфейсов
 * 
 * Этот файл проверяет, что все созданные интерфейсы и классы
 * правильно скомпилируются и не имеют циклических зависимостей.
 */

// Базовые интерфейсы
#include "HyperEngine/Rendering/Common/IRenderer.h"
#include "HyperEngine/Rendering/Common/IResourceManager.h"

// Render stages
#include "HyperEngine/Rendering/RenderStages/IRenderStage.h"
#include "HyperEngine/Rendering/RenderStages/PrimaryRasterizationStage.h"
#include "HyperEngine/Rendering/RenderStages/UpscalingStage.h"

// Upscaling strategies
#include "HyperEngine/Rendering/Upscaling/IUpscalingStrategy.h"
#include "HyperEngine/Rendering/Upscaling/DLSSUpscalingStrategy.h"

// Factory и DI
#include "HyperEngine/Rendering/RendererFactory.h"
#include "HyperEngine/Core/DependencyInjection/Container.h"

// Vulkan реализация
#include "HyperEngine/Rendering/Vulkan/VulkanRenderer.h"

using namespace HyperEngine;
using namespace HyperEngine::Rendering;
using namespace HyperEngine::Core::DI;

/**
 * @brief Тестирование основных интерфейсов
 */
void testBasicInterfaces() {
    // Проверка создания конфигурации рендерера
    RendererConfig config;
    config.width = 1920;
    config.height = 1080;
    config.vsync = true;
    
    // Проверка данных кадра
    FrameData frameData;
    frameData.camera.position = Math::Vector3(0, 0, 5);
    frameData.camera.target = Math::Vector3(0, 0, 0);
    frameData.renderTargetSize.width = 1920;
    frameData.renderTargetSize.height = 1080;
    
    // Проверка статистики
    RenderingStats stats;
    stats.frameTime = 16.7f;
    stats.fps = 60.0f;
    stats.drawCalls = 100;
}

/**
 * @brief Тестирование Factory Pattern
 */
void testFactoryPattern() {
    // Проверка типов рендереров
    RendererType vulkanType = RendererType::Vulkan;
    RendererType openglType = RendererType::OpenGL;
    
    // Проверка функций рендеринга
    RenderingFeature rayTracing = RenderingFeature::RayTracing;
    RenderingFeature upscaling = RenderingFeature::AIUpscaling;
    
    // Проверка создания рендерера через фабрику
    RendererConfig config;
    auto supportedRenderers = RendererFactory::getSupportedRenderers();
    auto recommendedType = RendererFactory::getRecommendedRenderer();
    
    // Проверка информации о рендерере
    auto rendererInfo = RendererFactory::getRendererInfo(RendererType::Vulkan);
}

/**
 * @brief Тестирование Strategy Pattern для upscaling
 */
void testUpscalingStrategy() {
    // Проверка типов upscaling
    Upscaling::UpscalingType dlssType = Upscaling::UpscalingType::DLSS3;
    Upscaling::UpscalingType fsrType = Upscaling::UpscalingType::FSR3;
    
    // Проверка режимов качества
    Upscaling::UpscalingQuality quality = Upscaling::UpscalingQuality::Quality;
    Upscaling::UpscalingQuality performance = Upscaling::UpscalingQuality::Performance;
    
    // Проверка конфигурации upscaling
    Upscaling::UpscalingConfig config;
    config.outputWidth = 1920;
    config.outputHeight = 1080;
    config.quality = Upscaling::UpscalingQuality::Quality;
    config.enableFrameGeneration = false;
    
    // Проверка параметров upscaling
    Upscaling::UpscalingParams params;
    params.inputColor = INVALID_HANDLE;
    params.inputDepth = INVALID_HANDLE;
    params.motionVectors = INVALID_HANDLE;
    params.outputTexture = INVALID_HANDLE;
    
    // Проверка результата upscaling
    Upscaling::UpscalingResult result;
    result.success = true;
    result.executionTime = 2.5f;
    result.actualUpscaleFactor = 2.0f;
}

/**
 * @brief Тестирование Dependency Injection
 */
void testDependencyInjection() {
    // Создание DI контейнера
    Container container;
    
    // Проверка lifetime'ов
    Container::Lifetime singleton = Container::Lifetime::Singleton;
    Container::Lifetime transient = Container::Lifetime::Transient;
    Container::Lifetime scoped = Container::Lifetime::Scoped;
    
    // Проверка scope guard
    {
        ScopeGuard scope(container, "test_scope");
        // scope автоматически завершится при выходе из блока
    }
    
    // Проверка статистики контейнера
    auto stats = container.getStats();
    auto registeredTypes = container.getRegisteredTypes();
}

/**
 * @brief Тестирование render stages
 */
void testRenderStages() {
    // Проверка создания render context
    RenderContext context;
    context.camera.cameraPosition = Math::Vector3(0, 0, 5);
    context.camera.cameraTarget = Math::Vector3(0, 0, 0);
    context.viewport.width = 1920;
    context.viewport.height = 1080;
    context.timing.deltaTime = 0.016f;
    context.timing.frameNumber = 1000;
    
    // Проверка буферов рендеринга
    context.primaryRaster.primaryColorBuffer = INVALID_HANDLE;
    context.primaryRaster.primaryDepthBuffer = INVALID_HANDLE;
    context.primaryRaster.gaussianCount = 50000;
    
    context.rayTracing.reflections = INVALID_HANDLE;
    context.rayTracing.shadows = INVALID_HANDLE;
    context.rayTracing.globalIllumination = INVALID_HANDLE;
    
    context.upscaling.upscaledImage = INVALID_HANDLE;
    context.upscaling.upscaleFactor = 2.0f;
    context.upscaling.outputWidth = 1920;
    context.upscaling.outputHeight = 1080;
    
    // Проверка настроек рендеринга
    context.settings.enableRayTracing = true;
    context.settings.enableDenoising = true;
    context.settings.enableUpscaling = true;
    context.settings.enableDebugOverlay = false;
}

/**
 * @brief Тестирование Vulkan рендерера
 */
void testVulkanRenderer() {
    // Проверка создания без ресурс менеджера (автоматически создастся)
    Vulkan::VulkanRenderer renderer;
    
    // Проверка получения информации о устройстве
    Vulkan::VulkanRenderer::VulkanDeviceInfo deviceInfo;
    deviceInfo.deviceName = "Test Device";
    deviceInfo.driverVersion = "1.0.0";
    deviceInfo.vendorId = 0x10DE; // NVIDIA
    deviceInfo.totalMemory = 8 * 1024 * 1024 * 1024; // 8GB
    deviceInfo.supportsRayTracing = true;
    deviceInfo.supportsComputeShaders = true;
}

/**
 * @brief Тестирование resource manager интерфейса
 */
void testResourceManager() {
    // Проверка дескрипторов ресурсов
    ResourceHandle handle = INVALID_HANDLE;
    BufferHandle bufferHandle = INVALID_HANDLE;
    TextureHandle textureHandle = INVALID_HANDLE;
    ShaderHandle shaderHandle = INVALID_HANDLE;
    
    // Проверка описания буфера
    BufferDesc bufferDesc;
    bufferDesc.size = 1024;
    bufferDesc.usage = BufferUsage::Vertex;
    bufferDesc.memoryProperties = MemoryProperty::DeviceLocal;
    bufferDesc.initialData = nullptr;
    bufferDesc.debugName = "Test Buffer";
    
    // Проверка описания текстуры
    TextureDesc textureDesc;
    textureDesc.width = 1920;
    textureDesc.height = 1080;
    textureDesc.depth = 1;
    textureDesc.mipLevels = 1;
    textureDesc.arrayLayers = 1;
    textureDesc.format = TextureFormat::RGBA8_UNORM;
    textureDesc.type = TextureType::Texture2D;
    textureDesc.usage = TextureUsage::ColorAttachment;
    textureDesc.samples = 1;
    textureDesc.debugName = "Test Texture";
    
    // Проверка типов шейдеров
    ShaderType vertexShader = ShaderType::Vertex;
    ShaderType fragmentShader = ShaderType::Fragment;
    ShaderType computeShader = ShaderType::Compute;
    ShaderType rayGenShader = ShaderType::RayGeneration;
    
    // Проверка статистики памяти
    MemoryStats memStats;
    memStats.totalMemory = 8 * 1024 * 1024 * 1024;
    memStats.usedMemory = 2 * 1024 * 1024 * 1024;
    memStats.freeMemory = 6 * 1024 * 1024 * 1024;
    memStats.bufferCount = 100;
    memStats.textureCount = 50;
    memStats.shaderCount = 25;
    
    float usagePercentage = memStats.getUsagePercentage();
    (void)usagePercentage; // Подавление предупреждения о неиспользуемой переменной
}

/**
 * @brief Главная функция для тестирования компиляции
 */
int main() {
    // Проверка, что все интерфейсы компилируются
    testBasicInterfaces();
    testFactoryPattern();
    testUpscalingStrategy();
    testDependencyInjection();
    testRenderStages();
    testVulkanRenderer();
    testResourceManager();
    
    return 0;
}
