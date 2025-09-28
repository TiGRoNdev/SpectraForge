#include "HyperEngine/Core/Component.h"
#include "HyperEngine/Core/GameObject3D.h"

using namespace HyperEngine::Core;

Component3D::Component3D() 
    : gameObject(nullptr)
    , enabled(true) {}

void Component3D::setEnabled(bool enabled) {
    this->enabled = enabled;
}

void Component3D::setGameObject(GameObject3D* gameObject) {
    this->gameObject = gameObject;
}

