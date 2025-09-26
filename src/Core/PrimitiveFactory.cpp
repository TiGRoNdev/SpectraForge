#include "Engine4D/Core/PrimitiveFactory.h"
#include "Engine4D/Core/GameObject4D.h"
#include "Engine4D/Rendering/Renderer.h"
#include "Engine4D/Physics/Physics4D.h"

using namespace Engine4D::Math;

namespace Engine4D {
namespace Core {

PrimitiveFactory::PrimitiveFactory() {
    registerDefaultPrimitives();
}

std::shared_ptr<GameObject4D> PrimitiveFactory::createPrimitive(const std::string& type) {
    auto it = primitiveCreators.find(type);
    if (it != primitiveCreators.end()) {
        return it->second();
    }
    return nullptr;
}

void PrimitiveFactory::registerPrimitive(const std::string& type, PrimitiveCreator creator) {
    primitiveCreators[type] = creator;
}

void PrimitiveFactory::unregisterPrimitive(const std::string& type) {
    primitiveCreators.erase(type);
}

bool PrimitiveFactory::isPrimitiveRegistered(const std::string& type) const {
    return primitiveCreators.find(type) != primitiveCreators.end();
}

PrimitiveFactory& PrimitiveFactory::getInstance() {
    static PrimitiveFactory instance;
    return instance;
}

void PrimitiveFactory::registerDefaultPrimitives() {
    registerPrimitive("Tesseract", [this]() { return createTesseract(); });
    registerPrimitive("Sphere", [this]() { return createSphere(); });
    registerPrimitive("Cube", [this]() { return createCube(); });
    registerPrimitive("Hypercube", [this]() { return createHypercube(); });
}

std::shared_ptr<GameObject4D> PrimitiveFactory::createTesseract() {
    auto obj = std::make_shared<GameObject4D>("Tesseract");
    
    // Add mesh renderer
    auto* renderer = obj->addComponent<MeshRenderer4D>();
    auto mesh = std::make_shared<Rendering::Mesh4D>(Rendering::Mesh4D::createTesseract());
    mesh->uploadToGPU();
    renderer->setMesh(mesh);
    
    // Add collider
    auto* collider = obj->addComponent<Collider4DComponent>();
    auto boxCollider = std::make_shared<Physics::BoxCollider4D>(Vector4::one());
    collider->setCollider(boxCollider);
    
    return obj;
}

std::shared_ptr<GameObject4D> PrimitiveFactory::createSphere() {
    auto obj = std::make_shared<GameObject4D>("Sphere");
    
    // Add mesh renderer
    auto* renderer = obj->addComponent<MeshRenderer4D>();
    auto mesh = std::make_shared<Rendering::Mesh4D>(Rendering::Mesh4D::createSimplex());
    mesh->uploadToGPU();
    renderer->setMesh(mesh);
    
    // Add collider
    auto* collider = obj->addComponent<Collider4DComponent>();
    auto sphereCollider = std::make_shared<Physics::SphereCollider4D>(1.0f);
    collider->setCollider(sphereCollider);
    
    return obj;
}

std::shared_ptr<GameObject4D> PrimitiveFactory::createCube() {
    auto obj = std::make_shared<GameObject4D>("Cube");
    
    // Add mesh renderer
    auto* renderer = obj->addComponent<MeshRenderer4D>();
    auto mesh = std::make_shared<Rendering::Mesh4D>(Rendering::Mesh4D::createTesseract(1.0f));
    mesh->uploadToGPU();
    renderer->setMesh(mesh);
    
    // Add collider
    auto* collider = obj->addComponent<Collider4DComponent>();
    auto boxCollider = std::make_shared<Physics::BoxCollider4D>(Vector4::one());
    collider->setCollider(boxCollider);
    
    return obj;
}

std::shared_ptr<GameObject4D> PrimitiveFactory::createHypercube() {
    auto obj = std::make_shared<GameObject4D>("Hypercube");
    
    // Add mesh renderer
    auto* renderer = obj->addComponent<MeshRenderer4D>();
    auto mesh = std::make_shared<Rendering::Mesh4D>(Rendering::Mesh4D::createHypercube());
    mesh->uploadToGPU();
    renderer->setMesh(mesh);
    
    // Add collider
    auto* collider = obj->addComponent<Collider4DComponent>();
    auto boxCollider = std::make_shared<Physics::BoxCollider4D>(Vector4(2, 2, 2, 2));
    collider->setCollider(boxCollider);
    
    return obj;
}

} // namespace Core
} // namespace Engine4D
