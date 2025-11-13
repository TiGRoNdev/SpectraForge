#include <iostream>
#include <glm/glm.hpp>
#include <cstddef>

struct alignas(16) Triangle {
    glm::vec4 v0, v1, v2;
    glm::vec4 texCoord0, texCoord1, texCoord2;
    glm::vec4 color;
    float opacity;
    float sigma;
    float padding1[2];
    glm::vec4 normal;
    int materialId;
    float padding2[3];
};

int main() {
    std::cout << "=== Triangle Structure Layout ===\n";
    std::cout << "sizeof(Triangle) = " << sizeof(Triangle) << " bytes\n";
    std::cout << "alignof(Triangle) = " << alignof(Triangle) << " bytes\n\n";
    
    std::cout << "Member offsets:\n";
    std::cout << "  v0:        " << offsetof(Triangle, v0) << "\n";
    std::cout << "  v1:        " << offsetof(Triangle, v1) << "\n";
    std::cout << "  v2:        " << offsetof(Triangle, v2) << "\n";
    std::cout << "  texCoord0: " << offsetof(Triangle, texCoord0) << "\n";
    std::cout << "  texCoord1: " << offsetof(Triangle, texCoord1) << "\n";
    std::cout << "  texCoord2: " << offsetof(Triangle, texCoord2) << "\n";
    std::cout << "  color:     " << offsetof(Triangle, color) << "\n";
    std::cout << "  opacity:   " << offsetof(Triangle, opacity) << "\n";
    std::cout << "  sigma:     " << offsetof(Triangle, sigma) << "\n";
    std::cout << "  padding1:  " << offsetof(Triangle, padding1) << "\n";
    std::cout << "  normal:    " << offsetof(Triangle, normal) << "\n";
    std::cout << "  materialId:" << offsetof(Triangle, materialId) << "\n";
    std::cout << "  padding2:  " << offsetof(Triangle, padding2) << "\n";
    
    // Test data
    Triangle t;
    t.color = glm::vec4(0.0f, 0.3f, 1.0f, 1.0f);
    
    std::cout << "\n=== Test Data ===\n";
    std::cout << "t.color = (" << t.color.r << ", " << t.color.g << ", " << t.color.b << ", " << t.color.a << ")\n";
    std::cout << "Expected RGB(0, 77, 255) for color (0, 0.3, 1.0)\n";
    
    // Raw bytes
    unsigned char* bytes = reinterpret_cast<unsigned char*>(&t);
    std::cout << "\nFirst 160 bytes (hex):\n";
    for (int i = 0; i < 160 && i < (int)sizeof(Triangle); i += 16) {
        std::cout << "  " << i << ": ";
        for (int j = 0; j < 16 && (i+j) < (int)sizeof(Triangle); j++) {
            printf("%02X ", bytes[i+j]);
        }
        std::cout << "\n";
    }
    
    return 0;
}
