#ifndef ADVANCED_IMAGE_PROCESSOR_ALGORITHMS_H__
#define ADVANCED_IMAGE_PROCESSOR_ALGORITHMS_H__

#include <cstdint>
#include <memory>

#include "image.h"

enum class ConvolutionEdgeHandlingMethod {
    EXTEND = 0,
    WRAP,
    MIRROR
};

uint8_t to_gray_average(const uint8_t *pixel);
void generate_gray_image_and_histogram(const std::shared_ptr<Image> image, std::shared_ptr<Image> out_image, float *histogram);
std::shared_ptr<Image> generate_histogram_image(const float *histogram);
void generate_gaussian_noise(float *out_noise, int count, float sigma);
void generate_histogram_from_array(const float *noise, int count, float *histogram);
std::shared_ptr<Image> haar_wavelet_transform(const std::shared_ptr<Image> image, int level, float scale=1.f);
std::shared_ptr<Image> histogram_equalization(const std::shared_ptr<Image> image);
std::shared_ptr<Image> image_convolution(const std::shared_ptr<Image> image, int kernel_size, const float *kernel,
        ConvolutionEdgeHandlingMethod edge_handling_method=ConvolutionEdgeHandlingMethod::EXTEND);

#endif // ADVANCED_IMAGE_PROCESSOR_ALGORITHMS_H__
