
#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "Common/IRenderer.h"
#include "Common/IResourceManager.h"
#include "HyperEngine/Core/DependencyInjection/Container.h"

namespace HyperEngine::Rendering {

/**
 * @brief Фабрика для создания рендереров
 *
 * Применяет Factory Pattern - централизованное создание объектов
 * с возможностью выбора конкретной реализации во время выполнения.
 *
 * Также применяет Open/Closed Principle - новые типы рендереров
 * могут быть добавлены без изменения существующего кода.
 */
class RendererFactory {
  public:
    /**
     * @brief Информация о доступном рендерере
     */
    struct RendererInfo {
        RendererType type;                       ///< Тип рендерера
        std::string name;                        ///< Человекочитаемое имя
        std::string description;                 ///< Описание рендерера
        std::vector<RenderingFeature> features;  ///< Поддерживаемые функции
        bool isSupported;  ///< Поддерживается ли на текущем оборудовании
        int priority;  ///< Приоритет (больше = лучше)
        struct {
            std::string vendor;      ///< Производитель GPU
            std::string apiVersion;  ///< Версия API
            bool hasRayTracing;      ///< Поддержка ray tracing
            bool hasComputeShaders;  ///< Поддержка compute shaders
            size_t dedicatedMemory;  ///< Объем видеопамяти в байтах
        } capabilities;
    };

    /**
     * @brief Создать рендерер указанного типа
     * @param type Тип рендерера
     * @param config Конфигурация рендерера
     * @return Умный указатель на созданный рендерер или nullptr при ошибке
     */
    static std::unique_ptr<IRenderer> createRenderer(RendererType type,
                                                     const RendererConfig& config);

    /**
     * @brief Создать рендерер с автоматическим выбором лучшего типа
     * @param config Конфигурация рендерера
     * @return Умный указатель на созданный рендерер
     */
    static std::unique_ptr<IRenderer> createBestRenderer(const RendererConfig& config);

    /**
     * @brief Создать рендерер с внедрением зависимостей
     * @param type Тип рендерера
     * @param config Конфигурация рендерера
     * @param resourceManager Менеджер ресурсов
     * @return Умный указатель на созданный рендерер
     */
    static std::unique_ptr<IRenderer> createRendererWithDI(
        RendererType type,
        const RendererConfig& config,
        std::shared_ptr<IResourceManager> resourceManager);

    /**
     * @brief Проверить поддержку типа рендерера на текущей платформе
     * @param type Тип рендерера
     * @return true если поддерживается
     */
    static bool isSupported(RendererType type);

    /**
     * @brief Получить список поддерживаемых рендереров
     * @return Вектор поддерживаемых типов
     */
    static std::vector<RendererType> getSupportedRenderers();

    /**
     * @brief Получить детальную информацию о всех доступных рендерерах
     * @return Вектор с информацией о рендерерах
     */
    static std::vector<RendererInfo> getAvailableRenderers();

    /**
     * @brief Получить информацию о конкретном рендерере
     * @param type Тип рендерера
     * @return Информация о рендерере или nullptr если не найден
     */
    static std::unique_ptr<RendererInfo> getRendererInfo(RendererType type);

    /**
     * @brief Получить рекомендованный тип рендерера для текущего оборудования
     * @return Тип рекомендованного рендерера
     */
    static RendererType getRecommendedRenderer();

    /**
     * @brief Получить рекомендованный тип рендерера с учетом требований
     * @param requiredFeatures Требуемые функции
     * @param preferPerformance Предпочесть производительность качеству
     * @return Тип рекомендованного рендерера
     */
    static RendererType getRecommendedRenderer(
        const std::vector<RenderingFeature>& requiredFeatures,
        bool preferPerformance = true);

    /**
     * @brief Проверить совместимость конфигурации с рендерером
     * @param type Тип рендерера
     * @param config Конфигурация для проверки
     * @return true если конфигурация совместима
     */
    static bool isConfigurationValid(RendererType type, const RendererConfig& config);

    /**
     * @brief Получить рекомендованную конфигурацию для рендерера
     * @param type Тип рендерера
     * @return Рекомендованная конфигурация
     */
    static RendererConfig getRecommendedConfig(RendererType type);

    /**
     * @brief Зарегистрировать пользовательскую фабрику рендерера
     * @param type Тип рендерера
     * @param factory Функция создания рендерера
     */
    static void registerCustomRenderer(
        RendererType type,
        std::function<std::unique_ptr<IRenderer>(const RendererConfig&)> factory);

    /**
     * @brief Отменить регистрацию пользовательского рендерера
     * @param type Тип рендерера
     */
    static void unregisterCustomRenderer(RendererType type);

    /**
     * @brief Получить строковое представление типа рендерера
     * @param type Тип рендерера
     * @return Строковое имя
     */
    static std::string getRendererTypeName(RendererType type);

    /**
     * @brief Получить тип рендерера по строковому имени
     * @param name Строковое имя рендерера
     * @return Тип рендерера или RendererType::Software если не найден
     */
    static RendererType getRendererTypeFromName(const std::string& name);

  private:
    RendererFactory() = default;  // Запретить создание экземпляров

    /**
     * @brief Создать Vulkan рендерер
     * @param config Конфигурация
     * @return Умный указатель на рендерер
     */
    static std::unique_ptr<IRenderer> createVulkanRenderer(const RendererConfig& config);

    /**
     * @brief Создать OpenGL рендерер
     * @param config Конфигурация
     * @return Умный указатель на рендерер
     */
    static std::unique_ptr<IRenderer> createOpenGLRenderer(const RendererConfig& config);

    /**
     * @brief Создать DirectX12 рендерер
     * @param config Конфигурация
     * @return Умный указатель на рендерер
     */
    static std::unique_ptr<IRenderer> createDirectX12Renderer(const RendererConfig& config);

    /**
     * @brief Создать Metal рендерер
     * @param config Конфигурация
     * @return Умный указатель на рендерер
     */
    static std::unique_ptr<IRenderer> createMetalRenderer(const RendererConfig& config);

    /**
     * @brief Создать программный рендерер
     * @param config Конфигурация
     * @return Умный указатель на рендерер
     */
    static std::unique_ptr<IRenderer> createSoftwareRenderer(const RendererConfig& config);

    /**
     * @brief Проверить поддержку Vulkan
     * @return true если Vulkan поддерживается
     */
    static bool checkVulkanSupport();

    /**
     * @brief Проверить поддержку OpenGL
     * @return true если OpenGL поддерживается
     */
    static bool checkOpenGLSupport();

    /**
     * @brief Проверить поддержку DirectX12
     * @return true если DirectX12 поддерживается
     */
    static bool checkDirectX12Support();

    /**
     * @brief Проверить поддержку Metal
     * @return true если Metal поддерживается
     */
    static bool checkMetalSupport();

    /**
     * @brief Получить информацию о GPU
     * @return Структура с информацией о GPU
     */
    static struct GPUInfo {
        std::string vendor;
        std::string deviceName;
        size_t dedicatedMemory;
        bool supportsRayTracing;
        bool supportsComputeShaders;
        bool supportsVulkan;
        bool supportsDirectX12;
        bool supportsMetal;
        std::string driverVersion;
    } getGPUInfo();

    /**
     * @brief Кэшированная информация о GPU
     */
    static GPUInfo cachedGPUInfo_;
    static bool gpuInfoCached_;

    /**
     * @brief Пользовательские фабрики
     */
    static std::unordered_map<RendererType,
                              std::function<std::unique_ptr<IRenderer>(const RendererConfig&)>>
        customFactories_;
};

/**
 * @brief Адаптер для интеграции с DI контейнером
 *
 * Позволяет зарегистрировать RendererFactory в DI контейнере
 * и автоматически выбирать лучший рендерер.
 */
class RendererFactoryAdapter {
  private:
    RendererType preferredType_;
    RendererConfig defaultConfig_;

  public:
    explicit RendererFactoryAdapter(RendererType preferredType = RendererType::Vulkan,
                                    const RendererConfig& defaultConfig = {})
        : preferredType_(preferredType), defaultConfig_(defaultConfig) {}

    /**
     * @brief Создать рендерер через DI систему
     * @param container DI контейнер для получения зависимостей
     * @return Умный указатель на рендерер
     */
    std::shared_ptr<IRenderer> createRenderer(Core::DI::Container& container) {
        auto resourceManager = container.resolve<IResourceManager>();

        // Попробовать создать предпочитаемый тип
        if (RendererFactory::isSupported(preferredType_)) {
            auto renderer = RendererFactory::createRendererWithDI(
                preferredType_, defaultConfig_, resourceManager);
            if (renderer) {
                return renderer;
            }
        }

        // Fallback на лучший доступный рендерер
        return RendererFactory::createBestRenderer(defaultConfig_);
    }

    /**
     * @brief Установить предпочитаемый тип рендерера
     * @param type Новый предпочитаемый тип
     */
    void setPreferredType(RendererType type) { preferredType_ = type; }

    /**
     * @brief Установить конфигурацию по умолчанию
     * @param config Новая конфигурация
     */
    void setDefaultConfig(const RendererConfig& config) { defaultConfig_ = config; }
};

}  // namespace HyperEngine::Rendering
