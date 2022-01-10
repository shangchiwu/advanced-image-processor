#ifndef ADVANCED_IMAGE_PROCESSOR_HANDLERS_H__
#define ADVANCED_IMAGE_PROCESSOR_HANDLERS_H__

#include <memory>
#include <string>

#include "algorithms.h"
#include "image.h"
#include "image_window.h"

/*
 * Helpers
 */

void display_image_helper(const std::shared_ptr<Image> image, const std::string &title="");

/*
 * File Menu
 */

std::string get_open_image_path();
std::string get_save_image_path();
void handle_open_image_from_file();
void handle_save_iamge(const std::shared_ptr<Image> image);
void handle_copy_image_title(const std::shared_ptr<ImageWindow> image_window);
void handle_copy_image_to_clipboard(const std::shared_ptr<Image> image);
void handle_open_image_from_clipboard();

/*
 * Operations Menu
 */

void handle_gray_histogram(const std::shared_ptr<Image> image);
void handle_gaussian_noise(const std::shared_ptr<Image> image, int sigma);
void handle_resize_image(const std::shared_ptr<Image> image, int width, int height);
void handle_haar_wavelet_transform(const std::shared_ptr<Image> image, int level, float scale=1.f);
void handle_histogram_equalization(const std::shared_ptr<Image> image);
void handle_convolution(const std::shared_ptr<Image> image, int kernel_size, const std::shared_ptr<float[]> kernel,
        ConvolutionEdgeHandlingMethod edge_handling_method);

#endif // ADVANCED_IMAGE_PROCESSOR_HANDLERS_H__
