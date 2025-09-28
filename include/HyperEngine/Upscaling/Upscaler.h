#pragma once

#include <glm/glm.hpp>
#include <memory>

namespace HyperEngine::Upscaling {

/**
 * @brief Денойзированное изображение
 */
struct DenoisedImage {
    float* colorBuffer;  // Цветовой буфер
    float* depthBuffer;  // Буфер глубины (опционально)
    uint32_t width;
    uint32_t height;
    uint32_t channels;
};

/**
 * @brief Целевое разрешение для upscaling
 */
struct ResolutionTarget {
    uint32_t width;
    uint32_t height;
    float scaleFactor;  // Коэффициент масштабирования
};

/**
 * @brief Финальное изображение после upscaling
 */
struct FinalImage {
    float* colorBuffer;  // Финальный цветовой буфер
    uint32_t width;
    uint32_t height;
    uint32_t channels;
};

/**
 * @brief Конфигурация железа для upscaler
 */
struct HardwareConfig {
    enum class VendorType { NVIDIA, AMD, INTEL, OTHER } vendor;

    bool supportsRayTracing;
    bool supportsTensorCores;
    bool supportsAsyncCompute;
    size_t vramSize;
    std::string deviceName;
};

/**
 * @brief Абстрактный базовый класс для upscaling
 *
 * Определяет интерфейс для различных технологий upscaling
 * (DLSS, FSR, и потенциально других в будущем).
 */
class Upscaler {
  public:
    /**
     * @brief Виртуальный деструктор
     */
    virtual ~Upscaler() = default;

    /**
     * @brief Инициализация upscaler
     * @param config Конфигурация железа
     * @return true если инициализация успешна
     */
    virtual bool init(const HardwareConfig& config) = 0;

    /**
     * @brief Завершение работы upscaler
     */
    virtual void shutdown() = 0;

    /**
     * @brief Upscaling изображения
     * @param image Входное денойзированное изображение
     * @param target Целевое разрешение
     * @return Финальное изображение
     */
    virtual FinalImage upscaleImage(const DenoisedImage& image, const ResolutionTarget& target) = 0;

    /**
     * @brief Проверка поддержки данной конфигурации
     * @param config Конфигурация железа
     * @return true если конфигурация поддерживается
     */
    virtual bool isSupported(const HardwareConfig& config) const = 0;

    /**
     * @brief Получение имени upscaler
     * @return Имя технологии upscaling
     */
    virtual const char* getName() const = 0;

    /**
     * @brief Получение рекомендуемого коэффициента масштабирования
     * @param inputWidth Ширина входного изображения
     * @param inputHeight Высота входного изображения
     * @param targetWidth Целевая ширина
     * @param targetHeight Целевая высота
     * @return Рекомендуемый коэффициент
     */
    virtual float getRecommendedScaleFactor(uint32_t inputWidth,
                                            uint32_t inputHeight,
                                            uint32_t targetWidth,
                                            uint32_t targetHeight) const;

    /**
     * @brief Проверка инициализации
     * @return true если upscaler инициализирован
     */
    virtual bool isInitialized() const = 0;

  protected:
    /**
     * @brief Защищенный конструктор
     */
    Upscaler() = default;

    // Запрет копирования
    Upscaler(const Upscaler&) = delete;
    Upscaler& operator=(const Upscaler&) = delete;
};

/**
 * @brief Фабрика для создания upscaler'ов
 */
class UpscalerFactory {
  public:
    /**
     * @brief Типы доступных upscaler'ов
     */
    enum class Type {
        DLSS,
        FSR,
        AUTO  // Автоматический выбор на основе железа
    };

    /**
     * @brief Создание upscaler
     * @param type Тип upscaler'а
     * @param config Конфигурация железа
     * @return Уникальный указатель на upscaler
     */
    static std::unique_ptr<Upscaler> create(Type type, const HardwareConfig& config);

    /**
     * @brief Автоматический выбор лучшего upscaler'а
     * @param config Конфигурация железа
     * @return Уникальный указатель на upscaler
     */
    static std::unique_ptr<Upscaler> createBest(const HardwareConfig& config);

    /**
     * @brief Проверка доступности типа upscaler'а
     * @param type Тип upscaler'а
     * @param config Конфигурация железа
     * @return true если доступен
     */
    static bool isAvailable(Type type, const HardwareConfig& config);

  private:
    UpscalerFactory() = delete;
};

}  // namespace HyperEngine::Upscaling
