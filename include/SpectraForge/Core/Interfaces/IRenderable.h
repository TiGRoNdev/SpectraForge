
/**
 * @file IRenderable.h
 * @brief Интерфейс для рендерируемых объектов
 *
 * Следует принципу ISP - разделяет обязанности рендеринга
 */

#pragma once

#include "../../Math/Matrix4.h"

namespace SpectraForge {
namespace Core {
namespace Interfaces {

/**
 * @brief Интерфейс для объектов, которые могут быть отрендерены
 *
 * Применяет принцип ISP - клиенты зависят только от методов рендеринга
 */
class IRenderable {
  public:
    virtual ~IRenderable() = default;

    /**
     * @brief Рендеринг объекта
     * @param transform Матрица трансформации
     */
    virtual void render(const Math::Matrix4& transform) = 0;

    /**
     * @brief Получение количества треугольников для статистики
     * @return Количество треугольников
     */
    virtual size_t getTriangleCount() const = 0;

    /**
     * @brief Получение количества вершин для статистики
     * @return Количество вершин
     */
    virtual size_t getVertexCount() const = 0;
};

}  // namespace Interfaces
}  // namespace Core
}  // namespace SpectraForge
