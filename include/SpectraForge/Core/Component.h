
#pragma once

#include <string>
#include "Interfaces.h"

namespace SpectraForge {
namespace Core {

// Forward declaration
class GameObject3D;

/**
 * @brief Базовый класс компонента, следующий принципу единственной ответственности
 */
class Component3D : public ILifecycle {
  public:
    Component3D();
    ~Component3D() override = default;

    // Реализация ILifecycle
    void start() override {}
    void cleanup() override {}

    // Управление компонентами
    void setEnabled(bool isEnabled);
    bool isEnabled() const { return enabled; }

    // Ассоциация с GameObject
    void setGameObject(GameObject3D* gameObj);
    GameObject3D* getGameObject() const { return gameObject; }

    // Идентификация компонента
    virtual std::string getComponentType() const = 0;

  protected:
    GameObject3D* gameObject;
    bool enabled;

    // Дружественный класс для доступа к protected членам
    friend class GameObject3D;
};

/**
 * @brief Компонент для объектов, которые нуждаются в обновлениях
 */
class UpdatableComponent : public Component3D, public IUpdatable {
  public:
    ~UpdatableComponent() override = default;
    std::string getComponentType() const override { return "UpdatableComponent"; }
};

/**
 * @brief Компонент для объектов, которые можно рендерить
 */
class RenderableComponent : public Component3D, public IRenderable {
  public:
    ~RenderableComponent() override = default;
    std::string getComponentType() const override { return "RenderableComponent"; }
};

/**
 * @brief Компонент для объектов, которые нуждаются в обновлении и рендеринге
 */
class UpdatableRenderableComponent : public Component3D, public IUpdatable, public IRenderable {
  public:
    ~UpdatableRenderableComponent() override = default;
    std::string getComponentType() const override { return "UpdatableRenderableComponent"; }
};

}  // namespace Core
}  // namespace SpectraForge
