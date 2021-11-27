# Advanced Image Processor

[![build status badge](https://github.com/shangchiwu/advanced-image-processor/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/shangchiwu/advanced-image-processor/actions?query=workflow%3Abuild+branch%3Amaster)
[![create github release status badge](https://github.com/shangchiwu/advanced-image-processor/actions/workflows/create-github-release.yml/badge.svg)](https://github.com/shangchiwu/advanced-image-processor/actions?query=workflow%3Acreate-github-release)
[![release version](https://img.shields.io/github/v/release/shangchiwu/advanced-image-processor)](https://github.com/shangchiwu/advanced-image-processor/releases/latest)
[![license: MIT](https://img.shields.io/badge/license-MIT-green.svg)](https://opensource.org/licenses/MIT)

The image editor used in the Advanced Image Processing course.

The Advanced Image Processor is a simple image editor, which is written in C++20 and runs on Windows, and can be compiled into single executable.

## Features

### Image File Formats

- Open: BMP, GIF, JPEG, PNG, PPM (P5 and P6)
- Save: JPG, PNG

### Functions

- Open and save image to another formats.
- Convert a image to gray scale and show its histogram.
- Apply gaussian noise on an image with adjustable sigma.
- Resize image.
- HAAR Wavelet Transform.
- ...and more features in the future.

## Dependencies

This project uses the following libraries / technologies:

- [Dear ImGUI](https://github.com/ocornut/imgui) (v1.84.2) - interactive GUI
- [GLFW](https://www.glfw.org/) (v3.3.4) - window display
- [Native File Dialog](https://github.com/mlabbe/nativefiledialog) (release_116) - GUI filesystem dialog
- [stb_image, stb_image_write](https://github.com/nothings/stb) (v2.27, v1.16) - load/save image file
- [stb_image_resize](https://github.com/nothings/stb) (v2.27, v1.16, v0.97) - resize image
- [Freetype](https://freetype.org/) (v2.11.0) - font rasterization
- [Source Han Sans](https://github.com/adobe-fonts/source-han-sans) (v2.004R) - CJK font support

## Build

### Environments and Requirements

The followings are the tested build tools and platform.

- Windows 10 x86-64
- MinGW-w64 from MSYS
- CMake

The dependent libraries are provided in the `libs` directory in order to simplify the preparation and explicitly specify the version of dependencies.

### Build Steps

Run `build.bat` in the project directory. Then the single executable file is in the `build/bin` directory.
