# Advanced Image Processor

The image editor used in the Advanced Image Processing course.

The Advanced Image Processor is a simple image editor, which is written in C++ 17 and runs on Windows, and can be compiled into single executable.

## Features

### File Formats

- BMP
- JPEG
- PNG
- PPM

### Functions

- Open / Save image.

## Third Party

This project uses the following libraries / technologies:

- [Dear ImGUI](https://github.com/ocornut/imgui) (v1.84.2) - interactive GUI
- [GLFW](https://www.glfw.org/) (v3.3.4) - window display
- [Native File Dialog](https://github.com/mlabbe/nativefiledialog) (release_116) - GUI filesystem dialog
- [stb_image, stb_image_write](https://github.com/nothings/stb) (v2.27, v1.16) - load/save image file
- OpenMP - parallel multi-threading computing for speedup
