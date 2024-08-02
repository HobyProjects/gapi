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

set(
    GAPI_HEADERS 
        ${PROJECT_SOURCE_DIR}/include/gapi.hpp
)

set(
    GAPI_SOURCES 
        ${PROJECT_SOURCE_DIR}/src/gapi.cpp
)

add_subdirectory(vendors)

if(NOT BUILD_SHARED_LIBS)
    add_compile_definitions(GAPI_STATIC)
    add_library(gapi STATIC ${GAPI_HEADERS} ${GAPI_SOURCES})
else()
    add_compile_definitions(GAPI_SHARED)
    add_library(gapi SHARED ${GAPI_HEADERS} ${GAPI_SOURCES})
endif()

target_include_directories(gapi PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(gapi PUBLIC glm::glm)