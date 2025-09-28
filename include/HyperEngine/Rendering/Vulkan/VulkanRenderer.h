#pragma once

#include "HyperEngine/Rendering/Common/IRenderer.h"
#include "HyperEngine/Rendering/Common/IResourceManager.h"
#include "HyperEngine/Rendering/RenderStages/IRenderStage.h"
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include <unordered_map>

namespace HyperEngine::Rendering::Vulkan {

// Forward declarations
class RenderPipeline;
class VulkanResourceManager;

/**
 * @brief Vulkan реализация рендерера
 * 
 * Применяет Single Responsibility Principle - отвечает ТОЛЬКО 
 * за координацию этапов рендеринга и управление жизненным циклом.
 * 
 * Конкретные задачи (rasterization, ray tracing, деноизинг) 
 * делегированы специализированным классам через RenderPipeline.
 */
class VulkanRenderer : public IRenderer {
private:
    std::unique_ptr<RenderPipeline> renderPipeline_;
    std::shared_ptr<IResourceManager> resourceManager_;
    
    // Состояние рендерера
    bool initialized_ = false;
    RendererConfig config_;
    std::string deviceName_;
    std::string apiVersion_;
    
    // Статистика производительности
    mutable RenderingStats stats_;
    std::chrono::high_resolution_clock::time_point lastFrameTime_;
    std::vector<float> frameTimeHistory_;
    static constexpr size_t FRAME_HISTORY_SIZE = 60; // 1 секунда при 60 FPS

public:
    /**
     * @brief Конструктор с внедрением зависимостей
     * @param resourceManager Менеджер ресурсов (может быть nullptr - создастся автоматически)
     */
    explicit VulkanRenderer(std::shared_ptr<IResourceManager> resourceManager = nullptr);
    
    /**
     * @brief Деструктор
     */
    ~VulkanRenderer() override;
    
    // Запретить копирование
    VulkanRenderer(const VulkanRenderer&) = delete;
    VulkanRenderer& operator=(const VulkanRenderer&) = delete;
    
    // Разрешить перемещение
    VulkanRenderer(VulkanRenderer&&) noexcept;
    VulkanRenderer& operator=(VulkanRenderer&&) noexcept;

    // Реализация IRenderer
    bool initialize() override;
    void renderFrame(const FrameData& frameData) override;
    void shutdown() override;
    RendererType getType() const override { return RendererType::Vulkan; }
    bool supportsFeature(RenderingFeature feature) const override;
    std::string getName() const override;
    std::string getApiVersion() const override;
    bool isReady() const override;
    RenderingStats getStats() const override;
    
    // VulkanRenderer специфичные методы
    
    /**
     * @brief Установить конфигурацию рендерера
     * @param config Новая конфигурация
     * @return true если конфигурация применена успешно
     */
    bool setConfig(const RendererConfig& config);
    
    /**
     * @brief Получить текущую конфигурацию
     * @return Копия текущей конфигурации
     */
    RendererConfig getConfig() const { return config_; }
    
    /**
     * @brief Получить менеджер ресурсов
     * @return Умный указатель на менеджер ресурсов
     */
    std::shared_ptr<IResourceManager> getResourceManager() const { return resourceManager_; }
    
    /**
     * @brief Получить render pipeline
     * @return Указатель на pipeline (для настройки этапов)
     */
    RenderPipeline* getRenderPipeline() const { return renderPipeline_.get(); }
    
    /**
     * @brief Пересоздать swapchain (при изменении размера окна)
     * @param newWidth Новая ширина
     * @param newHeight Новая высота
     * @return true если успешно
     */
    bool recreateSwapchain(int newWidth, int newHeight);
    
    /**
     * @brief Дождаться завершения всех операций GPU
     */
    void waitForIdle();
    
    /**
     * @brief Получить подробную информацию о GPU
     * @return Структура с информацией об устройстве
     */
    struct VulkanDeviceInfo {
        std::string deviceName;
        std::string driverVersion;
        uint32_t vendorId;
        uint32_t deviceId;
        size_t totalMemory;
        size_t availableMemory;
        bool supportsRayTracing;
        bool supportsComputeShaders;
        bool supportsMeshShaders;
        std::vector<std::string> supportedExtensions;
        struct {
            uint32_t queueFamilyIndex;
            uint32_t queueCount;
        } graphicsQueue, computeQueue, transferQueue;
    };
    
    VulkanDeviceInfo getDeviceInfo() const;
    
    /**
     * @brief Сделать скриншот текущего кадра
     * @param filePath Путь для сохранения
     * @return true если скриншот сохранен успешно
     */
    bool takeScreenshot(const std::string& filePath) const;
    
    /**
     * @brief Включить/выключить профилирование GPU
     * @param enable Включить профилирование
     */
    void setGPUProfiling(bool enable);
    
    /**
     * @brief Получить результаты профилирования GPU
     * @return Карта имен этапов и времени их выполнения в мс
     */
    std::unordered_map<std::string, float> getGPUProfilingResults() const;

private:
    /**
     * @brief Инициализация Vulkan API
     * @return true если успешно
     */
    bool initializeVulkan();
    
    /**
     * @brief Создание Vulkan устройства
     * @return true если успешно
     */
    bool createDevice();
    
    /**
     * @brief Создание swapchain
     * @return true если успешно
     */
    bool createSwapchain();
    
    /**
     * @brief Создание render pipeline с этапами
     * @return true если успешно
     */
    bool createRenderPipeline();
    
    /**
     * @brief Обновление статистики производительности
     * @param frameStartTime Время начала кадра
     */
    void updatePerformanceStats(std::chrono::high_resolution_clock::time_point frameStartTime);
    
    /**
     * @brief Логирование состояния рендерера
     * @param message Сообщение для лога
     */
    void logRendererState(const std::string& message) const;
    
    /**
     * @brief Проверка поддержки необходимых расширений Vulkan
     * @return true если все расширения поддерживаются
     */
    bool checkVulkanExtensionSupport() const;
    
    /**
     * @brief Выбор оптимального формата swapchain
     * @return Выбранный формат
     */
    uint32_t chooseSwapchainFormat() const;
    
    /**
     * @brief Выбор оптимального present mode
     * @return Выбранный present mode
     */
    uint32_t chooseSwapchainPresentMode() const;
};

/**
 * @brief Конвейер рендеринга Vulkan
 * 
 * Применяет Chain of Responsibility pattern - управляет 
 * последовательностью этапов рендеринга.
 * 
 * Каждый этап имеет единственную ответственность и может
 * быть легко заменен или переконфигурирован.
 */
class RenderPipeline {
private:
    std::vector<std::unique_ptr<IRenderStage>> stages_;
    std::shared_ptr<IResourceManager> resourceManager_;
    
    // Состояние pipeline
    bool initialized_ = false;
    RenderContext currentContext_;
    
    // Профилирование
    bool profilingEnabled_ = false;
    std::unordered_map<std::string, float> stageTimings_;

public:
    /**
     * @brief Конструктор pipeline
     * @param resourceManager Менеджер ресурсов
     */
    explicit RenderPipeline(std::shared_ptr<IResourceManager> resourceManager);
    
    /**
     * @brief Деструктор
     */
    ~RenderPipeline();
    
    // Запретить копирование и перемещение
    RenderPipeline(const RenderPipeline&) = delete;
    RenderPipeline& operator=(const RenderPipeline&) = delete;
    RenderPipeline(RenderPipeline&&) = delete;
    RenderPipeline& operator=(RenderPipeline&&) = delete;
    
    /**
     * @brief Инициализация pipeline
     * @return true если успешно
     */
    bool initialize();
    
    /**
     * @brief Выполнение всех этапов рендеринга
     * @param frameData Данные кадра
     */
    void execute(const FrameData& frameData);
    
    /**
     * @brief Завершение работы pipeline
     */
    void shutdown();
    
    /**
     * @brief Проверка поддержки функции
     * @param feature Функция для проверки
     * @return true если поддерживается хотя бы одним этапом
     */
    bool supportsFeature(RenderingFeature feature) const;
    
    /**
     * @brief Добавить этап в конвейер
     * @param stage Этап для добавления
     * @param insertBefore Имя этапа, перед которым вставить (пустая строка = в конец)
     */
    void addStage(std::unique_ptr<IRenderStage> stage, const std::string& insertBefore = "");
    
    /**
     * @brief Удалить этап из конвейера
     * @param stageName Имя этапа для удаления
     * @return true если этап был найден и удален
     */
    bool removeStage(const std::string& stageName);
    
    /**
     * @brief Получить этап по имени
     * @param stageName Имя этапа
     * @return Указатель на этап или nullptr если не найден
     */
    IRenderStage* getStage(const std::string& stageName) const;
    
    /**
     * @brief Получить список всех этапов
     * @return Вектор имен этапов в порядке выполнения
     */
    std::vector<std::string> getStageNames() const;
    
    /**
     * @brief Включить/выключить этап
     * @param stageName Имя этапа
     * @param enabled Включен ли этап
     */
    void setStageEnabled(const std::string& stageName, bool enabled);
    
    /**
     * @brief Проверить, включен ли этап
     * @param stageName Имя этапа
     * @return true если этап включен
     */
    bool isStageEnabled(const std::string& stageName) const;
    
    /**
     * @brief Получить текущий контекст рендеринга
     * @return Ссылка на контекст
     */
    const RenderContext& getCurrentContext() const { return currentContext_; }
    
    /**
     * @brief Включить профилирование этапов
     * @param enable Включить профилирование
     */
    void setProfiling(bool enable) { profilingEnabled_ = enable; }
    
    /**
     * @brief Получить результаты профилирования
     * @return Карта времени выполнения этапов
     */
    const std::unordered_map<std::string, float>& getStageTimings() const { return stageTimings_; }
    
    /**
     * @brief Сбросить профилирование
     */
    void resetProfiling() { stageTimings_.clear(); }
    
    /**
     * @brief Получить общее время выполнения всех этапов
     * @return Время в миллисекундах
     */
    float getTotalExecutionTime() const;

private:
    /**
     * @brief Настройка стандартных этапов рендеринга
     */
    void setupDefaultStages();
    
    /**
     * @brief Сортировка этапов по приоритету и зависимостям
     */
    void sortStages();
    
    /**
     * @brief Проверка зависимостей между этапами
     * @return true если все зависимости удовлетворены
     */
    bool validateStageDependencies() const;
    
    /**
     * @brief Подготовка контекста рендеринга
     * @param frameData Данные кадра
     */
    void prepareRenderContext(const FrameData& frameData);
    
    /**
     * @brief Выполнение одного этапа с профилированием
     * @param stage Этап для выполнения
     */
    void executeStageWithProfiling(IRenderStage& stage);
    
    /**
     * @brief Логирование выполнения этапов
     * @param stageName Имя этапа
     * @param executionTime Время выполнения
     */
    void logStageExecution(const std::string& stageName, float executionTime) const;
};

} // namespace HyperEngine::Rendering::Vulkan
