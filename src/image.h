#ifndef ADVANCED_IMAGE_PROCESSOR_IMAGE_H__
#define ADVANCED_IMAGE_PROCESSOR_IMAGE_H__

#include <string>
#include <GL/gl.h>

class Image {
public:

    enum Color { R = 0, G, B, A };

    Image();
    Image(const Image &other);
    Image(const std::string &filepath);
    ~Image();

    int getImageWidth() const;
    int getImageHeight() const;
    GLuint getTextureId() const;

    bool loadFromFile(const std::string &filepath);
    bool saveToFile(const std::string &filepath, const std::string &file_type) const;
    void close();

    void loadToTexture() const;

    const uint8_t *pixel(int x, int y) const;
    uint8_t *pixel(int x, int y);

private:
    int _image_w;
    int _image_h;
    uint8_t *_data;
    GLuint _texture_id;
};

#endif // ADVANCED_IMAGE_PROCESSOR_IMAGE_H__
