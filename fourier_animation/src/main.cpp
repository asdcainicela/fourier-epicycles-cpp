#include <iostream>
#include <string>
#include <chrono>
#include <spdlog/spdlog.h>
#include <indicators/progress_bar.hpp>

#include "fourier.hpp"
#include "contour_extractor.hpp"
#include "animation.hpp"
#include "video_writer.hpp"

void printUsage(const char* programName) {
    spdlog::info("Usage: {} <image_path> [options]\n"
                 "Options:\n"
                 "  --output <path>     Output video path (default: fourier_output.mp4)\n"
                 "  --circles <num>     Number of epicycles (default: 100)\n"
                 "  --frames <num>      Total frames (default: 600)\n"
                 "  --fps <num>         Frames per second (default: 60)\n"
                 "  --width <num>       Video width (default: 1920)\n"
                 "  --height <num>      Video height (default: 1080)\n"
                 "  --no-circles        Hide circle outlines\n"
                 "  --no-vectors        Hide radius vectors\n"
                 "  --no-path           Hide traced path\n"
                 "  --samples <num>     Contour sample points (default: 500)\n"
                 "  --cpu               Force CPU encoding\n"
                 "  --help              Show this help message", programName);
}

bool checkValidArgs(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return true;
    }
    return false;
}

bool checkHelp(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--help") {
            printUsage(argv[0]);
            return true;
        }
    }
    return false;
}

// Parse command line arguments
void parseArgs(int argc, char* argv[],
               fourier::ContourConfig& contourConfig,
               fourier::AnimationConfig& animConfig,
               fourier::VideoConfig& videoConfig) {
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--output" && i + 1 < argc) {
            videoConfig.outputPath = argv[++i];
        } else if (arg == "--circles" && i + 1 < argc) {
            animConfig.numCircles = std::stoi(argv[++i]);
        } else if (arg == "--frames" && i + 1 < argc) {
            animConfig.totalFrames = std::stoi(argv[++i]);
        } else if (arg == "--fps" && i + 1 < argc) {
            animConfig.fps = std::stod(argv[++i]);
            videoConfig.fps = animConfig.fps;
        } else if (arg == "--width" && i + 1 < argc) {
            int width = std::stoi(argv[++i]);
            animConfig.resolution.width = width;
            videoConfig.width = width;
            animConfig.center.x = width / 2.0;
        } else if (arg == "--height" && i + 1 < argc) {
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
}

int main(int argc, char* argv[]) {
    
    spdlog::set_level(spdlog::level::info);

    if (checkValidArgs(argc, argv)) return 0;

    std::string imagePath = argv[1];

    if (checkHelp(argc, argv)) return 0;

    fourier::ContourConfig contourConfig;
    fourier::AnimationConfig animConfig;
    fourier::VideoConfig videoConfig;

    // Parse command line arguments
    parseArgs(argc, argv, contourConfig, animConfig, videoConfig);
       
    spdlog::info("-- Fourier Animation Generator --");
    spdlog::info("Image: {}", imagePath);
    spdlog::info("Output: {}", videoConfig.outputPath);
    spdlog::info("Resolution: {}x{}", videoConfig.width, videoConfig.height);
    spdlog::info("Epicycles: {}", animConfig.numCircles);
    spdlog::info("Frames: {} @ {} fps", animConfig.totalFrames, animConfig.fps);

    auto startTime = std::chrono::high_resolution_clock::now();

    // Extract contour from image
    spdlog::warn("Extracting contour from image...");
    auto contourResult = fourier::extractContour(imagePath, contourConfig);

    if (!contourResult.success) {
        spdlog::error("Error: {}", contourResult.errorMessage);
        return 1;
    }

    spdlog::info("Found contour with {} points", contourResult.complexPoints.size());

    // Compute Fourier coefficients (DFT)
    spdlog::debug("Computing Fourier coefficients...");
    auto coefficients = fourier::computeDFT(contourResult.complexPoints, animConfig.numCircles);

    spdlog::info("Computed {} Fourier coefficients", coefficients.size());

    // Initialize animation
    spdlog::debug("Initializing animation engine...");
    fourier::AnimationEngine animator;
    animator.initialize(coefficients, animConfig);

    // Initialize video writer
    spdlog::debug("Writing video frames...");
    fourier::VideoWriter videoWriter;

    if (!videoWriter.open(videoConfig)) {
        spdlog::error("Failed to open video writer");
        return 1;
    }

    // Progress bar
    indicators::ProgressBar bar{
        indicators::option::BarWidth{50},
        indicators::option::Start{"["},
        indicators::option::Fill{"="},
        indicators::option::Lead{">"},
        indicators::option::Remainder{" "},
        indicators::option::End{"]"},
        indicators::option::ShowPercentage{true},
        indicators::option::PostfixText{"Rendering frames"}
    };

    // Render and write frames
    for (int frame = 0; frame < animConfig.totalFrames; ++frame) {
        cv::Mat frameImage = animator.renderFrame(frame);

        if (frameImage.empty()) {
            spdlog::error("Failed to render frame {}", frame);
            continue;
        }

        videoWriter.writeFrame(frameImage);

        // Update progress bar
        int progress = static_cast<int>(100.0 * (frame + 1) / animConfig.totalFrames);
        bar.set_progress(progress);
    }

    // Add 2-second pause at the end
    if (animConfig.totalFrames > 0) {
        int pauseFrames = static_cast<int>(videoConfig.fps * 2);
        spdlog::debug("Adding 2-second pause ({} frames)...", pauseFrames);
        cv::Mat lastFrame = animator.renderFrame(animConfig.totalFrames - 1);

        for (int i = 0; i < pauseFrames; ++i) {
            videoWriter.writeFrame(lastFrame);
        }
    }

    videoWriter.release();

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    spdlog::info("=== Complete ===");
    spdlog::info("Output: {}", videoConfig.outputPath);
    spdlog::info("Total time: {:.2f} seconds", duration.count() / 1000.0);
    spdlog::info("Average: {} ms/frame", duration.count() / animConfig.totalFrames);

    return 0;
}
