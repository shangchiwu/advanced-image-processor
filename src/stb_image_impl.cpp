#ifndef ADVANCED_IMAGE_PROCESSOR_STB_IMAGE_IMPL_H__
#define ADVANCED_IMAGE_PROCESSOR_STB_IMAGE_IMPL_H__

#ifdef _WIN32
    #define STBI_WINDOWS_UTF8
    #define STBIW_WINDOWS_UTF8
#endif // _WIN32

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

#endif // ADVANCED_IMAGE_PROCESSOR_STB_IMAGE_IMPL_H__
