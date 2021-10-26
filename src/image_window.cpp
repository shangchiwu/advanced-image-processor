#include "image_window.h"

#include <memory>

#include <imgui.h>

#include "image.h"

ImageWindow::ImageWindow(std::shared_ptr<Image> image) : _image(image), zoom_factor(1.f),
        is_expanded(true), is_open(true) {}

const std::shared_ptr<Image> ImageWindow::getImage() const {
    return _image;
}

std::shared_ptr<Image> ImageWindow::getImage() {
    return _image;
}

void ImageWindow::setImage(std::shared_ptr<Image> image) {
    _image = image;
}

ImVec2 ImageWindow::getImageRenderSize() const {
    return ImVec2(zoom_factor * _image->getImageWidth(), zoom_factor * _image->getImageHeight());
}
