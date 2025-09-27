#pragma once

#include "../Math/Vector3.h"
#include <string>
#include <memory>

namespace Engine3D {
namespace Rendering {

// Forward declarations
class Shader3D;
class Texture3D;

/**
 * @brief Класс материала для 3D объектов
 * 
 * Material3D определяет визуальные свойства поверхности объекта:
 * - Цвета (альбедо, диффузный, спекулярный)
 * - Текстуры (диффузная, нормалей, спекулярная, эмиссионная)
 * - Физические свойства (металличность, шероховатость, прозрачность)
 * - Шейдеры для рендеринга
 */
class Material3D {
public:
    /**
     * @brief Типы материалов
     */
    enum Type {
        STANDARD,    ///< Стандартный PBR материал
        UNLIT,       ///< Без освещения
        EMISSION,    ///< Эмиссионный материал  
        TRANSPARENT, ///< Прозрачный материал
        CUSTOM       ///< Пользовательский материал
    };

    /**
     * @brief Режимы смешивания
     */
    enum BlendMode {
        OPAQUE,      ///< Непрозрачный
        ALPHA_BLEND, ///< Альфа-смешивание
        ADDITIVE,    ///< Аддитивное смешивание
        MULTIPLY     ///< Мультипликативное смешивание
    };

public:
    /**
     * @brief Конструктор материала
     * @param name Название материала
     * @param type Тип материала
     */
    Material3D(const std::string& name = "Material", Type type = STANDARD);
    
    /**
     * @brief Деструктор
     */
    virtual ~Material3D() = default;

    // ============================================================
    // Основные свойства
    // ============================================================
    
    /**
     * @brief Получить название материала
     */
    const std::string& getName() const { return name; }
    
    /**
     * @brief Установить название материала
     */
    void setName(const std::string& newName) { name = newName; }
    
    /**
     * @brief Получить тип материала
     */
    Type getType() const { return type; }
    
    /**
     * @brief Установить тип материала
     */
    void setType(Type newType) { type = newType; }

    // ============================================================
    // Цветовые свойства
    // ============================================================
    
    /**
     * @brief Альбедо цвет (базовый цвет материала)
     */
    const Math::Vector3& getAlbedo() const { return albedo; }
    void setAlbedo(const Math::Vector3& color) { albedo = color; }
    void setAlbedo(float r, float g, float b) { albedo = Math::Vector3(r, g, b); }
    
    /**
     * @brief Прозрачность альбедо
     */
    float getAlbedoAlpha() const { return albedoAlpha; }
    void setAlbedoAlpha(float alpha) { albedoAlpha = std::max(0.0f, std::min(1.0f, alpha)); }
    
    /**
     * @brief Эмиссионный цвет (самосвечение)
     */
    const Math::Vector3& getEmission() const { return emission; }
    void setEmission(const Math::Vector3& color) { emission = color; }
    void setEmission(float r, float g, float b) { emission = Math::Vector3(r, g, b); }

    // ============================================================
    // PBR свойства (Physically Based Rendering)
    // ============================================================
    
    /**
     * @brief Металличность (0.0 = диэлектрик, 1.0 = металл)
     */
    float getMetallic() const { return metallic; }
    void setMetallic(float value) { metallic = std::max(0.0f, std::min(1.0f, value)); }
    
    /**
     * @brief Шероховатость (0.0 = зеркальная, 1.0 = матовая)
     */
    float getRoughness() const { return roughness; }
    void setRoughness(float value) { roughness = std::max(0.0f, std::min(1.0f, value)); }
    
    /**
     * @brief Коэффициент отражения для диэлектриков (обычно 0.04)
     */
    float getReflectance() const { return reflectance; }
    void setReflectance(float value) { reflectance = std::max(0.0f, std::min(1.0f, value)); }
    
    /**
     * @brief Прозрачность (0.0 = прозрачный, 1.0 = непрозрачный)
     */
    float getOpacity() const { return opacity; }
    void setOpacity(float value) { opacity = std::max(0.0f, std::min(1.0f, value)); }

    // ============================================================
    // Текстуры
    // ============================================================
    
    /**
     * @brief Диффузная текстура (альбедо)
     */
    std::shared_ptr<Texture3D> getDiffuseTexture() const { return diffuseTexture; }
    void setDiffuseTexture(std::shared_ptr<Texture3D> texture) { diffuseTexture = texture; }
    
    /**
     * @brief Текстура нормалей
     */
    std::shared_ptr<Texture3D> getNormalTexture() const { return normalTexture; }
    void setNormalTexture(std::shared_ptr<Texture3D> texture) { normalTexture = texture; }
    
    /**
     * @brief Спекулярная текстура
     */
    std::shared_ptr<Texture3D> getSpecularTexture() const { return specularTexture; }
    void setSpecularTexture(std::shared_ptr<Texture3D> texture) { specularTexture = texture; }
    
    /**
     * @brief Эмиссионная текстура
     */
    std::shared_ptr<Texture3D> getEmissionTexture() const { return emissionTexture; }
    void setEmissionTexture(std::shared_ptr<Texture3D> texture) { emissionTexture = texture; }
    
    /**
     * @brief Текстура шероховатости/металличности
     */
    std::shared_ptr<Texture3D> getRoughnessMetallicTexture() const { return roughnessMetallicTexture; }
    void setRoughnessMetallicTexture(std::shared_ptr<Texture3D> texture) { roughnessMetallicTexture = texture; }

    // ============================================================
    // Рендеринг
    // ============================================================
    
    /**
     * @brief Режим смешивания
     */
    BlendMode getBlendMode() const { return blendMode; }
    void setBlendMode(BlendMode mode) { blendMode = mode; }
    
    /**
     * @brief Двусторонний рендеринг
     */
    bool isDoubleSided() const { return doubleSided; }
    void setDoubleSided(bool enabled) { doubleSided = enabled; }
    
    /**
     * @brief Шейдер материала
     */
    std::shared_ptr<Shader3D> getShader() const { return shader; }
    void setShader(std::shared_ptr<Shader3D> newShader) { shader = newShader; }

    // ============================================================
    // Утилиты
    // ============================================================
    
    /**
     * @brief Применить материал к рендерингу
     * Устанавливает все uniforms в шейдере
     */
    void bind() const;
    
    /**
     * @brief Отменить применение материала
     */
    void unbind() const;
    
    /**
     * @brief Клонировать материал
     */
    std::shared_ptr<Material3D> clone() const;
    
    /**
     * @brief Валидация материала
     */
    bool isValid() const;

    // ============================================================
    // Статические фабричные методы
    // ============================================================
    
    /**
     * @brief Создать стандартный материал
     */
    static std::shared_ptr<Material3D> createStandard(const std::string& name = "Standard");
    
    /**
     * @brief Создать эмиссионный материал
     */
    static std::shared_ptr<Material3D> createEmissive(
        const Math::Vector3& emissionColor, 
        const std::string& name = "Emissive"
    );
    
    /**
     * @brief Создать прозрачный материал
     */
    static std::shared_ptr<Material3D> createTransparent(
        const Math::Vector3& color,
        float alpha = 0.5f,
        const std::string& name = "Transparent"
    );
    
    /**
     * @brief Создать металлический материал
     */
    static std::shared_ptr<Material3D> createMetallic(
        const Math::Vector3& color,
        float roughness = 0.1f,
        const std::string& name = "Metallic"
    );

private:
    // Основные свойства
    std::string name;
    Type type;
    
    // Цвета
    Math::Vector3 albedo;        ///< Альбедо цвет (RGB)
    float albedoAlpha;           ///< Прозрачность альбедо [0, 1]
    Math::Vector3 emission;      ///< Эмиссионный цвет (RGB)
    
    // PBR параметры
    float metallic;              ///< Металличность [0, 1]
    float roughness;             ///< Шероховатость [0, 1]
    float reflectance;           ///< Коэффициент отражения [0, 1]
    float opacity;               ///< Прозрачность [0, 1]
    
    // Текстуры
    std::shared_ptr<Texture3D> diffuseTexture;
    std::shared_ptr<Texture3D> normalTexture;
    std::shared_ptr<Texture3D> specularTexture;
    std::shared_ptr<Texture3D> emissionTexture;
    std::shared_ptr<Texture3D> roughnessMetallicTexture;
    
    // Рендеринг
    BlendMode blendMode;
    bool doubleSided;
    
    // Шейдер
    std::shared_ptr<Shader3D> shader;
};

} // namespace Rendering
} // namespace Engine3D
