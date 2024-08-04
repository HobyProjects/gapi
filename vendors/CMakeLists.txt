# glm lib Configuration
set(GLM_BUILD_TESTS OFF)
add_subdirectory(glm EXCLUDE_FROM_ALL)

# glew lib Configuration
set(${glew-cmake_BUILD_SHARED} OFF)
set(${glew-cmake_BUILD_STATIC} ON)
add_subdirectory(glew-cmake EXCLUDE_FROM_ALL)

# glfw lib Configuration
add_subdirectory(glfw EXCLUDE_FROM_ALL)

# Importing STB image lib configure
set(STB_IMAGE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendors/stb CACHE PATH "Path to stb image library")
set(STB_IMAGE_HEADER ${PROJECT_SOURCE_DIR}/vendors/stb/stb_image.h CACHE INTERNAL "Path to stb_image header")