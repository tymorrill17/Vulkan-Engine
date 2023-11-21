# CMakeList.txt : CMake project for vulkan_guide, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("Vulkan-Engine")
# Sets cmake version
set(CMAKE_CXX_STANDARD 17)
# Obtains all of the Vulkan API compiling information
find_package(Vulkan REQUIRED)
# Subdirectory to handle third_party library compilation
add_subdirectory(third_party)
# Sets the location where the executable will be placed
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
# Subdirectory that contains all the source code
add_subdirectory(src)
# glslangValidator converts glsl shader code to SPIR-V shaders. Find the program to use later in shader compilation
find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

# Find all the shader files under the shaders folder
file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/shaders/*.frag"
    "${PROJECT_SOURCE_DIR}/shaders/*.vert"
    "${PROJECT_SOURCE_DIR}/shaders/*.comp"
    )

# Iterate over each shader and compile them with glslangValidator
foreach(GLSL ${GLSL_SOURCE_FILES})
  message(STATUS "BUILDING SHADER")
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
  message(STATUS ${GLSL})
  # Execute glslang command to compile the current shader
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)
# A target is an executable, library, etc. that cmake builds. Create a custom shader target for our shader modules.
add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
    )
