/**
 * @file ModernRenderer3D.cpp
 * @brief Реализация современного рендерера согласно SOLID принципам
 */

#include "HyperEngine/Rendering/ModernRenderer3D.h"
#include "HyperEngine/Core/SafeConsole.h"
#include <algorithm>
#include <stdexcept>

using namespace HyperEngine::Rendering;
using namespace HyperEngine::Core;

namespace HyperEngine {
namespace Rendering {

// Конструктор с dependency injection (DIP)
ModernRenderer3D::ModernRenderer3D(
    std::shared_ptr<IRenderStrategy> renderStrategy,
    std::shared_ptr<ILightingSystem> lightingSystem,
    std::shared_ptr<ICameraSystem> cameraSystem,
    std::shared_ptr<IRenderStatistics> statistics,
    std::shared_ptr<Core::ILogger> logger)
    : renderStrategy(renderStrategy), 
      lightingSystem(lightingSystem),
      cameraSystem(cameraSystem),
      statistics(statistics),
      logger(logger) {
    
    if (!renderStrategy) {
        throw std::invalid_argument("RenderStrategy не может быть nullptr");
    }
    if (!lightingSystem) {
        throw std::invalid_argument("LightingSystem не может быть nullptr");
    }
    if (!cameraSystem) {
        throw std::invalid_argument("CameraSystem не может быть nullptr");
    }
    if (!statistics) {
        throw std::invalid_argument("Statistics не может быть nullptr");
    }
    if (!logger) {
        throw std::invalid_argument("Logger не может быть nullptr");
    }

    logger->logInfo("ModernRenderer3D создан с dependency injection");
}

ModernRenderer3D::~ModernRenderer3D() {
    if (initialized) {
        shutdown();
    }
    if (logger) {
        logger->logInfo("ModernRenderer3D уничтожен");
    }
}

// IRenderer interface implementation
bool ModernRenderer3D::initialize() {
    if (initialized) {
        logger->logWarning("ModernRenderer3D уже инициализирован");
        return true;
    }

    logger->logInfo("Инициализация ModernRenderer3D...");

    try {
        // Инициализация графического API
        if (!initializeGraphicsAPI()) {
            logger->logError("Ошибка инициализации графического API");
            return false;
        }

        // Инициализация стратегии рендеринга
        if (!renderStrategy->initialize()) {
            logger->logError("Ошибка инициализации стратегии рендеринга: " + renderStrategy->getName());
            return false;
        }

        // Инициализация системы освещения
        if (!lightingSystem->initialize()) {
            logger->logError("Ошибка инициализации системы освещения");
            return false;
        }

        // Инициализация системы камер
        if (!cameraSystem->initialize()) {
            logger->logError("Ошибка инициализации системы камер");
            return false;
        }

        // Инициализация пост-процессинг эффектов
        for (auto& effect : postProcessEffects) {
            if (!effect->initialize()) {
                logger->logWarning("Ошибка инициализации эффекта: " + effect->getName());
            }
        }

        initialized = true;
        logger->logInfo("ModernRenderer3D инициализирован успешно");
        return true;

    } catch (const std::exception& e) {
        logger->logError("Исключение при инициализации ModernRenderer3D: " + std::string(e.what()));
        return false;
    }
}

void ModernRenderer3D::renderFrame(const FrameData& frameData) {
    if (!initialized) {
        logger->logError("ModernRenderer3D не инициализирован");
        return;
    }

    try {
        // Начало измерения производительности
        statistics->beginFrame();

        // Обновление системы камер
        cameraSystem->updateCamera(frameData);

        // Обновление системы освещения
        lightingSystem->updateLighting(frameData);

        // Основной рендеринг через стратегию (Strategy Pattern)
        renderStrategy->render(frameData);

        // Применение пост-процессинг эффектов (OCP)
        applyPostProcessEffects();

        // Завершение измерения производительности
        statistics->endFrame();

    } catch (const std::exception& e) {
        logger->logError("Ошибка при рендеринге кадра: " + std::string(e.what()));
    }
}

void ModernRenderer3D::shutdown() {
    if (!initialized) {
        return;
    }

    logger->logInfo("Завершение работы ModernRenderer3D...");

    // Завершение работы пост-процессинг эффектов
    for (auto& effect : postProcessEffects) {
        effect->shutdown();
    }
    postProcessEffects.clear();

    // Завершение работы основных систем
    if (cameraSystem) {
        cameraSystem->shutdown();
    }
    if (lightingSystem) {
        lightingSystem->shutdown();
    }
    if (renderStrategy) {
        renderStrategy->shutdown();
    }

    initialized = false;
    logger->logInfo("ModernRenderer3D завершил работу");
}

RendererType ModernRenderer3D::getType() const {
    // Тип определяется стратегией рендеринга
    return RendererType::OpenGL; // TODO: Получать от стратегии
}

bool ModernRenderer3D::supportsFeature(RenderingFeature feature) const {
    if (!renderStrategy) {
        return false;
    }
    return renderStrategy->supportsFeature(feature);
}

std::string ModernRenderer3D::getName() const {
    if (renderStrategy) {
        return "ModernRenderer3D (" + renderStrategy->getName() + ")";
    }
    return "ModernRenderer3D";
}

std::string ModernRenderer3D::getApiVersion() const {
    // TODO: Получать от стратегии рендеринга
    return "OpenGL 4.6";
}

bool ModernRenderer3D::isReady() const {
    return initialized && renderStrategy != nullptr;
}

RenderingStats ModernRenderer3D::getStats() const {
    if (statistics) {
        return statistics->getStats();
    }
    return RenderingStats{};
}

// IInitializable interface implementation
bool ModernRenderer3D::isInitialized() const {
    return initialized;
}

// IConfigurable interface implementation
void ModernRenderer3D::setConfigParameter(const std::string& key, const std::any& value) {
    configParameters[key] = value;
    logger->logDebug("Установлен параметр конфигурации рендерера: " + key);

    // Применение некоторых параметров немедленно
    try {
        if (key == "width" && std::any_cast<int>(value) > 0) {
            config.width = std::any_cast<int>(value);
        } else if (key == "height" && std::any_cast<int>(value) > 0) {
            config.height = std::any_cast<int>(value);
        } else if (key == "vsync") {
            config.vsync = std::any_cast<bool>(value);
        } else if (key == "msaa_samples") {
            config.msaaSamples = std::any_cast<int>(value);
        }
    } catch (const std::bad_any_cast& e) {
        logger->logWarning("Неверный тип параметра конфигурации: " + key);
    }
}

std::any ModernRenderer3D::getConfigParameter(const std::string& key) const {
    auto it = configParameters.find(key);
    if (it != configParameters.end()) {
        return it->second;
    }
    throw std::runtime_error("Параметр конфигурации рендерера не найден: " + key);
}

bool ModernRenderer3D::hasConfigParameter(const std::string& key) const {
    return configParameters.find(key) != configParameters.end();
}

bool ModernRenderer3D::loadConfig(const std::string& configPath) {
    logger->logInfo("Загрузка конфигурации рендерера из: " + configPath);
    
    // TODO: Реализовать загрузку конфигурации из файла
    // Пока используем значения по умолчанию
    setConfigParameter("width", config.width);
    setConfigParameter("height", config.height);
    setConfigParameter("vsync", config.vsync);
    setConfigParameter("msaa_samples", config.msaaSamples);
    setConfigParameter("enable_validation", config.enableValidation);

    return true;
}

bool ModernRenderer3D::saveConfig(const std::string& configPath) const {
    logger->logInfo("Сохранение конфигурации рендерера в: " + configPath);
    
    // TODO: Реализовать сохранение конфигурации в файл
    return true;
}

// Управление стратегиями (Strategy Pattern + OCP)
void ModernRenderer3D::setRenderStrategy(std::shared_ptr<IRenderStrategy> newStrategy) {
    if (!newStrategy) {
        logger->logWarning("Попытка установить nullptr стратегию рендеринга");
        return;
    }

    logger->logInfo("Смена стратегии рендеринга с '" + 
                    (renderStrategy ? renderStrategy->getName() : "none") + 
                    "' на '" + newStrategy->getName() + "'");

    // Завершаем работу старой стратегии
    if (renderStrategy && initialized) {
        renderStrategy->shutdown();
    }

    // Устанавливаем новую стратегию
    renderStrategy = newStrategy;

    // Инициализируем новую стратегию, если рендерер уже инициализирован
    if (initialized && !renderStrategy->initialize()) {
        logger->logError("Ошибка инициализации новой стратегии рендеринга: " + newStrategy->getName());
    }
}

// Управление пост-процессинг эффектами (OCP)
void ModernRenderer3D::addPostProcessEffect(std::shared_ptr<IPostProcessEffect> effect) {
    if (!effect) {
        logger->logWarning("Попытка добавить nullptr пост-процессинг эффект");
        return;
    }

    // Проверяем, не добавлен ли уже этот эффект
    auto it = std::find(postProcessEffects.begin(), postProcessEffects.end(), effect);
    if (it != postProcessEffects.end()) {
        logger->logWarning("Пост-процессинг эффект уже добавлен: " + effect->getName());
        return;
    }

    postProcessEffects.push_back(effect);
    logger->logInfo("Добавлен пост-процессинг эффект: " + effect->getName());

    // Если рендерер уже инициализирован, инициализируем новый эффект
    if (initialized && !effect->initialize()) {
        logger->logError("Ошибка инициализации пост-процессинг эффекта: " + effect->getName());
    }
}

void ModernRenderer3D::removePostProcessEffect(std::shared_ptr<IPostProcessEffect> effect) {
    if (!effect) {
        return;
    }

    auto it = std::find(postProcessEffects.begin(), postProcessEffects.end(), effect);
    if (it != postProcessEffects.end()) {
        effect->shutdown();
        postProcessEffects.erase(it);
        logger->logInfo("Удален пост-процессинг эффект: " + effect->getName());
    }
}

// Приватные методы
bool ModernRenderer3D::initializeGraphicsAPI() {
    logger->logInfo("Инициализация графического API...");
    
    // TODO: Реализовать инициализацию конкретного графического API
    // Это может быть OpenGL, Vulkan, DirectX12 и т.д.
    
    logger->logInfo("Графический API инициализирован");
    return true;
}

void ModernRenderer3D::applyPostProcessEffects() {
    // Применяем только активные эффекты
    for (auto& effect : postProcessEffects) {
        if (effect->isEnabled()) {
            // TODO: Реализовать применение эффекта
            // effect->apply(inputTexture, outputTexture);
        }
    }
}

}  // namespace Rendering
}  // namespace HyperEngine
