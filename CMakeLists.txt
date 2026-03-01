project(NevareaRenderer LANGUAGES CXX)

add_library(NevareaRenderer STATIC
    src/Platform/WindowSystem.cpp
    src/Renderer/Vulkan/VulkanContext.cpp
    src/Renderer/Vulkan/VulkanDevice.cpp
    src/Renderer/Vulkan/VulkanSwapchain.cpp
    src/Application/Application.cpp
    src/Core/Config.cpp
 "src/Renderer/Vulkan/VulkanFrames.cpp")

target_compile_definitions(NevareaRenderer PUBLIC NEVAREA_DEBUG)

target_include_directories(NevareaRenderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(NevareaRenderer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/external
)

add_subdirectory(external/glfw)
target_link_libraries(NevareaRenderer PRIVATE glfw)

find_package(Vulkan REQUIRED)
target_link_libraries(NevareaRenderer PRIVATE Vulkan::Vulkan)

set_target_properties(NevareaRenderer PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

if(MSVC)
    target_compile_options(NevareaRenderer PRIVATE /W4 /permissive-)
else()
    target_compile_options(NevareaRenderer PRIVATE -Wall -Wextra -pedantic)
endif()