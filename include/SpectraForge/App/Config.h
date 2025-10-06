/**
 * @file Config.h
 * @brief Конфигурация высокого уровня приложения SpectraForge (РАСШИРЕННАЯ ВЕРСИЯ)
 */
#pragma once
#include <string>

namespace SpectraForge {
namespace App {

/**
 * @brief Конфигурация приложения (расширенная для debug support)
 */
struct AppConfig {
    // Основные параметры окна
    int window_width = 1280;
    int window_height = 720;
    std::string window_title = "SpectraForge";
    bool vsync = true;
    bool debug = false;
    
    // ДОБАВЛЕНО: Debug и validation параметры
    bool enableValidationLayers = false;    // Vulkan validation layers
    bool enableDebugOutput = false;         // Подробное логирование
    bool enableFrameDebugger = false;       // Frame-by-frame analysis
    bool enableGPUTiming = false;           // GPU performance timing
    bool enableMemoryTracking = false;     // Memory usage tracking
    
    // ДОБАВЛЕНО: Rendering параметры
    bool enableWireframe = false;           // Wireframe rendering mode
    bool enableBackfaceCulling = true;      // Backface culling
    bool enableDepthTest = true;            // Depth testing
    bool enableAlphaBlending = true;        // Alpha blending
    
    // ДОБАВЛЕНО: Performance параметры
    int targetFPS = 60;                     // Target FPS
    int maxTrianglesPerFrame = 100000;      // Triangle budget
    bool enableEarlyTermination = true;     // Early alpha termination
    bool enableTileCulling = true;          // Mobile GPU optimization
    
    // ДОБАВЛЕНО: Scene параметры
    float cameraSpeed = 5.0f;               // Camera movement speed
    float mouseSensitivity = 0.1f;          // Mouse sensitivity
    bool invertMouseY = false;              // Invert Y-axis
    
    // ДОБАВЛЕНО: Background color
    float backgroundColor[4] = {0.1f, 0.2f, 0.3f, 1.0f}; // RGBA background
    
    // ДОБАВЛЕНО: Debug режимы
    enum class DebugMode {
        NORMAL = 0,              // Обычный рендеринг
        SDF_VISUALIZATION = 1,   // SDF visualization (красные области)
        BARYCENTRIC = 2,         // Barycentric coordinates
        DEPTH_VISUALIZATION = 3, // Depth buffer visualization
        WIREFRAME = 4,           // Wireframe overlay
        TRIANGLE_COUNT = 5       // Triangle count heatmap
    };
    DebugMode debugMode = DebugMode::NORMAL;
};

} // namespace App
} // namespace SpectraForge