/**
 * @file MeshConnectivity.hpp
 * @brief Базовые структуры для перехода от triangle soup к vertex-based представлению с connectivity (Фаза 1)
 */

#pragma once

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <map>
#include <unordered_map>
#include <vector>

#include "SpectraForge/Math/Vector3.h"

namespace SpectraForge {
namespace Rendering {

/**
 * @brief Вершина с данными для Triangle Splatting и списками смежности
 */
struct TriangleSplattingVertex {
    alignas(16) Math::Vector3 position;  ///< Позиция вершины
    alignas(16) Math::Vector3 color;     ///< Цвет вершины
    float opacity;                       ///< Прозрачность [0,1]

    // Connectivity
    std::vector<uint32_t> adjacent_triangles;  ///< Индексы треугольников, использующих эту вершину
    std::vector<uint32_t> adjacent_vertices;   ///< Индексы смежных вершин

    /** @brief Конструктор по умолчанию */
    TriangleSplattingVertex() : position(0.0f, 0.0f, 0.0f), color(0.0f, 0.0f, 0.0f), opacity(1.0f) {}

    /** @brief Конструктор с параметрами */
    TriangleSplattingVertex(const Math::Vector3& pos, const Math::Vector3& col, float op)
        : position(pos), color(col), opacity(op) {}

    /**
     * @brief Сравнение вершин с допуском (epsilon)
     */
    bool operator==(const TriangleSplattingVertex& other) const {
        static constexpr float EPSILON = 1e-5f;
        return (position - other.position).magnitude() < EPSILON &&
               (color - other.color).magnitude() < EPSILON &&
               std::abs(opacity - other.opacity) < EPSILON;
    }

    /**
     * @brief Хэш для пространственного хеширования (дискретизация координат)
     */
    size_t hash() const {
        static constexpr float GRID_SIZE = 1e-5f;
        auto discretize = [](float x) { return static_cast<int64_t>(x / GRID_SIZE); };

        int64_t x = discretize(position.x);
        int64_t y = discretize(position.y);
        int64_t z = discretize(position.z);

        size_t h1 = std::hash<int64_t>{}(x);
        size_t h2 = std::hash<int64_t>{}(y);
        size_t h3 = std::hash<int64_t>{}(z);

        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }

    /** @brief Добавить треугольник в список смежности вершины */
    void add_adjacent_triangle(uint32_t triangle_id) {
        if (std::find(adjacent_triangles.begin(), adjacent_triangles.end(), triangle_id) == adjacent_triangles.end()) {
            adjacent_triangles.push_back(triangle_id);
        }
    }

    /** @brief Добавить смежную вершину */
    void add_adjacent_vertex(uint32_t vertex_id) {
        if (std::find(adjacent_vertices.begin(), adjacent_vertices.end(), vertex_id) == adjacent_vertices.end()) {
            adjacent_vertices.push_back(vertex_id);
        }
    }

    // Совместимость с существующими примерами (camelCase алиасы)
    void addAdjacentTriangle(uint32_t triangleId) { add_adjacent_triangle(triangleId); }
    void addAdjacentVertex(uint32_t vertexId) { add_adjacent_vertex(vertexId); }
};

/**
 * @brief Треугольник с полными данными смежности и геометрии
 */
struct ConnectedTriangle {
    uint32_t indices[3];     ///< Индексы вершин (i, j, k)
    float smoothness;        ///< Параметр сглаживания σ
    uint32_t neighbors[3];   ///< Индексы соседних треугольников (UINT32_MAX, если нет)

    // Геометрия
    Math::Vector3 normal;  ///< Нормаль грани
    float area;            ///< Площадь треугольника

    ConnectedTriangle() : smoothness(1.0f), area(0.0f) {
        indices[0] = indices[1] = indices[2] = 0;
        neighbors[0] = neighbors[1] = neighbors[2] = UINT32_MAX;
    }

    ConnectedTriangle(uint32_t i, uint32_t j, uint32_t k) : smoothness(1.0f), area(0.0f) {
        indices[0] = i; indices[1] = j; indices[2] = k;
        neighbors[0] = neighbors[1] = neighbors[2] = UINT32_MAX;
    }

    /**
     * @brief Барицентрические координаты точки относительно треугольника
     */
    Math::Vector3 compute_barycentric(const Math::Vector3& point,
                                      const std::vector<TriangleSplattingVertex>& vertices) const {
        const Math::Vector3& v0 = vertices[indices[0]].position;
        const Math::Vector3& v1 = vertices[indices[1]].position;
        const Math::Vector3& v2 = vertices[indices[2]].position;

        Math::Vector3 v0v1 = v1 - v0;
        Math::Vector3 v0v2 = v2 - v0;
        Math::Vector3 v0p  = point - v0;

        float d00 = Math::Vector3::dot(v0v1, v0v1);
        float d01 = Math::Vector3::dot(v0v1, v0v2);
        float d11 = Math::Vector3::dot(v0v2, v0v2);
        float d20 = Math::Vector3::dot(v0p,  v0v1);
        float d21 = Math::Vector3::dot(v0p,  v0v2);

        float denom = d00 * d11 - d01 * d01;
        if (std::abs(denom) < 1e-8f) {
            // Вырожденный треугольник: сводим к отрезку (или точке)
            // Выбираем самое длинное ребро и проектируем точку на него
            const Math::Vector3 e01 = v1 - v0;
            const Math::Vector3 e12 = v2 - v1;
            const Math::Vector3 e20 = v0 - v2;

            const float l01 = Math::Vector3::dot(e01, e01);
            const float l12 = Math::Vector3::dot(e12, e12);
            const float l20 = Math::Vector3::dot(e20, e20);

            uint32_t i = 0, j = 1, k = 2;  // пара вершин ребра (i,j), k — оставшаяся
            Math::Vector3 a = v0;
            Math::Vector3 b = v1;
            float max_len2 = l01;

            if (l12 > max_len2) {
                max_len2 = l12; i = 1; j = 2; k = 0; a = v1; b = v2;
            }
            if (l20 > max_len2) {
                max_len2 = l20; i = 2; j = 0; k = 1; a = v2; b = v0;
            }

            // Все вершины практически совпадают — выбираем ближайшую к точке
            if (max_len2 < 1e-12f) {
                const float d0 = Math::Vector3::dot(point - v0, point - v0);
                const float d1 = Math::Vector3::dot(point - v1, point - v1);
                const float d2 = Math::Vector3::dot(point - v2, point - v2);
                float w0 = 0.0f, w1 = 0.0f, w2 = 0.0f;
                if (d0 <= d1 && d0 <= d2) w0 = 1.0f;
                else if (d1 <= d0 && d1 <= d2) w1 = 1.0f;
                else w2 = 1.0f;
                return Math::Vector3(w0, w1, w2);
            }

            float t = Math::Vector3::dot(point - a, b - a) / max_len2;
            t = std::clamp(t, 0.0f, 1.0f);

            float w0 = 0.0f, w1 = 0.0f, w2 = 0.0f;
            const float wi = 1.0f - t;
            const float wj = t;

            if (i == 0) w0 = wi; else if (i == 1) w1 = wi; else w2 = wi;
            if (j == 0) w0 = wj; else if (j == 1) w1 = wj; else w2 = wj;
            // Вес третьей вершины k остаётся 0
            return Math::Vector3(w0, w1, w2);
        }

        float v = (d11 * d20 - d01 * d21) / denom;
        float w = (d00 * d21 - d01 * d20) / denom;
        float u = 1.0f - v - w;
        return Math::Vector3(u, v, w);
    }

    /**
     * @brief Барицентрическая интерполяция цвета
     */
    Math::Vector3 interpolate_color(const Math::Vector3& barycentric,
                                    const std::vector<TriangleSplattingVertex>& vertices) const {
        const Math::Vector3& c0 = vertices[indices[0]].color;
        const Math::Vector3& c1 = vertices[indices[1]].color;
        const Math::Vector3& c2 = vertices[indices[2]].color;
        return barycentric.x * c0 + barycentric.y * c1 + barycentric.z * c2;
    }

    /** @brief Пересчёт нормали и площади */
    void compute_geometry(const std::vector<TriangleSplattingVertex>& vertices) {
        const Math::Vector3& v0 = vertices[indices[0]].position;
        const Math::Vector3& v1 = vertices[indices[1]].position;
        const Math::Vector3& v2 = vertices[indices[2]].position;

        Math::Vector3 edge1 = v1 - v0;
        Math::Vector3 edge2 = v2 - v0;
        Math::Vector3 cross = edge1.cross(edge2);

        area = cross.magnitude() * 0.5f;
        normal = (area > 1e-8f) ? cross.normalized() : Math::Vector3(0.0f, 1.0f, 0.0f);
    }

    /** @brief Индекс вершины, противоположной ребру edge_index */
    uint32_t get_opposite_vertex(uint32_t edge_index) const { return indices[(edge_index + 2) % 3]; }

    /** @brief Проверка соседства с треугольником triangle_id */
    bool is_neighbor(uint32_t triangle_id) const {
        return neighbors[0] == triangle_id || neighbors[1] == triangle_id || neighbors[2] == triangle_id;
    }

    /** @brief Получить концы ребра по индексу */
    std::pair<uint32_t, uint32_t> get_edge(uint32_t edge_index) const {
        uint32_t v1 = indices[edge_index];
        uint32_t v2 = indices[(edge_index + 1) % 3];
        return std::make_pair(std::min(v1, v2), std::max(v1, v2));
    }

    // Совместимость с существующими примерами (camelCase алиасы)
    Math::Vector3 computeBarycentric(const Math::Vector3& p,
                                     const std::vector<TriangleSplattingVertex>& v) const { return compute_barycentric(p, v); }
    Math::Vector3 interpolateColor(const Math::Vector3& b,
                                   const std::vector<TriangleSplattingVertex>& v) const { return interpolate_color(b, v); }
    void computeGeometry(const std::vector<TriangleSplattingVertex>& v) { compute_geometry(v); }
    uint32_t getOppositeVertex(uint32_t e) const { return get_opposite_vertex(e); }
    bool isNeighbor(uint32_t t) const { return is_neighbor(t); }
    std::pair<uint32_t, uint32_t> getEdge(uint32_t e) const { return get_edge(e); }
};

/**
 * @brief Дедупликация вершин с использованием пространственного хеша
 */
class VertexDeduplicator {
  private:
    std::unordered_map<size_t, std::vector<uint32_t>> spatial_hash;  ///< Хэш: ключ -> список индексов
    std::vector<TriangleSplattingVertex> unique_vertices;            ///< Уникальные вершины

  public:
    /** @brief Добавить вершину или вернуть индекс уже существующей подобной */
    uint32_t add_or_get_vertex(const TriangleSplattingVertex& vertex) {
        size_t hv = vertex.hash();

        auto it = spatial_hash.find(hv);
        if (it != spatial_hash.end()) {
            for (uint32_t existing_id : it->second) {
                if (unique_vertices[existing_id] == vertex) {
                    return existing_id;
                }
            }
        }

        uint32_t new_id = static_cast<uint32_t>(unique_vertices.size());
        unique_vertices.push_back(vertex);
        spatial_hash[hv].push_back(new_id);
        return new_id;
    }

    /** @brief Доступ к массиву уникальных вершин */
    const std::vector<TriangleSplattingVertex>& get_vertices() const { return unique_vertices; }
    /** @brief Количество уникальных вершин */
    size_t get_vertex_count() const { return unique_vertices.size(); }

    // Совместимость с существующими примерами (camelCase алиасы)
    uint32_t addOrGetVertex(const TriangleSplattingVertex& v) { return add_or_get_vertex(v); }
    const std::vector<TriangleSplattingVertex>& getVertices() const { return get_vertices(); }
    size_t getVertexCount() const { return get_vertex_count(); }
};

}  // namespace Rendering
}  // namespace SpectraForge


