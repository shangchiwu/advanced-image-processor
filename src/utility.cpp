#include <cmath>
#include <chrono>
#include <random>
#include <thread>
#include "utility.h"

static std::mt19937 rng;

int nearest_power_of_2(int num) {
    return round(exp2(round(log2(num))));
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
