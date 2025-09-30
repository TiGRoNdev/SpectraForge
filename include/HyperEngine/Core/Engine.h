
/**
 * @file Engine.h
 * @brief Главный заголовочный файл движка HyperEngine
 *
 * Этот файл объединяет все основные компоненты движка для удобного включения
 */

#pragma once

// Математические библиотеки
#include "HyperEngine/Math/Matrix4.h"
#include "HyperEngine/Math/Vector3.h"

// Система рендеринга
#include "HyperEngine/Rendering/Camera3D.h"
#include "HyperEngine/Rendering/Material3D.h"
#include "HyperEngine/Rendering/Mesh3D.h"
#include "HyperEngine/Rendering/OptimalRenderer3D.h"
#include "HyperEngine/Rendering/Renderer3D.h"
#include "HyperEngine/Rendering/Shader3D.h"

// Vulkan рендеринг
#include "HyperEngine/Vulkan/ResourceManager.h"
#include "HyperEngine/Vulkan/VulkanRenderer.h"

// CUDA интеграция (если доступна)
#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
#include "HyperEngine/CUDA/CudaInterop.h"
#include "HyperEngine/CUDA/FlashGSSplatter.h"
#endif

// Физика
#include "HyperEngine/Physics/Physics3D.h"

// Система ввода
#include "HyperEngine/Input/Input3D.h"

// Ядро движка
#include "HyperEngine/Core/Window.h"

/**
 * @namespace HyperEngine
 * @brief Основное пространство имен движка
 */
namespace HyperEngine {

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
 * @brief Основной класс движка
 *
 * Управляет инициализацией и работой всех подсистем движка
 */
class Engine {
  public:
    /**
     * @brief Инициализация движка
     * @param windowWidth Ширина окна
     * @param windowHeight Высота окна
     * @param windowTitle Заголовок окна
     * @return true если инициализация успешна
     */
    static bool initialize(int windowWidth, int windowHeight, const char* windowTitle);

    /**
     * @brief Завершение работы движка
     */
    static void shutdown();

    /**
     * @brief Получение информации о движке
     * @return Структура с информацией о версии
     */
    static EngineInfo getEngineInfo();

    /**
     * @brief Проверка инициализации движка
     * @return true если движок инициализирован
     */
    static bool isInitialized();

  private:
    static bool initialized;
};

}  // namespace HyperEngine
