#include "Engine3D/Core/Component.h"
#include "Engine3D/Core/GameObject3D.h"

using namespace Engine3D::Core;

Component3D::Component3D() 
    : gameObject(nullptr)
    , enabled(true) {}

void Component3D::setEnabled(bool enabled) {
    this->enabled = enabled;
}

void Component3D::setGameObject(GameObject3D* gameObject) {
    this->gameObject = gameObject;
}
