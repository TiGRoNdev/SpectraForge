/**
 * @file IApp.h
 * @brief Публичный интерфейс высокого уровня для приложения SpectraForge
 */

#pragma once

#include <memory>
#include <string>
#include "SpectraForge/Vulkan/SceneManager.h"

namespace SpectraForge {
namespace App {

/**
 * @brief Интерфейс высокоуровневого фасада приложения
 */
class IApp {
  public:
    virtual ~IApp() = default;

    /**
     * @brief Инициализация подсистем и запуск окна
     * @return true при успешной инициализации
     */
    virtual bool init() = 0;

    /**
     * @brief Загрузка сцены через адаптер сцены
     * @param data Данные сцены
     * @return true если сцена загружена успешно
     */
    virtual bool load_scene(const Vulkan::SceneData &data) = 0;

    /**
     * @brief Обновление игрового цикла
     * @param delta_time Дельта времени в секундах
     */
    virtual void update(float delta_time) = 0;

    /**
     * @brief Рендеринг кадра
     */
    virtual void render() = 0;

    /**
     * @brief Корректное завершение работы и очистка ресурсов
     */
    virtual void shutdown() = 0;
};

}  // namespace App
}  // namespace SpectraForge


