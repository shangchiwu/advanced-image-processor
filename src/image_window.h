#ifndef ADVANCED_IMAGE_PROCESSOR_IMAGE_WINDOW_H__
#define ADVANCED_IMAGE_PROCESSOR_IMAGE_WINDOW_H__

#include <memory>
#include <string>

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
        SCALE_CUSTOM_SCALE
    };

    bool is_first_seen;
    bool is_open;
    ScaleType scale_type;
    float scale_factor;

    ImageWindow(std::shared_ptr<Image> image=nullptr, const std::string &title="");
    ImageWindow(const ImageWindow &other) = delete;

    const std::shared_ptr<Image> getImage() const;
    std::shared_ptr<Image> getImage();
    void setImage(std::shared_ptr<Image> image);
    const std::string &getTitle() const;
    void setTitle(const std::string &title);
    std::string getDisplayedTitle() const;
    const std::string &getRenderedTitle() const;

    ImVec2 computeImageRenderSize(const ImVec2 &window_size=ImVec2()) const;
    ImVec2 computeDefaultPosition() const;

private:
    static int _prev_id;

    const int _id;
    std::shared_ptr<Image> _image;
    std::string _title;
    std::string _ui_title;
};

#endif // ADVANCED_IMAGE_PROCESSOR_IMAGE_WINDOW_H__
