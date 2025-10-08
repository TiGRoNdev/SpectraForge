#pragma once

#include <cstdint>
#include <string>

namespace spectraforge {
namespace rendering {

/**
 * @brief Performance statistics collector для Triangle Splatting
 * 
 * Ответственность: Сбор и отображение метрик производительности
 * 
 * SOLID Compliance:
 * - SRP: Только статистика (никаких side effects)
 * - OCP: Можно расширить новыми метриками
 */
class TriangleSplattingStatistics {
public:
    /**
     * @brief Структура со статистикой
     */
    struct Stats {
        // Triangle counts
        uint32_t totalTriangles = 0;
        uint32_t visibleTriangles = 0;
        uint32_t culledTriangles = 0;
        float cullingRatio = 0.0f; // culledTriangles / totalTriangles
        
        // Timings (milliseconds)
        float frustumCullingTimeMs = 0.0f;
        float depthSortingTimeMs = 0.0f;
        float rasterizationTimeMs = 0.0f;
        float totalFrameTimeMs = 0.0f;
        
        // Memory usage (bytes)
        uint64_t memoryUsageBytes = 0;
        
        // Frame rate
        float fps = 0.0f;
    };
    
    /**
     * @brief Сброс всех статистик
     */
    void reset();
    
    /**
     * @brief Обновление triangle counts
     */
    void update(uint32_t totalTriangles, uint32_t visibleTriangles);
    
    /**
     * @brief Установка timings
     */
    void setTimings(float frustumMs, float sortMs, float rasterMs);
    
    /**
     * @brief Установка memory usage
     */
    void setMemoryUsage(uint64_t bytes);
    
    /**
     * @brief Вычисление FPS на основе frame time
     */
    void calculateFPS();
    
    /**
     * @brief Получить статистику
     */
    const Stats& getStats() const { return stats_; }
    
    /**
     * @brief Вывести статистику в console
     */
    void printStats() const;
    
    /**
     * @brief Экспорт статистики в JSON формат
     */
    std::string toJSON() const;

private:
    Stats stats_;
};

} // namespace rendering
} // namespace spectraforge

