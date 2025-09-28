#include "HyperEngine/Rendering/Mesh3D.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include "HyperEngine/Math/MathConstants.h"
#include "HyperEngine/Math/Matrix4.h"

using namespace HyperEngine::Math;
using namespace HyperEngine::Rendering;

namespace HyperEngine {
namespace Rendering {

// Конструктор и деструктор
Mesh3D::Mesh3D()
    : uploaded(false),
      VAO(0),
      VBO(0),
      EBO(0),
      minBounds(0.0f, 0.0f, 0.0f),
      maxBounds(0.0f, 0.0f, 0.0f) {}

Mesh3D::~Mesh3D() {
    cleanup();
}

// Управление вершинами и индексами
void Mesh3D::addVertex(const Vertex3D& vertex) {
    vertices.push_back(vertex);
    calculateBoundingBox();
}

void Mesh3D::addTriangle(unsigned int i1, unsigned int i2, unsigned int i3) {
    indices.push_back(i1);
    indices.push_back(i2);
    indices.push_back(i3);
}

void Mesh3D::addQuad(unsigned int i1, unsigned int i2, unsigned int i3, unsigned int i4) {
    // Первый треугольник
    addTriangle(i1, i2, i3);
    // Второй треугольник
    addTriangle(i1, i3, i4);
}

void Mesh3D::setVertices(const std::vector<Vertex3D>& newVertices) {
    vertices = newVertices;
    calculateBoundingBox();
    uploaded = false;  // Нужно загрузить заново
}

void Mesh3D::setIndices(const std::vector<unsigned int>& newIndices) {
    indices = newIndices;
    uploaded = false;  // Нужно загрузить заново
}

// Геометрические операции
void Mesh3D::calculateNormals() {
    // Сброс нормалей
    for (auto& vertex : vertices) {
        vertex.normal = Vector3::zero();
    }

    // Вычисление нормалей для каждого треугольника
    for (size_t i = 0; i < indices.size(); i += 3) {
        if (i + 2 < indices.size()) {
            const auto& v1 = vertices[indices[i]];
            const auto& v2 = vertices[indices[i + 1]];
            const auto& v3 = vertices[indices[i + 2]];

            Vector3 edge1 = v2.position - v1.position;
            Vector3 edge2 = v3.position - v1.position;
            Vector3 normal = edge1.cross(edge2).normalized();

            // Добавляем нормаль к каждой вершине треугольника
            vertices[indices[i]].normal += normal;
            vertices[indices[i + 1]].normal += normal;
            vertices[indices[i + 2]].normal += normal;
        }
    }

    // Нормализация нормалей
    for (auto& vertex : vertices) {
        vertex.normal.normalize();
    }

    uploaded = false;  // Нужно загрузить заново
}

void Mesh3D::calculateTangents() {
    // TODO: Реализовать вычисление касательных для normal mapping
    std::cout << "Warning: calculateTangents() not implemented yet\n";
}

Vector3 Mesh3D::getCenter() const {
    return (minBounds + maxBounds) * 0.5f;
}

Vector3 Mesh3D::getSize() const {
    return maxBounds - minBounds;
}

void Mesh3D::transform(const Matrix4& matrix) {
    for (auto& vertex : vertices) {
        vertex.position = matrix.transformPoint(vertex.position);
        vertex.normal = matrix.transformDirection(vertex.normal).normalized();
    }

    calculateBoundingBox();
    uploaded = false;  // Нужно загрузить заново
}

// GPU операции (заглушки для базового класса)
void Mesh3D::uploadToGPU() {
    std::cout << "Mesh3D::uploadToGPU() - base implementation, override in derived class\n";
    uploaded = true;
}

void Mesh3D::render() const {
    std::cout << "Mesh3D::render() - base implementation, override in derived class\n";
}

void Mesh3D::cleanup() {
    vertices.clear();
    indices.clear();
    uploaded = false;
}

// Создание примитивов
std::shared_ptr<Mesh3D> Mesh3D::createCube(float size) {
    auto mesh = std::make_shared<Mesh3D>();

    float halfSize = size * 0.5f;

    // 8 вершин куба
    std::vector<Vertex3D> cubeVertices = {
        // Передняя грань
        {{-halfSize, -halfSize, halfSize}, {0, 0, 1}, {1, 1, 1}, 0, 0},
        {{halfSize, -halfSize, halfSize}, {0, 0, 1}, {1, 1, 1}, 1, 0},
        {{halfSize, halfSize, halfSize}, {0, 0, 1}, {1, 1, 1}, 1, 1},
        {{-halfSize, halfSize, halfSize}, {0, 0, 1}, {1, 1, 1}, 0, 1},

        // Задняя грань
        {{-halfSize, -halfSize, -halfSize}, {0, 0, -1}, {1, 1, 1}, 1, 0},
        {{-halfSize, halfSize, -halfSize}, {0, 0, -1}, {1, 1, 1}, 1, 1},
        {{halfSize, halfSize, -halfSize}, {0, 0, -1}, {1, 1, 1}, 0, 1},
        {{halfSize, -halfSize, -halfSize}, {0, 0, -1}, {1, 1, 1}, 0, 0},

        // Левая грань
        {{-halfSize, -halfSize, -halfSize}, {-1, 0, 0}, {1, 1, 1}, 0, 0},
        {{-halfSize, -halfSize, halfSize}, {-1, 0, 0}, {1, 1, 1}, 1, 0},
        {{-halfSize, halfSize, halfSize}, {-1, 0, 0}, {1, 1, 1}, 1, 1},
        {{-halfSize, halfSize, -halfSize}, {-1, 0, 0}, {1, 1, 1}, 0, 1},

        // Правая грань
        {{halfSize, -halfSize, -halfSize}, {1, 0, 0}, {1, 1, 1}, 1, 0},
        {{halfSize, halfSize, -halfSize}, {1, 0, 0}, {1, 1, 1}, 1, 1},
        {{halfSize, halfSize, halfSize}, {1, 0, 0}, {1, 1, 1}, 0, 1},
        {{halfSize, -halfSize, halfSize}, {1, 0, 0}, {1, 1, 1}, 0, 0},

        // Нижняя грань
        {{-halfSize, -halfSize, -halfSize}, {0, -1, 0}, {1, 1, 1}, 0, 1},
        {{halfSize, -halfSize, -halfSize}, {0, -1, 0}, {1, 1, 1}, 1, 1},
        {{halfSize, -halfSize, halfSize}, {0, -1, 0}, {1, 1, 1}, 1, 0},
        {{-halfSize, -halfSize, halfSize}, {0, -1, 0}, {1, 1, 1}, 0, 0},

        // Верхняя грань
        {{-halfSize, halfSize, -halfSize}, {0, 1, 0}, {1, 1, 1}, 0, 0},
        {{-halfSize, halfSize, halfSize}, {0, 1, 0}, {1, 1, 1}, 0, 1},
        {{halfSize, halfSize, halfSize}, {0, 1, 0}, {1, 1, 1}, 1, 1},
        {{halfSize, halfSize, -halfSize}, {0, 1, 0}, {1, 1, 1}, 1, 0}};

    // Индексы для треугольников
    std::vector<unsigned int> cubeIndices = {
        0,  1,  2,  2,  3,  0,   // Передняя грань
        4,  5,  6,  6,  7,  4,   // Задняя грань
        8,  9,  10, 10, 11, 8,   // Левая грань
        12, 13, 14, 14, 15, 12,  // Правая грань
        16, 17, 18, 18, 19, 16,  // Нижняя грань
        20, 21, 22, 22, 23, 20   // Верхняя грань
    };

    mesh->setVertices(cubeVertices);
    mesh->setIndices(cubeIndices);

    return mesh;
}

std::shared_ptr<Mesh3D> Mesh3D::createSphere(float radius, int segments) {
    auto mesh = std::make_shared<Mesh3D>();

    std::vector<Vertex3D> sphereVertices;
    std::vector<unsigned int> sphereIndices;

    // Создание вершин сферы
    for (int lat = 0; lat <= segments; ++lat) {
        float theta = lat * M_PI / segments;
        float sinTheta = std::sin(theta);
        float cosTheta = std::cos(theta);

        for (int lon = 0; lon <= segments; ++lon) {
            float phi = lon * 2 * M_PI / segments;
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            Vector3 position(
                radius * cosPhi * sinTheta, radius * cosTheta, radius * sinPhi * sinTheta);
            Vector3 normal = position.normalized();
            float u = 1.0f - (float)lon / segments;
            float v = 1.0f - (float)lat / segments;

            sphereVertices.emplace_back(position, normal, Vector3(1, 1, 1), u, v);
        }
    }

    // Создание индексов
    for (int lat = 0; lat < segments; ++lat) {
        for (int lon = 0; lon < segments; ++lon) {
            int first = lat * (segments + 1) + lon;
            int second = first + segments + 1;

            sphereIndices.push_back(first);
            sphereIndices.push_back(second);
            sphereIndices.push_back(first + 1);

            sphereIndices.push_back(second);
            sphereIndices.push_back(second + 1);
            sphereIndices.push_back(first + 1);
        }
    }

    mesh->setVertices(sphereVertices);
    mesh->setIndices(sphereIndices);

    return mesh;
}

std::shared_ptr<Mesh3D> Mesh3D::createPlane(float width, float height) {
    auto mesh = std::make_shared<Mesh3D>();

    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;

    std::vector<Vertex3D> planeVertices = {
        {{-halfWidth, 0, -halfHeight}, {0, 1, 0}, {1, 1, 1}, 0, 0},
        {{halfWidth, 0, -halfHeight}, {0, 1, 0}, {1, 1, 1}, 1, 0},
        {{halfWidth, 0, halfHeight}, {0, 1, 0}, {1, 1, 1}, 1, 1},
        {{-halfWidth, 0, halfHeight}, {0, 1, 0}, {1, 1, 1}, 0, 1}};

    std::vector<unsigned int> planeIndices = {0, 1, 2, 2, 3, 0};

    mesh->setVertices(planeVertices);
    mesh->setIndices(planeIndices);

    return mesh;
}

std::shared_ptr<Mesh3D> Mesh3D::createCylinder(float radius, float height, int segments) {
    auto mesh = std::make_shared<Mesh3D>();

    std::vector<Vertex3D> cylinderVertices;
    std::vector<unsigned int> cylinderIndices;

    float halfHeight = height * 0.5f;

    // Центры оснований
    cylinderVertices.emplace_back(
        Vector3(0, halfHeight, 0), Vector3(0, 1, 0), Vector3(1, 1, 1), 0.5f, 0.5f);  // Верх
    cylinderVertices.emplace_back(
        Vector3(0, -halfHeight, 0), Vector3(0, -1, 0), Vector3(1, 1, 1), 0.5f, 0.5f);  // Низ

    // Вершины окружностей
    for (int i = 0; i <= segments; ++i) {
        float angle = 2 * M_PI * i / segments;
        float x = radius * std::cos(angle);
        float z = radius * std::sin(angle);
        float u = (float)i / segments;

        // Верхняя окружность
        cylinderVertices.emplace_back(
            Vector3(x, halfHeight, z), Vector3(0, 1, 0), Vector3(1, 1, 1), u, 0);
        // Нижняя окружность
        cylinderVertices.emplace_back(
            Vector3(x, -halfHeight, z), Vector3(0, -1, 0), Vector3(1, 1, 1), u, 1);
    }

    // Боковая поверхность
    for (int i = 0; i <= segments; ++i) {
        float angle = 2 * M_PI * i / segments;
        float x = radius * std::cos(angle);
        float z = radius * std::sin(angle);
        Vector3 normal(x, 0, z);
        normal.normalize();
        float u = (float)i / segments;

        cylinderVertices.emplace_back(Vector3(x, halfHeight, z), normal, Vector3(1, 1, 1), u, 0);
        cylinderVertices.emplace_back(Vector3(x, -halfHeight, z), normal, Vector3(1, 1, 1), u, 1);
    }

    // Индексы (упрощенная версия)
    // TODO: Реализовать правильные индексы для цилиндра

    mesh->setVertices(cylinderVertices);
    mesh->setIndices(cylinderIndices);

    return mesh;
}

std::shared_ptr<Mesh3D> Mesh3D::createCone(float radius, float height, int segments) {
    // TODO: Реализовать создание конуса
    std::cout << "Warning: createCone() not implemented yet\n";
    return createCylinder(radius, height, segments);
}

std::shared_ptr<Mesh3D> Mesh3D::loadFromFile(const std::string& filepath) {
    // TODO: Реализовать загрузку из файла (OBJ, FBX, etc.)
    std::cout << "Warning: loadFromFile() not implemented yet for " << filepath << "\n";
    return createCube();
}

// Вспомогательные методы
void Mesh3D::calculateBoundingBox() {
    if (vertices.empty()) {
        minBounds = maxBounds = Vector3::zero();
        return;
    }

    minBounds = maxBounds = vertices[0].position;

    for (const auto& vertex : vertices) {
        minBounds.x = std::min(minBounds.x, vertex.position.x);
        minBounds.y = std::min(minBounds.y, vertex.position.y);
        minBounds.z = std::min(minBounds.z, vertex.position.z);

        maxBounds.x = std::max(maxBounds.x, vertex.position.x);
        maxBounds.y = std::max(maxBounds.y, vertex.position.y);
        maxBounds.z = std::max(maxBounds.z, vertex.position.z);
    }
}

}  // namespace Rendering
}  // namespace HyperEngine
