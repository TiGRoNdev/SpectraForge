/**
 * @file MeshConnectivityConverter.cpp
 * @brief Implementation of Triangle Soup to Connected Mesh converter
 * @author SpectraForge Team
 */

#include "MeshConnectivityConverter.hpp"
#include <chrono>
#include <iostream>
#include <iomanip>

namespace Engine4D::Rendering {

// Example usage and integration code
class MeshConverter {
public:
    /**
     * @brief Example integration with SpectraForge rendering pipeline
     */
    static bool convertExistingMesh(/* your existing mesh data parameters */) {
        // Example data (replace with your actual triangle soup data)
        std::vector<Math::Vector3> soup_positions = {
            // Triangle 1
            {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.5f, 1.0f, 0.0f},
            // Triangle 2 (shares edge with triangle 1)
            {1.0f, 0.0f, 0.0f}, {2.0f, 0.0f, 0.0f}, {1.5f, 1.0f, 0.0f},
            // Triangle 3 (shares vertex with triangle 1)
            {0.0f, 0.0f, 0.0f}, {0.5f, 1.0f, 0.0f}, {-0.5f, 1.0f, 0.0f}
        };
        
        std::vector<Math::Vector3> soup_colors = {
            // Triangle 1 - Red gradient
            {1.0f, 0.0f, 0.0f}, {1.0f, 0.5f, 0.0f}, {1.0f, 1.0f, 0.0f},
            // Triangle 2 - Green gradient  
            {0.0f, 1.0f, 0.0f}, {0.5f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.5f},
            // Triangle 3 - Blue gradient
            {0.0f, 0.0f, 1.0f}, {0.5f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f}
        };
        
        std::vector<float> soup_opacities = {
            0.8f, 0.9f, 1.0f,  // Triangle 1
            0.7f, 0.8f, 0.9f,  // Triangle 2  
            0.6f, 0.7f, 0.8f   // Triangle 3
        };
        
        // Create converter
        TriangleSoupConverter converter;
        
        // Perform conversion
        bool success = converter.convertTriangleSoup(soup_positions, soup_colors, soup_opacities);
        
        if (success) {
            // Get results
            const auto& vertices = converter.getVertices();
            const auto& triangles = converter.getTriangles();
            const auto& stats = converter.getStats();
            
            // Print conversion statistics
            printConversionStats(stats);
            
            // Example: Test barycentric interpolation
            if (!triangles.empty()) {
                testBarycentricInterpolation(triangles[0], vertices);
            }
            
            return true;
        }
        
        return false;
    }
    
private:
    static void printConversionStats(const ConversionStats& stats) {
        std::cout << "=== Mesh Conversion Statistics ===\n";
        std::cout << "Original triangles: " << stats.original_triangle_count << "\n";
        std::cout << "Original vertices: " << stats.original_vertex_count << "\n"; 
        std::cout << "Final vertices: " << stats.final_vertex_count << "\n";
        std::cout << "Final triangles: " << stats.final_triangle_count << "\n";
        std::cout << "Vertices merged: " << stats.vertices_merged << "\n";
        std::cout << "Vertex reduction: " << std::fixed << std::setprecision(2) 
                  << (stats.getVertexReductionRatio() * 100) << "%\n";
        std::cout << "Conversion time: " << stats.conversion_time_ms << " ms\n";
        std::cout << "Memory before: " << stats.memory_before_bytes << " bytes\n";
        std::cout << "Memory after: " << stats.memory_after_bytes << " bytes\n";
        std::cout << "Memory ratio: " << std::fixed << std::setprecision(2)
                  << stats.getMemoryRatio() << "x\n";
        std::cout << "================================\n";
    }
    
    static void testBarycentricInterpolation(const ConnectedTriangle& triangle,
                                           const std::vector<TriangleSplattingVertex>& vertices) {
        std::cout << "=== Testing Barycentric Interpolation ===\n";
        
        // Test point at triangle center
        const Math::Vector3& v0 = vertices[triangle.indices[0]].position;
        const Math::Vector3& v1 = vertices[triangle.indices[1]].position;
        const Math::Vector3& v2 = vertices[triangle.indices[2]].position;
        
        Math::Vector3 center = (v0 + v1 + v2) / 3.0f;
        
        // Compute barycentric coordinates
        Math::Vector3 barycentric = triangle.computeBarycentric(center, vertices);
        
        std::cout << "Triangle center: (" << center.x << ", " << center.y << ", " << center.z << ")\n";
        std::cout << "Barycentric coords: (" << barycentric.x << ", " << barycentric.y << ", " << barycentric.z << ")\n";
        std::cout << "Sum: " << (barycentric.x + barycentric.y + barycentric.z) << " (should be 1.0)\n";
        
        // Test color interpolation
        Math::Vector3 interpolated_color = triangle.interpolateColor(barycentric, vertices);
        std::cout << "Interpolated color: (" << interpolated_color.x << ", " << interpolated_color.y << ", " << interpolated_color.z << ")\n";
        
        std::cout << "=========================================\n";
    }
};

/**
 * @brief Rendering integration example
 */
class ConnectedMeshRenderer {
private:
    std::vector<TriangleSplattingVertex> vertices;
    std::vector<ConnectedTriangle> triangles;
    
    // GPU buffers (OpenGL example)
    uint32_t VAO, VBO, EBO;
    uint32_t shader_program;
    
public:
    /**
     * @brief Initialize renderer with connected mesh data
     */
    bool initialize(const std::vector<TriangleSplattingVertex>& mesh_vertices,
                   const std::vector<ConnectedTriangle>& mesh_triangles) {
        vertices = mesh_vertices;
        triangles = mesh_triangles;
        
        return setupGPUBuffers() && setupShaders();
    }
    
    /**
     * @brief Render the connected mesh with barycentric interpolation
     */
    void render(const Math::Matrix4& mvp_matrix) {
        // Use shader program
        glUseProgram(shader_program);
        
        // Set MVP matrix
        int mvp_location = glGetUniformLocation(shader_program, "mvpMatrix");
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, mvp_matrix.data());
        
        // Bind VAO and draw
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(triangles.size() * 3), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    
    /**
     * @brief Update vertex colors dynamically using barycentric interpolation
     */
    void updateVertexColors(const std::vector<Math::Vector3>& new_colors) {
        if (new_colors.size() != vertices.size()) return;
        
        // Update vertex colors
        for (size_t i = 0; i < vertices.size(); ++i) {
            vertices[i].color = new_colors[i];
        }
        
        // Update GPU buffer
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 
                       offsetof(TriangleSplattingVertex, color), 
                       vertices.size() * sizeof(Math::Vector3),
                       &vertices[0].color);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    /**
     * @brief Get color at any point using barycentric interpolation
     */
    Math::Vector3 getColorAtPoint(const Math::Vector3& point) const {
        // Find containing triangle (simplified - use spatial acceleration in practice)
        for (const auto& triangle : triangles) {
            Math::Vector3 barycentric = triangle.computeBarycentric(point, vertices);
            
            // Check if point is inside triangle
            if (barycentric.x >= 0.0f && barycentric.y >= 0.0f && barycentric.z >= 0.0f) {
                return triangle.interpolateColor(barycentric, vertices);
            }
        }
        
        return Math::Vector3(0, 0, 0); // Default color if not found
    }
    
    ~ConnectedMeshRenderer() {
        cleanup();
    }
    
private:
    bool setupGPUBuffers() {
        // Create buffers
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        
        glBindVertexArray(VAO);
        
        // Upload vertex data
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(TriangleSplattingVertex), 
                    vertices.data(), GL_DYNAMIC_DRAW);
        
        // Position attribute (location = 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleSplattingVertex),
                             (void*)offsetof(TriangleSplattingVertex, position));
        glEnableVertexAttribArray(0);
        
        // Color attribute (location = 1) 
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleSplattingVertex),
                             (void*)offsetof(TriangleSplattingVertex, color));
        glEnableVertexAttribArray(1);
        
        // Opacity attribute (location = 2)
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(TriangleSplattingVertex),
                             (void*)offsetof(TriangleSplattingVertex, opacity));
        glEnableVertexAttribArray(2);
        
        // Upload index data
        std::vector<uint32_t> indices;
        indices.reserve(triangles.size() * 3);
        for (const auto& triangle : triangles) {
            indices.push_back(triangle.indices[0]);
            indices.push_back(triangle.indices[1]);
            indices.push_back(triangle.indices[2]);
        }
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t),
                    indices.data(), GL_STATIC_DRAW);
        
        glBindVertexArray(0);
        
        return true;
    }
    
    bool setupShaders() {
        // Vertex shader source
        const char* vertex_shader_source = R"(
            #version 450 core
            
            layout(location = 0) in vec3 position;
            layout(location = 1) in vec3 color;
            layout(location = 2) in float opacity;
            
            uniform mat4 mvpMatrix;
            
            out vec3 vertexColor;
            out float vertexOpacity;
            
            void main() {
                gl_Position = mvpMatrix * vec4(position, 1.0);
                vertexColor = color;
                vertexOpacity = opacity;
            }
        )";
        
        // Fragment shader source
        const char* fragment_shader_source = R"(
            #version 450 core
            
            in vec3 vertexColor;
            in float vertexOpacity;
            
            out vec4 fragColor;
            
            void main() {
                // Barycentric interpolation происходит автоматически в hardware
                fragColor = vec4(vertexColor, vertexOpacity);
            }
        )";
        
        // Compile and link shaders
        uint32_t vertex_shader = compileShader(GL_VERTEX_SHADER, vertex_shader_source);
        uint32_t fragment_shader = compileShader(GL_FRAGMENT_SHADER, fragment_shader_source);
        
        if (vertex_shader == 0 || fragment_shader == 0) {
            return false;
        }
        
        shader_program = glCreateProgram();
        glAttachShader(shader_program, vertex_shader);
        glAttachShader(shader_program, fragment_shader);
        glLinkProgram(shader_program);
        
        // Check linking
        int success;
        glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
        if (!success) {
            char info_log[512];
            glGetProgramInfoLog(shader_program, 512, nullptr, info_log);
            std::cerr << "Shader program linking failed: " << info_log << std::endl;
            return false;
        }
        
        // Cleanup
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        
        return true;
    }
    
    uint32_t compileShader(uint32_t type, const char* source) {
        uint32_t shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);
        
        // Check compilation
        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char info_log[512];
            glGetShaderInfoLog(shader, 512, nullptr, info_log);
            std::cerr << "Shader compilation failed: " << info_log << std::endl;
            return 0;
        }
        
        return shader;
    }
    
    void cleanup() {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);
        if (shader_program) glDeleteProgram(shader_program);
    }
};

/**
 * @brief Advanced mesh processing operations enabled by connectivity
 */
class ConnectedMeshProcessor {
public:
    /**
     * @brief Smooth mesh using Laplacian smoothing
     */
    static void laplacianSmoothing(std::vector<TriangleSplattingVertex>& vertices,
                                  const std::vector<ConnectedTriangle>& triangles,
                                  float lambda = 0.5f, int iterations = 1) {
        for (int iter = 0; iter < iterations; ++iter) {
            std::vector<Math::Vector3> new_positions(vertices.size());
            
            for (size_t i = 0; i < vertices.size(); ++i) {
                const auto& vertex = vertices[i];
                Math::Vector3 laplacian(0, 0, 0);
                
                // Compute Laplacian as average of adjacent vertices
                if (!vertex.adjacent_vertices.empty()) {
                    for (uint32_t adj_id : vertex.adjacent_vertices) {
                        laplacian += vertices[adj_id].position;
                    }
                    laplacian /= static_cast<float>(vertex.adjacent_vertices.size());
                    laplacian -= vertex.position;
                }
                
                new_positions[i] = vertex.position + lambda * laplacian;
            }
            
            // Update positions
            for (size_t i = 0; i < vertices.size(); ++i) {
                vertices[i].position = new_positions[i];
            }
        }
    }
    
    /**
     * @brief Compute mesh statistics
     */
    static void computeMeshStatistics(const std::vector<TriangleSplattingVertex>& vertices,
                                    const std::vector<ConnectedTriangle>& triangles) {
        std::cout << "=== Mesh Statistics ===\n";
        std::cout << "Vertices: " << vertices.size() << "\n";
        std::cout << "Triangles: " << triangles.size() << "\n";
        
        // Compute average vertex valence
        double avg_valence = 0.0;
        for (const auto& vertex : vertices) {
            avg_valence += vertex.adjacent_vertices.size();
        }
        avg_valence /= vertices.size();
        std::cout << "Average vertex valence: " << avg_valence << "\n";
        
        // Compute total surface area
        double total_area = 0.0;
        for (const auto& triangle : triangles) {
            total_area += triangle.area;
        }
        std::cout << "Total surface area: " << total_area << "\n";
        
        std::cout << "=======================\n";
    }
};

} // namespace Engine4D::Rendering

/**
 * @brief Example usage in main function
 */
int main() {
    using namespace Engine4D::Rendering;
    
    std::cout << "SpectraForge - Triangle Soup to Connected Mesh Converter\n";
    std::cout << "========================================================\n";
    
    // Test the converter
    bool success = MeshConverter::convertExistingMesh();
    
    if (success) {
        std::cout << "Conversion successful!\n";
        return 0;
    } else {
        std::cout << "Conversion failed!\n";
        return 1;
    }
}