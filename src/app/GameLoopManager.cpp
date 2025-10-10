/**
 * @file GameLoopManager.cpp
 * @brief Implementation of GameLoopManager (P0.3 - SRP compliant)
 */

#include "SpectraForge/App/Core/GameLoopManager.h"
#include <algorithm>
#include <thread>

namespace SpectraForge {
namespace App {
namespace Core {

GameLoopManager::GameLoopManager() 
    : startTime_(Clock::now()),
      lastFrameTime_(Clock::now()),
      currentFrameTime_(Clock::now()),
      deltaTime_(0.0f),
      fps_(0.0f),
      frameTime_(0.0f),
      targetFPS_(0.0f),
      fpsUpdateTime_(0.0f),
      frameCount_(0) {
}

void GameLoopManager::beginFrame() {
    currentFrameTime_ = Clock::now();
    
    // Calculate delta time
    std::chrono::duration<float> elapsed = currentFrameTime_ - lastFrameTime_;
    deltaTime_ = elapsed.count();
    
    // Clamp delta time to prevent spiral of death
    deltaTime_ = std::min(deltaTime_, 0.1f); // Max 100ms per frame
}

void GameLoopManager::endFrame() {
    // Calculate frame time in milliseconds
    auto frameEnd = Clock::now();
    std::chrono::duration<float, std::milli> frameTimeMs = frameEnd - currentFrameTime_;
    frameTime_ = frameTimeMs.count();
    
    // Update FPS calculation
    frameCount_++;
    fpsUpdateTime_ += deltaTime_;
    
    // Update FPS every 0.5 seconds
    if (fpsUpdateTime_ >= 0.5f) {
        fps_ = static_cast<float>(frameCount_) / fpsUpdateTime_;
        frameCount_ = 0;
        fpsUpdateTime_ = 0.0f;
    }
    
    // Frame rate limiting (if targetFPS > 0)
    if (targetFPS_ > 0.0f) {
        float targetFrameTime = 1.0f / targetFPS_;
        float currentFrameTime = deltaTime_;
        
        if (currentFrameTime < targetFrameTime) {
            float sleepTime = targetFrameTime - currentFrameTime;
            auto sleepDuration = std::chrono::duration<float>(sleepTime);
            std::this_thread::sleep_for(sleepDuration);
        }
    }
    
    // Update last frame time for next iteration
    lastFrameTime_ = currentFrameTime_;
}

float GameLoopManager::getTotalTime() const {
    auto now = Clock::now();
    std::chrono::duration<float> elapsed = now - startTime_;
    return elapsed.count();
}

void GameLoopManager::reset() {
    startTime_ = Clock::now();
    lastFrameTime_ = Clock::now();
    currentFrameTime_ = Clock::now();
    deltaTime_ = 0.0f;
    fps_ = 0.0f;
    frameTime_ = 0.0f;
    fpsUpdateTime_ = 0.0f;
    frameCount_ = 0;
}

}  // namespace Core
}  // namespace App
}  // namespace SpectraForge

