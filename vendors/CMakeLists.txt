# glm lib Configuration
set(GLM_BUILD_TESTS OFF)
add_subdirectory(glm EXCLUDE_FROM_ALL)

# glew lib Configuration
set(${glew-cmake_BUILD_SHARED} OFF)
set(${glew-cmake_BUILD_STATIC} ON)
add_subdirectory(glew-cmake EXCLUDE_FROM_ALL)

# glfw lib Configuration
add_subdirectory(glfw EXCLUDE_FROM_ALL)