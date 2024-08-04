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

set(GAPI_LIBRARY gapi)
set(GAPI_HEADERS_DIR ${PROJECT_SOURCE_DIR}/include)
set(GAPI_SOURCES_DIR ${PROJECT_SOURCE_DIR}/src)

set(
    GAPI_HEADERS 
        ${GAPI_HEADERS_DIR}/${GAPI_LIBRARY}/gapi.hpp
        ${GAPI_HEADERS_DIR}/${GAPI_LIBRARY}/gapi_impl_opengl.hpp
)

set(
    GAPI_SOURCES 
        ${GAPI_SOURCES_DIR}/gapi.cpp
        ${GAPI_SOURCES_DIR}/gapi_impl_opengl.cpp
)

add_library(
    gapi 
        INTERFACE
            ${GAPI_HEADERS}
            ${GAPI_SOURCES}
)

target_include_directories(
    gapi
        INTERFACE 
            ${GAPI_HEADERS_DIR}
            ${GAPI_SOURCES_DIR}
)

add_library(gapi::header_only ALIAS gapi)