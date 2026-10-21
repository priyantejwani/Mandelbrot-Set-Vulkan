cmake_minimum_required(VERSION 3.16)
project(MandelbrotVulkan LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Vulkan
find_package(Vulkan REQUIRED)

# GLFW for window management
find_package(glfw3 3.3 REQUIRED)

# GLM for math
find_package(glm REQUIRED)

add_executable(mandelbrot
    src/main.cpp
    src/vulkan_renderer.cpp
    src/vulkan_renderer.hpp
    src/mandelbrot.frag
    src/mandelbrot.vert
)

target_include_directories(mandelbrot PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${VULKAN_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
)

target_link_libraries(mandelbrot PRIVATE
    ${VULKAN_LIBRARIES}
    glfw
    ${GLM_LIBRARIES}
)

# Copy shaders to build directory
add_custom_command(TARGET mandelbrot POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/src/mandelbrot.frag
        ${CMAKE_CURRENT_SOURCE_DIR}/src/mandelbrot.vert
        $<TARGET_FILE_DIR:mandelbrot>
    COMMENT "Copying shaders to build directory"
)