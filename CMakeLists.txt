cmake_minimum_required(VERSION 3.17)
set(CMAKE_XCODE_GENERATE_SCHEME OFF CACHE INTERNAL "")

option(ST_ENABLE_TEST "Enable tests" ON)
PROJECT(ShaderTranspiler)

if(MSVC)
add_definitions("/std:c++latest")
endif()

SET(ST_DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps")

# dependencies
set(SKIP_GLSLANG_INSTALL ON CACHE INTERNAL "")
set(BUILD_EXTERNAL OFF CACHE INTERNAL "")
set(ENABLE_GLSLANG_BINARIES OFF CACHE INTERNAL "")
set(ENABLE_CTEST OFF CACHE INTERNAL "")
add_subdirectory("${ST_DEPS_DIR}/glslang" EXCLUDE_FROM_ALL)

set(SPIRV_CROSS_CLI OFF CACHE INTERNAL "")
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE INTERNAL "")
set(SPIRV_CROSS_ENABLE_C_API OFF CACHE INTERNAL "")
set(SPIRV_CROSS_SKIP_INSTALL ON CACHE INTERNAL "")
add_subdirectory("${ST_DEPS_DIR}/SPIRV-Cross" EXCLUDE_FROM_ALL)

set(SKIP_SPIRV_TOOLS_INSTALL ON CACHE INTERNAL "")
set(SPIRV_SKIP_EXECUTABLES ON CACHE INTERNAL "")
set(SPIRV_SKIP_TESTS ON CACHE INTERNAL "")
add_subdirectory("${ST_DEPS_DIR}/SPIRV-Tools" EXCLUDE_FROM_ALL)

file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/ShaderTranspiler/*.hpp")
file(GLOB CHEADERS "include/ShaderTranspiler/*.h")

add_library("${PROJECT_NAME}" ${SOURCES} ${CHEADERS} ${HEADERS})
target_compile_features("${PROJECT_NAME}" PRIVATE cxx_std_17)
set_target_properties("${PROJECT_NAME}" PROPERTIES XCODE_GENERATE_SCHEME ON)

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

if (ST_ENABLE_TEST)
    add_executable("${PROJECT_NAME}_test" "test/main.cpp")
    target_link_libraries("${PROJECT_NAME}_test" "${PROJECT_NAME}")
    set_target_properties("${PROJECT_NAME}_test" PROPERTIES XCODE_GENERATE_SCHEME ON)
    target_compile_features("${PROJECT_NAME}_test" PRIVATE cxx_std_17)
endif()
