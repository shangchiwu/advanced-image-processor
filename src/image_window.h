#ifndef ADVANCED_IMAGE_PROCESSOR_IMAGE_WINDOW_H__
#define ADVANCED_IMAGE_PROCESSOR_IMAGE_WINDOW_H__

#include <memory>

#include "imgui.h"

#include "image.h"

class ImageWindow {
public:
    float zoom_factor;
    bool is_expanded;
    bool is_open;

    ImageWindow(std::shared_ptr<Image> image = nullptr);
    ImageWindow(const Image &other) = delete;

    const std::shared_ptr<Image> getImage() const;
    std::shared_ptr<Image> getImage();
    void setImage(std::shared_ptr<Image> image);

    ImVec2 getImageRenderSize() const;

private:
    std::shared_ptr<Image> _image;
};

#endif // ADVANCED_IMAGE_PROCESSOR_IMAGE_WINDOW_H__
