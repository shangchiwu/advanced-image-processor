#include "image.h"

#include <stdint.h>
#include <iostream>
#include <GL/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Image::Image() : _image_w(0), _image_h(0), _image_texture_id(0) {}

Image::Image(const char *filepath) {
    // load from file
    uint8_t *image_data = stbi_load(filepath, &_image_w, &_image_h, nullptr, 4);
    if (image_data == nullptr)
        return;

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _image_w, _image_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);
}

Image::~Image() {
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