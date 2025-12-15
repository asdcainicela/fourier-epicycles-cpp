#pragma once

#include <opencv2/core.hpp>
#include <random>

namespace fourier {

/**
 * @brief Color enumeration for consistent color palette
 */
enum class ColorEnum {
    RED,
    ORANGE,
    YELLOW,
    LIME,
    GREEN,
    CYAN,
    SKY_BLUE,
    BLUE,
    PURPLE,
    MAGENTA,
    PINK,
    WHITE,
    COUNT  // Total number of colors (must be last)
};

/**
 * @brief Get cv::Scalar color from enum (BGR format)
 * @param color ColorEnum value
 * @return cv::Scalar in BGR format
 */
cv::Scalar getColor(ColorEnum color);

/**
 * @brief Get a random color from the enum palette
 * @return cv::Scalar in BGR format
 */
cv::Scalar getRandomColor();

/**
 * @brief Get a random color with a specific seed for reproducibility
 * @param seed Random seed
 * @return cv::Scalar in BGR format
 */
cv::Scalar getRandomColor(unsigned int seed);

/**
 * @brief Generate a vector of random colors
 * @param numColors Number of colors to generate
 * @return Vector of cv::Scalar colors in BGR format
 */
std::vector<cv::Scalar> generateRandomColors(int numColors);

/**
 * @brief Get total number of colors in palette
 * @return Number of colors
 */
constexpr int getColorCount() {
    return static_cast<int>(ColorEnum::COUNT);
}

} // namespace fourier
