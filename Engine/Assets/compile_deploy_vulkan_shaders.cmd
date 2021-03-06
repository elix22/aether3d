%VULKAN_SDK%\bin\glslangvalidator.exe -D -V -S vert -e main hlsl\sprite_vert.hlsl -o ..\..\..\aether3d_build\Samples\sprite_vert.spv
%VULKAN_SDK%\bin\glslangvalidator.exe -D -V -S frag -e main hlsl\sprite_frag.hlsl -o ..\..\..\aether3d_build\Samples\sprite_frag.spv
%VULKAN_SDK%\bin\glslangvalidator.exe -D -V -S vert -e main hlsl\unlit_vert.hlsl -o ..\..\..\aether3d_build\Samples\unlit_vert.spv
%VULKAN_SDK%\bin\glslangvalidator.exe -D -V -S frag -e main hlsl\unlit_frag.hlsl -o ..\..\..\aether3d_build\Samples\unlit_frag.spv
%VULKAN_SDK%\bin\glslangvalidator.exe -D -V -S vert -e main hlsl\unlit_skin_vert.hlsl -o ..\..\..\aether3d_build\Samples\unlit_skin_vert.spv
%VULKAN_SDK%\bin\glslangvalidator.exe -D -V -S vert -e main hlsl\skybox_vert.hlsl -o ..\..\..\aether3d_build\Samples\skybox_vert.spv
%VULKAN_SDK%\bin\glslangvalidator.exe -D -V -S frag -e main hlsl\skybox_frag.hlsl -o ..\..\..\aether3d_build\Samples\skybox_frag.spv
%VULKAN_SDK%\bin\glslangvalidator.exe -D -V -S vert -e main hlsl\depthnormals_vert.hlsl -o ..\..\..\aether3d_build\Samples\depthnormals_vert.spv
%VULKAN_SDK%\bin\glslangvalidator.exe -D -V -S frag -e main hlsl\depthnormals_frag.hlsl -o ..\..\..\aether3d_build\Samples\depthnormals_frag.spv
%VULKAN_SDK%\bin\glslangvalidator.exe -D -V -S comp -e CSMain hlsl\LightCuller.hlsl -o ..\..\..\aether3d_build\Samples\LightCuller.spv
%VULKAN_SDK%\bin\glslangvalidator.exe -D -V -S vert -e main hlsl\Standard_vert.hlsl -o ..\..\..\aether3d_build\Samples\Standard_vert.spv
%VULKAN_SDK%\bin\glslangvalidator.exe -D -V -S frag -e main hlsl\Standard_frag.hlsl -o ..\..\..\aether3d_build\Samples\Standard_frag.spv
%VULKAN_SDK%\bin\glslangvalidator.exe -D -V -S vert -e main hlsl\fullscreen_triangle_vert.hlsl -o ..\..\..\aether3d_build\Samples\fullscreen_triangle_vert.spv
%VULKAN_SDK%\bin\glslangvalidator.exe -D -V -S frag -e main hlsl\vr_companion_frag.hlsl -o ..\..\..\aether3d_build\Samples\vr_companion_frag.spv
%VULKAN_SDK%\bin\glslangvalidator.exe -D -V -S vert -e main hlsl\vr_companion_vert.hlsl -o ..\..\..\aether3d_build\Samples\vr_companion_vert.spv

@REM %VULKAN_SDK%\Bin\spirv-opt hlsl\sprite_vert.spv --inline-entry-points-exhaustive --convert-local-access-chains --eliminate-local-single-block --eliminate-local-single-store --eliminate-insert-extract --eliminate-dead-code-aggressive --eliminate-dead-branches --merge-blocks --eliminate-local-single-block --eliminate-local-single-store --eliminate-local-multi-store --eliminate-insert-extract --eliminate-dead-code-aggressive --eliminate-common-uniform -o ..\..\..\aether3d_build\Samples\sprite_vert_opt.spv
@REM %VULKAN_SDK%\Bin\spirv-remap --strip all --dce all -i hlsl\sprite_vert_opt.spv -o ..\..\..\aether3d_build\Samples\

pause

