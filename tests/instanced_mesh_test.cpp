/**
 * @file instanced_mesh_test.cpp
 * @brief Unit tests для упаковки данных инстансов (CPU-only)
 */

#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cstring>

// Структура данных инстанса (копия из InstancedMeshPass.h)
struct InstanceDataGPU {
    alignas(16) float model[16];
    alignas(16) float color[4];
};

/**
 * @brief Утилита для упаковки glm::mat4 в float[16] для GPU
 */
void pack_matrix4_to_array(const glm::mat4& mat, float* array) {
    std::memcpy(array, &mat[0][0], sizeof(float) * 16);
}

/**
 * @brief Утилита для упаковки glm::vec4 в float[4] для GPU
 */
void pack_vec4_to_array(const glm::vec4& vec, float* array) {
    std::memcpy(array, &vec[0], sizeof(float) * 4);
}

class InstancedMeshTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Подготовка тестовых данных
    }
};

TEST_F(InstancedMeshTest, PackMatrixData) {
    // Создаём матрицу трансформации
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 2.0f, 3.0f));
    transform = glm::rotate(transform, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    transform = glm::scale(transform, glm::vec3(2.0f));

    InstanceDataGPU instance{};
    pack_matrix4_to_array(transform, instance.model);
    pack_vec4_to_array(glm::vec4(1.0f, 0.5f, 0.2f, 1.0f), instance.color);

    // Проверяем, что данные упакованы корректно
    EXPECT_FLOAT_EQ(instance.model[12], transform[3][0]); // translation X
    EXPECT_FLOAT_EQ(instance.model[13], transform[3][1]); // translation Y
    EXPECT_FLOAT_EQ(instance.model[14], transform[3][2]); // translation Z
    EXPECT_FLOAT_EQ(instance.color[0], 1.0f); // R
    EXPECT_FLOAT_EQ(instance.color[1], 0.5f); // G
    EXPECT_FLOAT_EQ(instance.color[2], 0.2f); // B
    EXPECT_FLOAT_EQ(instance.color[3], 1.0f); // A
}

TEST_F(InstancedMeshTest, BatchInstanceData) {
    const size_t instanceCount = 1000;
    std::vector<InstanceDataGPU> instances;
    instances.reserve(instanceCount);

    // Генерируем данные для множества инстансов
    for (size_t i = 0; i < instanceCount; ++i) {
        InstanceDataGPU instance{};
        
        // Уникальная позиция для каждого инстанса
        float x = static_cast<float>(i % 10) * 2.0f;
        float z = static_cast<float>(i / 10) * 2.0f;
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, z));
        pack_matrix4_to_array(transform, instance.model);
        
        // Уникальный цвет
        float hue = static_cast<float>(i) / instanceCount;
        pack_vec4_to_array(glm::vec4(hue, 1.0f - hue, 0.5f, 1.0f), instance.color);
        
        instances.push_back(instance);
    }

    // Проверяем размер и выравнивание
    EXPECT_EQ(instances.size(), instanceCount);
    EXPECT_EQ(sizeof(InstanceDataGPU), 80); // 64 (mat4) + 16 (vec4) = 80 байт
    
    // Проверяем первый и последний инстанс
    EXPECT_FLOAT_EQ(instances[0].model[12], 0.0f); // первый X = 0
    EXPECT_FLOAT_EQ(instances[999].model[12], 18.0f); // последний X = (999%10)*2 = 18
    EXPECT_FLOAT_EQ(instances[999].model[14], 198.0f); // последний Z = (999/10)*2 = 198
}

TEST_F(InstancedMeshTest, MemoryAlignment) {
    // Проверяем выравнивание структуры
    InstanceDataGPU instance{};
    
    // Проверяем, что поля выровнены по 16 байт
    size_t modelOffset = offsetof(InstanceDataGPU, model);
    size_t colorOffset = offsetof(InstanceDataGPU, color);
    
    EXPECT_EQ(modelOffset % 16, 0); // model должна быть выровнена по 16 байт
    EXPECT_EQ(colorOffset % 16, 0); // color должна быть выровнена по 16 байт
    
    // Общий размер должен быть кратен 16 для корректной работы в массивах
    EXPECT_EQ(sizeof(InstanceDataGPU) % 16, 0);
}
