# Nevarea Renderer

## TODO (for the first triangle!!)
- [] Remove application creation and instead hook into a window/application. (Since Nevarea is a renderer, not an engine).
- [] Define some custom macros for ease of use internally
	- NEVAREA_ASSERT
	- NEVAREA_FORCE_INLINE
	- NEVAREA_API (for later .dll use)
- [] Implement VulkanMemoryAllocator https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/tree/master
- [] Resource Manager for handles instead of raw VkBuffers
- [] Implement a PipelineBuilder system
- [] Finally, render our first triangle

## Major Goals (Very Deep into the future)
- Add multi-layer graphics api support
- Add CUDA support