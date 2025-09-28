#pragma once

#include <string>
#include <unordered_map>
#include "../Math/Matrix4.h"
#include "../Math/Vector3.h"

namespace Engine3D {
namespace Rendering {

/**
 * @brief Шейдер для рендеринга 3D объектов
 */
class Shader3D {
  public:
    Shader3D();
    virtual ~Shader3D();

    // Загрузка шейдеров
    bool loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
    bool loadFromFiles(const std::string& vertexPath,
                       const std::string& fragmentPath,
                       const std::string& geometryPath);
    bool loadFromSource(const std::string& vertexSource, const std::string& fragmentSource);
    bool loadFromSource(const std::string& vertexSource,
                        const std::string& fragmentSource,
                        const std::string& geometrySource);

    // Использование шейдера
    virtual void use() const;
    virtual void cleanup();
    bool isLoaded() const { return loaded; }

    // Установка uniform переменных
    void setMatrix4(const std::string& name, const Math::Matrix4& matrix) const;
    void setVector3(const std::string& name, const Math::Vector3& vector) const;
    void setFloat(const std::string& name, float value) const;
    void setInt(const std::string& name, int value) const;
    void setBool(const std::string& name, bool value) const;
    void setTexture(const std::string& name, unsigned int textureId, int unit = 0) const;

    // Получение ID шейдерной программы
    unsigned int getProgramId() const { return programId; }

    // Получение локации uniform переменной
    int getUniformLocation(const std::string& name) const;

    // Предустановленные шейдеры
    static std::shared_ptr<Shader3D> createBasicShader();
    static std::shared_ptr<Shader3D> createPhongShader();
    static std::shared_ptr<Shader3D> createPBRShader();
    static std::shared_ptr<Shader3D> createUnlitShader();

  protected:
    unsigned int programId;
    bool loaded;

    // Кэш uniform локаций для оптимизации
    mutable std::unordered_map<std::string, int> uniformLocations;

    // Вспомогательные методы компиляции
    unsigned int compileShader(const std::string& source, unsigned int type);
    unsigned int linkProgram(unsigned int vertexShader,
                             unsigned int fragmentShader,
                             unsigned int geometryShader = 0);
    bool checkCompileErrors(unsigned int shader, const std::string& type);
    std::string readFile(const std::string& filepath);

    // Uniform блоки
    void bindUniformBlock(const std::string& name, unsigned int bindingPoint) const;
};

/**
 * @brief Менеджер шейдеров для кэширования и управления
 */
class ShaderManager {
  public:
    static ShaderManager& getInstance();

    // Управление шейдерами
    std::shared_ptr<Shader3D> loadShader(const std::string& name,
                                         const std::string& vertexPath,
                                         const std::string& fragmentPath);
    std::shared_ptr<Shader3D> getShader(const std::string& name);
    void removeShader(const std::string& name);
    void clear();

    // Предустановленные шейдеры
    void loadDefaultShaders();

  private:
    std::unordered_map<std::string, std::shared_ptr<Shader3D>> shaders;

    ShaderManager() = default;
    ~ShaderManager() = default;
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;
};

}  // namespace Rendering
}  // namespace Engine3D
