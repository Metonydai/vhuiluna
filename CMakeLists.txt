cmake_minimum_required(VERSION 3.15)
project(huiEngine)

find_package(Vulkan REQUIRED)

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

# Header/Source files
file( GLOB ALL_HEADER_FILES "*.hpp")
file( GLOB ALL_SOURCE_FILES "*.cpp")
source_group( "Header Files" FILES ${ALL_HEADER_FILES} )
source_group( "Source Files" FILES ${ALL_SOURCE_FILES} )

add_executable(${PROJECT_NAME} ${ALL_SOURCE_FILES} ${ALL_HEADER_FILES})

target_link_libraries(${PROJECT_NAME} PUBLIC cpp_compiler_flags stb tinyobjloader)

# Vulkan-SDK
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)

# GLFW
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
#target_link_libraries(${PROJECT_NAME} PRIVATE glm)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})