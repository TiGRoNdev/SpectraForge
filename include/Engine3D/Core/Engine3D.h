/**
 * @file Engine3D.h
 * @brief Главный заголовочный файл движка Engine3D
 * 
 * Этот файл объединяет все основные компоненты движка для удобного включения
 */

#pragma once

// Математические библиотеки
#include "Engine3D/Math/Vector3.h"
#include "Engine3D/Math/Matrix4.h"
// #include "Engine3D/Math/Transform.h" // Временно отключен

// Система рендеринга
#include "Engine3D/Rendering/Renderer3D.h"
#include "Engine3D/Rendering/Camera3D.h"
#include "Engine3D/Rendering/Mesh3D.h"
#include "Engine3D/Rendering/Material3D.h"
#include "Engine3D/Rendering/Shader3D.h"
#include "Engine3D/Rendering/OptimalRenderer3D.h"

// Vulkan рендеринг
#include "Engine3D/Vulkan/VulkanRenderer.h"
#include "Engine3D/Vulkan/ResourceManager.h"

// CUDA интеграция (если доступна)
#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
#include "Engine3D/CUDA/CudaInterop.h"
#include "Engine3D/CUDA/FlashGSSplatter.h"
#endif

// Физика
#include "Engine3D/Physics/Physics3D.h"

// Система ввода
#include "Engine3D/Input/Input3D.h"

// Ядро движка
#include "Engine3D/Core/Window.h"
// #include "Engine3D/Core/ResourceLoader.h" // TODO: Создать ResourceLoader.h если нужен

/**
 * @namespace Engine3D
 * @brief Основное пространство имен движка
 */
namespace Engine3D {

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

} // namespace Engine3D
