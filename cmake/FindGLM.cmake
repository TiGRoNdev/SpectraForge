# FindGLM.cmake - Find GLM (OpenGL Mathematics) library
# This module finds if GLM is installed and determines where the
# headers and libraries are. It sets the following variables:
#
# GLM_FOUND - True if GLM was found
# GLM_INCLUDE_DIRS - The path to the GLM headers
# GLM_LIBRARIES - The libraries to link against (empty for GLM)

# Try to find GLM in standard locations
find_path(GLM_INCLUDE_DIR
    NAMES glm/glm.hpp
    PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
        /usr/include/glm
        /usr/local/include/glm
        /opt/local/include/glm
    PATH_SUFFIXES
        glm
)

# If we found the header, we consider GLM found
if(GLM_INCLUDE_DIR)
    set(GLM_FOUND TRUE)
    set(GLM_INCLUDE_DIRS ${GLM_INCLUDE_DIR})
    set(GLM_LIBRARIES "")
    message(STATUS "Found GLM headers in: ${GLM_INCLUDE_DIR}")
else()
    set(GLM_FOUND FALSE)
    message(WARNING "GLM headers not found. Please install libglm-dev package.")
endif()

# Set the GLM_INCLUDE_DIRS variable for the user
if(GLM_FOUND)
    set(GLM_INCLUDE_DIRS ${GLM_INCLUDE_DIR})
else()
    set(GLM_INCLUDE_DIRS "")
endif()

# Report the result
if(GLM_FOUND)
    message(STATUS "GLM found: ${GLM_INCLUDE_DIR}")
else()
    message(WARNING "GLM not found. Please install the GLM library (libglm-dev).")
endif()