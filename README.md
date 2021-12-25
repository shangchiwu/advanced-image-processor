# Advanced Image Processor

[![Build Check](https://github.com/shangchiwu/advanced-image-processor/actions/workflows/build-check.yml/badge.svg)](https://github.com/shangchiwu/advanced-image-processor/actions/workflows/build-check.yml)
[![Create GitHub Release](https://github.com/shangchiwu/advanced-image-processor/actions/workflows/create-github-release.yml/badge.svg)](https://github.com/shangchiwu/advanced-image-processor/actions/workflows/create-github-release.yml)
[![release version](https://img.shields.io/github/v/release/shangchiwu/advanced-image-processor)](https://github.com/shangchiwu/advanced-image-processor/releases/latest)
[![license: MIT](https://img.shields.io/badge/license-MIT-green.svg)](https://opensource.org/licenses/MIT)

The image editor used in the Advanced Image Processing course.

The Advanced Image Processor is a simple image editor, which is written in C++20 and runs on Windows, and can be compiled into single executable.

## Features

- Supported image file formats:
  - open: BMP, GIF, JPEG, PNG, PPM (P5 and P6)
  - save: JPG, PNG
- Gray scale and gray histogram
- Gaussian noise (with adjustable sigma)
- Image resizing
- HAAR wavelet transform
- Histogram equalization
- Convolution
- ...and more features in the future

## Dependencies

This project uses the following third-party libraries:

- [Dear ImGUI](https://github.com/ocornut/imgui) (v1.84.2) - interactive GUI
- [GLFW](https://www.glfw.org/) (v3.3.4) - window display
- [Native File Dialog Extended](https://github.com/btzy/nativefiledialog-extended) (master (commit 800e880)) - GUI filesystem dialog
- [stb_image, stb_image_write](https://github.com/nothings/stb) (v2.27, v1.16) - load/save image file
- [stb_image_resize](https://github.com/nothings/stb) (v0.97) - resize image
- [clip](https://github.com/dacap/clip) - clipboard support
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
