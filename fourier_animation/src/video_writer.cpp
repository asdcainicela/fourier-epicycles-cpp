#include "video_writer.hpp"
#include <opencv2/videoio.hpp>
#include <iostream>

namespace fourier {

class VideoWriter::Impl {
public:
    cv::VideoWriter writer;
    VideoConfig config;
    int frameCount = 0;
    bool opened = false;
};

VideoWriter::VideoWriter() : pImpl(std::make_unique<Impl>()) {}

VideoWriter::~VideoWriter() {
    release();
}

bool VideoWriter::open(const VideoConfig& config) {
    pImpl->config = config;
    pImpl->frameCount = 0;
    
    if (config.useHardwareEncoding) {
        // Try GStreamer pipeline for hardware encoding (Jetson)
        std::string pipeline = getGStreamerPipeline(config);
        pImpl->writer.open(pipeline, cv::CAP_GSTREAMER, 0, config.fps, 
                          cv::Size(config.width, config.height), true);
        
        if (pImpl->writer.isOpened()) {
            pImpl->opened = true;
            std::cout << "[VideoWriter] Opened with GStreamer hardware encoding" << std::endl;
            return true;
        }
        std::cout << "[VideoWriter] GStreamer failed, falling back to FFmpeg" << std::endl;
    }
    
    // Fallback to FFmpeg/software encoding
    int fourcc = cv::VideoWriter::fourcc(
        config.codec[0], config.codec[1], config.codec[2], config.codec[3]
    );
    
    pImpl->writer.open(config.outputPath, fourcc, config.fps,
                       cv::Size(config.width, config.height), true);
    
    if (!pImpl->writer.isOpened()) {
        // Try alternative codecs
        std::vector<std::string> fallbackCodecs = {"mp4v", "XVID", "MJPG"};
        for (const auto& codec : fallbackCodecs) {
            fourcc = cv::VideoWriter::fourcc(codec[0], codec[1], codec[2], codec[3]);
            pImpl->writer.open(config.outputPath, fourcc, config.fps,
                              cv::Size(config.width, config.height), true);
            if (pImpl->writer.isOpened()) {
                std::cout << "[VideoWriter] Opened with codec: " << codec << std::endl;
                break;
            }
        }
    }
    
    pImpl->opened = pImpl->writer.isOpened();
    
    if (!pImpl->opened) {
        std::cerr << "[VideoWriter] Failed to open video writer" << std::endl;
    }
    
    return pImpl->opened;
}

bool VideoWriter::writeFrame(const cv::Mat& frame) {
    if (!pImpl->opened) return false;
    
    cv::Mat resizedFrame;
    if (frame.cols != pImpl->config.width || frame.rows != pImpl->config.height) {
        cv::resize(frame, resizedFrame, cv::Size(pImpl->config.width, pImpl->config.height));
    } else {
        resizedFrame = frame;
    }
    
    pImpl->writer.write(resizedFrame);
    pImpl->frameCount++;
    return true;
}

void VideoWriter::release() {
    if (pImpl->opened) {
        pImpl->writer.release();
        pImpl->opened = false;
        std::cout << "[VideoWriter] Released. Total frames: " << pImpl->frameCount << std::endl;
    }
}

bool VideoWriter::isOpened() const {
    return pImpl->opened;
}

int VideoWriter::getFrameCount() const {
    return pImpl->frameCount;
}

std::string VideoWriter::getGStreamerPipeline(const VideoConfig& config) {
    // GStreamer pipeline for Jetson hardware encoding (NVENC)
    std::string pipeline = 
        "appsrc ! "
        "video/x-raw, format=BGR ! "
        "videoconvert ! "
        "video/x-raw, format=BGRx ! "
        "nvvidconv ! "
        "video/x-raw(memory:NVMM), format=NV12 ! "
        "nvv4l2h264enc bitrate=8000000 ! "
        "h264parse ! "
        "mp4mux ! "
        "filesink location=" + config.outputPath;
    
    return pipeline;
}

cv::VideoWriter VideoWriter::createFFmpegWriter(const VideoConfig& config) {
    int fourcc = cv::VideoWriter::fourcc('a', 'v', 'c', '1');
    return cv::VideoWriter(config.outputPath, fourcc, config.fps, 
                          cv::Size(config.width, config.height), true);
}

} // namespace fourier
