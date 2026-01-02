#pragma once

#include "fourier.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

namespace fourier {

/**
 * @brief Animation configuration
 */
struct AnimationConfig {
    int numCircles = 100;           // Number of epicycles
    int totalFrames = 600;          // Total animation frames (10 sec @ 60fps)
    double fps = 60.0;              // Frames per second
    cv::Size resolution{1920, 1080}; // Output resolution
    
    // Visual settings
    cv::Scalar backgroundColor{0, 0, 0};  // Black background
    int circleThickness = 1;
    int vectorThickness = 2;
    int pathThickness = 3;
    
    // Toggle features
    bool showCircles = true;
    bool showVectors = true;
    bool showPath = true;
    bool showOriginMarker = true;
    
    // Animation center offset (to center in frame)
    cv::Point2d center{960, 540};
    double scale = 400.0;  // Scale factor for visualization
};

/**
 * @brief Animation engine for Fourier epicycles (Manim-style)
 */
class AnimationEngine {
public:
    AnimationEngine();
    ~AnimationEngine();
    
    /**
     * @brief Initialize animation with Fourier coefficients
     * @param coefficients Fourier coefficients from DFT
     * @param config Animation configuration
     */
    void initialize(const std::vector<FourierCoefficient>& coefficients,
                   const AnimationConfig& config = AnimationConfig());
    
    /**
     * @brief Render a single frame at time t
     * @param frameIndex Current frame index (0 to totalFrames-1)
     * @return Rendered frame
     */
    cv::Mat renderFrame(int frameIndex);
    
    /**
     * @brief Get the path traced so far
     * @return Vector of path points
     */
    const std::vector<cv::Point>& getTracedPath() const;
    
    /**
     * @brief Reset animation state
     */
    void reset();
    
    /**
     * @brief Check if animation is complete
     */
    bool isComplete() const;
    
    /**
     * @brief Get current progress (0.0 to 1.0)
     */
    double getProgress() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
    
    // OpenCV rendering methods
    cv::Mat renderFrameOpenCV(const std::vector<cv::Point2d>& positions, double t);
    void drawCircles(cv::Mat& frame, const std::vector<cv::Point2d>& positions, double t);
    void drawVectors(cv::Mat& frame, const std::vector<cv::Point2d>& positions);
    void drawPath(cv::Mat& frame);
    void drawOriginMarker(cv::Mat& frame);
    
#ifdef USE_CAIRO
    // Cairo rendering methods (high-quality)
    cv::Mat renderFrameCairo(const std::vector<cv::Point2d>& positions, double t);
    void drawCirclesCairo(struct _cairo* cr, const std::vector<cv::Point2d>& positions);
    void drawVectorsCairo(struct _cairo* cr, const std::vector<cv::Point2d>& positions);
    void drawPathCairo(struct _cairo* cr);
    void drawOriginMarkerCairo(struct _cairo* cr);
#endif
    
    cv::Point worldToScreen(const cv::Point2d& worldPoint) const;
};

} // namespace fourier
