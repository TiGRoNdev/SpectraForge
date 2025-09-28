#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace HyperEngine::Rendering {

// Forward declarations
struct TextureDesc;
struct BufferDesc;
struct MemoryStats;
enum class ShaderType;

/**
 * @brief Дескрипторы ресурсов - type-safe обертки для внутренних ID
 */
using ResourceHandle = uint64_t;
using BufferHandle = ResourceHandle;
using TextureHandle = ResourceHandle;
using ShaderHandle = ResourceHandle;

constexpr ResourceHandle INVALID_HANDLE = 0;

/**
 * @brief Интерфейс для управления ресурсами рендеринга
 *
 * Применяет Interface Segregation Principle -
 * клиенты зависят только от нужных им методов.
 *
 * Этот интерфейс отвечает ТОЛЬКО за управление ресурсами:
 * буферы, текстуры, шейдеры и память.
 */
class IResourceManager {
  public:
    virtual ~IResourceManager() = default;

    /**
     * @brief Инициализация менеджера ресурсов
     * @return true если инициализация прошла успешно
     */
    virtual bool initialize() = 0;

    /**
     * @brief Завершение работы менеджера ресурсов
     */
    virtual void shutdown() = 0;

    // === Управление буферами ===

    /**
     * @brief Выделить буфер
     * @param desc Описание буфера
     * @return Дескриптор буфера или INVALID_HANDLE при ошибке
     */
    virtual BufferHandle createBuffer(const BufferDesc& desc) = 0;

    /**
     * @brief Обновить данные буфера
     * @param handle Дескриптор буфера
     * @param data Указатель на данные
     * @param size Размер данных в байтах
     * @param offset Смещение в буфере
     */
    virtual void updateBuffer(BufferHandle handle,
                              const void* data,
                              size_t size,
                              size_t offset = 0) = 0;

    /**
     * @brief Прочитать данные из буфера
     * @param handle Дескриптор буфера
     * @param data Указатель для записи данных
     * @param size Размер читаемых данных
     * @param offset Смещение в буфере
     */
    virtual void readBuffer(BufferHandle handle, void* data, size_t size, size_t offset = 0) = 0;

    // === Управление текстурами ===

    /**
     * @brief Создать текстуру
     * @param desc Описание текстуры
     * @return Дескриптор текстуры или INVALID_HANDLE при ошибке
     */
    virtual TextureHandle createTexture(const TextureDesc& desc) = 0;

    /**
     * @brief Обновить данные текстуры
     * @param handle Дескриптор текстуры
     * @param data Указатель на пиксельные данные
     * @param mipLevel Уровень мипмапа
     * @param arrayLayer Слой массива текстур
     */
    virtual void updateTexture(TextureHandle handle,
                               const void* data,
                               uint32_t mipLevel = 0,
                               uint32_t arrayLayer = 0) = 0;

    // === Управление шейдерами ===

    /**
     * @brief Создать шейдер из исходного кода
     * @param source Исходный код шейдера
     * @param type Тип шейдера
     * @return Дескриптор шейдера или INVALID_HANDLE при ошибке
     */
    virtual ShaderHandle createShader(const std::string& source, ShaderType type) = 0;

    /**
     * @brief Создать шейдер из файла
     * @param filePath Путь к файлу шейдера
     * @param type Тип шейдера
     * @return Дескриптор шейдера или INVALID_HANDLE при ошибке
     */
    virtual ShaderHandle createShaderFromFile(const std::string& filePath, ShaderType type) = 0;

    // === Освобождение ресурсов ===

    /**
     * @brief Освободить ресурс
     * @param handle Дескриптор ресурса любого типа
     */
    virtual void releaseResource(ResourceHandle handle) = 0;

    /**
     * @brief Освободить все ресурсы
     */
    virtual void releaseAllResources() = 0;

    // === Информация о ресурсах ===

    /**
     * @brief Проверить валидность дескриптора
     * @param handle Дескриптор ресурса
     * @return true если ресурс существует и валиден
     */
    virtual bool isValid(ResourceHandle handle) const = 0;

    /**
     * @brief Получить размер ресурса в байтах
     * @param handle Дескриптор ресурса
     * @return Размер в байтах или 0 для невалидного handle
     */
    virtual size_t getResourceSize(ResourceHandle handle) const = 0;

    /**
     * @brief Получить статистику использования памяти
     * @return Структура с информацией о памяти
     */
    virtual MemoryStats getMemoryStats() const = 0;

    // === Синхронизация ===

    /**
     * @brief Дождаться завершения всех операций с ресурсами
     */
    virtual void waitForCompletion() = 0;

    /**
     * @brief Принудительно синхронизировать состояние ресурсов
     */
    virtual void flush() = 0;
};

/**
 * @brief Типы использования буферов
 */
enum class BufferUsage : uint32_t {
    Vertex = 1 << 0,        ///< Vertex buffer
    Index = 1 << 1,         ///< Index buffer
    Uniform = 1 << 2,       ///< Uniform buffer
    Storage = 1 << 3,       ///< Storage buffer
    Staging = 1 << 4,       ///< Staging buffer для копирования
    IndirectDraw = 1 << 5,  ///< Indirect draw buffer
    TransferSrc = 1 << 6,   ///< Источник для копирования
    TransferDst = 1 << 7    ///< Назначение для копирования
};

/**
 * @brief Свойства памяти
 */
enum class MemoryProperty : uint32_t {
    DeviceLocal = 1 << 0,     ///< Память на GPU
    HostVisible = 1 << 1,     ///< Видимая с CPU
    HostCoherent = 1 << 2,    ///< Когерентная с CPU
    HostCached = 1 << 3,      ///< Кэшированная на CPU
    LazilyAllocated = 1 << 4  ///< Выделяется по требованию
};

/**
 * @brief Описание буфера
 */
struct BufferDesc {
    size_t size = 0;                                                ///< Размер в байтах
    BufferUsage usage = BufferUsage::Vertex;                        ///< Способ использования
    MemoryProperty memoryProperties = MemoryProperty::DeviceLocal;  ///< Свойства памяти
    const void* initialData = nullptr;  ///< Начальные данные (опционально)
    std::string debugName;              ///< Имя для отладки
};

/**
 * @brief Форматы текстур
 */
enum class TextureFormat {
    // 8-bit форматы
    R8_UNORM,
    R8_SNORM,
    R8_UINT,
    R8_SINT,
    RG8_UNORM,
    RG8_SNORM,
    RG8_UINT,
    RG8_SINT,
    RGB8_UNORM,
    RGB8_SNORM,
    RGB8_UINT,
    RGB8_SINT,
    RGBA8_UNORM,
    RGBA8_SNORM,
    RGBA8_UINT,
    RGBA8_SINT,

    // 16-bit форматы
    R16_UNORM,
    R16_SNORM,
    R16_UINT,
    R16_SINT,
    R16_FLOAT,
    RG16_UNORM,
    RG16_SNORM,
    RG16_UINT,
    RG16_SINT,
    RG16_FLOAT,
    RGBA16_UNORM,
    RGBA16_SNORM,
    RGBA16_UINT,
    RGBA16_SINT,
    RGBA16_FLOAT,

    // 32-bit форматы
    R32_UINT,
    R32_SINT,
    R32_FLOAT,
    RG32_UINT,
    RG32_SINT,
    RG32_FLOAT,
    RGB32_UINT,
    RGB32_SINT,
    RGB32_FLOAT,
    RGBA32_UINT,
    RGBA32_SINT,
    RGBA32_FLOAT,

    // Специальные форматы
    D16_UNORM,          ///< 16-bit depth
    D32_FLOAT,          ///< 32-bit float depth
    D24_UNORM_S8_UINT,  ///< 24-bit depth + 8-bit stencil
    D32_FLOAT_S8_UINT   ///< 32-bit float depth + 8-bit stencil
};

/**
 * @brief Типы текстур
 */
enum class TextureType {
    Texture1D,        ///< 1D текстура
    Texture2D,        ///< 2D текстура
    Texture3D,        ///< 3D текстура
    TextureCube,      ///< Cube map
    Texture1DArray,   ///< Массив 1D текстур
    Texture2DArray,   ///< Массив 2D текстур
    TextureCubeArray  ///< Массив cube map'ов
};

/**
 * @brief Способы использования текстур
 */
enum class TextureUsage : uint32_t {
    Sampled = 1 << 0,          ///< Сэмплируется в шейдере
    Storage = 1 << 1,          ///< Storage image
    ColorAttachment = 1 << 2,  ///< Render target
    DepthAttachment = 1 << 3,  ///< Depth buffer
    TransferSrc = 1 << 4,      ///< Источник для копирования
    TransferDst = 1 << 5       ///< Назначение для копирования
};

/**
 * @brief Описание текстуры
 */
struct TextureDesc {
    uint32_t width = 1;                                 ///< Ширина
    uint32_t height = 1;                                ///< Высота
    uint32_t depth = 1;                                 ///< Глубина (для 3D текстур)
    uint32_t mipLevels = 1;                             ///< Количество мип-уровней
    uint32_t arrayLayers = 1;                           ///< Количество слоев
    TextureFormat format = TextureFormat::RGBA8_UNORM;  ///< Формат пикселей
    TextureType type = TextureType::Texture2D;          ///< Тип текстуры
    TextureUsage usage = TextureUsage::Sampled;         ///< Способ использования
    uint32_t samples = 1;                               ///< Количество MSAA сэмплов
    const void* initialData = nullptr;                  ///< Начальные данные
    std::string debugName;                              ///< Имя для отладки
};

/**
 * @brief Типы шейдеров
 */
enum class ShaderType {
    Vertex,          ///< Vertex shader
    Fragment,        ///< Fragment/Pixel shader
    Geometry,        ///< Geometry shader
    Compute,         ///< Compute shader
    TessControl,     ///< Tessellation control shader
    TessEvaluation,  ///< Tessellation evaluation shader

    // Ray tracing шейдеры
    RayGeneration,  ///< Ray generation shader
    AnyHit,         ///< Any hit shader
    ClosestHit,     ///< Closest hit shader
    Miss,           ///< Miss shader
    Intersection,   ///< Intersection shader
    Callable        ///< Callable shader
};

/**
 * @brief Статистика использования памяти
 */
struct MemoryStats {
    size_t totalMemory = 0;     ///< Общее количество доступной памяти
    size_t usedMemory = 0;      ///< Использованная память
    size_t freeMemory = 0;      ///< Свободная память
    uint32_t bufferCount = 0;   ///< Количество буферов
    uint32_t textureCount = 0;  ///< Количество текстур
    uint32_t shaderCount = 0;   ///< Количество шейдеров
    size_t bufferMemory = 0;    ///< Память, занятая буферами
    size_t textureMemory = 0;   ///< Память, занятая текстурами

    /**
     * @brief Вычислить процент использования памяти
     * @return Процент от 0.0 до 100.0
     */
    float getUsagePercentage() const {
        return totalMemory > 0 ? (static_cast<float>(usedMemory) / totalMemory) * 100.0f : 0.0f;
    }
};

}  // namespace HyperEngine::Rendering
