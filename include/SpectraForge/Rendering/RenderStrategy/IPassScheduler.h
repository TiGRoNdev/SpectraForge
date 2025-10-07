#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace SpectraForge::Rendering {

/**
 * @brief Минимальный интерфейс планировщика проходов рендеринга
 *
 * Предоставляет стратегиям способ регистрировать проходы и задавать их порядок
 * выполнения без знания деталей backend API.
 */
class IPassScheduler {
  public:
    virtual ~IPassScheduler() = default;

    /**
     * @brief Зарегистрировать проход рендеринга
     * @param pass_name Уникальное имя прохода (для отладки/зависимостей)
     * @param priority Порядок выполнения (меньше = раньше)
     * @param execute_fn Функция, выполняющая проход
     * @return true, если регистрация прошла успешно
     */
    virtual bool register_pass(const std::string& pass_name,
                               int priority,
                               std::function<void()> execute_fn) = 0;

    /**
     * @brief Задать зависимость между проходами
     * @param pass_name Имя прохода, который зависит
     * @param depends_on Имя прохода, от которого зависит
     * @return true при успешном добавлении зависимости
     */
    virtual bool add_dependency(const std::string& pass_name, const std::string& depends_on) = 0;

    /**
     * @brief Выполнить все зарегистрированные проходы в корректном порядке
     */
    virtual void execute_all() = 0;
};

}  // namespace SpectraForge::Rendering


