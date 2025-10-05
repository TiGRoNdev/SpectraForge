#include "SpectraForge/Core/Component.h"
#include "SpectraForge/Core/GameObject3D.h"

using namespace SpectraForge::Core;

Component3D::Component3D() : gameObject(nullptr), enabled(true) {}

void Component3D::setEnabled(bool isEnabled) {
    this->enabled = isEnabled;
}

void Component3D::setGameObject(GameObject3D* gameObj) {
    this->gameObject = gameObj;
}
