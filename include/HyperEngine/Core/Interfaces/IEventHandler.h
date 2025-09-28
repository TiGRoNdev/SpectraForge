/**
 * @file IEventHandler.h
 * @brief Интерфейс для обработки событий
 *
 * Следует принципу ISP - разделяет обязанности обработки событий
 */

#pragma once

#include <functional>
#include <memory>

namespace HyperEngine {
namespace Core {
namespace Interfaces {

/**
 * @brief Базовый класс события
 */
class IEvent {
  public:
    virtual ~IEvent() = default;
    virtual const char* getEventType() const = 0;
};

/**
 * @brief Интерфейс для обработчиков событий
 *
 * Применяет принцип ISP - клиенты зависят только от методов обработки событий
 */
class IEventHandler {
  public:
    virtual ~IEventHandler() = default;

    /**
     * @brief Обработка события
     * @param event Событие для обработки
     * @return true если событие обработано
     */
    virtual bool handleEvent(const std::shared_ptr<IEvent>& event) = 0;

    /**
     * @brief Проверка возможности обработки типа события
     * @param eventType Тип события
     * @return true если может обработать
     */
    virtual bool canHandle(const char* eventType) const = 0;
};

/**
 * @brief Интерфейс для диспетчера событий
 */
class IEventDispatcher {
  public:
    virtual ~IEventDispatcher() = default;

    /**
     * @brief Регистрация обработчика событий
     * @param handler Обработчик событий
     */
    virtual void registerHandler(std::shared_ptr<IEventHandler> handler) = 0;

    /**
     * @brief Отправка события
     * @param event Событие для отправки
     */
    virtual void dispatchEvent(std::shared_ptr<IEvent> event) = 0;

    /**
     * @brief Удаление обработчика
     * @param handler Обработчик для удаления
     */
    virtual void unregisterHandler(std::shared_ptr<IEventHandler> handler) = 0;
};

}  // namespace Interfaces
}  // namespace Core
}  // namespace HyperEngine
