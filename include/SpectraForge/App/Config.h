/**
 * @file Config.h
 * @brief Конфигурация высокого уровня приложения SpectraForge
 */

#pragma once

#include <string>

namespace SpectraForge {
namespace App {

/**
 * @brief Конфигурация приложения
 */
struct AppConfig {
    int window_width = 1280;
    int window_height = 720;
    std::string window_title = "SpectraForge";
    bool vsync = true;
    bool debug = false;
};

}  // namespace App
}  // namespace SpectraForge


