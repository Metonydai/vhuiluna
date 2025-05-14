cmake_minimum_required(VERSION 3.15)
project(huiEngine)

find_package(Vulkan REQUIRED)
find_program(glslangValidator_exe NAMES glslangValidator HINTS "$ENV{VULKAN_SDK}/bin")
message(STATUS "glslangValidator path: ${glslangValidator_exe}")

# specify the C++ standard
add_library(cpp_compiler_flags INTERFACE)
target_compile_features(cpp_compiler_flags INTERFACE cxx_std_17)
set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")

target_compile_options(cpp_compiler_flags INTERFACE
  "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
  "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(deps/GLFW)
add_subdirectory(deps/glm)
add_subdirectory(deps/tinyobjloader)
add_subdirectory(deps/stb)

# Shaders
set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)

file(GLOB SHADERS
  ${SHADER_SOURCE_DIR}/*.vert
  ${SHADER_SOURCE_DIR}/*.frag
  ${SHADER_SOURCE_DIR}/*.comp
  ${SHADER_SOURCE_DIR}/*.geom
  ${SHADER_SOURCE_DIR}/*.tesc
  ${SHADER_SOURCE_DIR}/*.tese
  ${SHADER_SOURCE_DIR}/*.mesh
  ${SHADER_SOURCE_DIR}/*.task
  ${SHADER_SOURCE_DIR}/*.rgen
  ${SHADER_SOURCE_DIR}/*.rchit
  ${SHADER_SOURCE_DIR}/*.rmiss)

foreach(source IN LISTS SHADERS)
  get_filename_component(FILENAME ${source} NAME)
  add_custom_command(
    COMMAND
      ${glslangValidator_exe}
      -o ${SHADER_BINARY_DIR}/${FILENAME}.spv
      -V ${source}
    OUTPUT ${SHADER_BINARY_DIR}/${FILENAME}.spv
    DEPENDS ${source} #${SHADER_BINARY_DIR}
    COMMENT "Compiling ${FILENAME}"
  )
  list(APPEND SPV_SHADERS ${SHADER_BINARY_DIR}/${FILENAME}.spv)
endforeach()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS} SOURCES ${SHADERS})

# Header/Source files
file( GLOB_RECURSE ALL_HEADER_FILES ${PROJECT_SOURCE_DIR}/src/*.hpp)
file( GLOB_RECURSE ALL_SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
source_group( "Header Files" FILES ${ALL_HEADER_FILES} )
source_group( "Source Files" FILES ${ALL_SOURCE_FILES} )

add_executable(${PROJECT_NAME} ${ALL_SOURCE_FILES} ${ALL_HEADER_FILES})
# Ensure shaders build first
add_dependencies(${PROJECT_NAME} shaders)


target_link_libraries(${PROJECT_NAME} PUBLIC
  cpp_compiler_flags
  stb 
  tinyobjloader
  Vulkan::Vulkan
  glfw
  )


target_include_directories(${PROJECT_NAME} PUBLIC 
  ${Vulkan_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/src
  )

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})