#include "Engine4D/Rendering/Renderer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>

using namespace Engine4D::Math;

namespace Engine4D {
namespace Rendering {

// Vertex4D
Vertex4D::Vertex4D(const Vector4& pos, const Vector4& norm, const Vector4& tex, const Vector4& col)
    : position(pos), normal(norm), texCoord(tex), color(col) {}

// Mesh4D
Mesh4D::Mesh4D() : VAO(0), VBO(0), EBO(0), isUploaded(false) {}

Mesh4D::~Mesh4D() {
    cleanup();
}

void Mesh4D::addVertex(const Vertex4D& vertex) {
    vertices.push_back(vertex);
}

void Mesh4D::addTriangle(unsigned int i1, unsigned int i2, unsigned int i3) {
    indices.push_back(i1);
    indices.push_back(i2);
    indices.push_back(i3);
}

void Mesh4D::addQuad(unsigned int i1, unsigned int i2, unsigned int i3, unsigned int i4) {
    // Разбиваем четырехугольник на два треугольника
    addTriangle(i1, i2, i3);
    addTriangle(i1, i3, i4);
}

void Mesh4D::uploadToGPU() {
    if (isUploaded) return;
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    // Загружаем вершины
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex4D), 
                 vertices.data(), GL_STATIC_DRAW);
    
    // Загружаем индексы
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), 
                 indices.data(), GL_STATIC_DRAW);
    
    // Настраиваем атрибуты вершин
    // Позиция (4D)
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex4D), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Нормаль (4D)
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex4D), 
                         (void*)offsetof(Vertex4D, normal));
    glEnableVertexAttribArray(1);
    
    // Текстурные координаты (4D)
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex4D), 
                         (void*)offsetof(Vertex4D, texCoord));
    glEnableVertexAttribArray(2);
    
    // Цвет (RGBA)
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex4D), 
                         (void*)offsetof(Vertex4D, color));
    glEnableVertexAttribArray(3);
    
    glBindVertexArray(0);
    isUploaded = true;
}

void Mesh4D::render() const {
    if (!isUploaded) return;
    
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh4D::cleanup() {
    if (VAO) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (VBO) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    if (EBO) {
        glDeleteBuffers(1, &EBO);
        EBO = 0;
    }
    isUploaded = false;
}

Mesh4D Mesh4D::createTesseract(float size) {
    Mesh4D mesh;
    
    // Создаем 16 вершин тессеракта (4D куба)
    float s = size * 0.5f;
    
    // 8 вершин 3D куба при w = -s
    mesh.addVertex(Vertex4D(Vector4(-s, -s, -s, -s), Vector4::zero(), Vector4::zero(), Vector4(1, 0, 0, 1)));
    mesh.addVertex(Vertex4D(Vector4( s, -s, -s, -s), Vector4::zero(), Vector4::zero(), Vector4(1, 0, 0, 1)));
    mesh.addVertex(Vertex4D(Vector4( s,  s, -s, -s), Vector4::zero(), Vector4::zero(), Vector4(1, 0, 0, 1)));
    mesh.addVertex(Vertex4D(Vector4(-s,  s, -s, -s), Vector4::zero(), Vector4::zero(), Vector4(1, 0, 0, 1)));
    mesh.addVertex(Vertex4D(Vector4(-s, -s,  s, -s), Vector4::zero(), Vector4::zero(), Vector4(1, 0, 0, 1)));
    mesh.addVertex(Vertex4D(Vector4( s, -s,  s, -s), Vector4::zero(), Vector4::zero(), Vector4(1, 0, 0, 1)));
    mesh.addVertex(Vertex4D(Vector4( s,  s,  s, -s), Vector4::zero(), Vector4::zero(), Vector4(1, 0, 0, 1)));
    mesh.addVertex(Vertex4D(Vector4(-s,  s,  s, -s), Vector4::zero(), Vector4::zero(), Vector4(1, 0, 0, 1)));
    
    // 8 вершин 3D куба при w = s
    mesh.addVertex(Vertex4D(Vector4(-s, -s, -s,  s), Vector4::zero(), Vector4::zero(), Vector4(0, 1, 0, 1)));
    mesh.addVertex(Vertex4D(Vector4( s, -s, -s,  s), Vector4::zero(), Vector4::zero(), Vector4(0, 1, 0, 1)));
    mesh.addVertex(Vertex4D(Vector4( s,  s, -s,  s), Vector4::zero(), Vector4::zero(), Vector4(0, 1, 0, 1)));
    mesh.addVertex(Vertex4D(Vector4(-s,  s, -s,  s), Vector4::zero(), Vector4::zero(), Vector4(0, 1, 0, 1)));
    mesh.addVertex(Vertex4D(Vector4(-s, -s,  s,  s), Vector4::zero(), Vector4::zero(), Vector4(0, 1, 0, 1)));
    mesh.addVertex(Vertex4D(Vector4( s, -s,  s,  s), Vector4::zero(), Vector4::zero(), Vector4(0, 1, 0, 1)));
    mesh.addVertex(Vertex4D(Vector4( s,  s,  s,  s), Vector4::zero(), Vector4::zero(), Vector4(0, 1, 0, 1)));
    mesh.addVertex(Vertex4D(Vector4(-s,  s,  s,  s), Vector4::zero(), Vector4::zero(), Vector4(0, 1, 0, 1)));
    
    // Соединяем вершины в грани (кубы)
    // Нижний куб (w = -s)
    mesh.addQuad(0, 1, 2, 3); // Передняя грань
    mesh.addQuad(4, 7, 6, 5); // Задняя грань
    mesh.addQuad(0, 4, 5, 1); // Нижняя грань
    mesh.addQuad(2, 6, 7, 3); // Верхняя грань
    mesh.addQuad(0, 3, 7, 4); // Левая грань
    mesh.addQuad(1, 5, 6, 2); // Правая грань
    
    // Верхний куб (w = s)
    mesh.addQuad(8, 9, 10, 11);  // Передняя грань
    mesh.addQuad(12, 15, 14, 13); // Задняя грань
    mesh.addQuad(8, 12, 13, 9);   // Нижняя грань
    mesh.addQuad(10, 14, 15, 11); // Верхняя грань
    mesh.addQuad(8, 11, 15, 12);  // Левая грань
    mesh.addQuad(9, 13, 14, 10);  // Правая грань
    
    // Соединяем кубы (гиперграни)
    for (int i = 0; i < 8; ++i) {
        mesh.addQuad(i, i + 8, i + 9, i + 1);
    }
    
    return mesh;
}

Mesh4D Mesh4D::createHypercube(float size) {
    // Гиперкуб - это тессеракт
    return createTesseract(size);
}

Mesh4D Mesh4D::createSimplex() {
    Mesh4D mesh;
    
    // Создаем 5-симплекс (4D тетраэдр)
    float s = 1.0f;
    
    // 5 вершин симплекса
    mesh.addVertex(Vertex4D(Vector4(0, 0, 0, 0), Vector4::zero(), Vector4::zero(), Vector4(1, 0, 0, 1)));
    mesh.addVertex(Vertex4D(Vector4(s, 0, 0, 0), Vector4::zero(), Vector4::zero(), Vector4(0, 1, 0, 1)));
    mesh.addVertex(Vertex4D(Vector4(0, s, 0, 0), Vector4::zero(), Vector4::zero(), Vector4(0, 0, 1, 1)));
    mesh.addVertex(Vertex4D(Vector4(0, 0, s, 0), Vector4::zero(), Vector4::zero(), Vector4(1, 1, 0, 1)));
    mesh.addVertex(Vertex4D(Vector4(0, 0, 0, s), Vector4::zero(), Vector4::zero(), Vector4(1, 0, 1, 1)));
    
    // Соединяем все вершины (тетраэдр в 4D)
    mesh.addTriangle(0, 1, 2);
    mesh.addTriangle(0, 1, 3);
    mesh.addTriangle(0, 1, 4);
    mesh.addTriangle(0, 2, 3);
    mesh.addTriangle(0, 2, 4);
    mesh.addTriangle(0, 3, 4);
    mesh.addTriangle(1, 2, 3);
    mesh.addTriangle(1, 2, 4);
    mesh.addTriangle(1, 3, 4);
    mesh.addTriangle(2, 3, 4);
    
    return mesh;
}

// Shader4D
Shader4D::Shader4D() : programID(0), isCompiled(false) {}

Shader4D::~Shader4D() {
    cleanup();
}

bool Shader4D::loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    std::ifstream vertexFile(vertexPath);
    std::ifstream fragmentFile(fragmentPath);
    
    if (!vertexFile.is_open() || !fragmentFile.is_open()) {
        std::cerr << "Ошибка: Не удалось открыть файлы шейдеров" << std::endl;
        return false;
    }
    
    std::stringstream vertexStream, fragmentStream;
    vertexStream << vertexFile.rdbuf();
    fragmentStream << fragmentFile.rdbuf();
    
    vertexFile.close();
    fragmentFile.close();
    
    return loadFromSource(vertexStream.str(), fragmentStream.str());
}

bool Shader4D::loadFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
    GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    
    if (vertexShader == 0 || fragmentShader == 0) {
        if (vertexShader) glDeleteShader(vertexShader);
        if (fragmentShader) glDeleteShader(fragmentShader);
        return false;
    }
    
    bool success = linkProgram(vertexShader, fragmentShader);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return success;
}

void Shader4D::use() const {
    if (this->isCompiled) {
        glUseProgram(this->programID);
    }
}

void Shader4D::cleanup() {
    if (this->programID) {
        glDeleteProgram(this->programID);
        this->programID = 0;
        this->isCompiled = false;
    }
}

void Shader4D::setMatrix4(const std::string& name, const Matrix4& matrix) const {
    if (!this->isCompiled) return;
    
    GLint location = glGetUniformLocation(this->programID, name.c_str());
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, &matrix.m[0][0]);
    }
}

void Shader4D::setVector4(const std::string& name, const Vector4& vector) const {
    if (!this->isCompiled) return;
    
    GLint location = glGetUniformLocation(this->programID, name.c_str());
    if (location != -1) {
        glUniform4f(location, vector.x, vector.y, vector.z, vector.w);
    }
}

void Shader4D::setFloat(const std::string& name, float value) const {
    if (!this->isCompiled) return;
    
    GLint location = glGetUniformLocation(this->programID, name.c_str());
    if (location != -1) {
        glUniform1f(location, value);
    }
}

void Shader4D::setInt(const std::string& name, int value) const {
    if (!this->isCompiled) return;
    
    GLint location = glGetUniformLocation(this->programID, name.c_str());
    if (location != -1) {
        glUniform1i(location, value);
    }
}

void Shader4D::setBool(const std::string& name, bool value) const {
    setInt(name, value ? 1 : 0);
}

GLuint Shader4D::compileShader(const std::string& source, GLenum type) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        std::cerr << "Ошибка компиляции шейдера: " << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

bool Shader4D::linkProgram(GLuint vertexShader, GLuint fragmentShader) {
    this->programID = glCreateProgram();
    glAttachShader(this->programID, vertexShader);
    glAttachShader(this->programID, fragmentShader);
    glLinkProgram(this->programID);
    
    GLint success;
    glGetProgramiv(this->programID, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[1024];
        glGetProgramInfoLog(this->programID, 1024, nullptr, infoLog);
        std::cerr << "Ошибка линковки программы: " << infoLog << std::endl;
        glDeleteProgram(this->programID);
        this->programID = 0;
        return false;
    }
    
    this->isCompiled = true;
    return true;
}

// Camera4D
Camera4D::Camera4D() {
    position = Vector4::zero();
    target = Vector4::unitZ();
    up = Vector4::unitY();
    right = Vector4::unitX();
    fov = 45.0f;
    nearPlane = 0.1f;
    farPlane = 100.0f;
    wDistance = 10.0f;
    crossSectionW = 0.0f;
    useCrossSection = false;
}

void Camera4D::setPosition(const Vector4& pos) {
    position = pos;
}

void Camera4D::setTarget(const Vector4& target) {
    this->target = target;
}

void Camera4D::lookAt(const Vector4& pos, const Vector4& target, const Vector4& up) {
    position = pos;
    this->target = target;
    this->up = up;
    
    // Вычисляем вектор "вправо"
    Vector4 forward = (target - pos).normalized();
    right = forward.cross(up).normalized();
}

void Camera4D::move(const Vector4& direction) {
    position += direction;
    target += direction;
}

void Camera4D::rotate(float angle, const Vector4& axis) {
    // Создаем матрицу поворота вокруг оси
    Matrix4 rotation = Matrix4::rotation(axis, angle);
    
    // Поворачиваем векторы направления
    Vector4 forward = (target - position).normalized();
    forward = rotation * forward;
    target = position + forward;
    
    up = rotation * up;
    right = forward.cross(up).normalized();
}

void Camera4D::setCrossSection(float wValue) {
    crossSectionW = wValue;
    useCrossSection = true;
}

Matrix4 Camera4D::getViewMatrix() const {
    Vector4 forward = (target - position).normalized();
    Vector4 right = forward.cross(up).normalized();
    Vector4 upCorrected = right.cross(forward).normalized();
    
    Matrix4 view;
    view.setIdentity();
    
    // Первая строка (право)
    view.m[0][0] = right.x;
    view.m[0][1] = right.y;
    view.m[0][2] = right.z;
    view.m[0][3] = right.w;
    
    // Вторая строка (вверх)
    view.m[1][0] = upCorrected.x;
    view.m[1][1] = upCorrected.y;
    view.m[1][2] = upCorrected.z;
    view.m[1][3] = upCorrected.w;
    
    // Третья строка (вперед)
    view.m[2][0] = -forward.x;
    view.m[2][1] = -forward.y;
    view.m[2][2] = -forward.z;
    view.m[2][3] = -forward.w;
    
    // Четвертая строка (позиция)
    view.m[3][0] = -position.x;
    view.m[3][1] = -position.y;
    view.m[3][2] = -position.z;
    view.m[3][3] = -position.w;
    
    return view;
}

Matrix4 Camera4D::getProjectionMatrix() const {
    if (useCrossSection) {
        return Matrix4::crossSectionProjection(crossSectionW);
    } else {
        return Matrix4::perspectiveProjection(wDistance);
    }
}

Matrix4 Camera4D::getViewProjectionMatrix() const {
    return getProjectionMatrix() * getViewMatrix();
}

void Camera4D::setAspectRatio(float aspect) {
    // Для 4D камеры aspect ratio не так важен, но можно использовать для настройки
    // В реальной реализации здесь должна быть логика настройки проекции
}

void Camera4D::setFieldOfView(float fov) {
    this->fov = fov;
}

void Camera4D::setNearPlane(float near) {
    this->nearPlane = near;
}

void Camera4D::setFarPlane(float far) {
    this->farPlane = far;
}

// Renderer
Renderer& Renderer::getInstance() {
    static Renderer instance;
    return instance;
}

bool Renderer::initialize(int width, int height) {
    m_width = width;
    m_height = height;
    m_initialized = true;
    m_projectionMode = ProjectionMode::Perspective;
    m_crossSectionW = 0.0f;
    
    // Инициализация OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glViewport(0, 0, width, height);
    
    return true;
}

void Renderer::cleanup() {
    m_initialized = false;
}

void Renderer::beginFrame() {
    if (!m_initialized) return;
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::endFrame() {
    // В OpenGL не нужно ничего делать в конце кадра
}

void Renderer::setClearColor(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
}

void Renderer::clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::renderMesh(const Mesh4D& mesh, const Matrix4& transform, const Shader4D& shader) {
    if (!m_initialized || !mesh.isUploaded) return;
    
    shader.use();
    shader.setMatrix4("model", transform);
    
    mesh.render();
}

void Renderer::renderWireframe(const Mesh4D& mesh, const Matrix4& transform, const Shader4D& shader) {
    if (!m_initialized || !mesh.isUploaded) return;
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    renderMesh(mesh, transform, shader);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Renderer::setViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
    m_width = width;
    m_height = height;
}

void Renderer::enableDepthTest(bool enable) {
    if (enable) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

void Renderer::enableBlending(bool enable) {
    if (enable) {
        glEnable(GL_BLEND);
    } else {
        glDisable(GL_BLEND);
    }
}

void Renderer::setProjectionMode(ProjectionMode mode) {
    m_projectionMode = mode;
}

void Renderer::setCrossSection(float wValue) {
    m_crossSectionW = wValue;
}

} // namespace Rendering
} // namespace Engine4D
