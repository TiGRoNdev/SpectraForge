#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace SpectraForge::Rendering {

class IRenderContextView;
class IPassScheduler;

/**
 * @brief Абстракция стратегии рендеринга (Strategy Pattern)
 *
 * Контракт позволяет подключать разные реализации пайплайнов
 * рендеринга (например, FreGS или Triangle) без изменения кода
 * движка. Предполагается, что реализация использует узкий
 * read-only интерфейс `IRenderContextView` и объявляет необходимые
 * проходы через `IPassScheduler`.
 */
class IRenderStrategy {
  public:
    virtual ~IRenderStrategy() = default;

    /**
     * @brief Инициализация стратегии
     * @param context_view Read-only доступ к устройству/свапчейну/аллокатору
     * @return true при успешной инициализации
     */
    virtual bool initialize(std::shared_ptr<IRenderContextView> context_view) = 0;

    /**
     * @brief Освобождение ресурсов стратегии
     */
    virtual void shutdown() = 0;

    /**
     * @brief Подготовка и регистрация проходов/пайплайнов
     * @param scheduler Планировщик для регистрации проходов и их порядка
     * @return true, если подготовка успешна
     */
    virtual bool prepare_pipelines(IPassScheduler& scheduler) = 0;

    /**
     * @brief Обновление состояния стратегии для кадра
     * @param frame_index Номер кадра (ring-buffer/frame-in-flight)
     * @return true, если обновление успешно
     */
    virtual bool update_for_frame(uint64_t frame_index) = 0;

    /**
     * @brief Запись команд рендеринга (command buffers / lists)
     *
     * Замечание: реализация самостоятельно получает необходимые
     * дескрипторы/ресурсы через `IRenderContextView`.
     */
    virtual void record_commands() = 0;

    /**
     * @brief Выполнение кадра (submit/present либо offscreen)
     */
    virtual void render_frame() = 0;

    /**
     * @brief Идентификатор стратегии для логирования/диагностики
     */
    virtual std::string strategy_name() const = 0;
};

}  // namespace SpectraForge::Rendering


