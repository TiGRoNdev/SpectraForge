#include <iostream>
#include <glm/glm.hpp>

struct alignas(16) Triangle {
    glm::vec3 v0, v1, v2;                      // 36 bytes
    glm::vec2 texCoord0, texCoord1, texCoord2; // 24 bytes  
    glm::vec3 color;                           // 12 bytes
    float opacity;                             // 4 bytes
    float sigma;                               // 4 bytes
    float padding1[3];                         // 12 bytes
    glm::vec3 normal;                          // 12 bytes
    int materialId;                            // 4 bytes
};

int main() {
    std::cout << "sizeof(Triangle) = " << sizeof(Triangle) << " bytes\n";
    std::cout << "alignof(Triangle) = " << alignof(Triangle) << " bytes\n";
    std::cout << "\nOffsets:\n";
    std::cout << "  v0: " << offsetof(Triangle, v0) << "\n";
    std::cout << "  v1: " << offsetof(Triangle, v1) << "\n";
    std::cout << "  v2: " << offsetof(Triangle, v2) << "\n";
    std::cout << "  texCoord0: " << offsetof(Triangle, texCoord0) << "\n";
    std::cout << "  color: " << offsetof(Triangle, color) << "\n";
    std::cout << "  opacity: " << offsetof(Triangle, opacity) << "\n";
    std::cout << "  sigma: " << offsetof(Triangle, sigma) << "\n";
    std::cout << "  padding1: " << offsetof(Triangle, padding1) << "\n";
    std::cout << "  normal: " << offsetof(Triangle, normal) << "\n";
    std::cout << "  materialId: " << offsetof(Triangle, materialId) << "\n";
    
    Triangle t;
    t.color = glm::vec3(0.0f, 0.3f, 1.0f);  // Синий
    std::cout << "\nTest triangle color: (" << t.color.r << ", " << t.color.g << ", " << t.color.b << ")\n";
    
    return 0;
}
