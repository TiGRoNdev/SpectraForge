/**
 * @file SceneCoordinator.cpp
 * @brief Implementation of SceneCoordinator (P0.3 - SRP compliant)
 */

#include "SpectraForge/App/Core/SceneCoordinator.h"
#include "SpectraForge/App/Engine.h"
#include "SpectraForge/Core/Logger.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace SpectraForge {
namespace App {
namespace Core {

SceneCoordinator::SceneCoordinator()
    : camera_(nullptr),
      sceneLoaded_(false),
      triangleCount_(0),
      materialCount_(0),
      boundsMin_(0.0f, 0.0f, 0.0f),
      boundsMax_(0.0f, 0.0f, 0.0f),
      scenePath_(""),
      cameraSpeed_(5.0f),
      mouseSensitivity_(0.1f),
      yaw_(-90.0f),
      pitch_(0.0f) {
}

bool SceneCoordinator::loadScene(const Vulkan::SceneData& data) {
    // SceneData contains only paths, actual triangle count will be loaded later
    triangleCount_ = 0;
    materialCount_ = 0;
    
    // Store scene path
    scenePath_ = data.scenePath;
    
    // Create camera if not exists
    if (!camera_) {
        camera_ = std::make_shared<Rendering::Camera3D>();
        resetCameraForSponza(); // Set default position
    }
    
    sceneLoaded_ = true;
    return true;
}

void SceneCoordinator::updateCamera(float deltaTime, const InputState* input, bool externalControl) {
    if (!camera_ || externalControl || !input) {
        return; // Skip if camera is externally controlled or no input
    }
    
    // Get current camera transform
    auto currentPos = camera_->getPosition();
    glm::vec3 position(currentPos.x, currentPos.y, currentPos.z);
    
    // Calculate camera direction from yaw and pitch
    glm::vec3 front;
    front.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    front.y = sin(glm::radians(pitch_));
    front.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    front = glm::normalize(front);
    
    glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 up = glm::normalize(glm::cross(right, front));
    
    // WASD movement
    float velocity = cameraSpeed_ * deltaTime;
    
    if (input->keys[GLFW_KEY_W]) {
        position += front * velocity;
    }
    if (input->keys[GLFW_KEY_S]) {
        position -= front * velocity;
    }
    if (input->keys[GLFW_KEY_A]) {
        position -= right * velocity;
    }
    if (input->keys[GLFW_KEY_D]) {
        position += right * velocity;
    }
    if (input->keys[GLFW_KEY_SPACE]) {
        position += up * velocity;
    }
    if (input->keys[GLFW_KEY_LEFT_SHIFT]) {
        position -= up * velocity;
    }
    
    // Mouse look
    yaw_ += input->deltaMouseX * mouseSensitivity_;
    pitch_ -= input->deltaMouseY * mouseSensitivity_; // Inverted Y
    
    // Constrain pitch
    if (pitch_ > 89.0f) pitch_ = 89.0f;
    if (pitch_ < -89.0f) pitch_ = -89.0f;
    
    // Update camera
    camera_->setPosition(Math::Vector3(position.x, position.y, position.z));
    
    // Calculate target (look-at point)
    glm::vec3 target = position + front;
    camera_->lookAt(
        Math::Vector3(position.x, position.y, position.z),
        Math::Vector3(target.x, target.y, target.z),
        Math::Vector3(0.0f, 1.0f, 0.0f)
    );
}

void SceneCoordinator::resetCameraForSponza() {
    if (!camera_) {
        camera_ = std::make_shared<Rendering::Camera3D>();
    }
    
    // Default Sponza camera position (outside looking in)
    camera_->setPosition(Math::Vector3(0.0f, 2.0f, -5.0f));
    camera_->lookAt(
        Math::Vector3(0.0f, 2.0f, -5.0f),
        Math::Vector3(0.0f, 2.0f, 0.0f), // Look forward
        Math::Vector3(0.0f, 1.0f, 0.0f)
    );
    
    // Reset angles
    yaw_ = 0.0f;   // Looking forward (+Z)
    pitch_ = -5.0f; // Slightly down
}

SceneInfo SceneCoordinator::getSceneInfo() const {
    SceneInfo info;
    info.triangleCount = triangleCount_;
    info.materialCount = materialCount_;
    info.bounds.min = boundsMin_;
    info.bounds.max = boundsMax_;
    info.isLoaded = sceneLoaded_;
    info.scenePath = scenePath_;
    return info;
}

void SceneCoordinator::getSceneBounds(Math::Vector3& min, Math::Vector3& max) const {
    min = boundsMin_;
    max = boundsMax_;
}

}  // namespace Core
}  // namespace App
}  // namespace SpectraForge

