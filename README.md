# wxVulkanTutorial

wxVukanTutorial is a port of the Hello Triangle portion of the Vulkan Tutorial at [www.vulkan-tutorial.com](https://www.vulkan-tutorial.com).

<h2>Requirements</h2>

This repository is set up to be built using Visual Studio 2017. I have built and tested only the x64 Debug build. Some changes
will be required for other configurations. I will leave that to you to determine the required changes.
If you wish to build using any other
toolset, you are on your own. I will, of course, accept any contributions related to doing so.

In addition to Visual Studio 2017, the following tools and libraries are required in order to build the
tutorial:
- wxWidgets vcpkg package.
- Vulkan SDK. I do not use the vcpkg package because it does not do anything.
- GLM vcpkg package.

<h2>Projects</h2>

More information about wxVulkanTutorial is available via a [blog post](https://usingcpp.wordpress.com/2016/12/10/vulkan-with-wxwidgets/).

<h2>Notes</h2>

<h3>Shaders</h3>

In the tutorial at www.vulkan-tutorial.com, shaders are stored in subdirectories off the source directory. In the project in this solution, all of the files are maintained in the source directory. The shaders are compiled directly into the output directory ($(OutDir) in Visual Studio).
Appropriate changes were made to the code in VulkanCanvas.cpp to pick up the files from there.
