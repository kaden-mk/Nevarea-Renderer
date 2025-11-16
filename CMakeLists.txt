cmake_minimum_required(VERSION 3.8)

project(NevareaRenderer LANGUAGES CXX)

add_library(NevareaRenderer STATIC
    src/Platform/WindowSystem.cpp
    src/Renderer/Vulkan/VulkanContext.cpp
    src/Renderer/Vulkan/VulkanDevice.cpp
    src/Application/NevareaApplication.cpp
 "src/Renderer/Vulkan/VulkanSwapchain.cpp")

target_compile_definitions(NevareaRenderer PUBLIC NEVAREA_DEBUG)

target_include_directories(NevareaRenderer
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/external
)

add_subdirectory(external/glfw)
target_link_libraries(NevareaRenderer PUBLIC glfw)

find_package(Vulkan REQUIRED)
target_link_libraries(NevareaRenderer PUBLIC Vulkan::Vulkan)

set_target_properties(NevareaRenderer PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
if(MSVC)
    target_compile_options(NevareaRenderer PRIVATE /W4 /permissive-)
else()
    target_compile_options(NevareaRenderer PRIVATE -Wall -Wextra -pedantic)
endif()

add_subdirectory(Editor)