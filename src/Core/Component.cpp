#include "Engine4D/Core/Component.h"
#include "Engine4D/Core/GameObject4D.h"

namespace Engine4D {
namespace Core {

Component4D::Component4D() : gameObject(nullptr), enabled(true) {}

void Component4D::setEnabled(bool enabled) {
    this->enabled = enabled;
}

void Component4D::setGameObject(GameObject4D* gameObject) {
    this->gameObject = gameObject;
}

GameObject4D* Component4D::getGameObject() const {
    return gameObject;
}

} // namespace Core
} // namespace Engine4D
