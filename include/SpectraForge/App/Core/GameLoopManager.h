/**
 * @file GameLoopManager.h
 * @brief Game loop timing and frame management (P0.3 - SRP compliant)
 * 
 * SOLID COMPLIANCE:
 * - SRP ✅: Only timing and frame rate management
 * - DIP ✅: Independent of rendering
 * - ISP ✅: Minimal interface
 */

#pragma once

#include <chrono>
#include "SpectraForge/App/Core/Interfaces/IGameLoopManager.h"

namespace SpectraForge {
namespace App {
namespace Core {

/**
 * @brief Manages game loop timing and frame pacing
 * 
 * Single Responsibility: Time tracking, delta time calculation, FPS limiting
 */
class GameLoopManager : public IGameLoopManager {
public:
    GameLoopManager();
    ~GameLoopManager() = default;

    /**
     * @brief Begin new frame (call at frame start)
     */
    void beginFrame() override;
    
    /**
     * @brief End current frame (call at frame end)
     */
    void endFrame() override;
    
    /**
     * @brief Get delta time for current frame (seconds)
     */
    float getDeltaTime() const override { return deltaTime_; }
    
    /**
     * @brief Get current FPS
     */
    float getFPS() const override { return fps_; }
    
    /**
     * @brief Get frame time in milliseconds
     */
    float getFrameTime() const override { return frameTime_; }
    
    /**
     * @brief Set target FPS (0 = unlimited)
     */
    void setTargetFPS(float targetFPS) override { targetFPS_ = targetFPS; }
    
    /**
     * @brief Get target FPS
     */
    float getTargetFPS() const override { return targetFPS_; }
    
    /**
     * @brief Get total elapsed time since start (seconds)
     */
    float getTotalTime() const override;
    
    /**
     * @brief Reset timing (useful for scene changes)
     */
    void reset() override;
    
private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    
    TimePoint startTime_;
    TimePoint lastFrameTime_;
    TimePoint currentFrameTime_;
    
    float deltaTime_ = 0.0f;
    float fps_ = 0.0f;
    float frameTime_ = 0.0f;
    float targetFPS_ = 0.0f;  // 0 = unlimited
    
    // FPS calculation
    float fpsUpdateTime_ = 0.0f;
    int frameCount_ = 0;
};

}  // namespace Core
}  // namespace App
}  // namespace SpectraForge

