#pragma once

#include "../Math/Vector4.h"
#include "../Math/Matrix4.h"
#include <vector>
#include <memory>
#include <GL/glew.h>

namespace Engine4D {
namespace Rendering {

/**
 * @brief Вершина для 4D меша
 * 
 * Содержит 4D позицию, нормаль и текстурные координаты
 */
struct Vertex4D {
    Vector4 position;    // 4D позиция [x, y, z, w]
    Vector4 normal;      // 4D нормаль
    Vector4 texCoord;    // 4D текстурные координаты (для 3D текстур)
    Vector4 color;       // RGBA цвет
    
    Vertex4D() = default;
    Vertex4D(const Vector4& pos, const Vector4& norm = Vector4::zero(), 
             const Vector4& tex = Vector4::zero(), const Vector4& col = Vector4::one());
};

/**
 * @brief 4D меш для хранения геометрии
 */
class Mesh4D {
public:
    std::vector<Vertex4D> vertices;
    std::vector<unsigned int> indices;
    
    // OpenGL буферы
    GLuint VAO, VBO, EBO;
    bool isUploaded;
    
    Mesh4D();
    ~Mesh4D();
    
    void addVertex(const Vertex4D& vertex);
    void addTriangle(unsigned int i1, unsigned int i2, unsigned int i3);
    void addQuad(unsigned int i1, unsigned int i2, unsigned int i3, unsigned int i4);
    
    void uploadToGPU();
    void render() const;
    void cleanup();
    
    // Создание примитивов
    static Mesh4D createTesseract(float size = 1.0f);
    static Mesh4D createHypercube(float size = 1.0f);
    static Mesh4D createSimplex();
};

/**
 * @brief Шейдер для рендеринга 4D объектов
 */
class Shader4D {
public:
    GLuint programID;
    bool isCompiled;
    
    Shader4D();
    ~Shader4D();
    
    bool loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
    bool loadFromSource(const std::string& vertexSource, const std::string& fragmentSource);
    void use() const;
    void cleanup();
    
    // Униформы
    void setMatrix4(const std::string& name, const Matrix4& matrix) const;
    void setVector4(const std::string& name, const Vector4& vector) const;
    void setFloat(const std::string& name, float value) const;
    void setInt(const std::string& name, int value) const;
    void setBool(const std::string& name, bool value) const;
    
private:
    GLuint compileShader(const std::string& source, GLenum type);
    bool linkProgram(GLuint vertexShader, GLuint fragmentShader);
};

/**
 * @brief Камера для 4D пространства
 */
class Camera4D {
public:
    Vector4 position;        // Позиция камеры в 4D
    Vector4 target;          // Целевая точка
    Vector4 up;              // Вектор "вверх"
    Vector4 right;           // Вектор "вправо"
    
    float fov;               // Поле зрения
    float nearPlane;         // Ближняя плоскость
    float farPlane;          // Дальняя плоскость
    float wDistance;         // Расстояние для перспективной проекции
    
    // Параметры сечения
    float crossSectionW;     // W-координата сечения
    bool useCrossSection;    // Использовать ли сечение
    
    Camera4D();
    
    void setPosition(const Vector4& pos);
    void setTarget(const Vector4& target);
    void lookAt(const Vector4& pos, const Vector4& target, const Vector4& up);
    
    Matrix4 getViewMatrix() const;
    Matrix4 getProjectionMatrix() const;
    Matrix4 getViewProjectionMatrix() const;
    
    // Дополнительные методы для совместимости
    void setAspectRatio(float aspect);
    void setFieldOfView(float fov);
    void setNearPlane(float near);
    void setFarPlane(float far);
    
    // Навигация в 4D
    void move(const Vector4& direction);
    void rotate(float angle, const Vector4& axis);
    void setCrossSection(float wValue);
};

/**
 * @brief Основной класс рендерера
 */
class Renderer {
public:
    static Renderer& getInstance();
    
    bool initialize(int width, int height);
    void cleanup();
    
    void beginFrame();
    void endFrame();
    
    void setClearColor(float r, float g, float b, float a);
    void clear();
    
    void renderMesh(const Mesh4D& mesh, const Matrix4& transform, const Shader4D& shader);
    void renderWireframe(const Mesh4D& mesh, const Matrix4& transform, const Shader4D& shader);
    
    // Проекции
    void setProjectionMode(ProjectionMode mode);
    void setCrossSection(float wValue);
    
    // Утилиты
    void setViewport(int x, int y, int width, int height);
    void enableDepthTest(bool enable);
    void enableBlending(bool enable);
    
    // Геттеры
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    
private:
    Renderer() = default;
    ~Renderer() = default;
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    
    int m_width, m_height;
    bool m_initialized;
    
    // Проекции
    enum class ProjectionMode {
        Orthographic,    // Ортогональная проекция 4D->3D
        Perspective,     // Перспективная проекция
        CrossSection     // Сечение
    } m_projectionMode;
    
    float m_crossSectionW;
};

} // namespace Rendering
} // namespace Engine4D
