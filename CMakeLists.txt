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
 "src/Renderer/Vulkan/VulkanAllocator.cpp" "src/Renderer/Vulkan/VulkanResourceManager.cpp" "src/Renderer/Vulkan/VulkanPipeline.cpp")

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

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BIN_DIR ${CMAKE_BINARY_DIR}/shaders)

file(MAKE_DIRECTORY ${SHADER_BIN_DIR})

set(SHADERS
    ${SHADER_DIR}/triangle.vert
    ${SHADER_DIR}/triangle.frag
)

add_custom_target(NevareaShaders)

foreach(SHADER ${SHADERS})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    set(SPV "${SHADER_BIN_DIR}/${FILE_NAME}.spv")

    add_custom_command(
        OUTPUT ${SPV}
        COMMAND glslc ${SHADER} -o ${SPV}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader ${FILE_NAME}"
        VERBATIM
    )

    list(APPEND SPV_FILES ${SPV})
endforeach()

add_custom_target(NevareaShadersBin DEPENDS ${SPV_FILES})
add_dependencies(NevareaRenderer NevareaShadersBin)

set_target_properties(NevareaRenderer PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

if(MSVC)
    target_compile_options(NevareaRenderer PRIVATE /W4 /permissive-)
else()
    target_compile_options(NevareaRenderer PRIVATE -Wall -Wextra -pedantic)
endif()