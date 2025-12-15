#include "colors.hpp"
#include <random>
#include <chrono>

namespace fourier {

// Thread-local random engine for thread safety
static thread_local std::mt19937 rng(
    static_cast<unsigned int>(std::chrono::steady_clock::now().time_since_epoch().count())
);

cv::Scalar getColor(ColorEnum color) {
    // Colors in BGR format for OpenCV
    switch (color) {
        case ColorEnum::RED:       return cv::Scalar(0, 0, 255);
        case ColorEnum::ORANGE:    return cv::Scalar(0, 128, 255);
        case ColorEnum::YELLOW:    return cv::Scalar(0, 255, 255);
        case ColorEnum::LIME:      return cv::Scalar(0, 255, 128);
        case ColorEnum::GREEN:     return cv::Scalar(0, 255, 0);
        case ColorEnum::CYAN:      return cv::Scalar(255, 255, 0);
        case ColorEnum::SKY_BLUE:  return cv::Scalar(255, 191, 0);
        case ColorEnum::BLUE:      return cv::Scalar(255, 0, 0);
        case ColorEnum::PURPLE:    return cv::Scalar(255, 0, 128);
        case ColorEnum::MAGENTA:   return cv::Scalar(255, 0, 255);
        case ColorEnum::PINK:      return cv::Scalar(203, 192, 255);
        case ColorEnum::WHITE:     return cv::Scalar(255, 255, 255);
        default:                   return cv::Scalar(255, 255, 255);
    }
}

cv::Scalar getRandomColor() {
    std::uniform_int_distribution<int> dist(0, static_cast<int>(ColorEnum::COUNT) - 1);
    return getColor(static_cast<ColorEnum>(dist(rng)));
}

cv::Scalar getRandomColor(unsigned int seed) {
    std::mt19937 seededRng(seed);
    std::uniform_int_distribution<int> dist(0, static_cast<int>(ColorEnum::COUNT) - 1);
    return getColor(static_cast<ColorEnum>(dist(seededRng)));
}

std::vector<cv::Scalar> generateRandomColors(int numColors) {
    std::vector<cv::Scalar> colors;
    colors.reserve(numColors);
    
    for (int i = 0; i < numColors; ++i) {
        colors.push_back(getRandomColor());
    }
    
    return colors;
}

} // namespace fourier
