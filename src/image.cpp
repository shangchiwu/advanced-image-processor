#include "image.h"

#include <stdint.h>
#include <stdlib.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <GL/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

Image::Image() : _image_w(0), _image_h(0), _data(nullptr), _texture_id(0) {}

Image::Image(const Image &other): Image(other._image_w, other._image_h, other._data) {}

Image::Image(int width, int height, const uint8_t *data) : Image() {
    init(width, height, data);
}

Image::Image(const std::string &filepath) : Image() {
    loadFromFile(filepath);
}

Image::~Image() {
    close();
}

void Image::init(int width, int height, const uint8_t *data) {
    // close previous image
    close();

    _image_w = width;
    _image_h = height;

    // create pixel data array
    const int size_in_bytes = 4 * _image_w * _image_h * sizeof(uint8_t);
    _data = new uint8_t[size_in_bytes];

    if (data == nullptr) {
        // set default color to white
        memset(_data, 255, size_in_bytes * sizeof(uint8_t));
    } else {
        // copy pixel data
        memcpy(_data, data, size_in_bytes);
    }

    // create a OpenGL texture
    glGenTextures(1, &_texture_id);
    loadToTexture();
}

void Image::close() {
    if (_data != nullptr) {
        delete [] _data;
        glDeleteTextures(1, &_texture_id);
        _image_w = 0;
        _image_h = 0;
        _data = nullptr;
        _texture_id = 0;
    }
}

bool Image::good() const {
    return _data != nullptr;
}

bool Image::loadFromFile(const std::string &filepath) {
    // load from file
    int image_w = 0;
    int image_h = 0;
    uint8_t *image_data = stbi_load(filepath.c_str(), &image_w, &image_h, nullptr, 4);
    if (image_data == nullptr)
        return false;

    // init image
    close();
    init(image_w, image_h, image_data);
    stbi_image_free(image_data);

    return true;
}

bool Image::saveToFile(const std::string &filepath) const {
    const std::string file_extension = std::filesystem::path(filepath).extension().string();

    if (file_extension == ".jpg") {
        constexpr int quality = 95;
        return stbi_write_jpg(filepath.c_str(), _image_w, _image_h, 4, _data, quality) != 0;
    } else if (file_extension == ".png") {
        const int stride_in_bytes = _image_w * 4;
        return stbi_write_png(filepath.c_str(), _image_w, _image_h, 4, _data, stride_in_bytes) != 0;
    } else {
        return false;
    }
}

int Image::getImageWidth() const {
    return _image_w;
}

int Image::getImageHeight() const {
    return _image_h;
}

const uint8_t *Image::pixel(int x, int y) const {
    return &_data[(y * _image_w + x) * 4 * sizeof(uint8_t)];
}

uint8_t *Image::pixel(int x, int y) {
    return &_data[(y * _image_w + x) * 4 * sizeof(uint8_t)];
}

void Image::loadToTexture() const {
    // use texture
    glBindTexture(GL_TEXTURE_2D, _texture_id);

    // setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _image_w, _image_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, _data);
}

GLuint Image::getTextureId() const {
    return _texture_id;
}
