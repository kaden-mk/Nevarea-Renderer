cmake_minimum_required(VERSION 3.10)
project(NevareaRenderer LANGUAGES CXX C)

add_library(VulkanMemoryAllocator INTERFACE)
target_include_directories(VulkanMemoryAllocator SYSTEM INTERFACE vendor/vma/)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT DEFINED NEVAREA_BUILD_DYNAMIC)
  set(NEVAREA_BUILD_DYNAMIC ON)
endif()

if(NEVAREA_BUILD_DYNAMIC)
  set(NEVAREA_LIB_TYPE SHARED)
else()
  set(NEVAREA_LIB_TYPE STATIC)
endif()

add_library(NevareaRenderer ${NEVAREA_LIB_TYPE}
    src/Platform/WindowSystem.cpp
    src/Renderer/Vulkan/VulkanContext.cpp
    src/Renderer/Vulkan/VulkanDevice.cpp
    src/Renderer/Vulkan/VulkanSwapchain.cpp
    src/Renderer/Vulkan/VulkanFrames.cpp
    src/Renderer/Rendering.cpp
 "src/Renderer/Vulkan/VulkanAllocator.cpp" "src/Renderer/Vulkan/VulkanResourceManager.cpp" "src/Renderer/Vulkan/VulkanPipeline.cpp" "src/Renderer/Vulkan/VulkanDebug.cpp" "src/Renderer/Vulkan/VulkanTranslate.cpp" "src/Renderer/Vulkan/vulkan_interop.cpp")

target_precompile_headers(NevareaRenderer PRIVATE
  "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/src/Core/n_pch.hpp>"
)

target_compile_definitions(NevareaRenderer PUBLIC
  $<$<CONFIG:Debug,RelWithDebInfo>:NEVAREA_DEBUG>
)

if(NEVAREA_BUILD_DYNAMIC)
  target_compile_definitions(NevareaRenderer PUBLIC NEVAREA_BUILD_DLL)
  target_compile_definitions(NevareaRenderer PRIVATE NEVAREA_RENDERER_EXPORT)
endif()

target_include_directories(NevareaRenderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(NevareaRenderer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

find_package(Vulkan REQUIRED)
target_link_libraries(NevareaRenderer PRIVATE Vulkan::Headers)

target_link_libraries(NevareaRenderer PRIVATE VulkanMemoryAllocator)

target_sources(NevareaRenderer PRIVATE vendor/volk/volk.c)
target_include_directories(NevareaRenderer SYSTEM PRIVATE vendor/volk/)
if(WIN32)
  target_compile_definitions(NevareaRenderer PRIVATE VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
  target_compile_definitions(NevareaRenderer PRIVATE VK_USE_PLATFORM_METAL_EXT)
elseif(UNIX)
  target_compile_definitions(NevareaRenderer PRIVATE VK_USE_PLATFORM_XLIB_KHR)
endif()

set_target_properties(NevareaRenderer PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

if(MSVC)
  target_compile_options(NevareaRenderer PRIVATE /W4 /permissive- /FS)
else()
  target_compile_options(NevareaRenderer PRIVATE -Wall -Wextra -pedantic)
endif()

include(GNUInstallDirs)

install(TARGETS NevareaRenderer
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(NEVAREA_BUILD_DYNAMIC AND MSVC)
  install(FILES $<TARGET_PDB_FILE:NevareaRenderer> DESTINATION ${CMAKE_INSTALL_BINDIR} OPTIONAL)
endif()
