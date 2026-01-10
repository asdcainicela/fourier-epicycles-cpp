#include "animation.hpp"
#include <numbers>
#include <iostream>

#ifdef USE_CAIRO
#include <cairo.h>
#endif

namespace fourier {

constexpr double TWO_PI = 2.0 * std::numbers::pi;

class AnimationEngine::Impl {
public:
    std::vector<FourierCoefficient> coefficients;
    AnimationConfig config;
    std::vector<cv::Point> tracedPath;
    int currentFrame = 0;
    bool initialized = false;
    
#ifdef USE_CAIRO
    cairo_surface_t* surface = nullptr;
    cairo_t* cr = nullptr;
    
    void initCairo(int width, int height) {
        if (surface) cairo_surface_destroy(surface);
        if (cr) cairo_destroy(cr);
        
        surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
        cr = cairo_create(surface);
        
        // Enable antialiasing
        cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);
    }
    
    void destroyCairo() {
        if (cr) { cairo_destroy(cr); cr = nullptr; }
        if (surface) { cairo_surface_destroy(surface); surface = nullptr; }
    }
    
    cv::Mat cairoToMat() {
        cairo_surface_flush(surface);
        unsigned char* data = cairo_image_surface_get_data(surface);
        int width = cairo_image_surface_get_width(surface);
        int height = cairo_image_surface_get_height(surface);
        int stride = cairo_image_surface_get_stride(surface);
        
        // Cairo uses ARGB, OpenCV uses BGRA
        cv::Mat mat(height, width, CV_8UC4, data, stride);
        cv::Mat result;
        cv::cvtColor(mat, result, cv::COLOR_BGRA2BGR);
        return result.clone();
    }
#endif
};

AnimationEngine::AnimationEngine() : pImpl(std::make_unique<Impl>()) {}

AnimationEngine::~AnimationEngine() {
#ifdef USE_CAIRO
    pImpl->destroyCairo();
#endif
}

void AnimationEngine::initialize(const std::vector<FourierCoefficient>& coefficients,
                                 const AnimationConfig& config) {
    pImpl->coefficients = coefficients;
    pImpl->config = config;
    pImpl->tracedPath.clear();
    pImpl->tracedPath.reserve(config.totalFrames);
    pImpl->currentFrame = 0;
    pImpl->initialized = true;
    
#ifdef USE_CAIRO
    pImpl->initCairo(config.resolution.width, config.resolution.height);
    std::cout << "[Animation] Using Cairo for high-quality rendering" << std::endl;
#else
    std::cout << "[Animation] Using OpenCV for rendering (install Cairo for better quality)" << std::endl;
#endif
    
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
    
    // Calculate time parameter (0 to 2*PI for one full cycle)
    double t = TWO_PI * static_cast<double>(frameIndex) / config.totalFrames;
    
    // Get epicycle positions
    auto positions = getEpicyclePositions(pImpl->coefficients, t);
    
    // Add final point to traced path
    if (!positions.empty()) {
        cv::Point screenPoint = worldToScreen(positions.back());
        pImpl->tracedPath.push_back(screenPoint);
    }

#ifdef USE_CAIRO
    return renderFrameCairo(positions, t);
#else
    return renderFrameOpenCV(positions, t);
#endif
}

#ifdef USE_CAIRO
cv::Mat AnimationEngine::renderFrameCairo(const std::vector<cv::Point2d>& positions, double t) {
    const auto& config = pImpl->config;
    cairo_t* cr = pImpl->cr;
    
    // Clear background
    cairo_set_source_rgb(cr, 
        config.backgroundColor[2] / 255.0,
        config.backgroundColor[1] / 255.0, 
        config.backgroundColor[0] / 255.0);
    cairo_paint(cr);
    
    // Draw path first (back layer)
    if (config.showPath) {
        drawPathCairo(cr);
    }
    
    // Draw circles
    if (config.showCircles) {
        drawCirclesCairo(cr, positions);
    }
    
    // Draw vectors
    if (config.showVectors) {
        drawVectorsCairo(cr, positions);
    }
    
    // Draw origin marker
    if (config.showOriginMarker) {
        drawOriginMarkerCairo(cr);
    }
    
    // Draw current drawing point
    if (!positions.empty()) {
        cv::Point endPoint = worldToScreen(positions.back());
        
        // Yellow filled circle with white outline
        cairo_arc(cr, endPoint.x, endPoint.y, 6, 0, TWO_PI);
        cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);  // Yellow
        cairo_fill_preserve(cr);
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);  // White outline
        cairo_set_line_width(cr, 2);
        cairo_stroke(cr);
    }
    
    return pImpl->cairoToMat();
}

void AnimationEngine::drawCirclesCairo(cairo_t* cr, const std::vector<cv::Point2d>& positions) {
    const auto& config = pImpl->config;
    const auto& coefficients = pImpl->coefficients;
    
    cairo_set_line_width(cr, config.circleThickness);
    
    for (size_t i = 0; i < coefficients.size() && i < positions.size(); ++i) {
        cv::Point center = worldToScreen(positions[i]);
        double radius = coefficients[i].amplitude * config.scale;
        
        if (radius > 1) {
            // Set color (coefficients store BGR, convert to RGB)
            cairo_set_source_rgba(cr,
                coefficients[i].color[2] / 255.0,
                coefficients[i].color[1] / 255.0,
                coefficients[i].color[0] / 255.0,
                0.6);  // Semi-transparent
            
            cairo_arc(cr, center.x, center.y, radius, 0, TWO_PI);
            cairo_stroke(cr);
        }
    }
}

void AnimationEngine::drawVectorsCairo(cairo_t* cr, const std::vector<cv::Point2d>& positions) {
    const auto& config = pImpl->config;
    const auto& coefficients = pImpl->coefficients;
    
    cairo_set_line_width(cr, config.vectorThickness);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    
    for (size_t i = 0; i + 1 < positions.size() && i < coefficients.size(); ++i) {
        cv::Point start = worldToScreen(positions[i]);
        cv::Point end = worldToScreen(positions[i + 1]);
        
        cairo_set_source_rgb(cr,
            coefficients[i].color[2] / 255.0,
            coefficients[i].color[1] / 255.0,
            coefficients[i].color[0] / 255.0);
        
        cairo_move_to(cr, start.x, start.y);
        cairo_line_to(cr, end.x, end.y);
        cairo_stroke(cr);
    }
}

void AnimationEngine::drawPathCairo(cairo_t* cr) {
    const auto& config = pImpl->config;
    const auto& path = pImpl->tracedPath;
    
    if (path.size() < 2) return;
    
    cairo_set_line_width(cr, config.pathThickness);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    
    // Draw path with gradient effect
    for (size_t i = 1; i < path.size(); ++i) {
        double alpha = static_cast<double>(i) / path.size();
        
        cairo_set_source_rgba(cr,
            alpha,                          // R
            0.8 * alpha,                    // G
            (100 + 155 * alpha) / 255.0,    // B
            0.8 + 0.2 * alpha);             // Alpha
        
        cairo_move_to(cr, path[i-1].x, path[i-1].y);
        cairo_line_to(cr, path[i].x, path[i].y);
        cairo_stroke(cr);
    }
}

void AnimationEngine::drawOriginMarkerCairo(cairo_t* cr) {
    const auto& config = pImpl->config;
    cv::Point origin = worldToScreen(cv::Point2d(0, 0));
    
    int markerSize = 10;
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);  // Gray
    cairo_set_line_width(cr, 1);
    
    cairo_move_to(cr, origin.x - markerSize, origin.y);
    cairo_line_to(cr, origin.x + markerSize, origin.y);
    cairo_stroke(cr);
    
    cairo_move_to(cr, origin.x, origin.y - markerSize);
    cairo_line_to(cr, origin.x, origin.y + markerSize);
    cairo_stroke(cr);
    
    // Label "a₀"
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 12);
    cairo_move_to(cr, origin.x + 12, origin.y - 5);
    cairo_show_text(cr, "a0");
}
#endif

cv::Mat AnimationEngine::renderFrameOpenCV(const std::vector<cv::Point2d>& positions, double t) {
    const auto& config = pImpl->config;
    
    // Create frame with background color
    cv::Mat frame(config.resolution, CV_8UC3, config.backgroundColor);
    
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
    (void)t;  // Unused in OpenCV version
    const auto& config = pImpl->config;
    const auto& coefficients = pImpl->coefficients;
    
    for (size_t i = 0; i < coefficients.size() && i < positions.size(); ++i) {
        cv::Point center = worldToScreen(positions[i]);
        int radius = static_cast<int>(coefficients[i].amplitude * config.scale);
        
        if (radius > 1) {
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
        cv::line(frame, start, end, coefficients[i].color, config.vectorThickness);
    }
}

void AnimationEngine::drawPath(cv::Mat& frame) {
    const auto& config = pImpl->config;
    const auto& path = pImpl->tracedPath;
    
    if (path.size() < 2) return;
    
    for (size_t i = 1; i < path.size(); ++i) {
        double alpha = static_cast<double>(i) / path.size();
        cv::Scalar color(
            static_cast<int>(100 + 155 * alpha),
            static_cast<int>(200 * alpha),
            static_cast<int>(255 * alpha)
        );
        cv::line(frame, path[i-1], path[i], color, config.pathThickness);
    }
}

void AnimationEngine::drawOriginMarker(cv::Mat& frame) {
    const auto& config = pImpl->config;
    cv::Point origin = worldToScreen(cv::Point2d(0, 0));
    
    int markerSize = 10;
    cv::Scalar markerColor(128, 128, 128);
    
    cv::line(frame, cv::Point(origin.x - markerSize, origin.y),
             cv::Point(origin.x + markerSize, origin.y), markerColor, 1);
    cv::line(frame, cv::Point(origin.x, origin.y - markerSize),
             cv::Point(origin.x, origin.y + markerSize), markerColor, 1);
}

cv::Point AnimationEngine::worldToScreen(const cv::Point2d& worldPoint) const {
    const auto& config = pImpl->config;
    
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
