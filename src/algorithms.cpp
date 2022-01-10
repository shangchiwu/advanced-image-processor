#include "algorithms.h"

#include <cmath>
#include <cstdint>
#include <memory>
#include <numbers>

#include "image.h"
#include "utility.h"

uint8_t to_gray_average(const uint8_t *pixel) {
    int a = round((((int) pixel[Image::R]) + ((int) pixel[Image::G]) + ((int) pixel[Image::B])) / 3.0f);
    // std::cout << a << std::endl;
    return a;
}

void generate_gray_image_and_histogram(const std::shared_ptr<Image> image, std::shared_ptr<Image> out_image, float *histogram) {
    if (out_image != nullptr)
        out_image->init(image->getImageWidth(), image->getImageHeight());

    int level_count[256] = {};

    // transform to gray scale
    for (int y = 0; y < image->getImageHeight(); ++y) {
        for (int x = 0; x < image->getImageWidth(); ++x) {
            const uint8_t gray_level = to_gray_average(image->pixel(x, y));
            ++level_count[gray_level];
            if (out_image != nullptr) {
                out_image->pixel(x, y)[Image::R] = gray_level;
                out_image->pixel(x, y)[Image::G] = gray_level;
                out_image->pixel(x, y)[Image::B] = gray_level;
            }
        }
    }
    if (out_image != nullptr) {
        out_image->loadToTexture();
    }

    // normalize histogram
    if (histogram != nullptr) {
        const int max_num = *std::max_element(level_count, level_count + 256);

        for (int i = 0; i < 256; ++i) {
            histogram[i] = (double) level_count[i] / max_num;
        }
    }
}

std::shared_ptr<Image> generate_histogram_image(const float *histogram) {
    constexpr int image_size = 300;
    std::shared_ptr<Image> image = std::make_shared<Image>(image_size, image_size);

    // draw the histogram
    constexpr int histogram_start = (image_size - 256) / 2;
    constexpr int histogram_end = histogram_start + 256;
    for (int level = 0; level < 256; ++level) {
        const int h = histogram_start + 256 * (1.0f - histogram[level]);
        // draw background
        for (int y = histogram_start; y < h; ++y) {
            image->pixel(histogram_start + level, y)[Image::R] = 127;
            image->pixel(histogram_start + level, y)[Image::G] = 127;
            image->pixel(histogram_start + level, y)[Image::B] = 127;
        }
        // draw bar
        for (int y = h; y < histogram_end; ++y) {
            image->pixel(histogram_start + level, y)[Image::R] = 0;
            image->pixel(histogram_start + level, y)[Image::G] = 0;
            image->pixel(histogram_start + level, y)[Image::B] = 0;
        }
    }
    image->loadToTexture();
    return image;
}

void generate_gaussian_noise(float *out_noise, int count, float sigma) {
    using std::numbers::pi;

    for (int i = 0; i < count; i += 2) {
        // generate uniform random number pairs
        const double r = random_float();
        const double phi = random_float();

        // Box-Muller transform
        const double ln_r = (r == 0.) ? 0. : log(r);
        const double z1 = sigma * cos(2. * pi * phi) * sqrt(-2. * ln_r);
        const double z2 = sigma * sin(2. * pi * phi) * sqrt(-2. * ln_r);

        // save noise
        out_noise[i] = z1;
        if (i + 1 < count)
            out_noise[i + 1] = z2;
    }
}

void generate_histogram_from_array(const float *noise, int count, float *histogram) {
    int level_count[256] = {};

    // transform to gray scale
    for (int i = 0; i < count; ++i) {
        const uint8_t gray_level = 255 * clamp(noise[i], 0.f, 1.f);
        ++level_count[gray_level];
    }

    // normalize histogram
    const int max_num = *std::max_element(level_count, level_count + 256);
    for (int i = 0; i < 256; ++i) {
        histogram[i] = (double) level_count[i] / max_num;
    }
}

std::shared_ptr<Image> haar_wavelet_transform(const std::shared_ptr<Image> image, int level, float scale) {
    if (level < 0) return nullptr;
    if (level == 0) return std::make_shared<Image>(*image);

    int cur_w = image->getImageWidth();
    int cur_h = image->getImageHeight();

    std::shared_ptr<Image> in_image = image;
    std::shared_ptr<Image> out_image = image;

    for (int current_level = 0; current_level < level; ++current_level) {
        out_image = std::make_shared<Image>(*in_image);
        const int half_w = cur_w / 2;
        const int half_h = cur_h / 2;

        for (int y = 0; y < half_h; ++y) {
            for (int x = 0; x < half_w; ++x) {
                const uint8_t a = in_image->pixel(2 * x    , 2 * y    )[Image::R];
                const uint8_t b = in_image->pixel(2 * x + 1, 2 * y    )[Image::R];
                const uint8_t c = in_image->pixel(2 * x    , 2 * y + 1)[Image::R];
                const uint8_t d = in_image->pixel(2 * x + 1, 2 * y + 1)[Image::R];

                const uint8_t ll = clamp((int) (   ((int) a + (int) b + (int) c + (int) d) / 4                ), 0, 255);
                const uint8_t hl = clamp((int) (abs((int) a - (int) b + (int) c - (int) d) / 4 * scale        ), 0, 255);
                const uint8_t lh = clamp((int) (abs((int) a + (int) b - (int) c - (int) d) / 4 * scale        ), 0, 255);
                const uint8_t hh = clamp((int) (abs((int) a - (int) b - (int) c + (int) d) / 4 * scale * scale), 0, 255);

                out_image->pixel(         x,          y)[Image::R] = ll;  // LL (left-top)
                out_image->pixel(half_w + x,          y)[Image::R] = hl;  // HL (right-top)
                out_image->pixel(         x, half_h + y)[Image::R] = lh;  // LH (left-bottom)
                out_image->pixel(half_w + x, half_h + y)[Image::R] = hh;  // HH (right-bottom)
            }
        }

        cur_w = half_w;
        cur_h = half_h;
        in_image = out_image;
    }

    // fill other color in pixels
    for (int y = 0; y < out_image->getImageHeight(); ++y) {
        for (int x = 0; x < out_image->getImageWidth(); ++x) {
            const uint8_t color = out_image->pixel(x, y)[Image::R];
            out_image->pixel(x, y)[Image::G] = color;
            out_image->pixel(x, y)[Image::B] = color;
        }
    }

    return out_image;
}

std::shared_ptr<Image> histogram_equalization(const std::shared_ptr<Image> image) {
    // compute the histogram
    int histogram[256] = {};
    for (int y = 0; y < image->getImageHeight(); y++) {
        for (int x = 0; x < image->getImageHeight(); x++) {
            ++histogram[image->pixel(x, y)[Image::R]];
        }
    }
    int g_min = 0;
    while (g_min < 256 && histogram[g_min] == 0) {
        ++g_min;
    }

    // compute the cumulative histogram
    for (int g = 1; g < 256; ++g) {
        histogram[g] += histogram[g - 1];
    }
    const int h_min = histogram[g_min];

    // compute map
    uint8_t transform_map[256] = {};
    const double const_part = 255.0 / (image->getImageWidth() * image->getImageHeight() - h_min);
    for (int g = 0; g < 256; ++g) {
        transform_map[g] = clamp(round((histogram[g] - h_min) * const_part), 0.0, 255.0);
    }

    // map color to new image
    std::shared_ptr<Image> result = std::make_shared<Image>(image->getImageWidth(), image->getImageHeight());
    for (int y = 0; y < image->getImageHeight(); ++y) {
        for (int x = 0; x < image->getImageWidth(); ++x) {
            result->pixel(x, y)[Image::R] = transform_map[image->pixel(x, y)[Image::R]];
            result->pixel(x, y)[Image::G] = transform_map[image->pixel(x, y)[Image::G]];
            result->pixel(x, y)[Image::B] = transform_map[image->pixel(x, y)[Image::B]];
            result->pixel(x, y)[Image::A] = image->pixel(x, y)[Image::A];
        }
    }

    result->loadToTexture();

    return result;
}

std::shared_ptr<Image> image_convolution(const std::shared_ptr<Image> image, int kernel_size, const float *kernel,
        ConvolutionEdgeHandlingMethod edge_handling_method) {
    const int half_kernel_size = kernel_size / 2;

    // make padded image
    std::shared_ptr<Image> padded_image = std::make_shared<Image>(
            image->getImageWidth() + 2 * (kernel_size - 1), image->getImageHeight() + 2 * (kernel_size - 1));
    padded_image->fill(0);

    const int original_w = image->getImageWidth();
    const int original_h = image->getImageHeight();

    for (int padded_y = 0; padded_y < padded_image->getImageHeight(); ++padded_y) {
        const int original_y = padded_y - kernel_size + 1;
        for (int padded_x = 0; padded_x < padded_image->getImageWidth(); ++padded_x) {
            const int original_x = padded_x - kernel_size + 1;

            // copy from original pixel
            int map_x = original_x;
            int map_y = original_y;

            // edge handling
            if (original_x < 0 || original_x >= original_w || original_y < 0 || original_y >= original_h) {
                if (edge_handling_method == ConvolutionEdgeHandlingMethod::EXTEND) {
                    map_x = clamp(original_x, 0, original_w - 1);
                    map_y = clamp(original_y, 0, original_h - 1);
                } else if (edge_handling_method == ConvolutionEdgeHandlingMethod::WRAP) {
                    map_x = (original_x + original_w) % original_w;
                    map_y = (original_y + original_h) % original_h;
                } else if (edge_handling_method == ConvolutionEdgeHandlingMethod::MIRROR) {
                    if (original_x < 0)            map_x = -original_x - 1;
                    if (original_x >= original_w ) map_x = original_w - (original_x - original_w) - 1;
                    if (original_y < 0)            map_y = -original_y - 1;
                    if (original_y >= original_h ) map_y = original_h - (original_y - original_h) - 1;
                }
            }

            // copy the right pixel from the original image
            padded_image->pixel(padded_x, padded_y)[Image::R] = image->pixel(map_x, map_y)[Image::R];
            padded_image->pixel(padded_x, padded_y)[Image::G] = image->pixel(map_x, map_y)[Image::G];
            padded_image->pixel(padded_x, padded_y)[Image::B] = image->pixel(map_x, map_y)[Image::B];
        }
    }

    // make result image
    std::shared_ptr<Image> result = std::make_shared<Image>(image->getImageWidth(), image->getImageHeight());

    // do convolution on each pixel
    for (int y = 0; y < image->getImageHeight(); ++y) {
        const int padded_y = y + kernel_size - 1;
        for (int x = 0; x < image->getImageWidth(); ++x) {
            const int padded_x = x + kernel_size - 1;

            float sum[3] = {};
            for (int t = -half_kernel_size; t <= half_kernel_size; ++t) {
                for (int s = -half_kernel_size; s <= half_kernel_size; ++s) {
                    const int kernel_index = (half_kernel_size - t) * kernel_size + (half_kernel_size - s);
                    sum[Image::R] += kernel[kernel_index] * padded_image->pixel(padded_x + s, padded_y + t)[Image::R];
                    sum[Image::G] += kernel[kernel_index] * padded_image->pixel(padded_x + s, padded_y + t)[Image::G];
                    sum[Image::B] += kernel[kernel_index] * padded_image->pixel(padded_x + s, padded_y + t)[Image::B];
                }
            }

            result->pixel(x, y)[Image::R] = clamp(round(sum[Image::R]), 0.0, 255.0);
            result->pixel(x, y)[Image::G] = clamp(round(sum[Image::G]), 0.0, 255.0);
            result->pixel(x, y)[Image::B] = clamp(round(sum[Image::B]), 0.0, 255.0);
            result->pixel(x, y)[Image::A] = image->pixel(x, y)[Image::A];  // preserve original alpha channel
        }
    }

    result->loadToTexture();
    return result;
}
