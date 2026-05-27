cmake_minimum_required(VERSION 3.10)
project(NevareaRenderer LANGUAGES CXX)

add_library(VulkanMemoryAllocator INTERFACE)

target_include_directories(VulkanMemoryAllocator INTERFACE vendor/vma/)

add_library(NevareaRenderer STATIC
    src/Platform/WindowSystem.cpp
    src/Renderer/Vulkan/VulkanContext.cpp
    src/Renderer/Vulkan/VulkanDevice.cpp
    src/Renderer/Vulkan/VulkanSwapchain.cpp
    src/Core/Config.cpp
    src/Renderer/Vulkan/VulkanFrames.cpp
    src/Renderer/Rendering.cpp
 "src/Renderer/Vulkan/VulkanAllocator.cpp" "src/Renderer/Vulkan/VulkanResourceManager.cpp" "src/Renderer/Vulkan/VulkanPipeline.cpp" "src/Renderer/Vulkan/VulkanDebug.cpp")

 target_precompile_headers(NevareaRenderer PRIVATE
    src/Core/n_pch.hpp
)

target_compile_definitions(NevareaRenderer PUBLIC
    NEVAREA_DEBUG
)

target_include_directories(NevareaRenderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(NevareaRenderer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

find_package(Vulkan REQUIRED)
target_link_libraries(NevareaRenderer PRIVATE Vulkan::Vulkan)

target_link_libraries(NevareaRenderer PRIVATE VulkanMemoryAllocator)

set_target_properties(NevareaRenderer PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

if(MSVC)
    target_compile_options(NevareaRenderer PRIVATE /W4 /permissive-)
else()
    target_compile_options(NevareaRenderer PRIVATE -Wall -Wextra -pedantic)
endif()