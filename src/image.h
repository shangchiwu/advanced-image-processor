#ifndef ADVANCED_IMAGE_PROCESSOR_IMAGE_H__
#define ADVANCED_IMAGE_PROCESSOR_IMAGE_H__

#include <GL/gl.h>

class Image {
public:
    Image();
    Image(const Image &other);
    Image(const char *filepath);
    ~Image();

    int getImageWidth() const;
    int getImageHeight() const;
    GLuint getTextureId() const;

private:
    int _image_w;
    int _image_h;
    GLuint _image_texture_id;
};

#endif // ADVANCED_IMAGE_PROCESSOR_IMAGE_H__
