
/**
 * @file Engine.h
 * @brief Главный заголовочный файл движка HyperEngine
 *
 * Этот файл объединяет все основные компоненты движка для удобного включения
 */

#pragma once

// Математические библиотеки
#include "SpectraForge/Math/Matrix4.h"
#include "SpectraForge/Math/Vector3.h"

// Система рендеринга
#include "SpectraForge/Rendering/Camera3D.h"
#include "SpectraForge/Rendering/Material3D.h"
#include "SpectraForge/Rendering/Mesh3D.h"
#include "SpectraForge/Rendering/OptimalRenderer3D.h"
#include "SpectraForge/Rendering/Renderer3D.h"
#include "SpectraForge/Rendering/Shader3D.h"

// Vulkan рендеринг
#include "SpectraForge/Vulkan/ResourceManager.h"
#include "SpectraForge/Vulkan/VulkanRenderer.h"

// CUDA интеграция (если доступна)
#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
#include "SpectraForge/CUDA/CudaInterop.h"
#include "SpectraForge/CUDA/FlashGSSplatter.h"
#endif

// Физика
#include "SpectraForge/Physics/Physics3D.h"

// Система ввода
#include "SpectraForge/Input/Input3D.h"

// Ядро движка
#include "SpectraForge/Core/Window.h"

/**
 * @namespace SpectraForge
 * @brief Основное пространство имен движка
 */
namespace SpectraForge {

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

}  // namespace SpectraForge
