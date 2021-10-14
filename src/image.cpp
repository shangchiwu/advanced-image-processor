#include "image.h"

#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <GL/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

Image::Image() : _image_w(0), _image_h(0), _data(nullptr), _image_texture_id(0) {}

Image::Image(const char *filepath) : Image() {
    // load from file
    uint8_t *image_data = stbi_load(filepath, &_image_w, &_image_h, nullptr, 4);
    if (image_data == nullptr)
        return;

    // copy pixel data
    const int size_in_bytes = 4 * _image_w * _image_h;
    _data = new uint8_t[size_in_bytes];
    memcpy(_data, image_data, size_in_bytes);
    stbi_image_free(image_data);

    // create a OpenGL texture identifier
    glGenTextures(1, &_image_texture_id);
    glBindTexture(GL_TEXTURE_2D, _image_texture_id);

    // setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _image_w, _image_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, _data);
}

Image::~Image() {
    delete [] _data;
    glDeleteTextures(1, &_image_texture_id);
}

int Image::getImageWidth() const {
    return _image_w;
}

int Image::getImageHeight() const {
    return _image_h;
}

GLuint Image::getTextureId() const {
    return _image_texture_id;
}

bool Image::saveToFile(const char *filepath, const std::string &file_type) const {
    if (file_type == "jpg") {
        constexpr int quality = 95;
        return stbi_write_jpg(filepath, _image_w, _image_h, 4, _data, quality) != 0;
    } else if (file_type == "png") {
        const int stride_in_bytes = _image_w * 4;
        return stbi_write_png(filepath, _image_w, _image_h, 4, _data, stride_in_bytes) != 0;
    } else {
        return false;
    }
}
