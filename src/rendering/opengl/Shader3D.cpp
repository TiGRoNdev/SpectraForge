#include "HyperEngine/Rendering/Shader3D.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "HyperEngine/Core/Console.h"
#include "HyperEngine/Core/SafeConsole.h"

using namespace HyperEngine::Math;
using namespace HyperEngine::Rendering;
using namespace HyperEngine::Core;

namespace HyperEngine {
namespace Rendering {

// Конструктор и деструктор
Shader3D::Shader3D() : programId(0), loaded(false) {}

Shader3D::~Shader3D() {
    cleanup();
}

// Загрузка шейдеров
bool Shader3D::loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexSource = readFile(vertexPath);
    std::string fragmentSource = readFile(fragmentPath);
    
    if (vertexSource.empty() || fragmentSource.empty()) {
        SAFE_ERROR("Failed to read shader files: " + vertexPath + " or " + fragmentPath);
        return false;
    }
    
    return loadFromSource(vertexSource, fragmentSource);
}

bool Shader3D::loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath) {
    std::string vertexSource = readFile(vertexPath);
    std::string fragmentSource = readFile(fragmentPath);
    std::string geometrySource = readFile(geometryPath);
    
    if (vertexSource.empty() || fragmentSource.empty() || geometrySource.empty()) {
        SAFE_ERROR("Failed to read shader files");
        return false;
    }
    
    return loadFromSource(vertexSource, fragmentSource, geometrySource);
}

bool Shader3D::loadFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
    // TODO: Реализовать компиляцию шейдеров для конкретного API (OpenGL/Vulkan)
    SAFE_PRINT_LINE("Shader3D::loadFromSource() - vertex and fragment shaders");
    SAFE_PRINT_LINE("Vertex source length: " + SAFE_TO_STRING(vertexSource.length()));
    SAFE_PRINT_LINE("Fragment source length: " + SAFE_TO_STRING(fragmentSource.length()));
    
    // Заглушка - считаем что загрузка прошла успешно
    programId = 1; // Фиктивный ID
    loaded = true;
    return true;
}

bool Shader3D::loadFromSource(const std::string& vertexSource, const std::string& fragmentSource, const std::string& geometrySource) {
    // TODO: Реализовать компиляцию шейдеров с геометрическим шейдером
    SAFE_PRINT_LINE("Shader3D::loadFromSource() - vertex, fragment and geometry shaders");
    SAFE_PRINT_LINE("Vertex source length: " + SAFE_TO_STRING(vertexSource.length()));
    SAFE_PRINT_LINE("Fragment source length: " + SAFE_TO_STRING(fragmentSource.length()));
    SAFE_PRINT_LINE("Geometry source length: " + SAFE_TO_STRING(geometrySource.length()));
    
    // Заглушка
    programId = 1;
    loaded = true;
    return true;
}

// Использование шейдера
void Shader3D::use() const {
    if (!loaded) {
        SAFE_ERROR("Warning: Trying to use unloaded shader");
        return;
    }
    
    // TODO: Реализовать использование шейдера для конкретного API
    SAFE_PRINT_LINE("Using shader program ID: " + SAFE_TO_STRING(programId));
}

void Shader3D::cleanup() {
    if (programId != 0) {
        // TODO: Удаление шейдерной программы
        SAFE_PRINT_LINE("Cleaning up shader program ID: " + SAFE_TO_STRING(programId));
        programId = 0;
    }
    loaded = false;
    uniformLocations.clear();
}

// Установка uniform переменных
void Shader3D::setMatrix4(const std::string& name, const Matrix4& matrix) const {
    int location = getUniformLocation(name);
    if (location != -1) {
        // TODO: Реализовать установку матрицы для конкретного API
        SAFE_PRINT_LINE("Setting Matrix4 uniform '" + name + "' at location " + SAFE_TO_STRING(location));
    }
}

void Shader3D::setVector3(const std::string& name, const Vector3& vector) const {
    int location = getUniformLocation(name);
    if (location != -1) {
        // TODO: Реализовать установку вектора для конкретного API
        SAFE_PRINT_LINE("Setting Vector3 uniform '" + name + "' at location " + SAFE_TO_STRING(location) + " value: (" + SAFE_TO_STRING(vector.x) + ", " + SAFE_TO_STRING(vector.y) + ", " + SAFE_TO_STRING(vector.z) + ")");
    }
}

void Shader3D::setFloat(const std::string& name, float value) const {
    int location = getUniformLocation(name);
    if (location != -1) {
        // TODO: Реализовать установку float для конкретного API
        SAFE_PRINT_LINE("Setting float uniform '" + name + "' at location " + SAFE_TO_STRING(location) + " value: " + SAFE_TO_STRING(value));
    }
}

void Shader3D::setInt(const std::string& name, int value) const {
    int location = getUniformLocation(name);
    if (location != -1) {
        // TODO: Реализовать установку int для конкретного API
        SAFE_PRINT_LINE("Setting int uniform '" + name + "' at location " + SAFE_TO_STRING(location) + " value: " + SAFE_TO_STRING(value));
    }
}

void Shader3D::setBool(const std::string& name, bool value) const {
    setInt(name, value ? 1 : 0);
}

void Shader3D::setTexture(const std::string& name, unsigned int textureId, int unit) const {
    int location = getUniformLocation(name);
    if (location != -1) {
        // TODO: Реализовать установку текстуры для конкретного API
        SAFE_PRINT_LINE("Setting texture uniform '" + name + "' at location " + SAFE_TO_STRING(location) + " textureId: " + SAFE_TO_STRING(textureId) + " unit: " + SAFE_TO_STRING(unit));
    }
}

// Получение локации uniform переменной
int Shader3D::getUniformLocation(const std::string& name) const {
    // Проверяем кэш
    auto it = uniformLocations.find(name);
    if (it != uniformLocations.end()) {
        return it->second;
    }
    
    // TODO: Получить локацию из API
    int location = static_cast<int>(uniformLocations.size()); // Фиктивная локация
    uniformLocations[name] = location;
    
    return location;
}

// Предустановленные шейдеры
std::shared_ptr<Shader3D> Shader3D::createBasicShader() {
    auto shader = std::make_shared<Shader3D>();
    
    std::string vertexSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec3 aColor;
        layout (location = 3) in vec2 aTexCoord;
        
        uniform mat4 uModel;
        uniform mat4 uView;
        uniform mat4 uProjection;
        
        out vec3 FragPos;
        out vec3 Normal;
        out vec3 Color;
        out vec2 TexCoord;
        
        void main() {
            FragPos = vec3(uModel * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(uModel))) * aNormal;
            Color = aColor;
            TexCoord = aTexCoord;
            
            gl_Position = uProjection * uView * vec4(FragPos, 1.0);
        }
    )";
    
    std::string fragmentSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec3 FragPos;
        in vec3 Normal;
        in vec3 Color;
        in vec2 TexCoord;
        
        uniform vec3 uLightPos;
        uniform vec3 uLightColor;
        uniform vec3 uViewPos;
        
        void main() {
            // Ambient
            float ambientStrength = 0.1;
            vec3 ambient = ambientStrength * uLightColor;
            
            // Diffuse
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(uLightPos - FragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * uLightColor;
            
            vec3 result = (ambient + diffuse) * Color;
            FragColor = vec4(result, 1.0);
        }
    )";
    
    shader->loadFromSource(vertexSource, fragmentSource);
    return shader;
}

std::shared_ptr<Shader3D> Shader3D::createPhongShader() {
    // TODO: Реализовать Phong шейдер
    SAFE_PRINT_LINE("Creating Phong shader...");
    return createBasicShader();
}

std::shared_ptr<Shader3D> Shader3D::createPBRShader() {
    // TODO: Реализовать PBR шейдер
    SAFE_PRINT_LINE("Creating PBR shader...");
    return createBasicShader();
}

std::shared_ptr<Shader3D> Shader3D::createUnlitShader() {
    auto shader = std::make_shared<Shader3D>();
    
    std::string vertexSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 2) in vec3 aColor;
        
        uniform mat4 uModel;
        uniform mat4 uView;
        uniform mat4 uProjection;
        
        out vec3 Color;
        
        void main() {
            Color = aColor;
            gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
        }
    )";
    
    std::string fragmentSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec3 Color;
        
        void main() {
            FragColor = vec4(Color, 1.0);
        }
    )";
    
    shader->loadFromSource(vertexSource, fragmentSource);
    return shader;
}

// Вспомогательные методы
unsigned int Shader3D::compileShader(const std::string& source, unsigned int type) {
    // TODO: Реализовать компиляцию шейдера для конкретного API
    SAFE_PRINT_LINE("Compiling shader of type: " + SAFE_TO_STRING(type));
    return 1; // Фиктивный ID
}

unsigned int Shader3D::linkProgram(unsigned int vertexShader, unsigned int fragmentShader, unsigned int geometryShader) {
    // TODO: Реализовать линковку программы для конкретного API
    SAFE_PRINT_LINE("Linking program with shaders: " + SAFE_TO_STRING(vertexShader) + ", " + SAFE_TO_STRING(fragmentShader));
    if (geometryShader != 0) {
        SAFE_PRINT_LINE(", " + SAFE_TO_STRING(geometryShader));
    }
    // End of linking
    return 1; // Фиктивный ID
}

bool Shader3D::checkCompileErrors(unsigned int shader, const std::string& type) {
    // TODO: Реализовать проверку ошибок компиляции
    SAFE_PRINT_LINE("Checking compile errors for " + type + " shader ID: " + SAFE_TO_STRING(shader));
    return true; // Предполагаем, что ошибок нет
}

std::string Shader3D::readFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        SAFE_ERROR("Failed to open file: " + filepath);
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void Shader3D::bindUniformBlock(const std::string& name, unsigned int bindingPoint) const {
    // TODO: Реализовать привязку uniform блоков
    SAFE_PRINT_LINE("Binding uniform block '" + name + "' to binding point " + SAFE_TO_STRING(bindingPoint));
}

// ShaderManager implementation
ShaderManager& ShaderManager::getInstance() {
    static ShaderManager instance;
    return instance;
}

std::shared_ptr<Shader3D> ShaderManager::loadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) {
    auto shader = std::make_shared<Shader3D>();
    if (shader->loadFromFiles(vertexPath, fragmentPath)) {
        shaders[name] = shader;
        return shader;
    }
    return nullptr;
}

std::shared_ptr<Shader3D> ShaderManager::getShader(const std::string& name) {
    auto it = shaders.find(name);
    return (it != shaders.end()) ? it->second : nullptr;
}

void ShaderManager::removeShader(const std::string& name) {
    shaders.erase(name);
}

void ShaderManager::clear() {
    shaders.clear();
}

void ShaderManager::loadDefaultShaders() {
    shaders["basic"] = Shader3D::createBasicShader();
    shaders["phong"] = Shader3D::createPhongShader();
    shaders["pbr"] = Shader3D::createPBRShader();
    shaders["unlit"] = Shader3D::createUnlitShader();
    
    SAFE_PRINT_LINE("Loaded " + SAFE_TO_STRING(shaders.size()) + " default shaders");
}

} // namespace Rendering
} // namespace HyperEngine

