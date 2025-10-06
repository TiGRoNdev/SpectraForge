#pragma once
#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "SpectraForge/Math/Vector3.h"

namespace SpectraForge::Rendering {

// Forward declarations
struct FrameData;
enum class RendererType;
enum class RenderingFeature;
struct RendererConfig;

/**
 * @brief Базовый интерфейс для всех рендереров (РАСШИРЕННАЯ ВЕРСИЯ)
 */
class IRenderer {
public:
    virtual ~IRenderer() = default;

    // Основные методы рендеринга
    virtual bool initialize() = 0;
    virtual void renderFrame(const FrameData& frameData) = 0;
    virtual void shutdown() = 0;
    virtual RendererType getType() const = 0;
    virtual bool supportsFeature(RenderingFeature feature) const = 0;
    virtual std::string getName() const = 0;
    virtual std::string getApiVersion() const = 0;
    virtual bool isReady() const = 0;
    virtual bool isInitialized() const = 0;
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    virtual struct RenderingStats getStats() const = 0;

    // ДОБАВЛЕНО: Debug и visualization методы
    
    /**
     * @brief Установить режим отладки
     * @param mode 0=normal, 1=SDF, 2=barycentric, 3=depth, 4=wireframe
     */
    virtual void setDebugMode(int mode) = 0;
    
    /**
     * @brief Получить текущий режим отладки
     */
    virtual int getDebugMode() const = 0;
    
    /**
     * @brief Включить/выключить wireframe режим
     */
    virtual void enableWireframe(bool enable) = 0;
    
    /**
     * @brief Включить/выключить backface culling
     */
    virtual void enableBackfaceCulling(bool enable) = 0;
    
    /**
     * @brief Включить/выключить depth test
     */
    virtual void enableDepthTest(bool enable) = 0;
    
    /**
     * @brief Установить цвет фона
     */
    virtual void setBackgroundColor(float r, float g, float b, float a = 1.0f) = 0;
    
    /**
     * @brief Получить цвет фона
     */
    virtual glm::vec4 getBackgroundColor() const = 0;
    
    /**
     * @brief Установить viewport
     */
    virtual void setViewport(int x, int y, int width, int height) = 0;
    
    /**
     * @brief Включить/выключить alpha blending
     */
    virtual void enableAlphaBlending(bool enable) = 0;
    
    /**
     * @brief Установить triangle budget для performance tuning
     */
    virtual void setTriangleBudget(uint32_t maxTriangles) = 0;
    
    /**
     * @brief Включить/выключить early termination в alpha blending
     */
    virtual void enableEarlyTermination(bool enable) = 0;
    
    /**
     * @brief Получить подробную статистику производительности
     */
    virtual struct DetailedRenderingStats getDetailedStats() const = 0;
    
    /**
     * @brief Сохранить скриншот в файл
     */
    virtual bool saveScreenshot(const std::string& filename) const = 0;
    
    /**
     * @brief Получить данные framebuffer для анализа
     */
    virtual std::vector<uint8_t> getFramebufferData() const = 0;
    
    /**
     * @brief Установить debug callback для логирования
     */
    virtual void setDebugCallback(std::function<void(const std::string&)> callback) = 0;
    
    /**
     * @brief Принудительно обновить все uniform буферы
     */
    virtual void flushUniforms() = 0;
    
    /**
     * @brief Получить информацию о GPU
     */
    virtual struct GPUInfo getGPUInfo() const = 0;
};

/**
 * @brief Типы рендереров
 */
enum class RendererType {
    OpenGL,    ///< OpenGL рендерер
    Vulkan,    ///< Vulkan рендерер
    DirectX12, ///< DirectX 12 рендерер
    Metal,     ///< Metal рендерер (macOS/iOS)
    Software   ///< Программный рендерер
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
    TemporalEffects,      ///< Temporal эффекты
    TriangleSplatting     ///< Triangle Splatting support
};

/**
 * @brief Данные кадра для рендеринга
 */
struct FrameData {
    struct {
        Math::Vector3 position{0, 0, 5}; ///< Позиция камеры
        Math::Vector3 target{0, 0, 0};   ///< Цель камеры
        Math::Vector3 up{0, 1, 0};       ///< Вектор "вверх"
        float fov = 60.0f;               ///< Поле зрения в градусах
        float nearPlane = 0.1f;          ///< Ближняя плоскость отсечения
        float farPlane = 1000.0f;        ///< Дальняя плоскость отсечения
        glm::mat4 viewMatrix{1.0f};      ///< View matrix
        glm::mat4 projectionMatrix{1.0f}; ///< Projection matrix
    } camera;

    struct {
        int width = 1920;  ///< Ширина render target
        int height = 1080; ///< Высота render target
    } renderTargetSize;

    struct {
        float deltaTime = 0.016f;    ///< Время кадра в секундах
        uint64_t frameNumber = 0;    ///< Номер кадра
    } timing;

    struct {
        bool enableRayTracing = true;    ///< Включить ray tracing
        bool enableUpscaling = true;     ///< Включить upscaling
        bool enableDenoising = true;     ///< Включить деноизинг
        bool enableDebugOverlay = false; ///< Включить отладочный overlay
        int debugMode = 0;               ///< Режим отладки
        glm::vec4 backgroundColor{0.1f, 0.2f, 0.3f, 1.0f}; ///< Цвет фона
    } renderingOptions;
};

/**
 * @brief Конфигурация рендерера
 */
struct RendererConfig {
    int width = 1920;              ///< Ширина окна
    int height = 1080;             ///< Высота окна
    bool fullscreen = false;       ///< Полноэкранный режим
    bool vsync = true;             ///< Вертикальная синхронизация
    int msaaSamples = 1;           ///< Количество MSAA сэмплов
    bool enableValidation = false; ///< Включить validation layers
    std::string deviceName;        ///< Предпочитаемое устройство
};

/**
 * @brief Статистика производительности рендеринга
 */
struct RenderingStats {
    float frameTime = 0.0f;         ///< Время кадра в миллисекундах
    float fps = 0.0f;               ///< Кадры в секунду
    uint32_t drawCalls = 0;         ///< Количество draw calls
    uint32_t primitives = 0;        ///< Количество примитивов
    size_t memoryUsed = 0;          ///< Использованная память в байтах
    size_t memoryTotal = 0;         ///< Общая доступная память
    uint32_t gaussianCount = 0;     ///< Количество Gaussian'ов
    float upscaleFactor = 1.0f;     ///< Коэффициент upscaling
};

/**
 * @brief ДОБАВЛЕНО: Подробная статистика производительности
 */
struct DetailedRenderingStats : public RenderingStats {
    // Triangle Splatting специфичные метрики
    uint32_t visibleTriangles = 0;      ///< Видимые треугольники после culling
    uint32_t culledTriangles = 0;       ///< Отбракованные треугольники
    uint32_t rasterizedPixels = 0;      ///< Обработанные пиксели
    uint32_t shadedPixels = 0;          ///< Закрашенные пиксели
    
    // GPU timing
    float gpuTime = 0.0f;               ///< Время GPU в миллисекундах
    float vertexShaderTime = 0.0f;      ///< Время vertex shader
    float fragmentShaderTime = 0.0f;    ///< Время fragment shader
    float computeShaderTime = 0.0f;     ///< Время compute shader
    
    // Memory usage breakdown
    size_t vertexBufferMemory = 0;      ///< Память vertex буферов
    size_t indexBufferMemory = 0;       ///< Память index буферов
    size_t textureMemory = 0;           ///< Память текстур
    size_t uniformBufferMemory = 0;     ///< Память uniform буферов
    
    // Pipeline stages
    uint32_t frustumCulledTriangles = 0; ///< Frustum culled треугольники
    uint32_t backfaceCulledTriangles = 0; ///< Backface culled треугольники
    uint32_t depthCulledPixels = 0;     ///< Depth culled пиксели
    
    // Debug information
    std::string lastError;              ///< Последняя ошибка
    bool hasErrors = false;             ///< Есть ли ошибки
};

/**
 * @brief ДОБАВЛЕНО: Информация о GPU
 */
struct GPUInfo {
    std::string deviceName;             ///< Название GPU
    std::string driverVersion;          ///< Версия драйвера
    std::string apiVersion;             ///< Версия API (Vulkan/OpenGL)
    size_t totalMemory = 0;             ///< Общая память GPU
    size_t availableMemory = 0;         ///< Доступная память
    uint32_t maxTextureSize = 0;        ///< Максимальный размер текстуры
    uint32_t maxComputeWorkGroupSize = 0; ///< Максимальный размер workgroup
    bool supportsRayTracing = false;    ///< Поддержка ray tracing
    bool supportsVariableRateShading = false; ///< Поддержка VRS
    bool supportsMeshShaders = false;   ///< Поддержка mesh шейдеров
};

} // namespace SpectraForge::Rendering