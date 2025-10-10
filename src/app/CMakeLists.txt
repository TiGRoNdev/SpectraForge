# ============================================================================
# App Layer - High-level application components (P0.3 decomposition)
# ============================================================================

set(APP_SOURCES
    # P0.3: Engine decomposition components
    GameLoopManager.cpp
    WindowManager.cpp
    InputManager.cpp
    SceneCoordinator.cpp
)

# Create object library for App module
add_library(SpectraForge_App OBJECT ${APP_SOURCES})

# Include directories
target_include_directories(SpectraForge_App 
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/include
)

# Link dependencies
target_link_libraries(SpectraForge_App PUBLIC glfw)

message(STATUS "App layer configured (P0.3 Engine decomposition)")
