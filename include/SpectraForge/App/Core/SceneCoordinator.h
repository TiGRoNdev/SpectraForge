/**
 * @file SceneCoordinator.h
 * @brief Scene and camera coordination (P0.3 - SRP compliant)
 * 
 * SOLID COMPLIANCE:
 * - SRP ✅: Only scene/camera coordination
 * - DIP ✅: Uses camera interface
 * - OCP ✅: Extensible for different scenes
 */

#pragma once

#include <memory>
#include <string>
#include "SpectraForge/Rendering/Camera3D.h"
#include "SpectraForge/Vulkan/SceneManager.h"
#include "SpectraForge/Math/Vector3.h"

namespace SpectraForge {
namespace App {

// Forward declarations
struct SceneInfo;

namespace Core {

// Forward declaration for InputState
struct InputState;

/**
 * @brief Coordinates scene loading and camera management
 * 
 * Single Responsibility: Scene/camera coordination and updates
 */
class SceneCoordinator {
public:
    SceneCoordinator();
    ~SceneCoordinator() = default;
    
    /**
     * @brief Load scene data
     */
    bool loadScene(const Vulkan::SceneData& data);
    
    /**
     * @brief Update camera based on input
     */
    void updateCamera(float deltaTime, const InputState* input, bool externalControl);
    
    /**
     * @brief Get camera instance
     */
    std::shared_ptr<Rendering::Camera3D> getCamera() const { return camera_; }
    
    /**
     * @brief Set camera instance
     */
    void setCamera(std::shared_ptr<Rendering::Camera3D> camera) { camera_ = camera; }
    
    /**
     * @brief Reset camera to default position for Sponza scene
     */
    void resetCameraForSponza();
    
    /**
     * @brief Get scene information
     */
    SceneInfo getSceneInfo() const;
    
    /**
     * @brief Check if scene is loaded
     */
    bool isSceneLoaded() const { return sceneLoaded_; }
    
    /**
     * @brief Get scene bounds
     */
    void getSceneBounds(Math::Vector3& min, Math::Vector3& max) const;
    
private:
    std::shared_ptr<Rendering::Camera3D> camera_;
    bool sceneLoaded_ = false;
    
    // Scene info
    uint32_t triangleCount_ = 0;
    uint32_t materialCount_ = 0;
    Math::Vector3 boundsMin_{0.0f, 0.0f, 0.0f};
    Math::Vector3 boundsMax_{0.0f, 0.0f, 0.0f};
    std::string scenePath_;
    
    // Camera movement
    float cameraSpeed_ = 5.0f;
    float mouseSensitivity_ = 0.1f;
    float yaw_ = -90.0f;
    float pitch_ = 0.0f;
};

}  // namespace Core
}  // namespace App
}  // namespace SpectraForge

