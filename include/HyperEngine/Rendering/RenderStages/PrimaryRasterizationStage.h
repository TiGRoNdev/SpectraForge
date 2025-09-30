#pragma once

#include <chrono>
#include <memory>
#include "IRenderStage.h"

namespace HyperEngine::Rendering {

// Forward declarations
class FlashGSSplatter;

/**
 * @brief Этап первичной растеризации
 *
 * Применяет Single Responsibility Principle -
 * отвечает ТОЛЬКО за 3D Gaussian Splatting.
 *
 * Этот этап рендерит основную геометрию сцены используя
 * алгоритм 3D Gaussian Splatting для максимальной производительности.
 */
class PrimaryRasterizationStage : public IRenderStage {
  private:
    std::unique_ptr<FlashGSSplatter> splatter_;
    std::shared_ptr<IResourceManager> resourceManager_;

    // Состояние этапа
    bool initialized_ = false;
    bool ready_ = false;

    // Статистика производительности
    mutable float lastExecutionTime_ = 0.0f;
    mutable uint32_t gaussiansRendered_ = 0;
    mutable std::vector<float> executionTimeHistory_;
    static constexpr size_t HISTORY_SIZE = 60;

    // Настройки Gaussian Splatting
    struct GaussianSettings {
        float lodBias = 0.0f;              ///< LOD bias для дальних Gaussian'ов
        float cullingThreshold = 0.01f;    ///< Порог отсечения по размеру
        bool enableOcclusion = true;       ///< Включить occlusion culling
        bool enableFrustumCulling = true;  ///< Включить frustum culling
        uint32_t maxGaussians = 1000000;  ///< Максимальное количество Gaussian'ов
        bool enableLOD = true;            ///< Включить level-of-detail
        float lodDistance = 100.0f;  ///< Расстояние начала LOD
    } settings_;

  public:
    PrimaryRasterizationStage();
    ~PrimaryRasterizationStage() override;

    // Запретить копирование
    PrimaryRasterizationStage(const PrimaryRasterizationStage&) = delete;
    PrimaryRasterizationStage& operator=(const PrimaryRasterizationStage&) = delete;

    // Разрешить перемещение
    PrimaryRasterizationStage(PrimaryRasterizationStage&&) noexcept;
    PrimaryRasterizationStage& operator=(PrimaryRasterizationStage&&) noexcept;

    // Реализация IRenderStage
    bool initialize(std::shared_ptr<IResourceManager> resourceManager) override;
    void execute(RenderContext& context) override;
    void shutdown() override;
    std::string getName() const override { return "PrimaryRasterization"; }
    bool isReady() const override { return ready_; }
    int getPriority() const override { return 1000; }  // Высокий приоритет - выполняется первым
    std::vector<std::string> getDependencies() const override { return {}; }  // Нет зависимостей
    float getExecutionTime() const override { return lastExecutionTime_; }
    bool supportsFeature(const std::string& feature) const override;

    // Специфичные методы этапа

    /**
     * @brief Загрузить Gaussian данные из файла
     * @param filePath Путь к файлу с Gaussian данными
     * @return true если загрузка прошла успешно
     */
    bool loadGaussianData(const std::string& filePath);

    /**
     * @brief Установить Gaussian данные напрямую
     * @param gaussians Указатель на данные Gaussian'ов
     * @param count Количество Gaussian'ов
     * @return true если данные установлены успешно
     */
    bool setGaussianData(const void* gaussians, uint32_t count);

    /**
     * @brief Получить настройки Gaussian Splatting
     * @return Ссылка на настройки
     */
    GaussianSettings& getSettings() { return settings_; }
    const GaussianSettings& getSettings() const { return settings_; }

    /**
     * @brief Получить количество отрендеренных Gaussian'ов в последнем кадре
     * @return Количество Gaussian'ов
     */
    uint32_t getRenderedGaussianCount() const { return gaussiansRendered_; }

    /**
     * @brief Получить среднее время выполнения
     * @return Время в миллисекундах
     */
    float getAverageExecutionTime() const;

    /**
     * @brief Включить/выключить отладочную визуализацию
     * @param enable Включить отладку
     */
    void setDebugVisualization(bool enable);

    /**
     * @brief Сбросить статистику производительности
     */
    void resetPerformanceStats();

  private:
    /**
     * @brief Обновление статистики
     * @param executionTime Время выполнения
     * @param gaussianCount Количество Gaussian'ов
     */
    void updateStats(float executionTime, uint32_t gaussianCount);

    /**
     * @brief Валидация входных данных
     * @param context Контекст рендеринга
     * @return true если данные валидны
     */
    bool validateInputs(const RenderContext& context) const;

    /**
     * @brief Подготовка Gaussian данных для рендеринга
     * @param context Контекст рендеринга
     * @return true если подготовка прошла успешно
     */
    bool prepareGaussianData(RenderContext& context);

    /**
     * @brief Выполнение culling операций
     * @param context Контекст рендеринга
     * @return Количество видимых Gaussian'ов
     */
    uint32_t performCulling(const RenderContext& context);

    /**
     * @brief Сортировка Gaussian'ов по глубине
     * @param context Контекст рендеринга
     */
    void sortGaussiansByDepth(const RenderContext& context);
};

}  // namespace HyperEngine::Rendering
