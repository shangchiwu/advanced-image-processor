#ifndef UTILITY_H__
#define UTILITY_H__

#include <numbers>

/* Utility Functions */

constexpr double degree_to_radius(double degree) {
    return degree * (std::numbers::pi / 180.0);
}

template <typename T>
constexpr T clamp(T x, T min, T max) {
    return x < min ? min : x > max ? max : x;
}

template <typename T>
constexpr T partition(T a, T b, double t) {
    return (1.0 - t) * a + t * b;
}

int nearest_power_of_2(int num);

void random_seed(int seed);
double random_float();
double random_float(double min, double max);

void sleep(int ms);

#endif // UTILITY_H__
