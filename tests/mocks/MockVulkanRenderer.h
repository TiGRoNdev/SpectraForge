#pragma once
#include <gmock/gmock.h>
#include "Engine3D/Vulkan/VulkanRenderer.h"

namespace HyperEngine::Testing::Mocks {

using namespace Engine3D::Vulkan;

/**
 * @brief Mock объект для VulkanRenderer
 * 
 * Используется для тестирования гибридного rendering pipeline
 * без необходимости инициализации реального Vulkan контекста
 */
class MockVulkanRenderer {
public:
    // Основные методы жизненного цикла
    MOCK_METHOD(bool, init, (vk::Device device, ResourceManager* resourceManager), ());
    MOCK_METHOD(void, shutdown, (), ());
    MOCK_METHOD(bool, isInitialized, (), (const));
    
    // Этапы рендеринга согласно UML архитектуре
    MOCK_METHOD(PrimaryImage, rasterizePrimary, (const Gaussians& gaussians), ());
    MOCK_METHOD(RawEffects, rayTraceSecondary, (const PrimaryImage& image), ());
    MOCK_METHOD(DenoisedImage, denoiseAI, (const RawEffects& effects), ());
    MOCK_METHOD(FinalImage, upscale, (const DenoisedImage& image, const ResolutionTarget& target), ());
    MOCK_METHOD(void, presentFinalImage, (), ());
};

/**
 * @brief Mock объект для ResourceManager
 */
class MockResourceManager {
public:
    MOCK_METHOD(bool, initialize, (vk::Device device, vk::PhysicalDevice physicalDevice), ());
    MOCK_METHOD(void, cleanup, (), ());
    
    // Управление буферами
    MOCK_METHOD(vk::Buffer, createBuffer, 
                (vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties), ());
    MOCK_METHOD(void, destroyBuffer, (vk::Buffer buffer, vk::DeviceMemory memory), ());
    
    // Управление изображениями
    MOCK_METHOD(vk::Image, createImage, 
                (uint32_t width, uint32_t height, vk::Format format, vk::ImageUsageFlags usage), ());
    MOCK_METHOD(void, destroyImage, (vk::Image image, vk::DeviceMemory memory), ());
    
    // Интероперабельность CUDA-Vulkan
    MOCK_METHOD(void*, mapCudaMemory, (vk::DeviceMemory memory), ());
    MOCK_METHOD(void, unmapCudaMemory, (void* cudaPtr), ());
    
    // Статистика
    MOCK_METHOD(size_t, getAllocatedMemory, (), (const));
    MOCK_METHOD(size_t, getAvailableMemory, (), (const));
};

/**
 * @brief Mock объект для FlashGSSplatter (CUDA компонент)
 */
class MockFlashGSSplatter {
public:
    MOCK_METHOD(bool, initialize, (void* cudaContext), ());
    MOCK_METHOD(void, shutdown, (), ());
    
    // Оптимизация гауссианов
    MOCK_METHOD(void, optimizeGaussians, (const struct MultiViewImages& images), ());
    
    // Рендеринг
    MOCK_METHOD(PrimaryImage, rasterizeGaussians, (const struct CameraParams& params), ());
    
    // Настройки производительности
    MOCK_METHOD(void, setQualityLevel, (int level), ());
    MOCK_METHOD(void, enableTileOptimization, (bool enable), ());
};

/**
 * @brief Mock объект для OptiXRayTracer
 */
class MockOptiXRayTracer {
public:
    MOCK_METHOD(bool, init, (void* cudaContext), ());
    MOCK_METHOD(void, shutdown, (), ());
    
    // Построение acceleration structures
    MOCK_METHOD(void, buildAccelerationStructures, (const struct SceneGeometry& geometry), ());
    
    // Трассировка лучей
    MOCK_METHOD(RawEffects, traceRays, (const struct LaunchParams& params), ());
    
    // Shader Execution Reordering
    MOCK_METHOD(void, applySER, (const struct CoherencyHints& hints), ());
    
    // Настройки
    MOCK_METHOD(void, setMaxTraceDepth, (uint32_t depth), ());
};

/**
 * @brief Mock объект для DenoiseModule
 */
class MockDenoiseModule {
public:
    MOCK_METHOD(bool, initialize, (), ());
    MOCK_METHOD(void, shutdown, (), ());
    
    // AI деноизинг
    MOCK_METHOD(DenoisedImage, denoise, 
                (const RawEffects& effects, const struct AuxBuffers& buffers), ());
    
    // Настройки деноизинга
    MOCK_METHOD(void, setDenoiseStrength, (float strength), ());
    MOCK_METHOD(void, enableTemporalAccumulation, (bool enable), ());
};

/**
 * @brief Mock объект для Upscaler
 */
class MockUpscaler {
public:
    MOCK_METHOD(bool, initialize, (const struct HardwareConfig& config), ());
    MOCK_METHOD(void, shutdown, (), ());
    
    // Upscaling
    MOCK_METHOD(FinalImage, upscaleImage, 
                (const DenoisedImage& image, const ResolutionTarget& target), ());
    
    // Получение типа upscaler
    MOCK_METHOD(std::string, getUpscalerType, (), (const));
    MOCK_METHOD(bool, isSupported, (), (const));
};

/**
 * @brief Фабрика для создания mock объектов Vulkan компонентов
 */
class VulkanMockFactory {
public:
    /**
     * @brief Создает полностью инициализированный mock VulkanRenderer
     */
    static std::unique_ptr<MockVulkanRenderer> createInitializedRenderer() {
        auto mock = std::make_unique<MockVulkanRenderer>();
        
        // Настройка успешной инициализации
        EXPECT_CALL(*mock, init(testing::_, testing::_))
            .WillOnce(testing::Return(true));
        EXPECT_CALL(*mock, isInitialized())
            .WillRepeatedly(testing::Return(true));
        
        // Настройка базового rendering pipeline
        setupBasicRenderingExpectations(*mock);
        
        return mock;
    }
    
    /**
     * @brief Создает mock VulkanRenderer с отказом инициализации
     */
    static std::unique_ptr<MockVulkanRenderer> createFailingRenderer() {
        auto mock = std::make_unique<MockVulkanRenderer>();
        
        EXPECT_CALL(*mock, init(testing::_, testing::_))
            .WillRepeatedly(testing::Return(false));
        EXPECT_CALL(*mock, isInitialized())
            .WillRepeatedly(testing::Return(false));
            
        return mock;
    }
    
    /**
     * @brief Создает mock ResourceManager
     */
    static std::unique_ptr<MockResourceManager> createResourceManager() {
        auto mock = std::make_unique<MockResourceManager>();
        
        EXPECT_CALL(*mock, initialize(testing::_, testing::_))
            .WillRepeatedly(testing::Return(true));
        EXPECT_CALL(*mock, getAllocatedMemory())
            .WillRepeatedly(testing::Return(0));
        EXPECT_CALL(*mock, getAvailableMemory())
            .WillRepeatedly(testing::Return(1024 * 1024 * 1024)); // 1GB
            
        return mock;
    }
    
    /**
     * @brief Создает mock FlashGSSplatter
     */
    static std::unique_ptr<MockFlashGSSplatter> createFlashGSSplatter() {
        auto mock = std::make_unique<MockFlashGSSplatter>();
        
        EXPECT_CALL(*mock, initialize(testing::_))
            .WillRepeatedly(testing::Return(true));
        
        // Настройка ожиданий для primary rasterization
        PrimaryImage mockImage{};
        mockImage.width = 1920;
        mockImage.height = 1080;
        
        EXPECT_CALL(*mock, rasterizeGaussians(testing::_))
            .WillRepeatedly(testing::Return(mockImage));
            
        return mock;
    }
    
    /**
     * @brief Создает mock OptiXRayTracer
     */
    static std::unique_ptr<MockOptiXRayTracer> createOptiXRayTracer() {
        auto mock = std::make_unique<MockOptiXRayTracer>();
        
        EXPECT_CALL(*mock, init(testing::_))
            .WillRepeatedly(testing::Return(true));
        
        // Настройка ожиданий для ray tracing
        RawEffects mockEffects{};
        EXPECT_CALL(*mock, traceRays(testing::_))
            .WillRepeatedly(testing::Return(mockEffects));
            
        return mock;
    }

private:
    /**
     * @brief Настраивает базовые ожидания для rendering pipeline
     */
    static void setupBasicRenderingExpectations(MockVulkanRenderer& mock) {
        // Primary rasterization
        PrimaryImage mockPrimaryImage{};
        mockPrimaryImage.width = 1920;
        mockPrimaryImage.height = 1080;
        
        EXPECT_CALL(mock, rasterizePrimary(testing::_))
            .WillRepeatedly(testing::Return(mockPrimaryImage));
        
        // Secondary ray tracing
        RawEffects mockEffects{};
        EXPECT_CALL(mock, rayTraceSecondary(testing::_))
            .WillRepeatedly(testing::Return(mockEffects));
        
        // AI denoising
        DenoisedImage mockDenoised{};
        EXPECT_CALL(mock, denoiseAI(testing::_))
            .WillRepeatedly(testing::Return(mockDenoised));
        
        // Upscaling
        FinalImage mockFinal{};
        EXPECT_CALL(mock, upscale(testing::_, testing::_))
            .WillRepeatedly(testing::Return(mockFinal));
        
        // Презентация
        EXPECT_CALL(mock, presentFinalImage())
            .Times(testing::AnyNumber());
        
        // Завершение работы
        EXPECT_CALL(mock, shutdown())
            .Times(testing::AtMost(1));
    }
};

} // namespace HyperEngine::Testing::Mocks
