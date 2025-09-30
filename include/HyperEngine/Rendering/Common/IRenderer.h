
#pragma once

#include <memory>
#include <string>
#include <vector>
#include "HyperEngine/Math/Vector3.h"

namespace HyperEngine::Rendering {

// Forward declarations
struct FrameData;
enum class RendererType;
enum class RenderingFeature;
struct RendererConfig;

/**
 * @brief Базовый интерфейс для всех рендереров
 *
 * Применяет Open/Closed Principle - закрыт для модификации,
 * открыт для расширения через наследование.
 *
 * Этот интерфейс определяет контракт, который должен выполнять
 * любой рендерер в системе HyperEngine.
 */
class IRenderer {
  public:
    virtual ~IRenderer() = default;

    /**
     * @brief Инициализация рендерера
     * @return true если инициализация прошла успешно
     */
    virtual bool initialize() = 0;

    /**
     * @brief Рендеринг кадра
     * @param frameData Данные для рендеринга кадра
     */
    virtual void renderFrame(const FrameData& frameData) = 0;

    /**
     * @brief Завершение работы рендерера
     */
    virtual void shutdown() = 0;

    /**
     * @brief Получить тип рендерера
     * @return Тип рендерера (Vulkan, OpenGL, DirectX12, etc.)
     */
    virtual RendererType getType() const = 0;

    /**
     * @brief Проверить поддержку функции
     * @param feature Функция для проверки
     * @return true если функция поддерживается
     */
    virtual bool supportsFeature(RenderingFeature feature) const = 0;

    /**
     * @brief Получить имя рендерера для отладки
     * @return Человекочитаемое имя рендерера
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Получить версию API рендерера
     * @return Строка с версией (например, "Vulkan 1.3", "OpenGL 4.6")
     */
    virtual std::string getApiVersion() const = 0;

    /**
     * @brief Проверить готовность к рендерингу
     * @return true если рендерер готов к работе
     */
    virtual bool isReady() const = 0;

    /**
     * @brief Проверить инициализацию рендерера
     * @return true если рендерер инициализирован
     */
    virtual bool isInitialized() const = 0;

    /**
     * @brief Начать рендеринг кадра
     */
    virtual void beginFrame() = 0;

    /**
     * @brief Завершить рендеринг кадра
     */
    virtual void endFrame() = 0;

    /**
     * @brief Получить статистику производительности
     * @return Структура с метриками производительности
     */
    virtual struct RenderingStats getStats() const = 0;
};

/**
 * @brief Типы рендереров
 */
enum class RendererType {
    OpenGL,     ///< OpenGL рендерер
    Vulkan,     ///< Vulkan рендерер
    DirectX12,  ///< DirectX 12 рендерер
    Metal,      ///< Metal рендерер (macOS/iOS)
    Software    ///< Программный рендерер
};

/**
 * @brief Функции рендеринга
 */
enum class RenderingFeature {
    RayTracing,           ///< Аппаратная трассировка лучей
    VariableRateShading,  ///< Variable Rate Shading
    MeshShaders,          ///< Mesh шейдеры
    ComputeShaders,       ///< Вычислительные шейдеры
    AsyncCompute,         ///< Асинхронные вычисления
    MultiDrawIndirect,    ///< Multi-draw indirect
    BindlessTextures,     ///< Bindless текстуры
    GaussianSplatting,    ///< 3D Gaussian Splatting
    AIUpscaling,          ///< AI-based upscaling (DLSS/FSR)
    TemporalEffects       ///< Temporal эффекты
};

/**
 * @brief Данные кадра для рендеринга
 */
struct FrameData {
    struct {
        Math::Vector3 position{0, 0, 5};  ///< Позиция камеры
        Math::Vector3 target{0, 0, 0};    ///< Цель камеры
        Math::Vector3 up{0, 1, 0};        ///< Вектор "вверх"
        float fov = 60.0f;                ///< Поле зрения в градусах
        float nearPlane = 0.1f;           ///< Ближняя плоскость отсечения
        float farPlane = 1000.0f;         ///< Дальняя плоскость отсечения
    } camera;

    struct {
        int width = 1920;   ///< Ширина render target
        int height = 1080;  ///< Высота render target
    } renderTargetSize;

    struct {
        float deltaTime = 0.016f;  ///< Время кадра в секундах
        uint64_t frameNumber = 0;  ///< Номер кадра
    } timing;

    struct {
        bool enableRayTracing = true;     ///< Включить ray tracing
        bool enableUpscaling = true;      ///< Включить upscaling
        bool enableDenoising = true;      ///< Включить деноизинг
        bool enableDebugOverlay = false;  ///< Включить отладочный overlay
    } renderingOptions;
};

/**
 * @brief Конфигурация рендерера
 */
struct RendererConfig {
    int width = 1920;               ///< Ширина окна
    int height = 1080;              ///< Высота окна
    bool fullscreen = false;        ///< Полноэкранный режим
    bool vsync = true;              ///< Вертикальная синхронизация
    int msaaSamples = 1;            ///< Количество MSAA сэмплов
    bool enableValidation = false;  ///< Включить validation layers
    std::string deviceName;         ///< Предпочитаемое устройство
};

/**
 * @brief Статистика производительности рендеринга
 */
struct RenderingStats {
    float frameTime = 0.0f;      ///< Время кадра в миллисекундах
    float fps = 0.0f;            ///< Кадры в секунду
    uint32_t drawCalls = 0;      ///< Количество draw calls
    uint32_t primitives = 0;     ///< Количество примитивов
    size_t memoryUsed = 0;       ///< Использованная память в байтах
    size_t memoryTotal = 0;      ///< Общая доступная память
    uint32_t gaussianCount = 0;  ///< Количество Gaussian'ов (если применимо)
    float upscaleFactor = 1.0f;  ///< Коэффициент upscaling
};

}  // namespace HyperEngine::Rendering
