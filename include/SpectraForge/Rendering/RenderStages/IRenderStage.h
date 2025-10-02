
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "SpectraForge/Math/Matrix4.h"
#include "SpectraForge/Math/Vector3.h"
#include "SpectraForge/Rendering/Common/IResourceManager.h"

namespace SpectraForge::Rendering {

// Forward declarations
struct RenderContext;
class IResourceManager;

/**
 * @brief Базовый интерфейс для этапов рендеринга
 *
 * Применяет Single Responsibility Principle -
 * каждый этап имеет единственную ответственность.
 *
 * Также применяет Chain of Responsibility pattern -
 * этапы могут быть скомбинированы в цепочку.
 */
class IRenderStage {
  public:
    virtual ~IRenderStage() = default;

    /**
     * @brief Инициализация этапа
     * @param resourceManager Менеджер ресурсов для создания буферов/текстур
     * @return true если инициализация прошла успешно
     */
    virtual bool initialize(std::shared_ptr<IResourceManager> resourceManager) = 0;

    /**
     * @brief Выполнить этап рендеринга
     * @param context Контекст рендеринга с входными и выходными данными
     */
    virtual void execute(RenderContext& context) = 0;

    /**
     * @brief Завершение работы этапа
     */
    virtual void shutdown() = 0;

    /**
     * @brief Получить имя этапа
     * @return Уникальное имя этапа для отладки
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Проверить готовность к выполнению
     * @return true если этап готов к выполнению
     */
    virtual bool isReady() const = 0;

    /**
     * @brief Получить приоритет этапа
     * @return Приоритет для сортировки (меньше = выполняется раньше)
     */
    virtual int getPriority() const = 0;

    /**
     * @brief Проверить зависимости от других этапов
     * @return Список имен этапов, которые должны выполниться раньше
     */
    virtual std::vector<std::string> getDependencies() const = 0;

    /**
     * @brief Получить статистику производительности этапа
     * @return Время выполнения в миллисекундах
     */
    virtual float getExecutionTime() const = 0;

    /**
     * @brief Проверить поддержку функции
     * @param feature Функция для проверки
     * @return true если функция поддерживается этим этапом
     */
    virtual bool supportsFeature(const std::string& feature) const = 0;
};

/**
 * @brief Контекст рендеринга
 *
 * Содержит входные и выходные данные для этапов рендеринга.
 * Этапы могут читать данные предыдущих этапов и записывать
 * данные для следующих этапов.
 */
struct RenderContext {
    // === Входные данные ===
    struct {
        Math::Vector3 cameraPosition{0, 0, 5};
        Math::Vector3 cameraTarget{0, 0, 0};
        Math::Vector3 cameraUp{0, 1, 0};
        float fieldOfView = 60.0f;
        float nearPlane = 0.1f;
        float farPlane = 1000.0f;
        Math::Matrix4 viewMatrix;
        Math::Matrix4 projectionMatrix;
        Math::Matrix4 viewProjectionMatrix;
    } camera;

    struct {
        int width = 1920;
        int height = 1080;
        float aspectRatio = 16.0f / 9.0f;
    } viewport;

    struct {
        float deltaTime = 0.016f;
        uint64_t frameNumber = 0;
        float totalTime = 0.0f;
    } timing;

    // === Промежуточные данные между этапами ===
    struct {
        TextureHandle primaryColorBuffer = INVALID_HANDLE;  ///< Результат первичной растеризации
        TextureHandle primaryDepthBuffer = INVALID_HANDLE;  ///< Depth buffer первичной растеризации
        TextureHandle motionVectors = INVALID_HANDLE;  ///< Motion vectors для temporal effects
        TextureHandle normalBuffer = INVALID_HANDLE;  ///< Буфер нормалей для деноизинга
        BufferHandle gaussianBuffer = INVALID_HANDLE;  ///< Буфер с данными Gaussian'ов
        uint32_t gaussianCount = 0;                    ///< Количество Gaussian'ов
    } primaryRaster;

    struct {
        TextureHandle reflections = INVALID_HANDLE;         ///< Ray-traced отражения
        TextureHandle shadows = INVALID_HANDLE;             ///< Ray-traced тени
        TextureHandle globalIllumination = INVALID_HANDLE;  ///< Global illumination
        TextureHandle ambientOcclusion = INVALID_HANDLE;    ///< Ray-traced AO
        BufferHandle rayCounters = INVALID_HANDLE;  ///< Счетчики лучей для оптимизации
    } rayTracing;

    struct {
        TextureHandle denoisedReflections = INVALID_HANDLE;  ///< Очищенные отражения
        TextureHandle denoisedShadows = INVALID_HANDLE;      ///< Очищенные тени
        TextureHandle denoisedGI = INVALID_HANDLE;           ///< Очищенный GI
        TextureHandle varianceBuffer = INVALID_HANDLE;  ///< Буфер variance для деноизинга
    } denoising;

    struct {
        TextureHandle upscaledImage = INVALID_HANDLE;  ///< Результат upscaling
        TextureHandle motionVectorsHiRes = INVALID_HANDLE;  ///< Motion vectors высокого разрешения
        float upscaleFactor = 1.0f;  ///< Коэффициент увеличения
        int outputWidth = 1920;      ///< Итоговая ширина
        int outputHeight = 1080;     ///< Итоговая высота
    } upscaling;

    // === Выходные данные ===
    struct {
        TextureHandle finalImage = INVALID_HANDLE;    ///< Финальное изображение
        TextureHandle debugOverlay = INVALID_HANDLE;  ///< Отладочный overlay
    } output;

    // === Настройки рендеринга ===
    struct {
        bool enableRayTracing = true;  ///< Включить ray tracing
        bool enableDenoising = true;   ///< Включить AI деноизинг
        bool enableUpscaling = true;   ///< Включить upscaling
        bool enableDebugOverlay = false;  ///< Показать отладочную информацию
        bool enableProfiling = false;  ///< Включить профилирование
    } settings;

    // === Ресурсы ===
    std::shared_ptr<IResourceManager> resourceManager;  ///< Менеджер ресурсов

    // === Отладка и профилирование ===
    struct {
        std::unordered_map<std::string, float> stageTimes;  ///< Время выполнения этапов
        uint32_t drawCalls = 0;                             ///< Количество draw calls
        uint32_t computeDispatches = 0;  ///< Количество compute dispatches
        size_t memoryUsed = 0;           ///< Использованная память
    } debug;
};

}  // namespace SpectraForge::Rendering
