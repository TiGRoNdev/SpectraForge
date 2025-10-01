/**
 * @file EngineCore.cpp
 * @brief Реализация основного класса движка согласно SOLID принципам
 */

#include "HyperEngine/Core/EngineCore.h"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <stdexcept>
#include "HyperEngine/Core/SafeConsole.h"

using namespace HyperEngine::Core;
using namespace HyperEngine::Core::Interfaces;

namespace HyperEngine {
namespace Core {

// Конструктор с dependency injection (DIP)
EngineCore::EngineCore(std::shared_ptr<Rendering::IRenderer> renderer,
                       std::shared_ptr<Rendering::IResourceManager> resourceManager,
                       std::shared_ptr<ILogger> logger)
    : renderer(renderer), resourceManager(resourceManager), logger(logger) {
    if (!renderer) {
        throw std::invalid_argument("Renderer не может быть nullptr");
    }
    if (!resourceManager) {
        throw std::invalid_argument("ResourceManager не может быть nullptr");
    }
    if (!logger) {
        throw std::invalid_argument("Logger не может быть nullptr");
    }

    logger->logInfo("EngineCore создан с dependency injection");
}

EngineCore::~EngineCore() {
    if (initialized) {
        shutdown();
    }
    if (logger) {
        logger->logInfo("EngineCore уничтожен");
    }
}

// IInitializable interface implementation
bool EngineCore::initialize() {
    if (initialized) {
        logger->logWarning("EngineCore уже инициализирован");
        return true;
    }

    logger->logInfo("Инициализация EngineCore...");

    try {
        // Инициализация рендерера
        if (!renderer->initialize()) {
            logger->logError("Ошибка инициализации рендерера");
            return false;
        }

        // Инициализация менеджера ресурсов
        if (!resourceManager->initialize()) {
            logger->logError("Ошибка инициализации менеджера ресурсов");
            return false;
        }

        // Инициализация подсистем
        if (!initializeSubsystems()) {
            logger->logError("Ошибка инициализации подсистем");
            return false;
        }

        initialized = true;
        logger->logInfo("EngineCore инициализирован успешно");
        return true;

    } catch (const std::exception& e) {
        logger->logError("Исключение при инициализации EngineCore: " + std::string(e.what()));
        return false;
    }
}

void EngineCore::shutdown() {
    if (!initialized) {
        return;
    }

    logger->logInfo("Завершение работы EngineCore...");

    running = false;

    // Завершение работы подсистем
    shutdownSubsystems();

    // Завершение работы основных компонентов
    if (resourceManager) {
        resourceManager->shutdown();
    }
    if (renderer) {
        renderer->shutdown();
    }

    initialized = false;
    logger->logInfo("EngineCore завершил работу");
}

bool EngineCore::isInitialized() const {
    return initialized;
}

// IConfigurable interface implementation
void EngineCore::setConfigParameter(const std::string& key, const std::any& value) {
    configParameters[key] = value;
    logger->logDebug("Установлен параметр конфигурации: " + key);
}

std::any EngineCore::getConfigParameter(const std::string& key) const {
    auto it = configParameters.find(key);
    if (it != configParameters.end()) {
        return it->second;
    }
    throw std::runtime_error("Параметр конфигурации не найден: " + key);
}

bool EngineCore::hasConfigParameter(const std::string& key) const {
    return configParameters.find(key) != configParameters.end();
}

bool EngineCore::loadConfig(const std::string& configPath) {
    try {
        logger->logInfo("Загрузка конфигурации из: " + configPath);

        std::ifstream file(configPath);
        if (!file.is_open()) {
            logger->logWarning("Файл конфигурации не найден: " + configPath);
            return false;
        }

        // TODO: Реализовать парсинг конфигурации (JSON/YAML)
        // Пока используем значения по умолчанию
        setConfigParameter("window_width", config.windowWidth);
        setConfigParameter("window_height", config.windowHeight);
        setConfigParameter("window_title", config.windowTitle);
        setConfigParameter("enable_vsync", config.enableVSync);
        setConfigParameter("enable_debug", config.enableDebugMode);
        setConfigParameter("log_level", config.logLevel);

        logger->logInfo("Конфигурация загружена успешно");
        return true;

    } catch (const std::exception& e) {
        logger->logError("Ошибка загрузки конфигурации: " + std::string(e.what()));
        return false;
    }
}

bool EngineCore::saveConfig(const std::string& configPath) const {
    try {
        logger->logInfo("Сохранение конфигурации в: " + configPath);

        std::ofstream file(configPath);
        if (!file.is_open()) {
            logger->logError("Не удалось создать файл конфигурации: " + configPath);
            return false;
        }

        // TODO: Реализовать сериализацию конфигурации (JSON/YAML)
        file << "# HyperEngine Configuration\n";
        file << "# Generated automatically\n";

        logger->logInfo("Конфигурация сохранена успешно");
        return true;

    } catch (const std::exception& e) {
        logger->logError("Ошибка сохранения конфигурации: " + std::string(e.what()));
        return false;
    }
}

// IEventHandler interface implementation
bool EngineCore::handleEvent(const std::shared_ptr<IEvent>& event) {
    if (!event) {
        return false;
    }

    logger->logDebug("Обработка события: " + std::string(event->getEventType()));

    // TODO: Реализовать обработку специфичных событий движка
    return false;
}

bool EngineCore::canHandle(const char* eventType) const {
    // TODO: Определить типы событий, которые может обработать движок
    return false;
}

// Управление подсистемами (OCP - открыт для расширения)
void EngineCore::registerSubsystem(std::shared_ptr<ISubsystem> subsystem) {
    if (!subsystem) {
        logger->logWarning("Попытка регистрации nullptr подсистемы");
        return;
    }

    // Проверяем, не зарегистрирована ли уже эта подсистема
    auto it = std::find(subsystems.begin(), subsystems.end(), subsystem);
    if (it != subsystems.end()) {
        logger->logWarning("Подсистема уже зарегистрирована: " + std::string(subsystem->getName()));
        return;
    }

    subsystems.push_back(subsystem);

    // Сортируем по приоритету
    std::sort(subsystems.begin(),
              subsystems.end(),
              [](const std::shared_ptr<ISubsystem>& a, const std::shared_ptr<ISubsystem>& b) {
                  return a->getUpdatePriority() < b->getUpdatePriority();
              });

    logger->logInfo("Зарегистрирована подсистема: " + std::string(subsystem->getName()));

    // Если движок уже инициализирован, инициализируем новую подсистему
    if (initialized && !subsystem->isInitialized()) {
        if (!subsystem->initialize()) {
            logger->logError("Ошибка инициализации подсистемы: "
                             + std::string(subsystem->getName()));
        }
    }
}

void EngineCore::unregisterSubsystem(std::shared_ptr<ISubsystem> subsystem) {
    if (!subsystem) {
        return;
    }

    auto it = std::find(subsystems.begin(), subsystems.end(), subsystem);
    if (it != subsystems.end()) {
        if (subsystem->isInitialized()) {
            subsystem->shutdown();
        }
        subsystems.erase(it);
        logger->logInfo("Подсистема удалена: " + std::string(subsystem->getName()));
    }
}

// Основной цикл движка
void EngineCore::run() {
    if (!initialized) {
        logger->logError("Движок не инициализирован");
        return;
    }

    logger->logInfo("Запуск основного цикла движка");
    running = true;

    auto lastTime = std::chrono::high_resolution_clock::now();

    while (running) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        try {
            // Обновление подсистем
            updateSubsystems(deltaTime);

            // Рендеринг кадра
            renderFrame();

        } catch (const std::exception& e) {
            logger->logError("Ошибка в основном цикле: " + std::string(e.what()));
            break;
        }
    }

    logger->logInfo("Основной цикл движка завершен");
}

void EngineCore::stop() {
    logger->logInfo("Запрос на остановку движка");
    running = false;
}

EngineInfo EngineCore::getEngineInfo() {
    return EngineInfo{};
}

// Приватные методы
bool EngineCore::initializeSubsystems() {
    logger->logInfo("Инициализация подсистем (" + SAFE_TO_STRING(subsystems.size()) + ")...");

    for (auto& subsystem : subsystems) {
        if (!subsystem->initialize()) {
            logger->logError("Ошибка инициализации подсистемы: "
                             + std::string(subsystem->getName()));
            return false;
        }
        logger->logInfo("Подсистема инициализирована: " + std::string(subsystem->getName()));
    }

    return true;
}

void EngineCore::shutdownSubsystems() {
    logger->logInfo("Завершение работы подсистем...");

    // Завершаем в обратном порядке
    for (auto it = subsystems.rbegin(); it != subsystems.rend(); ++it) {
        if ((*it)->isInitialized()) {
            (*it)->shutdown();
            logger->logInfo("Подсистема завершена: " + std::string((*it)->getName()));
        }
    }
}

void EngineCore::updateSubsystems(float deltaTime) {
    for (auto& subsystem : subsystems) {
        if (subsystem->isInitialized()) {
            subsystem->update(deltaTime);
        }
    }
}

void EngineCore::renderFrame() {
    if (renderer && renderer->isInitialized()) {
        renderer->beginFrame();
        // TODO: Рендеринг сцены
        renderer->endFrame();
    }
}

}  // namespace Core
}  // namespace HyperEngine
