/**
 * @file IInitializable.h
 * @brief Интерфейс для инициализируемых компонентов
 *
 * Следует принципу ISP - разделяет обязанности инициализации
 */

#pragma once

namespace HyperEngine {
namespace Core {
namespace Interfaces {

/**
 * @brief Интерфейс для компонентов, требующих инициализации
 *
 * Применяет принцип ISP - клиенты зависят только от методов инициализации
 */
class IInitializable {
  public:
    virtual ~IInitializable() = default;

    /**
     * @brief Инициализация компонента
     * @return true если инициализация успешна
     */
    virtual bool initialize() = 0;

    /**
     * @brief Завершение работы компонента
     */
    virtual void shutdown() = 0;

    /**
     * @brief Проверка состояния инициализации
     * @return true если компонент инициализирован
     */
    virtual bool isInitialized() const = 0;
};

}  // namespace Interfaces
}  // namespace Core
}  // namespace HyperEngine
