#include "image_window.h"

#include <memory>

#include <imgui.h>

#include "image.h"

ImageWindow::ImageWindow(std::shared_ptr<Image> image) :
        is_expanded(true), is_open(true), _image(image) {}

const std::shared_ptr<Image> ImageWindow::getImage() const {
    return _image;
}

std::shared_ptr<Image> ImageWindow::getImage() {
    return _image;
}

void ImageWindow::setImage(std::shared_ptr<Image> image) {
    _image = image;
}

ImVec2 ImageWindow::computeImageRenderSize(const ImVec2 &window_size) const {
    const float image_aspect_ratio = (float) _image->getImageWidth() / _image->getImageHeight();
    const float window_aspect_ratio = (float) window_size.x / window_size.y;

    if (image_aspect_ratio > window_aspect_ratio) // image is wider
        return ImVec2(window_size.x, window_size.x / image_aspect_ratio);
    else // image is taller
        return ImVec2(window_size.y * image_aspect_ratio, window_size.y);
}
