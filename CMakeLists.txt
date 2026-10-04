# Specify the minimum CMake version required for this project
cmake_minimum_required(VERSION 3.14)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define the project name and set the C++ standard
project(BeLightVulkan)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build output directories to keep build artifacts organized
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find required system packages using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
find_package(Vulkan REQUIRED)

include(FetchContent)

FetchContent_Declare(glm
  URL https://github.com/g-truc/glm/releases/download/0.9.9.8/glm-0.9.9.8.zip
  SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/glm-src
)

FetchContent_Declare(stb_image
  URL https://github.com/nothings/stb/archive/refs/heads/master.zip
  SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/stb-src
)

FetchContent_Declare(imgui
  URL https://github.com/ocornut/imgui/archive/refs/tags/v1.91.9.tar.gz
  SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/imgui-src
)

FetchContent_Declare(tinyobjloader
  URL https://github.com/tinyobjloader/tinyobjloader/archive/refs/tags/v2.0.0rc13.tar.gz
  SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/tinyobjloader-src
)

FetchContent_MakeAvailable(glm stb_image imgui tinyobjloader)

# List all source files that need to be compiled into the executable
set(SOURCES
    src/main.cpp
)

# Create the executable target named "BeLightVulkan" from the specified source files
add_executable(BeLightVulkan ${SOURCES})

target_sources(BeLightVulkan PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

# Enable Vulkan-Hpp designated initializers
target_compile_definitions(BeLightVulkan PRIVATE
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
    VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
)

# Link the required system libraries to the executable
target_link_libraries(BeLightVulkan
    PkgConfig::SDL2
    Vulkan::Vulkan
    tinyobjloader
    dl
)

# Include the directories that contain headers for the linked libraries and fetched dependencies
target_include_directories(BeLightVulkan PRIVATE
    include
    $ENV{VULKAN_SDK}/include
    ${CMAKE_BINARY_DIR}/_deps/glm-src
    ${CMAKE_BINARY_DIR}/_deps/stb-src
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
