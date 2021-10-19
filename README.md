# Advanced Image Processor

The image editor used in the Advanced Image Processing course.

The Advanced Image Processor is a simple image editor, which is written in C++17 and runs on Windows, and can be compiled into single executable.

## Features

### Image File Formats

- Open: BMP, GIF, JPEG, PNG, PPM (P5 and P6)
- Save: JPG, PNG

### Functions

- Open and save image to another formats.
- Convert a image to gray scale and show its histogram.
- ...and more features in the future.

## Dependencies

This project uses the following libraries / technologies:

- [Dear ImGUI](https://github.com/ocornut/imgui) (v1.84.2) - interactive GUI
- [GLFW](https://www.glfw.org/) (v3.3.4) - window display
- [Native File Dialog](https://github.com/mlabbe/nativefiledialog) (release_116) - GUI filesystem dialog
- [stb_image, stb_image_write](https://github.com/nothings/stb) (v2.27, v1.16) - load/save image file

## Build

### Environments and Requirements

The followings are the tested build tools and platform.

- Windows 10 x86-64
- MinGW-w64 from MSYS
- CMake

The dependent libraries are provided in the `libs` directory in order to simplify the preparation and explicitly specify the version of dependencies.

### Build Steps

Run `build.bat` in the project directory. Then the single executable file is in the `build/bin` directory.
