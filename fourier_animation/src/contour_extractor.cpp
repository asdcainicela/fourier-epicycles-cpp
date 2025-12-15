#include "contour_extractor.hpp"
#include <algorithm>
#include <cmath>

namespace fourier {

ContourResult extractContour(const std::string& imagePath, const ContourConfig& config) {
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);
    
    if (image.empty()) {
        ContourResult result;
        result.success = false;
        result.errorMessage = "Failed to load image: " + imagePath;
        return result;
    }
    
    return extractContour(image, config);
}

ContourResult extractContour(const cv::Mat& image, const ContourConfig& config) {
    ContourResult result;
    result.success = false;
    
    cv::Mat gray;
    if (image.channels() == 3 || image.channels() == 4) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // Apply Gaussian blur
    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(config.blurSize, config.blurSize), 0);
    
    // Edge detection
    cv::Mat edges;
    if (config.useAdaptiveThreshold) {
        cv::adaptiveThreshold(blurred, edges, 255, 
            cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV,
            config.adaptiveBlockSize, config.adaptiveC);
    } else {
        cv::Canny(blurred, edges, config.cannyThreshold1, config.cannyThreshold2);
    }
    
    // Find contours
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    
    if (contours.empty()) {
        result.errorMessage = "No contours found in image";
        return result;
    }
    
    // Find largest contour by arc length
    auto largestIt = std::max_element(contours.begin(), contours.end(),
        [](const std::vector<cv::Point>& a, const std::vector<cv::Point>& b) {
            return cv::arcLength(a, true) < cv::arcLength(b, true);
        }
    );
    
    result.originalContour = *largestIt;
    
    // Sample points uniformly
    auto sampledContour = sampleContour(result.originalContour, config.numSamplePoints);
    
    // Convert to complex numbers
    result.complexPoints = contourToComplex(sampledContour, result.centroid, result.scale);
    result.success = true;
    
    return result;
}

std::vector<cv::Point> sampleContour(const std::vector<cv::Point>& contour, int numPoints) {
    if (contour.size() <= static_cast<size_t>(numPoints)) {
        return contour;
    }
    
    std::vector<cv::Point> sampled;
    sampled.reserve(numPoints);
    
    // Calculate cumulative arc length
    std::vector<double> arcLengths;
    arcLengths.reserve(contour.size());
    arcLengths.push_back(0.0);
    
    for (size_t i = 1; i < contour.size(); ++i) {
        double dx = contour[i].x - contour[i-1].x;
        double dy = contour[i].y - contour[i-1].y;
        arcLengths.push_back(arcLengths.back() + std::sqrt(dx*dx + dy*dy));
    }
    
    double totalLength = arcLengths.back();
    double step = totalLength / numPoints;
    
    // Sample at uniform arc length intervals
    size_t contourIdx = 0;
    for (int i = 0; i < numPoints; ++i) {
        double targetLength = i * step;
        
        while (contourIdx < arcLengths.size() - 1 && arcLengths[contourIdx + 1] < targetLength) {
            ++contourIdx;
        }
        
        sampled.push_back(contour[contourIdx]);
    }
    
    return sampled;
}

std::vector<std::complex<double>> contourToComplex(
    const std::vector<cv::Point>& contour,
    cv::Point2d& centroid,
    double& scale
) {
    if (contour.empty()) {
        return {};
    }
    
    // Calculate centroid
    double sumX = 0, sumY = 0;
    for (const auto& pt : contour) {
        sumX += pt.x;
        sumY += pt.y;
    }
    centroid.x = sumX / contour.size();
    centroid.y = sumY / contour.size();
    
    // Find max distance from centroid for scaling
    double maxDist = 0;
    for (const auto& pt : contour) {
        double dx = pt.x - centroid.x;
        double dy = pt.y - centroid.y;
        double dist = std::sqrt(dx*dx + dy*dy);
        maxDist = std::max(maxDist, dist);
    }
    
    // Scale to normalize (max radius = 1)
    scale = (maxDist > 0) ? 1.0 / maxDist : 1.0;
    
    // Convert to complex numbers centered at origin
    std::vector<std::complex<double>> complexPoints;
    complexPoints.reserve(contour.size());
    
    for (const auto& pt : contour) {
        double x = (pt.x - centroid.x) * scale;
        double y = (pt.y - centroid.y) * scale;
        complexPoints.emplace_back(x, y);
    }
    
    return complexPoints;
}

std::vector<std::vector<cv::Point>> findAllContours(
    const cv::Mat& image,
    const ContourConfig& config
) {
    cv::Mat gray;
    if (image.channels() == 3 || image.channels() == 4) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(config.blurSize, config.blurSize), 0);
    
    cv::Mat edges;
    cv::Canny(blurred, edges, config.cannyThreshold1, config.cannyThreshold2);
    
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edges, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
    
    return contours;
}

} // namespace fourier
