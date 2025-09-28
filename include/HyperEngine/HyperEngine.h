/**
 * @file HyperEngine.h
 * @brief Главный заголовочный файл движка HyperEngine
 *
 * Этот файл объединяет все основные компоненты движка для удобного включения
 */

#pragma once

// Версия движка
#define HYPERENGINE_VERSION_MAJOR 1
#define HYPERENGINE_VERSION_MINOR 0
#define HYPERENGINE_VERSION_PATCH 0
#define HYPERENGINE_VERSION_STRING "1.0.0"

// Основные модули
#include "Core/Component.h"
#include "Core/Console.h"
#include "Core/Engine.h"
#include "Core/GameObject3D.h"
#include "Core/SafeConsole.h"
#include "Core/Transform3D.h"
#include "Core/Window.h"

// Математическая библиотека
#include "Math/Math.h"
#include "Math/MathConstants.h"
#include "Math/Matrix4.h"
#include "Math/Quaternion.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"

// Система рендеринга
#include "Rendering/Camera3D.h"
#include "Rendering/Common/IRenderer.h"
#include "Rendering/Gaussian3D.h"
#include "Rendering/HybridRenderer3D.h"
#include "Rendering/Material3D.h"
#include "Rendering/Mesh3D.h"
#include "Rendering/OptimalRenderer3D.h"
#include "Rendering/Renderer3D.h"
#include "Rendering/RendererAdapter.h"
#include "Rendering/RendererFactory.h"
#include "Rendering/Shader3D.h"

// Vulkan рендеринг
#include "Vulkan/HardwareDetector.h"
#include "Vulkan/ResourceManager.h"
#include "Vulkan/SceneManager.h"
#include "Vulkan/VulkanEngine.h"
#include "Vulkan/VulkanRenderer.h"

// CUDA интеграция (если доступна)
#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
#include "CUDA/CudaInterop.h"
#include "CUDA/FlashGSSplatter.h"
#endif

// OptiX ray tracing (если доступен)
#ifdef VULKAN_RENDERER_OPTIX_SUPPORT
#include "OptiX/DenoiseModule.h"
#include "OptiX/OptiXRayTracer.h"
#endif

// Upscaling технологии
#include "Upscaling/DLSSUpscaler.h"
#include "Upscaling/Upscaler.h"

// Физика
#include "Physics/Physics3D.h"

// Система ввода
#include "Input/Input3D.h"

/**
 * @namespace HyperEngine
 * @brief Основное пространство имен движка
 */
namespace HyperEngine {

/**
 * @brief Информация о версии движка
 */
struct EngineInfo {
    static constexpr int MAJOR_VERSION = HYPERENGINE_VERSION_MAJOR;
    static constexpr int MINOR_VERSION = HYPERENGINE_VERSION_MINOR;
    static constexpr int PATCH_VERSION = HYPERENGINE_VERSION_PATCH;
    static constexpr const char* VERSION_STRING = HYPERENGINE_VERSION_STRING;
    static constexpr const char* BUILD_DATE = __DATE__;
    static constexpr const char* BUILD_TIME = __TIME__;
};

/**
 * @brief Получение информации о движке
 * @return Структура с информацией о версии
 */
inline EngineInfo getEngineInfo() {
    return EngineInfo{};
}

}  // namespace HyperEngine
