#include <cmath>
#include <chrono>
#include <random>
#include <thread>
#include "utility.h"

static std::mt19937 rng;

int nearest_power_of_2(int num) {
    return round(exp2(round(log2(num))));
}

ImVec2 compute_max_target_size(const ImVec2 &target_size, const ImVec2 &box_size) {
    if (target_size.x <= box_size.x && target_size.y <= box_size.y)
        return target_size;

    const float target_aspect_ratio = target_size.x / target_size.y;
    const float box_aspect_ratio = box_size.x / box_size.y;

    if (target_aspect_ratio > box_aspect_ratio) // target is wider
        return ImVec2(box_size.x, box_size.x / target_aspect_ratio);
    else  // target is taller
        return ImVec2(box_size.y * target_aspect_ratio, box_size.y);
}

void random_seed(int seed) {
    rng.seed(seed);
}

double random_float() {
    return random_float(0.0, 1.0);
}

double random_float(double min, double max) {
    return (std::uniform_real_distribution<double>(min, max))(rng);
}

void sleep(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
