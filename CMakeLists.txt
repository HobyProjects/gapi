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

add_library(
    ${GAPI_LIBRARY} 
        INTERFACE
            ${GAPI_HEADERS_DIR}/${GAPI_LIBRARY}/gapi.hpp
)

target_include_directories(
    ${GAPI_LIBRARY}
        INTERFACE 
            ${GAPI_HEADERS_DIR}
)

add_library(gapi::header_only ALIAS gapi)