#include "animation.hpp"
#include <cmath>
#include <iostream>

namespace fourier {

constexpr double PI = 3.14159265358979323846;
constexpr double TWO_PI = 2.0 * PI;

class AnimationEngine::Impl {
public:
    std::vector<FourierCoefficient> coefficients;
    AnimationConfig config;
    std::vector<cv::Point> tracedPath;
    int currentFrame = 0;
    bool initialized = false;
};

AnimationEngine::AnimationEngine() : pImpl(std::make_unique<Impl>()) {}

AnimationEngine::~AnimationEngine() = default;

void AnimationEngine::initialize(const std::vector<FourierCoefficient>& coefficients,
                                 const AnimationConfig& config) {
    pImpl->coefficients = coefficients;
    pImpl->config = config;
    pImpl->tracedPath.clear();
    pImpl->tracedPath.reserve(config.totalFrames);
    pImpl->currentFrame = 0;
    pImpl->initialized = true;
    
    std::cout << "[Animation] Initialized with " << coefficients.size() 
              << " epicycles, " << config.totalFrames << " frames" << std::endl;
}

cv::Mat AnimationEngine::renderFrame(int frameIndex) {
    if (!pImpl->initialized) {
        std::cerr << "[Animation] Not initialized!" << std::endl;
        return cv::Mat();
    }
    
    const auto& config = pImpl->config;
    pImpl->currentFrame = frameIndex;
    
    // Create frame with background color
    cv::Mat frame(config.resolution, CV_8UC3, config.backgroundColor);
    
    // Calculate time parameter (0 to 2*PI for one full cycle)
    double t = TWO_PI * static_cast<double>(frameIndex) / config.totalFrames;
    
    // Get epicycle positions
    auto positions = getEpicyclePositions(pImpl->coefficients, t);
    
    // Add final point to traced path
    if (!positions.empty()) {
        cv::Point screenPoint = worldToScreen(positions.back());
        pImpl->tracedPath.push_back(screenPoint);
    }
    
    // Draw components in order (back to front)
    if (config.showPath) {
        drawPath(frame);
    }
    
    if (config.showCircles) {
        drawCircles(frame, positions, t);
    }
    
    if (config.showVectors) {
        drawVectors(frame, positions);
    }
    
    if (config.showOriginMarker) {
        drawOriginMarker(frame);
    }
    
    // Draw current drawing point
    if (!positions.empty()) {
        cv::Point endPoint = worldToScreen(positions.back());
        cv::circle(frame, endPoint, 6, cv::Scalar(0, 255, 255), -1);  // Yellow filled
        cv::circle(frame, endPoint, 6, cv::Scalar(255, 255, 255), 2); // White outline
    }
    
    return frame;
}

void AnimationEngine::drawCircles(cv::Mat& frame, const std::vector<cv::Point2d>& positions, double t) {
    const auto& config = pImpl->config;
    const auto& coefficients = pImpl->coefficients;
    
    for (size_t i = 0; i < coefficients.size() && i < positions.size(); ++i) {
        cv::Point center = worldToScreen(positions[i]);
        int radius = static_cast<int>(coefficients[i].amplitude * config.scale);
        
        if (radius > 1) {
            // Draw circle with the coefficient's color
            cv::circle(frame, center, radius, coefficients[i].color, config.circleThickness);
        }
    }
}

void AnimationEngine::drawVectors(cv::Mat& frame, const std::vector<cv::Point2d>& positions) {
    const auto& config = pImpl->config;
    const auto& coefficients = pImpl->coefficients;
    
    for (size_t i = 0; i + 1 < positions.size() && i < coefficients.size(); ++i) {
        cv::Point start = worldToScreen(positions[i]);
        cv::Point end = worldToScreen(positions[i + 1]);
        
        // Draw vector with same color as corresponding circle
        cv::line(frame, start, end, coefficients[i].color, config.vectorThickness);
    }
}

void AnimationEngine::drawPath(cv::Mat& frame) {
    const auto& config = pImpl->config;
    const auto& path = pImpl->tracedPath;
    
    if (path.size() < 2) return;
    
    // Draw path with gradient effect (older parts fade)
    for (size_t i = 1; i < path.size(); ++i) {
        double alpha = static_cast<double>(i) / path.size();
        cv::Scalar color(
            static_cast<int>(100 + 155 * alpha),  // B
            static_cast<int>(200 * alpha),         // G
            static_cast<int>(255 * alpha)          // R
        );
        cv::line(frame, path[i-1], path[i], color, config.pathThickness);
    }
}

void AnimationEngine::drawOriginMarker(cv::Mat& frame) {
    const auto& config = pImpl->config;
    cv::Point origin = worldToScreen(cv::Point2d(0, 0));
    
    // Draw a0 marker (crosshair style)
    int markerSize = 10;
    cv::Scalar markerColor(128, 128, 128);  // Gray
    
    cv::line(frame, cv::Point(origin.x - markerSize, origin.y),
             cv::Point(origin.x + markerSize, origin.y), markerColor, 1);
    cv::line(frame, cv::Point(origin.x, origin.y - markerSize),
             cv::Point(origin.x, origin.y + markerSize), markerColor, 1);
    
    // Label "a₀"
    cv::putText(frame, "a0", cv::Point(origin.x + 12, origin.y - 5),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
}

cv::Point AnimationEngine::worldToScreen(const cv::Point2d& worldPoint) const {
    const auto& config = pImpl->config;
    
    // Transform from world coordinates to screen coordinates
    // World Y is inverted for screen (screen Y increases downward)
    int screenX = static_cast<int>(config.center.x + worldPoint.x * config.scale);
    int screenY = static_cast<int>(config.center.y + worldPoint.y * config.scale);
    
    return cv::Point(screenX, screenY);
}

const std::vector<cv::Point>& AnimationEngine::getTracedPath() const {
    return pImpl->tracedPath;
}

void AnimationEngine::reset() {
    pImpl->tracedPath.clear();
    pImpl->currentFrame = 0;
}

bool AnimationEngine::isComplete() const {
    return pImpl->currentFrame >= pImpl->config.totalFrames - 1;
}

double AnimationEngine::getProgress() const {
    if (pImpl->config.totalFrames <= 0) return 0.0;
    return static_cast<double>(pImpl->currentFrame) / pImpl->config.totalFrames;
}

} // namespace fourier
