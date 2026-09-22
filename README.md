# SplatTeleportVR

### Features

<ul>
	<li> Undo and Redo
	<li> Virtual Reality viewing and editing
	<li> Select via brush, sphere, rectangle; Delete with brush or del button
	<li> Translate, scale, rotate of selected nodes or splats.
	<li> Painting
	<li> No pip or conda issues! (no Python)
	<li> CUDA Driver API - Can edit and hot-reload CUDA code like shaders.
	<li> Includes support for <a href="https://github.com/nerficg-project/HTGS">perspective-correct gaussians</a>. (Splat models need to be trained with respective method)
</ul>

### Known Issues
<ul>
	<li> Academic Prototype - expect a fair amount of bugs and crashes -> Consider it an "alpha" version.
	<li> VR only on windows - No idea how to include OpenVR on linux. 
	<li> No Spherical Harmonics due to large mem req. with modest gains - we will check out more frugal SH options in the future. 
	<li> CUDA Driver API - Long startup because CUDA compiles at runtime. 
	<li> No precompiled binaries yet. 
	<li> Undo and redo may not cover all functionality yet. 
	<li> Undo of translate/scale/rotate is currently lossy. Especially scale is prone to loss of precision when scaling too far down. 
	<li> App freezing: Line rendering caused freezes on RTX 20xx series cards. That was fixed, but there is a chance that triangle models in VR mode may also freeze RTX 20xx series cards. 
</ul>

## Installing

<details>
<summary>Windows</summary>

Dependencies: 
* CUDA 12.4
* Visual Studio 2022 (version 17.10.3)
* CMake 3.27
* RTX 4070 or better recommended. 

Create Visual Studio solution files in a build folder via cmake:

```
mkdir build
cd build
cmake ../
```

Open build/SplatEditor.sln and compile in Release mode.<br>
Then run from project directory. 

```
./build/Release/SplatEditor.exe
```

Running from project directory is semi-important because the editor will look for resources such as ```./src/gaussians_rendering.cu``` and ```./resources/images/symbols_32_32.png``` relative to the project directory. Alternatively, you can copy the resources and src folders into the binary directory.



</details>

<details>
<summary>Linux</summary>

Dependencies:
* gcc 14, g++14
* CUDA 12.4 or higher
* NVIDIA driver version 550 or higher. (Lower may cause issues when compiling GPUSorting)

```
mkdir build
cd build
export CUDA_PATH=/usr/local/cuda-12.4/
cmake -DCUDA_CUDART_LIBRARY=/usr/local/cuda/lib64/libcudart.so ..
make
```

Potential Issues:
- May or may not require TBB. Uncomment the TBB part in CMakeLists, if it does.

```
// set CUDA_PATH variable to wherever your CUDA Toolkit is installed
export CUDA_PATH=/usr/local/cuda-12.4/

// May or may not require setting LD_LIBRARY_PATH to gcc 14, similar to this, or whereever gcc14 is installed
export LD_LIBRARY_PATH=~/gcc-14.1.0/x86_64-linux-gnu/libstdc++-v3/src/.libs:$LD_LIBRARY_PATH

// run from workspace root
./build/SplatEditor
```

</details>

## Getting Started


	ISSN = {1467-8659},
	DOI = {10.1111/cgf.70214}
}
</pre>
