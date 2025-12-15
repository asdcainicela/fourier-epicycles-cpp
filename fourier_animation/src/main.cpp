#include <iostream>
#include <string>
#include <chrono>

#include "colors.hpp"
#include "fourier.hpp"
#include "contour_extractor.hpp"
#include "animation.hpp"
#include "video_writer.hpp"

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " <image_path> [options]\n"
              << "\nOptions:\n"
              << "  -o, --output <path>     Output video path (default: fourier_output.mp4)\n"
              << "  -n, --circles <num>     Number of epicycles (default: 100)\n"
              << "  -f, --frames <num>      Total frames (default: 600)\n"
              << "  --fps <num>             Frames per second (default: 60)\n"
              << "  -w, --width <num>       Video width (default: 1920)\n"
              << "  -h, --height <num>      Video height (default: 1080)\n"
              << "  --no-circles            Hide circle outlines\n"
              << "  --no-vectors            Hide radius vectors\n"
              << "  --no-path               Hide traced path\n"
              << "  --samples <num>         Contour sample points (default: 500)\n"
              << "  --cpu                   Force CPU encoding (FFmpeg) instead of hardware encoding\n"
              << "  --help                  Show this help message\n"
              << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string imagePath = argv[1];
    
    // Check for help flag first
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help") {
            printUsage(argv[0]);
            return 0;
        }
    }
    
    // Default configuration
    fourier::ContourConfig contourConfig;
    fourier::AnimationConfig animConfig;
    fourier::VideoConfig videoConfig;
    
    // Parse command line arguments
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        
        if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            videoConfig.outputPath = argv[++i];
        } else if ((arg == "-n" || arg == "--circles") && i + 1 < argc) {
            animConfig.numCircles = std::stoi(argv[++i]);
        } else if ((arg == "-f" || arg == "--frames") && i + 1 < argc) {
            animConfig.totalFrames = std::stoi(argv[++i]);
        } else if (arg == "--fps" && i + 1 < argc) {
            animConfig.fps = std::stod(argv[++i]);
            videoConfig.fps = animConfig.fps;
        } else if ((arg == "-w" || arg == "--width") && i + 1 < argc) {
            int width = std::stoi(argv[++i]);
            animConfig.resolution.width = width;
            videoConfig.width = width;
            animConfig.center.x = width / 2.0;
        } else if ((arg == "-h" || arg == "--height") && i + 1 < argc) {
            int height = std::stoi(argv[++i]);
            animConfig.resolution.height = height;
            videoConfig.height = height;
            animConfig.center.y = height / 2.0;
        } else if (arg == "--no-circles") {
            animConfig.showCircles = false;
        } else if (arg == "--no-vectors") {
            animConfig.showVectors = false;
        } else if (arg == "--no-path") {
            animConfig.showPath = false;
        } else if (arg == "--samples" && i + 1 < argc) {
            contourConfig.numSamplePoints = std::stoi(argv[++i]);
        } else if (arg == "--cpu") {
            videoConfig.useHardwareEncoding = false;
        }
    }
    
    std::cout << "=== Fourier Animation Generator ===" << std::endl;
    std::cout << "Image: " << imagePath << std::endl;
    std::cout << "Output: " << videoConfig.outputPath << std::endl;
    std::cout << "Resolution: " << videoConfig.width << "x" << videoConfig.height << std::endl;
    std::cout << "Epicycles: " << animConfig.numCircles << std::endl;
    std::cout << "Frames: " << animConfig.totalFrames << " @ " << animConfig.fps << " fps" << std::endl;
    std::cout << std::endl;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Step 1: Extract contour from image
    std::cout << "[1/4] Extracting contour from image..." << std::endl;
    auto contourResult = fourier::extractContour(imagePath, contourConfig);
    
    if (!contourResult.success) {
        std::cerr << "Error: " << contourResult.errorMessage << std::endl;
        return 1;
    }
    
    std::cout << "  -> Found contour with " << contourResult.complexPoints.size() << " points" << std::endl;
    
    // Step 2: Compute Fourier coefficients (DFT)
    std::cout << "[2/4] Computing Fourier coefficients..." << std::endl;
    auto coefficients = fourier::computeDFT(contourResult.complexPoints, animConfig.numCircles);
    
    std::cout << "  -> Computed " << coefficients.size() << " Fourier coefficients" << std::endl;
    
    // Step 3: Initialize animation
    std::cout << "[3/4] Initializing animation engine..." << std::endl;
    fourier::AnimationEngine animator;
    animator.initialize(coefficients, animConfig);
    
    // Step 4: Initialize video writer
    std::cout << "[4/4] Writing video frames..." << std::endl;
    fourier::VideoWriter videoWriter;
    
    if (!videoWriter.open(videoConfig)) {
        std::cerr << "Error: Failed to open video writer" << std::endl;
        return 1;
    }
    
    // Render and write frames
    int lastProgress = -1;
    for (int frame = 0; frame < animConfig.totalFrames; ++frame) {
        cv::Mat frameImage = animator.renderFrame(frame);
        
        if (frameImage.empty()) {
            std::cerr << "Error: Failed to render frame " << frame << std::endl;
            continue;
        }
        
        videoWriter.writeFrame(frameImage);
        
        // Progress indicator
        int progress = static_cast<int>(100.0 * frame / animConfig.totalFrames);
        if (progress != lastProgress && progress % 10 == 0) {
            std::cout << "  -> Progress: " << progress << "%" << std::endl;
            lastProgress = progress;
        }
    }
    
    // Add 2-second pause at the end
    if (animConfig.totalFrames > 0) {
        std::cout << "Adding 2-second pause (" << (videoConfig.fps * 2) << " frames)..." << std::endl;
        cv::Mat lastFrame = animator.renderFrame(animConfig.totalFrames - 1);
        int pauseFrames = static_cast<int>(videoConfig.fps * 2);
        
        for (int i = 0; i < pauseFrames; ++i) {
            videoWriter.writeFrame(lastFrame);
        }
    }

    videoWriter.release();
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    std::cout << std::endl;
    std::cout << "=== Complete ===" << std::endl;
    std::cout << "Output: " << videoConfig.outputPath << std::endl;
    std::cout << "Total time: " << duration.count() / 1000.0 << " seconds" << std::endl;
    std::cout << "Average: " << (duration.count() / animConfig.totalFrames) << " ms/frame" << std::endl;
    
    return 0;
}
