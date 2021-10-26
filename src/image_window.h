#ifndef ADVANCED_IMAGE_PROCESSOR_IMAGE_WINDOW_H__
#define ADVANCED_IMAGE_PROCESSOR_IMAGE_WINDOW_H__

#include <memory>

#include "imgui.h"

#include "image.h"

class ImageWindow {
public:

    enum ScaleType {
        SCALE_ORIGINAL,
        SCALE_FILL,
        SCALE_FIT_WIDTH,
        SCALE_FIT_HEIGHT,
        SCALE_FIT_WINDOW,
        SCALE_CUSTOM_SCALE // TODO
    };

    bool is_expanded;
    bool is_open;
    ScaleType scale_type;

    ImageWindow(std::shared_ptr<Image> image = nullptr);
    ImageWindow(const Image &other) = delete;

    const std::shared_ptr<Image> getImage() const;
    std::shared_ptr<Image> getImage();
    void setImage(std::shared_ptr<Image> image);

    ImVec2 computeImageRenderSize(const ImVec2 &window_size=ImVec2()) const;

private:
    std::shared_ptr<Image> _image;
};

#endif // ADVANCED_IMAGE_PROCESSOR_IMAGE_WINDOW_H__
