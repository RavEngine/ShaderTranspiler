cmake_minimum_required(VERSION 3.17)
# TODO: not rely on std::filesystem
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
PROJECT(ShaderTranspiler)

if(MSVC)
add_definitions("/std:c++latest")
endif()

SET(ST_DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps")

add_subdirectory("${ST_DEPS_DIR}/glslang")
add_subdirectory("${ST_DEPS_DIR}/SPIRV-Cross")
add_subdirectory("${ST_DEPS_DIR}/SPIRV-Tools")

file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/ShaderTranspiler/*.hpp")
file(GLOB CHEADERS "include/ShaderTranspiler/*.h")

add_library("${PROJECT_NAME}" ${SOURCES} ${CHEADERS} ${HEADERS})

target_include_directories("${PROJECT_NAME}" PRIVATE 
    "include/ShaderTranspiler/"
)

target_link_libraries("${PROJECT_NAME}" PRIVATE 
    "glslang"
    "SPIRV"
    "SPIRV-Tools-opt"
    "SPIRV-Tools-reduce"
    "spirv-cross-glsl"
    "spirv-cross-hlsl"
    "spirv-cross-msl"
    "spirv-cross-reflect"
)
target_include_directories("${PROJECT_NAME}" 
    PUBLIC "include/"
    PUBLIC "${ST_DEPS_DIR}/glslang/glslang/" # TODO: figure out how to not need this   
)

add_executable("${PROJECT_NAME}_test" "test/main.cpp")
target_link_libraries("${PROJECT_NAME}_test" "${PROJECT_NAME}")
