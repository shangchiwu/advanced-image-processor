#include <cmath>
#include <memory>

#include "handlers.h"
#include "image.h"
#include "view.h"

int main(int argc, const char **argv) {

    // init gui
    bool result = view_init();
    if (!result) {
        return 1;
    }

    // make default images

    std::shared_ptr<Image> default_image = std::make_shared<Image>("image.png");
    if (default_image->good())
        display_image_helper(default_image, "default image.png");

    std::shared_ptr<Image> formula_image = std::make_shared<Image>(400, 500);
    for (int y = 0; y < formula_image->getImageHeight(); ++y) {
        for (int x = 0; x < formula_image->getImageWidth(); ++x) {
            // fill some color
            formula_image->pixel(x, y)[Image::R] = x * 2 % 40 + y * 57 % 203 - y * int(exp(x) * 4) % 23;
            formula_image->pixel(x, y)[Image::G] = x * 54 % 87 + int(log(y) * 13) % 147 - y * x * 2 % 76;
            formula_image->pixel(x, y)[Image::B] = int(atan(x) * 73) % 143 + y * 46 % 86 - y * x * 54 % 31;
        }
    }
    formula_image->loadToTexture();
    display_image_helper(formula_image, "default math formula image");

    // start window render loop
    view_loop();

    // end
    view_cleanup();

    return 0;
}
