# Aether3D Game Engine
This codebase will evolve into the next generation [Aether3D](http://twiren.kapsi.fi/aether3d.html). More info: http://bioglaze.blogspot.fi/2014/12/planning-aether3d-rewrite-for-2015.html

![Screenshot](/Engine/Assets/sample.jpg)

# Features

  - Windows, OS X, iOS and Linux support.
  - OpenGL 4.1, Vulkan (WIP), D3D12 (WIP) and Metal renderers.
  - Component-based game object system.
  - Oculus Rift support.
  - Sprite rendering, texture atlasing and batching.
  - Bitmap and Signed Distance Field font rendering using BMFont fonts.
  - Variance shadow mapping.
  - Audio support for .wav and .ogg.
  - Hot-reloading of assets.
  - Custom model format with .obj and Blender exporter.
  - Virtual file system for .pak files.
  - XBox controller support.
  - Cross-Platform scene editor.
  - Statically linked into your application.

# Status (as of 2015-12-26)

  - API is not stable until 1.0 (developing 0.5 now)
  - OpenGL renderer is the most featureful but others will catch up.
  - Master branch may or may not compile but releases should always work.
  - Editor is still missing many features that are needed for proper scene creation.
  - Lights cast shadows but don't shade objects (planned for 0.6)

# Planned Features

  - Clustered Forward lighting.
  - Most of the features in my [previous engine](http://twiren.kapsi.fi/aether3d.html)

# Build

  - After building build artifacts can be found in aether3d_build next to aether3d.
  - Grab the [sample asset archive](http://twiren.kapsi.fi/files/aether3d_sample_v0.5.zip) and extract it into aether3d_build/Samples after building.

## Windows

  - Open the project in Engine\VisualStudio_* in VS2015 and build it. For MinGW you can use Engine/Makefile.
  - Build and run Samples\01_OpenWindow.
  - Vulkan users: built-in shader sources are located in Engine\assets. If you modify them, you can build and deploy them by running compile_deploy_vulkan_shaders.cmd. 
  
## OS X / GNU/Linux

  - Open the project Engine/Aether3D_OSX or Engine/Aether3D_OSX_Metal in Xcode and build it or run the Makefile.
  - Open the project Samples/01_OpenWindow or Samples/MetalSampleOSX and run it or run the Makefile.

## iOS
  - Build Aether3D_iOS in Engine. It creates a framework.
  - Open Samples/MetalSampleiOS and add the framework into the project.

# Running Tests

## Visual Studio

  - Unit test project can be found in Engine\Tests\UnitTests.

## GCC or Clang

  - You can find Makefiles in Engine/Tests.

# License

The engine is licensed under zlib license.

Third party library licenses are:

  - stb_image.c is in public domain
  - stb_vorbis.c is in public domain
  - glxw is under zlib license
  - OpenAL-soft is under LGPLv2 license

# Roadmap/internal TODO

https://docs.google.com/document/d/1jKuEkIUHiFtF4Ys2-duCNKfV0SrxGgCSN_A2GhWeLDw/edit?usp=sharing
