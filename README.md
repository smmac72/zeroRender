
  

# Zero Renderer

  

A very small util for rendering .obj files in .tga format.

  

Supports two rasterization types - Line Sweep and via the barycentric coordinates. The latter supports shaders - currently Gouraud and Phong shaders are available.

Includes ability to set scene parameters (check **"Usage"** for more info)

  

## Usage
[DOWNLOAD BUILD](https://github.com/smmac72/basicRender/releases/tag/1.0)

**./zr filename.obj**

  

Automatically links **filename_diffuse.tga** as the diffuse texture. If not present, doesn't work, as no shading is possible. Probably will fix it someday.

**Options:**

-  **./zr -\-help** - Display help message

-  **-o fileName(.tga)** - Output filename. Can ignore the type, fills automatically. If not stated, saved as inputFilename.tga

-  **-\-light x y z** - Light source direction. Default: 0 0 1

-  **-\-center x y z** - Lookat position. Default: 0 0 0

-  **-\-camera x y z** - Camera position. Default: 0 0 1

-  **-\-up x y z** - Up vector direction. Default: 0 1 0

-  **-\-width x** - Width of the output. Default: 800

-  **-\-height x** - Height of the output. Default: 800

-  **-\-raster type** - Rasterization type (linesweep/barycentric) Default: barycentric

-  **-\-shader type (gouraud/phong)** - Sets active shader. Always Gouraud for the Line Sweep rasterization. Barycentric rasterization can choose between Gouraud and Phong shaders. Default for barycentric: phong

-  **-\-phongcoef x y z w** - Set Phong lighting coefficients in order - Ambient, Diffuse, Specular, Shininess. Default: 0.5 0.5 0.5 0.5

-  **--dumpZBuffer file** - Dump Z-buffer file. If not stated, doesn't do the dumping

## Examples

Repo includes two test input files with corresponding diffuse textures - **model.obj** (stolen) and **smmac.dff** (peak 3d design)

Due to the virus threat of a **model.obj**, only the latter one is in the downloadable build. lmao.
## Build

Doesn't include the Makefile yet. You can enjoy the .sln file now, sorry mates