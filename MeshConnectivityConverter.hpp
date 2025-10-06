/**
 * @file MeshConnectivityConverter.hpp  
 * @brief Система конвертации Triangle Soup в Vertex-Based Mesh с connectivity
 * @author SpectraForge Team
 */

#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <algorithm>
#include <cmath>
#include "Math/Vector3.hpp"

namespace Engine4D::Rendering {

/**
 * @brief Vertex structure with connectivity information for Triangle Splatting
 */
struct TriangleSplattingVertex {
    alignas(16) Math::Vector3 position;    ///< (x_i, y_i, z_i) ∈ ℝ³
    alignas(16) Math::Vector3 color;       ///< c_i ∈ ℝ³ vertex color
    float opacity;                         ///< o_i ∈ [0,1] vertex opacity
    
    // Connectivity data
    std::vector<uint32_t> adjacent_triangles;  ///< Triangle indices using this vertex
    std::vector<uint32_t> adjacent_vertices;   ///< Connected vertex indices
    
    // Construction
    TriangleSplattingVertex() : position(0,0,0), color(0,0,0), opacity(1.0f) {}
    TriangleSplattingVertex(const Math::Vector3& pos, const Math::Vector3& col, float op)
        : position(pos), color(col), opacity(op) {}
    
    // Comparison for deduplication (with epsilon tolerance)
    bool operator==(const TriangleSplattingVertex& other) const {
        static constexpr float EPSILON = 1e-6f;
        return (position - other.position).magnitude() < EPSILON &&
               (color - other.color).magnitude() < EPSILON &&
               std::abs(opacity - other.opacity) < EPSILON;
    }
    
    // Hash function for spatial hashing
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
    
    void addAdjacentTriangle(uint32_t triangleId) {
        if (std::find(adjacent_triangles.begin(), adjacent_triangles.end(), triangleId) == adjacent_triangles.end()) {
            adjacent_triangles.push_back(triangleId);
        }
    }
    
    void addAdjacentVertex(uint32_t vertexId) {
        if (std::find(adjacent_vertices.begin(), adjacent_vertices.end(), vertexId) == adjacent_vertices.end()) {
            adjacent_vertices.push_back(vertexId);
        }
    }
};

/**
 * @brief Triangle with full connectivity information
 */
struct ConnectedTriangle {
    uint32_t indices[3];                   ///< Vertex indices (i, j, k)
    float smoothness;                      ///< σ trainable parameter
    uint32_t neighbors[3];                 ///< Adjacent triangle IDs (UINT32_MAX if no neighbor)
    
    // Geometric properties
    Math::Vector3 normal;                  ///< Face normal
    float area;                           ///< Triangle area
    
    // Construction
    ConnectedTriangle() : smoothness(1.0f), area(0.0f) {
        indices[0] = indices[1] = indices[2] = 0;
        neighbors[0] = neighbors[1] = neighbors[2] = UINT32_MAX;
    }
    
    ConnectedTriangle(uint32_t i, uint32_t j, uint32_t k) : smoothness(1.0f), area(0.0f) {
        indices[0] = i; indices[1] = j; indices[2] = k;
        neighbors[0] = neighbors[1] = neighbors[2] = UINT32_MAX;
    }
    
    /**
     * @brief Compute barycentric coordinates for a point relative to this triangle
     * @param point Point in 3D space
     * @param vertices Vertex array reference
     * @return Barycentric coordinates (λi, λj, λk)
     */
    Math::Vector3 computeBarycentric(const Math::Vector3& point, 
                                   const std::vector<TriangleSplattingVertex>& vertices) const {
        const Math::Vector3& v0 = vertices[indices[0]].position;
        const Math::Vector3& v1 = vertices[indices[1]].position;
        const Math::Vector3& v2 = vertices[indices[2]].position;
        
        Math::Vector3 v0v1 = v1 - v0;
        Math::Vector3 v0v2 = v2 - v0;
        Math::Vector3 v0p = point - v0;
        
        float d00 = Math::dot(v0v1, v0v1);
        float d01 = Math::dot(v0v1, v0v2);
        float d11 = Math::dot(v0v2, v0v2);
        float d20 = Math::dot(v0p, v0v1);
        float d21 = Math::dot(v0p, v0v2);
        
        float denom = d00 * d11 - d01 * d01;
        if (std::abs(denom) < 1e-8f) {
            // Degenerate triangle
            return Math::Vector3(1.0f, 0.0f, 0.0f);
        }
        
        float v = (d11 * d20 - d01 * d21) / denom;
        float w = (d00 * d21 - d01 * d20) / denom;
        float u = 1.0f - v - w;
        
        return Math::Vector3(u, v, w);
    }
    
    /**
     * @brief Barycentric color interpolation: cT(λi,λj,λk) = λi*ci + λj*cj + λk*ck
     * @param barycentric Barycentric coordinates
     * @param vertices Vertex array reference  
     * @return Interpolated color
     */
    Math::Vector3 interpolateColor(const Math::Vector3& barycentric,
                                 const std::vector<TriangleSplattingVertex>& vertices) const {
        const Math::Vector3& c0 = vertices[indices[0]].color;
        const Math::Vector3& c1 = vertices[indices[1]].color;
        const Math::Vector3& c2 = vertices[indices[2]].color;
        
        return barycentric.x * c0 + barycentric.y * c1 + barycentric.z * c2;
    }
    
    /**
     * @brief Compute face normal and area
     */
    void computeGeometry(const std::vector<TriangleSplattingVertex>& vertices) {
        const Math::Vector3& v0 = vertices[indices[0]].position;
        const Math::Vector3& v1 = vertices[indices[1]].position;
        const Math::Vector3& v2 = vertices[indices[2]].position;
        
        Math::Vector3 edge1 = v1 - v0;
        Math::Vector3 edge2 = v2 - v0;
        Math::Vector3 cross = Math::cross(edge1, edge2);
        
        area = cross.magnitude() * 0.5f;
        normal = (area > 1e-8f) ? cross.normalized() : Math::Vector3(0, 1, 0);
    }
    
    /**
     * @brief Get the vertex index opposite to the given edge
     */
    uint32_t getOppositeVertex(uint32_t edgeIndex) const {
        return indices[(edgeIndex + 2) % 3];
    }
    
    /**
     * @brief Check if this triangle is neighbor to another
     */
    bool isNeighbor(uint32_t triangleId) const {
        return neighbors[0] == triangleId || neighbors[1] == triangleId || neighbors[2] == triangleId;
    }
    
    /**
     * @brief Get edge endpoints for the given edge index
     */
    std::pair<uint32_t, uint32_t> getEdge(uint32_t edgeIndex) const {
        uint32_t v1 = indices[edgeIndex];
        uint32_t v2 = indices[(edgeIndex + 1) % 3];
        return std::make_pair(std::min(v1, v2), std::max(v1, v2));
    }
};

/**
 * @brief Vertex deduplication using spatial hashing
 */
class VertexDeduplicator {
private:
    static constexpr float EPSILON = 1e-6f;
    std::unordered_map<size_t, std::vector<uint32_t>> spatial_hash;
    std::vector<TriangleSplattingVertex> unique_vertices;
    
public:
    /**
     * @brief Add vertex or return index of existing similar vertex
     */
    uint32_t addOrGetVertex(const TriangleSplattingVertex& vertex) {
        size_t hash = vertex.hash();
        
        // Check existing vertices in the same hash bucket
        auto it = spatial_hash.find(hash);
        if (it != spatial_hash.end()) {
            for (uint32_t existingId : it->second) {
                if (unique_vertices[existingId] == vertex) {
                    return existingId;
                }
            }
        }
        
        // Add new vertex
        uint32_t newId = static_cast<uint32_t>(unique_vertices.size());
        unique_vertices.push_back(vertex);
        spatial_hash[hash].push_back(newId);
        
        return newId;
    }
    
    const std::vector<TriangleSplattingVertex>& getVertices() const { return unique_vertices; }
    size_t getVertexCount() const { return unique_vertices.size(); }
};

/**
 * @brief Connectivity builder for constructing adjacency information
 */
class ConnectivityBuilder {
private:
    std::vector<TriangleSplattingVertex>* vertices;
    std::vector<ConnectedTriangle>* triangles;
    
    // Edge to triangles mapping for adjacency construction
    std::map<std::pair<uint32_t, uint32_t>, std::vector<uint32_t>> edge_to_triangles;
    
public:
    ConnectivityBuilder(std::vector<TriangleSplattingVertex>& v, std::vector<ConnectedTriangle>& t)
        : vertices(&v), triangles(&t) {}
    
    /**
     * @brief Main connectivity building function
     */
    bool buildConnectivity() {
        buildEdgeMapping();
        buildTriangleAdjacency(); 
        buildVertexAdjacency();
        return validateMeshTopology();
    }
    
private:
    void buildEdgeMapping() {
        edge_to_triangles.clear();
        
        for (uint32_t triId = 0; triId < triangles->size(); ++triId) {
            ConnectedTriangle& triangle = (*triangles)[triId];
            
            // Add all three edges
            for (int edgeIdx = 0; edgeIdx < 3; ++edgeIdx) {
                auto edge = triangle.getEdge(edgeIdx);
                edge_to_triangles[edge].push_back(triId);
            }
        }
    }
    
    void buildTriangleAdjacency() {
        for (uint32_t triId = 0; triId < triangles->size(); ++triId) {
            ConnectedTriangle& triangle = (*triangles)[triId];
            
            for (int edgeIdx = 0; edgeIdx < 3; ++edgeIdx) {
                auto edge = triangle.getEdge(edgeIdx);
                const auto& adjacentTris = edge_to_triangles[edge];
                
                // Find neighbor (should be exactly 2 triangles for manifold mesh)
                for (uint32_t adjTriId : adjacentTris) {
                    if (adjTriId != triId) {
                        triangle.neighbors[edgeIdx] = adjTriId;
                        break;
                    }
                }
            }
            
            // Compute geometric properties
            triangle.computeGeometry(*vertices);
        }
    }
    
    void buildVertexAdjacency() {
        for (uint32_t triId = 0; triId < triangles->size(); ++triId) {
            const ConnectedTriangle& triangle = (*triangles)[triId];
            
            // Add triangle to each vertex's adjacent triangles list
            for (int i = 0; i < 3; ++i) {
                uint32_t vertexId = triangle.indices[i];
                (*vertices)[vertexId].addAdjacentTriangle(triId);
                
                // Add vertex-to-vertex adjacencies
                uint32_t nextVertexId = triangle.indices[(i + 1) % 3];
                uint32_t prevVertexId = triangle.indices[(i + 2) % 3];
                
                (*vertices)[vertexId].addAdjacentVertex(nextVertexId);
                (*vertices)[vertexId].addAdjacentVertex(prevVertexId);
            }
        }
    }
    
    bool validateMeshTopology() const {
        // Check for manifold properties
        for (const auto& edge_pair : edge_to_triangles) {
            const auto& triangleList = edge_pair.second;
            
            if (triangleList.size() > 2) {
                // Non-manifold edge
                return false;
            }
        }
        
        // Additional topology checks can be added here
        return true;
    }
};

/**
 * @brief Statistics for conversion process
 */
struct ConversionStats {
    size_t original_triangle_count = 0;
    size_t original_vertex_count = 0;
    size_t final_vertex_count = 0;
    size_t final_triangle_count = 0;
    size_t vertices_merged = 0;
    double conversion_time_ms = 0.0;
    size_t memory_before_bytes = 0;
    size_t memory_after_bytes = 0;
    
    double getVertexReductionRatio() const {
        return original_vertex_count > 0 ? 
            static_cast<double>(vertices_merged) / original_vertex_count : 0.0;
    }
    
    double getMemoryRatio() const {
        return memory_before_bytes > 0 ?
            static_cast<double>(memory_after_bytes) / memory_before_bytes : 0.0;
    }
};

/**
 * @brief Main converter class for Triangle Soup to Vertex-Based Mesh
 */
class TriangleSoupConverter {
private:
    VertexDeduplicator deduplicator;
    
    // Result data
    std::vector<TriangleSplattingVertex> vertices;
    std::vector<ConnectedTriangle> triangles;
    
    ConversionStats stats;
    
public:
    /**
     * @brief Convert triangle soup to connected mesh
     * @param soup_positions Triangle soup vertex positions (3 per triangle)
     * @param soup_colors Triangle soup vertex colors (3 per triangle)
     * @param soup_opacities Triangle soup vertex opacities (3 per triangle)
     * @return Success status
     */
    bool convertTriangleSoup(const std::vector<Math::Vector3>& soup_positions,
                           const std::vector<Math::Vector3>& soup_colors,
                           const std::vector<float>& soup_opacities) {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Validate input
        if (soup_positions.size() % 3 != 0 || 
            soup_positions.size() != soup_colors.size() ||
            soup_positions.size() != soup_opacities.size()) {
            return false;
        }
        
        size_t triangle_count = soup_positions.size() / 3;
        
        // Initialize stats
        stats.original_triangle_count = triangle_count;
        stats.original_vertex_count = soup_positions.size();
        stats.memory_before_bytes = soup_positions.size() * (sizeof(Math::Vector3) * 2 + sizeof(float));
        
        // Clear previous results
        vertices.clear();
        triangles.clear();
        triangles.reserve(triangle_count);
        
        // Step 1: Deduplicate vertices and build triangles
        for (size_t i = 0; i < triangle_count; ++i) {
            size_t base = i * 3;
            
            uint32_t indices[3];
            for (int j = 0; j < 3; ++j) {
                TriangleSplattingVertex vertex(
                    soup_positions[base + j],
                    soup_colors[base + j], 
                    soup_opacities[base + j]
                );
                indices[j] = deduplicator.addOrGetVertex(vertex);
            }
            
            // Skip degenerate triangles
            if (indices[0] != indices[1] && indices[1] != indices[2] && indices[0] != indices[2]) {
                triangles.emplace_back(indices[0], indices[1], indices[2]);
            }
        }
        
        // Get deduplicated vertices
        vertices = deduplicator.getVertices();
        
        // Step 2: Build connectivity
        ConnectivityBuilder connectivity_builder(vertices, triangles);
        bool success = connectivity_builder.buildConnectivity();
        
        // Update stats
        stats.final_vertex_count = vertices.size();
        stats.final_triangle_count = triangles.size();
        stats.vertices_merged = stats.original_vertex_count - stats.final_vertex_count;
        stats.memory_after_bytes = calculateMemoryUsage();
        
        auto end_time = std::chrono::high_resolution_clock::now();
        stats.conversion_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        
        return success;
    }
    
    // Getters
    const std::vector<TriangleSplattingVertex>& getVertices() const { return vertices; }
    const std::vector<ConnectedTriangle>& getTriangles() const { return triangles; }
    const ConversionStats& getStats() const { return stats; }
    
private:
    size_t calculateMemoryUsage() const {
        size_t vertex_memory = vertices.size() * sizeof(TriangleSplattingVertex);
        size_t triangle_memory = triangles.size() * sizeof(ConnectedTriangle);
        
        // Add dynamic vector memory (approximation)
        for (const auto& vertex : vertices) {
            vertex_memory += vertex.adjacent_triangles.capacity() * sizeof(uint32_t);
            vertex_memory += vertex.adjacent_vertices.capacity() * sizeof(uint32_t);
        }
        
        return vertex_memory + triangle_memory;
    }
};

} // namespace Engine4D::Rendering