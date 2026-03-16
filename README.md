# Nevarea Renderer

## TODO (for the first triangle!!)
- [] Remove application creation and instead hook into a window/application. (Since Nevarea is a renderer, not an engine).
- [X] Create a precompiled header. dk why i didnt do this sooner
- [X] Define some custom macros for ease of use internally
	- NEVAREA_ASSERT
	- NEVAREA_FORCE_INLINE
	- NEVAREA_API (for later .dll use)
	- NEVAREA_LOG (possibly for a logging system?)
- [] Implement VulkanMemoryAllocator https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/tree/master
- [] Resource Manager for handles instead of raw VkBuffers
- [] Implement a PipelineBuilder system
- [] Finally, render our first triangle

## What does Nevarea handle for you?

## Major Goals (Very Deep into the future)
- Add multi-layer graphics api support
- Add CUDA support

- Add hardware-accelerated raytracing support hand to hand with rasterization
	- should be like super compatible & modifiable:
	- if you want path-traced lighting but rasterized geometry then u can have that type of thing
	- somehow abstract it into a way where it fits into the renderer category and not like a full on           engine