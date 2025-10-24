cmake_minimum_required(VERSION 3.8)

project(NevareaRenderer LANGUAGES CXX)

add_executable(Nevarea
    src/main.cpp
    src/Platform/WindowSystem.cpp
    src/Application/NevareaApplication.cpp
 "src/Renderer/Vulkan/VulkanContext.cpp" "src/Renderer/Vulkan/VulkanDevice.cpp")

target_compile_definitions(Nevarea PRIVATE NEVAREA_DEBUG)

target_include_directories(Nevarea
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/external
)

add_subdirectory(external/glfw)
target_link_libraries(Nevarea PRIVATE glfw)

find_package(Vulkan REQUIRED)
target_link_libraries(Nevarea PRIVATE Vulkan::Vulkan)

set_target_properties(Nevarea PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
if(MSVC)
    target_compile_options(Nevarea PRIVATE /W4 /permissive-)
else()
    target_compile_options(Nevarea PRIVATE -Wall -Wextra -pedantic)
endif()

add_custom_target(Run
    COMMAND Nevarea
    DEPENDS Nevarea
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)