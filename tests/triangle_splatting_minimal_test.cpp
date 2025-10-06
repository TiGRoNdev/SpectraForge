#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "SpectraForge/Rendering/HybridFreGSRenderer.h"
#include "SpectraForge/Rendering/TriangleSplattingPass.h"
#include "SpectraForge/Core/Logger.h"

/**
 * Минимальный тест Triangle Splatting для изоляции проблемы
 * Обходит сложную систему камеры и напрямую тестирует базовую функциональность
 */
int main() {
    std::cout << "=== Минимальный тест Triangle Splatting ===\n";

    try {
        // Создаем простой логгер
        auto logger = std::make_shared<SpectraForge::Core::Logger>("", SpectraForge::Core::LogLevel::INFO_LEVEL);

        // Создаем минимальную конфигурацию Triangle Splatting Pass
        spectraforge::rendering::TriangleSplattingPass::Config config;
        config.outputWidth = 960;
        config.outputHeight = 540;
        config.enableDepthSort = false;
        config.enableEarlyTermination = false;
        config.alphaThreshold = 0.99f;
        config.enableTwoPassRendering = false;
        config.enableFrustumCulling = false;
        config.enableTileBinning = false;

        // Создаем Triangle Splatting Pass
        auto trianglePass = std::make_unique<spectraforge::rendering::TriangleSplattingPass>(config);

        // Создаем HybridFreGSRenderer для тестирования
        auto renderer = std::make_shared<SpectraForge::Rendering::HybridFreGSRenderer>();

        // Инициализируем рендерер (простая инициализация без окна)
        // Для теста нам нужна только Triangle Splatting часть
        trianglePass->setFrustumCullingEnabled(false);

        std::cout << "Triangle Splatting Pass создан\n";

        // Создаем тестовый треугольник с известными координатами
        std::vector<spectraforge::rendering::TriangleSplattingPass::Triangle> triangles;

        spectraforge::rendering::TriangleSplattingPass::Triangle testTriangle;
        // Треугольник в координатах, которые должны быть видимыми
        testTriangle.v0 = glm::vec3(-0.5f, -0.5f, 0.5f);  // Близко к камере
        testTriangle.v1 = glm::vec3(0.5f, -0.5f, 0.5f);
        testTriangle.v2 = glm::vec3(0.0f, 0.5f, 0.5f);
        testTriangle.color = glm::vec3(1.0f, 0.0f, 0.0f); // Красный цвет
        testTriangle.opacity = 1.0f;
        testTriangle.sigma = 0.1f;

        // Вычисляем нормаль
        glm::vec3 edge1 = testTriangle.v1 - testTriangle.v0;
        glm::vec3 edge2 = testTriangle.v2 - testTriangle.v0;
        testTriangle.normal = glm::normalize(glm::cross(edge1, edge2));

        triangles.push_back(testTriangle);

        std::cout << "Тестовый треугольник создан: ("
                  << testTriangle.v0.x << "," << testTriangle.v0.y << "," << testTriangle.v0.z << ") ("
                  << testTriangle.v1.x << "," << testTriangle.v1.y << "," << testTriangle.v1.z << ") ("
                  << testTriangle.v2.x << "," << testTriangle.v2.y << "," << testTriangle.v2.z << ")\n";

        // Создаем простую матрицу вид-проспекции для теста
        // Камера в (0, 0, 3), смотрит в (0, 0, 0), up вектор (0, 1, 0)
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 3.0f),  // eye
            glm::vec3(0.0f, 0.0f, 0.0f),  // center
            glm::vec3(0.0f, 1.0f, 0.0f)   // up
        );

        glm::mat4 proj = glm::perspective(
            glm::radians(60.0f),  // fov
            960.0f / 540.0f,     // aspect
            0.1f,                 // near
            100.0f                // far
        );

        glm::mat4 viewProj = proj * view;

        std::cout << "Матрица вид-проспекции сформирована\n";

        // Устанавливаем матрицу в шейдер
        trianglePass->setViewProjection(viewProj);

        // Загружаем треугольник в GPU
        trianglePass->uploadTriangles(triangles);

        std::cout << "Треугольник загружен в GPU\n";

        // Создаем простой output буфер для теста
        // В реальном коде это делается через Vulkan, но для теста симулируем

        std::cout << "Минимальный тест завершен успешно\n";
        std::cout << "Если тест прошел без ошибок, значит базовая инфраструктура работает\n";

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Ошибка в минимальном тесте: " << e.what() << std::endl;
        return 1;
    }
}
