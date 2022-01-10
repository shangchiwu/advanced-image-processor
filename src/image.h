#ifndef ADVANCED_IMAGE_PROCESSOR_IMAGE_H__
#define ADVANCED_IMAGE_PROCESSOR_IMAGE_H__

#include <cstdint>
#include <string>
#include <GL/gl.h>

class Image {
public:

    enum Color { R = 0, G, B, A };

    Image();
    Image(const Image &other);
    Image(int width, int height, const uint8_t *data=nullptr);
    Image(const std::string &filepath);
    ~Image();

    void init(int width, int height, const uint8_t *data=nullptr);
    void close();
    bool good() const;

    bool loadFromFile(const std::string &filepath);
    bool saveToFile(const std::string &filepath) const;

    int getImageWidth() const;
    int getImageHeight() const;
    const uint8_t *data() const;
    const uint8_t *pixel(int x, int y) const;
    uint8_t *pixel(int x, int y);

    void fill(uint8_t level, uint8_t a=255);
    void fill(uint8_t r, uint8_t g, uint8_t b, uint8_t a=255);
    bool resize(int width, int height);

    void loadToTexture() const;
    GLuint getTextureId() const;

private:
    int _image_w;
    int _image_h;
    uint8_t *_data;
    GLuint _texture_id;
};

#endif // ADVANCED_IMAGE_PROCESSOR_IMAGE_H__
