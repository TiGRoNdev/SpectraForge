
/**
 * @file ModernRenderer3D.h
 * @brief Рефакторенный рендерер согласно SOLID принципам
 *
 * Применяет все SOLID принципы:
 * - SRP: Разделение обязанностей на отдельные компоненты
 * - OCP: Открыт для расширения через стратегии и фабрики
 * - LSP: Все компоненты взаимозаменяемы через интерфейсы
 * - ISP: Использует специализированные интерфейсы
 * - DIP: Зависит от абстракций, а не от конкретных реализаций
 */

#pragma once

#include <memory>
#include <vector>
#include "../Core/EngineCore.h"
#include "../Core/Interfaces/IConfigurable.h"
#include "../Core/Interfaces/IInitializable.h"
#include "../Math/Matrix4.h"
#include "../Math/Vector3.h"
#include "Common/IRenderer.h"

namespace SpectraForge {
namespace Rendering {

// Forward declarations для dependency injection
class IRenderStrategy;
class ILightingSystem;
class ICameraSystem;
class IRenderStatistics;

/**
 * @brief Современный рендерер, следующий SOLID принципам
 *
 * SRP: Отвечает только за координацию рендеринга
 * OCP: Открыт для расширения через стратегии
 * LSP: Все стратегии взаимозаменяемы
 * ISP: Использует специализированные интерфейсы
 * DIP: Зависит от абстракций (стратегий), а не от конкретных реализаций
 */
class ModernRenderer3D : public IRenderer,
                         public Core::Interfaces::IInitializable,
                         public Core::Interfaces::IConfigurable {
  public:
    /**
     * @brief Конструктор с dependency injection
     * @param renderStrategy Стратегия рендеринга (DIP)
     * @param lightingSystem Система освещения (DIP)
     * @param cameraSystem Система камер (DIP)
     * @param statistics Система статистики (DIP)
     * @param logger Логгер (DIP)
     */
    ModernRenderer3D(const std::shared_ptr<IRenderStrategy>& renderStrategy,
                     const std::shared_ptr<ILightingSystem>& lightingSystem,
                     const std::shared_ptr<ICameraSystem>& cameraSystem,
                     const std::shared_ptr<IRenderStatistics>& statistics,
                     const std::shared_ptr<Core::ILogger>& logger);

    /**
     * @brief Деструктор
     */
    ~ModernRenderer3D() override;

    // IRenderer interface
    bool initialize() override;
    void renderFrame(const FrameData& frameData) override;
    void shutdown() override;
    RendererType getType() const override;
    bool supportsFeature(RenderingFeature feature) const override;
    std::string getName() const override;
    std::string getApiVersion() const override;
    bool isReady() const override;
    RenderingStats getStats() const override;

    // IInitializable interface (ISP)
    bool isInitialized() const override;

    // IConfigurable interface (ISP)
    void setConfigParameter(const std::string& key, const std::any& value) override;
    std::any getConfigParameter(const std::string& key) const override;
    bool hasConfigParameter(const std::string& key) const override;
    bool loadConfig(const std::string& configPath) override;
    bool saveConfig(const std::string& configPath) const override;

    /**
     * @brief Смена стратегии рендеринга (Strategy Pattern + OCP)
     * @param newStrategy Новая стратегия рендеринга
     */
    void setRenderStrategy(const std::shared_ptr<IRenderStrategy>& newStrategy);

    /**
     * @brief Получение текущей стратегии рендеринга
     * @return Указатель на текущую стратегию
     */
    std::shared_ptr<IRenderStrategy> getRenderStrategy() const { return renderStrategy; }

    /**
     * @brief Добавление пост-процессинг эффекта (OCP)
     * @param effect Эффект для добавления
     */
    void addPostProcessEffect(const std::shared_ptr<class IPostProcessEffect>& effect);

    /**
     * @brief Удаление пост-процессинг эффекта
     * @param effect Эффект для удаления
     */
    void removePostProcessEffect(std::shared_ptr<class IPostProcessEffect> effect);

  private:
    // Dependency injection (DIP)
    std::shared_ptr<IRenderStrategy> renderStrategy;
    std::shared_ptr<ILightingSystem> lightingSystem;
    std::shared_ptr<ICameraSystem> cameraSystem;
    std::shared_ptr<IRenderStatistics> statistics;
    std::shared_ptr<Core::ILogger> logger;

    // Пост-процессинг эффекты (OCP)
    std::vector<std::shared_ptr<class IPostProcessEffect>> postProcessEffects;

    // Состояние рендерера
    bool initialized = false;
    RendererConfig config;
    std::unordered_map<std::string, std::any> configParameters;

    /**
     * @brief Инициализация графического API
     * @return true если инициализация успешна
     */
    bool initializeGraphicsAPI();

    /**
     * @brief Применение пост-процессинг эффектов
     */
    void applyPostProcessEffects();

    // Запрет копирования
    ModernRenderer3D(const ModernRenderer3D&) = delete;
    ModernRenderer3D& operator=(const ModernRenderer3D&) = delete;
};

/**
 * @brief Интерфейс стратегии рендеринга (Strategy Pattern + ISP)
 */
class IRenderStrategy {
  public:
    virtual ~IRenderStrategy() = default;

    /**
     * @brief Инициализация стратегии
     * @return true если инициализация успешна
     */
    virtual bool initialize() = 0;

    /**
     * @brief Рендеринг кадра
     * @param frameData Данные кадра
     */
    virtual void render(const FrameData& frameData) = 0;

    /**
     * @brief Завершение работы стратегии
     */
    virtual void shutdown() = 0;

    /**
     * @brief Получение имени стратегии
     * @return Имя стратегии
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Проверка поддержки функции
     * @param feature Функция для проверки
     * @return true если поддерживается
     */
    virtual bool supportsFeature(RenderingFeature feature) const = 0;
};

/**
 * @brief Интерфейс системы освещения (ISP)
 */
class ILightingSystem {
  public:
    virtual ~ILightingSystem() = default;

    /**
     * @brief Инициализация системы освещения
     * @return true если инициализация успешна
     */
    virtual bool initialize() = 0;

    /**
     * @brief Обновление освещения
     * @param frameData Данные кадра
     */
    virtual void updateLighting(const FrameData& frameData) = 0;

    /**
     * @brief Завершение работы системы
     */
    virtual void shutdown() = 0;

    /**
     * @brief Добавление источника света
     * @param light Источник света
     */
    virtual void addLight(std::shared_ptr<class ILight> light) = 0;

    /**
     * @brief Удаление источника света
     * @param light Источник света
     */
    virtual void removeLight(std::shared_ptr<class ILight> light) = 0;
};

/**
 * @brief Интерфейс системы камер (ISP)
 */
class ICameraSystem {
  public:
    virtual ~ICameraSystem() = default;

    /**
     * @brief Инициализация системы камер
     * @return true если инициализация успешна
     */
    virtual bool initialize() = 0;

    /**
     * @brief Обновление камеры
     * @param frameData Данные кадра
     */
    virtual void updateCamera(const FrameData& frameData) = 0;

    /**
     * @brief Завершение работы системы
     */
    virtual void shutdown() = 0;

    /**
     * @brief Получение матрицы вида
     * @return Матрица вида
     */
    virtual Math::Matrix4 getViewMatrix() const = 0;

    /**
     * @brief Получение матрицы проекции
     * @return Матрица проекции
     */
    virtual Math::Matrix4 getProjectionMatrix() const = 0;
};

/**
 * @brief Интерфейс системы статистики (ISP)
 */
class IRenderStatistics {
  public:
    virtual ~IRenderStatistics() = default;

    /**
     * @brief Начало измерения кадра
     */
    virtual void beginFrame() = 0;

    /**
     * @brief Завершение измерения кадра
     */
    virtual void endFrame() = 0;

    /**
     * @brief Регистрация draw call
     * @param primitiveCount Количество примитивов
     */
    virtual void recordDrawCall(uint32_t primitiveCount) = 0;

    /**
     * @brief Получение статистики
     * @return Структура со статистикой
     */
    virtual RenderingStats getStats() const = 0;

    /**
     * @brief Сброс статистики
     */
    virtual void reset() = 0;
};

/**
 * @brief Интерфейс пост-процессинг эффекта (ISP)
 */
class IPostProcessEffect {
  public:
    virtual ~IPostProcessEffect() = default;

    /**
     * @brief Инициализация эффекта
     * @return true если инициализация успешна
     */
    virtual bool initialize() = 0;

    /**
     * @brief Применение эффекта
     * @param inputTexture Входная текстура
     * @param outputTexture Выходная текстура
     */
    virtual void apply(class ITexture* inputTexture, class ITexture* outputTexture) = 0;

    /**
     * @brief Завершение работы эффекта
     */
    virtual void shutdown() = 0;

    /**
     * @brief Получение имени эффекта
     * @return Имя эффекта
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Проверка активности эффекта
     * @return true если эффект активен
     */
    virtual bool isEnabled() const = 0;

    /**
     * @brief Включение/выключение эффекта
     * @param enabled Состояние эффекта
     */
    virtual void setEnabled(bool enabled) = 0;
};

}  // namespace Rendering
}  // namespace SpectraForge
