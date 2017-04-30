# wxVulkanTutorial

wxVukanTutorial is a port of the Vulkan Tutorial at [www.vulkan-tutorial.com](https://www.vulkan-tutorial.com).

<h2>Requirements</h2>

This repository is set up to be built using Visual Studio 2017. If you wish to build using any other
toolset, you are on your own. I will, of course, accept any contributions related to doing so.

In addition to Visual Studio 2017, the following tools and libraries are required in order to build the
tutorial:
- wxWidgets version 3.03+
- Vulkan SDK
- GLM
- the file stb_image.h from the [stb collection](https://github.com/nothings/stb)
- an image to use as a texture. I used the one provided in the [Texture mapping -> Images chapter of vulkan-tutorial.com](https://vulkan-tutorial.com/Texture_mapping/Images).

<h2>Projects</h2>

There are a number of projects in this archive. They correspond to various chapters in the tutorial:
- <b>HelloTriangle</b> contains all code up to the end of Drawing a triangle.
- <b>VertexBuffers</b> contains all code up to the end of Vertex buffers.
- <b>UniformBuffers</b> contains all code up to the end of Uniform buffers.
- <b>TextureMapping</b> contains all code up to the end of Texture mapping.

More information about wxVulkanTutorial is available via a [blog post](https://usingcpp.wordpress.com/2016/12/10/vulkan-with-wxwidgets/).

<h2>Notes</h2>
<h3>UniformBuffers</h3>

In the Uniform buffers -> Descriptor layout and buffer page at www.vulkan-tutorial.com, the following line of code is added to the updateUniformBuffer method:
<pre>ubo.proj[1][1] *= -1;</pre>
along with the following reason:

"GLM was originally designed for OpenGL, where the Y coordinate of the clip coordinates is inverted. The easiest way to compensate for that is to flip the sign on the scaling factor of the Y axis in the projection matrix. If you don't do this, then the image will be rendered upside down."

In the wxWidgets implementation, this line of code is not needed. This will have an effect on the code in the DepthBuffering and LoadingModels projects. Comments will be added there to discuss the required changes.
