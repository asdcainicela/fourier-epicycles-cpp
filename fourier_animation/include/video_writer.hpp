#pragma once

#include <opencv2/core.hpp>
#include <string>

namespace fourier {

/**
 * @brief Video output configuration
 */
struct VideoConfig {
    int width = 1920;           // Video width (Full HD default)
    int height = 1080;          // Video height
    double fps = 60.0;          // Frames per second
    std::string codec = "avc1"; // Codec (H.264)
    std::string outputPath = "output.mp4";
    bool useHardwareEncoding = true;  // Use NVENC on Jetson
};

/**
 * @brief Video writer wrapper with FFmpeg/GStreamer support
 */
class VideoWriter {
public:
    VideoWriter();
    ~VideoWriter();
    
    /**
     * @brief Initialize video writer
     * @param config Video configuration
     * @return true if successful
     */
    bool open(const VideoConfig& config);
    
    /**
     * @brief Write a frame to the video
     * @param frame BGR image frame
     * @return true if successful
     */
    bool writeFrame(const cv::Mat& frame);
    
    /**
     * @brief Close and finalize the video
     */
    void release();
    
    /**
     * @brief Check if writer is open
     */
    bool isOpened() const;
    
    /**
     * @brief Get total frames written
     */
    int getFrameCount() const;
    
    /**
     * @brief Get GStreamer pipeline string for Jetson hardware encoding
     * @param config Video configuration
     * @return GStreamer pipeline string
     */
    static std::string getGStreamerPipeline(const VideoConfig& config);
    
    /**
     * @brief Get FFmpeg backend writer
     * @param config Video configuration
     * @return VideoWriter with FFmpeg backend
     */
    static cv::VideoWriter createFFmpegWriter(const VideoConfig& config);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

/**
 * @brief Common video resolutions
 */
struct VideoResolution {
    static constexpr cv::Size HD{1280, 720};
    static constexpr cv::Size FULL_HD{1920, 1080};
    static constexpr cv::Size QHD{2560, 1440};
    static constexpr cv::Size UHD_4K{3840, 2160};
};

} // namespace fourier
