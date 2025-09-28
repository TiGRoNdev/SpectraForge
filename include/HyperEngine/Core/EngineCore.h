/**
 * @file EngineCore.h
 * @brief Рефакторенный основной класс движка согласно SOLID принципам
 *
 * Применяет все SOLID принципы:
 * - SRP: Разделение обязанностей на отдельные компоненты
 * - OCP: Открыт для расширения через интерфейсы
 * - LSP: Все компоненты взаимозаменяемы через интерфейсы
 * - ISP: Использует специализированные интерфейсы
 * - DIP: Зависит от абстракций, а не от конкретных реализаций
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include "../Rendering/Common/IRenderer.h"
#include "../Rendering/Common/IResourceManager.h"
#include "Interfaces/IConfigurable.h"
#include "Interfaces/IEventHandler.h"
#include "Interfaces/IInitializable.h"

namespace HyperEngine {
namespace Core {

// Forward declarations
class ISubsystem;
class ILogger;

/**
 * @brief Информация о версии движка
 */
struct EngineInfo {
    static constexpr int MAJOR_VERSION = 1;
    static constexpr int MINOR_VERSION = 0;
    static constexpr int PATCH_VERSION = 0;
    static constexpr const char* VERSION_STRING = "1.0.0";
    static constexpr const char* BUILD_DATE = __DATE__;
    static constexpr const char* BUILD_TIME = __TIME__;
};

/**
 * @brief Конфигурация движка
 */
struct EngineConfig {
    int windowWidth = 1280;
    int windowHeight = 720;
    std::string windowTitle = "HyperEngine";
    bool enableVSync = true;
    bool enableDebugMode = false;
    std::string logLevel = "INFO";
    std::string configPath = "engine.config";
};

/**
 * @brief Основной класс движка, следующий SOLID принципам
 *
 * SRP: Отвечает только за координацию подсистем
 * OCP: Открыт для расширения через добавление новых подсистем
 * LSP: Все подсистемы взаимозаменяемы через интерфейсы
 * ISP: Использует специализированные интерфейсы для разных аспектов
 * DIP: Зависит от абстракций (интерфейсов), а не от конкретных классов
 */
class EngineCore : public Interfaces::IInitializable,
                   public Interfaces::IConfigurable,
                   public Interfaces::IEventHandler {
  public:
    /**
     * @brief Конструктор с dependency injection
     * @param renderer Рендерер (DIP - зависимость от абстракции)
     * @param resourceManager Менеджер ресурсов (DIP)
     * @param logger Логгер (DIP)
     */
    EngineCore(std::shared_ptr<Rendering::IRenderer> renderer,
               std::shared_ptr<Rendering::IResourceManager> resourceManager,
               std::shared_ptr<ILogger> logger);

    /**
     * @brief Деструктор
     */
    virtual ~EngineCore();

    // IInitializable interface (ISP)
    bool initialize() override;
    void shutdown() override;
    bool isInitialized() const override;

    // IConfigurable interface (ISP)
    void setConfigParameter(const std::string& key, const std::any& value) override;
    std::any getConfigParameter(const std::string& key) const override;
    bool hasConfigParameter(const std::string& key) const override;
    bool loadConfig(const std::string& configPath) override;
    bool saveConfig(const std::string& configPath) const override;

    // IEventHandler interface (ISP)
    bool handleEvent(const std::shared_ptr<Interfaces::IEvent>& event) override;
    bool canHandle(const char* eventType) const override;

    /**
     * @brief Регистрация подсистемы (OCP - открыт для расширения)
     * @param subsystem Подсистема для регистрации
     */
    void registerSubsystem(std::shared_ptr<ISubsystem> subsystem);

    /**
     * @brief Удаление подсистемы
     * @param subsystem Подсистема для удаления
     */
    void unregisterSubsystem(std::shared_ptr<ISubsystem> subsystem);

    /**
     * @brief Основной цикл движка
     */
    void run();

    /**
     * @brief Остановка движка
     */
    void stop();

    /**
     * @brief Получение информации о движке
     * @return Структура с информацией о версии
     */
    static EngineInfo getEngineInfo();

    /**
     * @brief Получение рендерера
     * @return Указатель на рендерер
     */
    std::shared_ptr<Rendering::IRenderer> getRenderer() const { return renderer; }

    /**
     * @brief Получение менеджера ресурсов
     * @return Указатель на менеджер ресурсов
     */
    std::shared_ptr<Rendering::IResourceManager> getResourceManager() const {
        return resourceManager;
    }

  private:
    // Dependency injection (DIP)
    std::shared_ptr<Rendering::IRenderer> renderer;
    std::shared_ptr<Rendering::IResourceManager> resourceManager;
    std::shared_ptr<ILogger> logger;
    std::shared_ptr<Interfaces::IEventDispatcher> eventDispatcher;

    // Подсистемы (OCP)
    std::vector<std::shared_ptr<ISubsystem>> subsystems;

    // Состояние движка
    bool initialized = false;
    bool running = false;
    EngineConfig config;
    std::unordered_map<std::string, std::any> configParameters;

    /**
     * @brief Инициализация подсистем
     * @return true если все подсистемы инициализированы успешно
     */
    bool initializeSubsystems();

    /**
     * @brief Завершение работы подсистем
     */
    void shutdownSubsystems();

    /**
     * @brief Обновление подсистем
     * @param deltaTime Время с последнего обновления
     */
    void updateSubsystems(float deltaTime);

    /**
     * @brief Рендеринг кадра
     */
    void renderFrame();

    // Запрет копирования (следует правилу Rule of Five)
    EngineCore(const EngineCore&) = delete;
    EngineCore& operator=(const EngineCore&) = delete;
    EngineCore(EngineCore&&) = delete;
    EngineCore& operator=(EngineCore&&) = delete;
};

/**
 * @brief Интерфейс для подсистем движка (ISP)
 */
class ISubsystem : public Interfaces::IInitializable {
  public:
    virtual ~ISubsystem() = default;

    /**
     * @brief Обновление подсистемы
     * @param deltaTime Время с последнего обновления
     */
    virtual void update(float deltaTime) = 0;

    /**
     * @brief Получение имени подсистемы
     * @return Имя подсистемы
     */
    virtual const char* getName() const = 0;

    /**
     * @brief Получение приоритета обновления
     * @return Приоритет (меньше = выше приоритет)
     */
    virtual int getUpdatePriority() const = 0;
};

/**
 * @brief Интерфейс для логгера (DIP)
 */
class ILogger {
  public:
    virtual ~ILogger() = default;

    virtual void logInfo(const std::string& message) = 0;
    virtual void logWarning(const std::string& message) = 0;
    virtual void logError(const std::string& message) = 0;
    virtual void logDebug(const std::string& message) = 0;
};

}  // namespace Core
}  // namespace HyperEngine
