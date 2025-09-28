#pragma once

#include <memory>
#include <vector>
#include "../Math/Matrix4.h"
#include "../Math/Vector3.h"

namespace HyperEngine {
namespace Rendering {

/**
 * @brief Структура вершины для 3D меша
 */
struct Vertex3D {
    Math::Vector3 position;  // Позиция вершины
    Math::Vector3 normal;    // Нормаль вершины
    Math::Vector3 color;     // Цвет вершины
    float u, v;              // Текстурные координаты

    Vertex3D()
        : position(0.0f, 0.0f, 0.0f),
          normal(0.0f, 1.0f, 0.0f),
          color(1.0f, 1.0f, 1.0f),
          u(0.0f),
          v(0.0f) {}

    Vertex3D(const Math::Vector3& pos,
             const Math::Vector3& norm = Math::Vector3(0.0f, 1.0f, 0.0f),
             const Math::Vector3& col = Math::Vector3(1.0f, 1.0f, 1.0f),
             float u = 0.0f,
             float v = 0.0f)
        : position(pos), normal(norm), color(col), u(u), v(v) {}
};

/**
 * @brief 3D меш для хранения геометрии
 */
class Mesh3D {
  public:
    Mesh3D();
    virtual ~Mesh3D();

    // Управление вершинами и индексами
    void addVertex(const Vertex3D& vertex);
    void addTriangle(unsigned int i1, unsigned int i2, unsigned int i3);
    void addQuad(unsigned int i1, unsigned int i2, unsigned int i3, unsigned int i4);
    void setVertices(const std::vector<Vertex3D>& vertices);
    void setIndices(const std::vector<unsigned int>& indices);

    // Геттеры
    const std::vector<Vertex3D>& getVertices() const { return vertices; }
    const std::vector<unsigned int>& getIndices() const { return indices; }
    size_t getVertexCount() const { return vertices.size(); }
    size_t getIndexCount() const { return indices.size(); }
    size_t getTriangleCount() const { return indices.size() / 3; }

    // Геометрические операции
    void calculateNormals();   // Вычисление нормалей
    void calculateTangents();  // Вычисление касательных для normal mapping
    Math::Vector3 getCenter() const;              // Центр меша
    Math::Vector3 getSize() const;                // Размер bounding box
    void transform(const Math::Matrix4& matrix);  // Трансформация меша

    // GPU операции (интерфейс)
    virtual void uploadToGPU();
    virtual void render() const;
    virtual void cleanup();
    bool isUploaded() const { return uploaded; }

    // Создание примитивов
    static std::shared_ptr<Mesh3D> createCube(float size = 1.0f);
    static std::shared_ptr<Mesh3D> createSphere(float radius = 1.0f, int segments = 32);
    static std::shared_ptr<Mesh3D> createPlane(float width = 1.0f, float height = 1.0f);
    static std::shared_ptr<Mesh3D> createCylinder(float radius = 1.0f,
                                                  float height = 2.0f,
                                                  int segments = 32);
    static std::shared_ptr<Mesh3D> createCone(float radius = 1.0f,
                                              float height = 2.0f,
                                              int segments = 32);

    // Загрузка из файла
    static std::shared_ptr<Mesh3D> loadFromFile(const std::string& filepath);

  protected:
    std::vector<Vertex3D> vertices;
    std::vector<unsigned int> indices;
    bool uploaded;

    // GPU ресурсы (будут реализованы в наследниках)
    unsigned int VAO, VBO, EBO;

    void calculateBoundingBox();
    Math::Vector3 minBounds, maxBounds;
};

}  // namespace Rendering
}  // namespace HyperEngine

