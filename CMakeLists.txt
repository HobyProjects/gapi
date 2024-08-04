cmake_minimum_required(VERSION 3.20)
project(gapi C CXX)

# C standards
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# C++ standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(GAPI_HEADERS_DIR ${PROJECT_SOURCE_DIR}/include/gapi)
set(GAPI_SOURCES_DIR ${PROJECT_SOURCE_DIR}/src)

set(
    GAPI_HEADERS 
        ${GAPI_HEADERS_DIR}/gapi.hpp
        ${GAPI_HEADERS_DIR}/gapi_impl_opengl.hpp
)

set(
    GAPI_SOURCES 
        ${GAPI_SOURCES_DIR}/gapi.cpp
        ${GAPI_SOURCES_DIR}/gapi_impl_opengl.cpp
)


add_subdirectory(vendors)

if(NOT BUILD_SHARED_LIBS)
    add_library(
        gapi 
            STATIC 
                ${GAPI_HEADERS} 
                ${GAPI_SOURCES}
    )
else()
    message(FATAL_ERROR "Shared library is not supported yet")
endif()

target_include_directories(
    gapi
        PUBLIC 
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/gapi>
            $<BUILD_INTERFACE:${STB_IMAGE_SOURCE_DIR}>

            $<INSTALL_INTERFACE:include/gapi>
)

target_include_directories(
    gapi
        PRIVATE 
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${STB_IMAGE_SOURCE_DIR}>

            $<INSTALL_INTERFACE:include>
)

target_link_libraries(
    gapi 
        PUBLIC 
            glm::glm
            libglew_static
            glfw
)