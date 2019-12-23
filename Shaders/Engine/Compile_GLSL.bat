@echo off
for %%f in (*.frag *.vert) do %VULKAN_SDK%\Bin32\glslc.exe -O -x glsl --target-env=vulkan1.1 -c %%f
pause