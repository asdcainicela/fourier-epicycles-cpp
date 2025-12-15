#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <complex>
#include <vector>
#include <string>

namespace fourier {

/**
 * @brief Configuration for contour extraction
 */
struct ContourConfig {
    int cannyThreshold1 = 50;       // Canny edge detection threshold 1
    int cannyThreshold2 = 150;      // Canny edge detection threshold 2
    int blurSize = 5;               // Gaussian blur kernel size
    int numSamplePoints = 500;      // Number of points to sample from contour
    bool useAdaptiveThreshold = true;
    int adaptiveBlockSize = 11;
    double adaptiveC = 2.0;
};

/**
 * @brief Contour extraction result
 */
struct ContourResult {
    std::vector<std::complex<double>> complexPoints;  // Contour as complex numbers
    std::vector<cv::Point> originalContour;           // Original OpenCV contour
    cv::Point2d centroid;                             // Center of contour
    double scale;                                     // Scale factor applied
    bool success;
    std::string errorMessage;
};

/**
 * @brief Load image and extract the largest contour
 * @param imagePath Path to input image
 * @param config Contour extraction configuration
 * @return ContourResult with complex points
 */
ContourResult extractContour(const std::string& imagePath, const ContourConfig& config = ContourConfig());

/**
 * @brief Extract contour from already loaded image
 * @param image Input image (grayscale or BGR)
 * @param config Contour extraction configuration
 * @return ContourResult with complex points
 */
ContourResult extractContour(const cv::Mat& image, const ContourConfig& config = ContourConfig());

/**
 * @brief Sample points uniformly along a contour
 * @param contour Input contour points
 * @param numPoints Number of points to sample
 * @return Sampled contour points
 */
std::vector<cv::Point> sampleContour(const std::vector<cv::Point>& contour, int numPoints);

/**
 * @brief Convert contour points to complex numbers, centered at origin
 * @param contour Input contour points
 * @param centroid Output centroid of the contour
 * @param scale Output scale factor applied
 * @return Vector of complex numbers
 */
std::vector<std::complex<double>> contourToComplex(
    const std::vector<cv::Point>& contour,
    cv::Point2d& centroid,
    double& scale
);

/**
 * @brief Find all contours in an image
 * @param image Input image
 * @param config Contour extraction configuration
 * @return Vector of all contours found
 */
std::vector<std::vector<cv::Point>> findAllContours(
    const cv::Mat& image,
    const ContourConfig& config = ContourConfig()
);

} // namespace fourier
