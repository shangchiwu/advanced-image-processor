#include "handlers.h"

#include <iostream>
#include <memory>

#include <clip.h>
#include <nfd.hpp>
#include <stb_image.h>

#include "algorithms.h"
#include "image.h"
#include "image_window.h"
#include "models.h"
#include "utility.h"

void display_image_helper(const std::shared_ptr<Image> image, const std::string &title) {
    image_windows.emplace_back(std::make_shared<ImageWindow>(image, title));
}

std::string get_open_image_path() {
    if (NFD::Init() != NFD_OKAY)
        return std::string();

    constexpr nfdfiltersize_t filter_list_size = 6;
    const nfdfilteritem_t filter_list[filter_list_size] = {
        {"All supported image", "bmp,jpg,jpeg,gif,png,ppm,pgm"},
        {"BMP image", "bmp"},
        {"JPG image", "jpg,jpeg"},
        {"GIF image", "gif"},
        {"PNG image", "png"},
        {"PPM image", "ppm,pgm"}};

    nfdchar_t *nfd_filepath = nullptr;
    nfdresult_t result = NFD::OpenDialog(nfd_filepath, filter_list, filter_list_size, nullptr);

    if (result == NFD_ERROR)
        std::cout << "NFD ERROR: " << NFD::GetError() << std::endl;

    if (result == NFD_CANCEL)
        return std::string();

    std::string filename(nfd_filepath);
    NFD::FreePath(nfd_filepath);

    NFD::Quit();
    return filename;
}

std::string get_save_image_path() {
    if (NFD::Init() != NFD_OKAY)
        return std::string();

    constexpr nfdfiltersize_t filter_list_size = 3;
    const nfdfilteritem_t filter_list[filter_list_size] = {
        {"All supported image", "jpg,png"},
        {"JPG image", "jpg"},
        {"PNG image", "png"}};

    nfdchar_t *nfd_filepath = nullptr;
    nfdresult_t result = NFD::SaveDialog(nfd_filepath, filter_list, filter_list_size);

    if (result == NFD_ERROR)
        std::cout << "NFD ERROR: " << NFD::GetError() << std::endl;

    if (result == NFD_CANCEL)
        return std::string();

    std::string filename(nfd_filepath);
    NFD::FreePath(nfd_filepath);

    NFD::Quit();
    return filename;
}

void handle_open_image_from_file() {
    std::string filepath = get_open_image_path();
    if (filepath.empty()) {
        std::cout << "Open image canceled." << std::endl;
        return;
    }

    std::cout << "Open image: \"" << filepath << "\"" << std::endl;
    std::shared_ptr<Image> image = std::make_shared<Image>(filepath);
    if (!image->good()) {
        std::cout << "Error: Open image \"" << filepath << "\" failed!" << std::endl;
        return;
    }
    display_image_helper(image, filepath);
}

void handle_save_iamge(const std::shared_ptr<Image> image) {
    std::string filepath = get_save_image_path();
    if (filepath.empty())
        return;
    std::cout << "Save image: \"" << filepath << "\"" << std::endl;
    const bool result = image->saveToFile(filepath);
    if (!result) {
        std::cout << "Error: Save image \"" << filepath << "\" failed!" << std::endl;
        return;
    }
}

void handle_copy_image_title(const std::shared_ptr<ImageWindow> image_window) {
    clip::set_text(image_window->getDisplayedTitle());
}

void handle_copy_image_to_clipboard(const std::shared_ptr<Image> image) {
    const uint8_t *data = image->data();
    clip::image_spec spec;
    spec.width = image->getImageWidth();
    spec.height = image->getImageHeight();
    spec.bits_per_pixel = 32;
    spec.bytes_per_row = spec.width * 4;
    spec.red_mask    = 0x000000ff;
    spec.green_mask  = 0x0000ff00;
    spec.blue_mask   = 0x00ff0000;
    spec.alpha_mask  = 0xff000000;
    spec.red_shift   = 0;
    spec.green_shift = 8;
    spec.blue_shift  = 16;
    spec.alpha_shift = 24;
    const clip::image img(data, spec);
    clip::set_image(img);
}

void handle_open_image_from_clipboard() {
    if (!clip::has(clip::image_format())) {
        std::cout << "Error: Clipboard doesn't contain an image!" << std::endl;
        return;
    }

    clip::image img;
    if (!clip::get_image(img)) {
        std::cout << "Error: Getting image from clipboard failed!" << std::endl;
        return;
    }

    const clip::image_spec spec = img.spec();
    std::shared_ptr<Image> image = std::make_shared<Image>(spec.width, spec.height);

    // save pixel data
    for (int y = 0; y < spec.height; ++y) {
        const uint8_t *p = (uint8_t *) (img.data() + y * spec.bytes_per_row);
        for (int x = 0; x < spec.width; ++x) {
            const uint64_t pixel = (*((uint64_t *) p));
            image->pixel(x, y)[Image::R] = (pixel & spec.red_mask) >> spec.red_shift;
            image->pixel(x, y)[Image::G] = (pixel & spec.green_mask) >> spec.green_shift;
            image->pixel(x, y)[Image::B] = (pixel & spec.blue_mask) >> spec.blue_shift;
            if (spec.alpha_mask)
                image->pixel(x, y)[Image::A] = (pixel & spec.alpha_mask) >> spec.alpha_shift;
            p += spec.bits_per_pixel / 8;
        }
    }

    image->loadToTexture();
    display_image_helper(image, "clipboard");
}

void handle_gray_histogram(const std::shared_ptr<Image> image) {
    std::cout << "compute histogram" << std::endl;
    std::shared_ptr<Image> gray_image = std::make_shared<Image>();
    float histogram[256] = {};
    generate_gray_image_and_histogram(image, gray_image, histogram);
    display_image_helper(gray_image, "gray image");

    std::cout << "generate histogram image" << std::endl;
    const std::shared_ptr<Image> histogram_image = generate_histogram_image(histogram);
    display_image_helper(histogram_image, "gray histogram");
}

void handle_gaussian_noise(const std::shared_ptr<Image> image, int sigma) {
    const float sigma_normalized = sigma / 255.f;

    // generate noise
    const int num_pixels = image->getImageWidth() * image->getImageHeight();
    float *noise = new float[num_pixels];
    generate_gaussian_noise(noise, num_pixels, sigma_normalized);

    // generate image with noise added
    std::shared_ptr<Image> image_with_noise = std::make_shared<Image>(image->getImageWidth(), image->getImageHeight());
    for (int y = 0; y < image->getImageHeight(); ++y) {
        for (int x = 0; x < image->getImageWidth(); ++x) {
            const int i = y * image->getImageWidth() + x;
            image_with_noise->pixel(x, y)[Image::R] = clamp(image->pixel(x, y)[Image::R] + 255 * noise[i], 0.f, 255.f);
            image_with_noise->pixel(x, y)[Image::G] = clamp(image->pixel(x, y)[Image::G] + 255 * noise[i], 0.f, 255.f);
            image_with_noise->pixel(x, y)[Image::B] = clamp(image->pixel(x, y)[Image::B] + 255 * noise[i], 0.f, 255.f);
            image_with_noise->pixel(x, y)[Image::A] = image->pixel(x, y)[Image::A];
        }
    }
    image_with_noise->loadToTexture();
    display_image_helper(image_with_noise, "image with noise");

    // draw histogram of noise
    for (int i = 0; i < num_pixels; ++i)
        noise[i] += 0.5f;
    float histogram[256] = {};
    generate_histogram_from_array(noise, num_pixels, histogram);
    std::shared_ptr<Image> noise_histogram_image = generate_histogram_image(histogram);
    display_image_helper(noise_histogram_image, "noise histogram");

    // clean
    delete [] noise;
}

void handle_resize_image(const std::shared_ptr<Image> image, int width, int height) {
    std::shared_ptr<Image> resized_image = std::make_shared<Image>(*image);
    resized_image->resize(width, height);
    display_image_helper(resized_image, "resized image");
}

void handle_haar_wavelet_transform(const std::shared_ptr<Image> image, int level, float scale) {
    // check
    if (level < 0) {
        std::cout << "level can't be negative!" << std::endl;
        return;
    }

    // to grey
    std::shared_ptr<Image> in_image = std::make_shared<Image>(*image);
    generate_gray_image_and_histogram(image, in_image, nullptr);

    // resize image
    in_image->resize(
        nearest_power_of_2(in_image->getImageWidth()),
        nearest_power_of_2(in_image->getImageHeight()));

    std::shared_ptr<Image> out_image = haar_wavelet_transform(in_image, level, scale);
    out_image->loadToTexture();

    display_image_helper(out_image, "haar wavelet result");
}

void handle_histogram_equalization(const std::shared_ptr<Image> image) {
    // input
    std::shared_ptr<Image> input_image = std::make_shared<Image>();
    float histogram[256] = {};
    generate_gray_image_and_histogram(image, input_image, histogram);
    const std::shared_ptr<Image> input_image_histogram = generate_histogram_image(histogram);
    display_image_helper(input_image, "histogram equalization - input");
    display_image_helper(input_image_histogram, "histogram equalization - input histogram");

    // process
    std::shared_ptr<Image> output_image = histogram_equalization(input_image);

    // output
    generate_gray_image_and_histogram(output_image, nullptr, histogram);
    const std::shared_ptr<Image> output_image_histogram = generate_histogram_image(histogram);
    display_image_helper(output_image, "histogram equalization - output");
    display_image_helper(output_image_histogram, "histogram equalization - output histogram");
}

void handle_convolution(const std::shared_ptr<Image> image, int kernel_size, const std::shared_ptr<float[]> kernel,
        ConvolutionEdgeHandlingMethod edge_handling_method) {
    std::shared_ptr<Image> result = image_convolution(image, kernel_size, kernel.get(), edge_handling_method);
    display_image_helper(result, "convolution result");
}
