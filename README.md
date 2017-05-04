# wxVulkanTutorial

wxVukanTutorial is a port of the Vulkan Tutorial at [www.vulkan-tutorial.com](https://www.vulkan-tutorial.com).

<h2>Requirements</h2>

This repository is set up to be built using Visual Studio 2017. If you wish to build using any other
toolset, you are on your own. I will, of course, accept any contributions related to doing so.

In addition to Visual Studio 2017, the following tools and libraries are required in order to build the
tutorial:
- wxWidgets version 3.10+
- Vulkan SDK
- GLM
- the file stb_image.h from the [stb collection](https://github.com/nothings/stb)
- an image to use as a texture. I used the one provided in the [Texture mapping -> Images chapter of vulkan-tutorial.com](https://vulkan-tutorial.com/Texture_mapping/Images).
- the file tiny_obj_loader.h from the [TINYOBJLOADER library](https://github.com/syoyo/tinyobjloader).
- chalet.jpg and chalet.obj texture and model files for the [Chalet Hippolyte Chassande Baroz](https://sketchfab.com/models/e925320e1d5744d9ae661aeff61e7aef). These files can be downloaded directly from the Vulkan Tutorial website (https://vulkan-tutorial.com/Loading_models).

<h2>Projects</h2>

There are a number of projects in this archive. They correspond to various chapters in the tutorial:
- <b>HelloTriangle</b> contains all code up to the end of Drawing a triangle.
- <b>VertexBuffers</b> contains all code up to the end of Vertex buffers.
- <b>UniformBuffers</b> contains all code up to the end of Uniform buffers.
- <b>TextureMapping</b> contains all code up to the end of Texture mapping.
- <b>DepthBuffering</b> contains all code up to the end of Depth buffering.
- <b>LoadingModels</b> contains all code up to the end of Loading models.

More information about wxVulkanTutorial is available via a [blog post](https://usingcpp.wordpress.com/2016/12/10/vulkan-with-wxwidgets/).

<h2>Notes</h2>

<h2>stb_image.h and tiny_obj_loader.h<h3>

These files are from the stb_collection and the TINYOBJLOADER library respectively. I placed these two header files directly into the source files directory instead of maintaining them in separate library directories as suggested or stated in the tutorial at www.vulkan-tutorial.com. Appropriate changes were made in VulkanCanvas.cpp to pick up the files from the correct location. They are also included in the git repository, so you will not have to load them separately.

<h3>Shaders, Texture Files, Models</h3>

In the tutorial at www.vulkan-tutorial.com, shaders, texture files and models are stored in subdirectories off the source directory. In the projects in this solution, all of the files are maintained in the source directory. The shaders are compiled directly into the output directory ($(OutDir) in Visual Studio). The texture files and model are simply copied from the source directory to the output directory. Appropriate changes were made to the code in VulkanCanvas.cpp to pick up the files from there.

<h3>UniformBuffers</h3>

In the Uniform buffers -> Descriptor layout and buffer page at www.vulkan-tutorial.com, the following line of code is added to the updateUniformBuffer method:
<pre>ubo.proj[1][1] *= -1;</pre>
along with the following reason:

"GLM was originally designed for OpenGL, where the Y coordinate of the clip coordinates is inverted. The easiest way to compensate for that is to flip the sign on the scaling factor of the Y axis in the projection matrix. If you don't do this, then the image will be rendered upside down."

In the wxWidgets implementation, this line of code is not needed. In the code on the tutorial website, the <i>updateUniformBuffer</i> method contains the line (two lines above the code shown above):
<pre>ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));</pre>

This should be changed to:
<pre>ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));</pre>

The only change is to set the Z-component of the <i>up</i> argument to <i>-1.0f</i> from <i>1.0f</i>.

Making this change in UniformBuffers does not result in any visible change in the resulting display. It does have an effect later, on the DepthBuffer project. If you do not make this change, then depth buffering will not function correctly.

<h3>LoadingModels</h3>

The tutorial for this project (https://vulkan-tutorial.com/Loading_models), suggests building a Release configuration. The project in this repository only has a Debug X64 configuration because that is what I built. And yes, as stated in the tutorial, loading the model is very slow.
