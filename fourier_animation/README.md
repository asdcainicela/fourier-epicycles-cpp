# Fourier Animation

C++ application for generating Fourier series epicycle animations from image contours, targeting **Jetson AGX Orin**.

![Manim-style Fourier visualization](https://upload.wikimedia.org/wikipedia/commons/2/2b/Fourier_series_and_transform.gif)

## Features

- 🎨 **Color System**: Enum-based color palette with `getColor()` and random color generation
- 📐 **Contour Extraction**: OpenCV-based contour detection from any image
- 🔢 **Complex FFT**: Discrete Fourier Transform for computing epicycle coefficients
- 🎬 **HD+ Video Output**: Hardware-accelerated encoding via GStreamer/NVENC
- 🎥 **Manim-style Animation**: Rotating circles with vectors and traced path

## Requirements

- CMake 3.18+
- OpenCV 4.x (with CUDA support recommended)
- CUDA Toolkit (for Jetson)
- GStreamer 1.0 (optional, for hardware encoding)

## Build (Jetson AGX Orin)

```bash
# Make build script executable
chmod +x build.sh

# Build release
./build.sh

# Build debug
./build.sh --debug

# Clean and rebuild
./build.sh --clean
```

## Usage

```bash
./build/fourier_animation <image_path> [options]
```

### Options

| Option | Description | Default |
|--------|-------------|---------|
| `-o, --output <path>` | Output video path | `fourier_output.mp4` |
| `-n, --circles <num>` | Number of epicycles | 100 |
| `-f, --frames <num>` | Total frames | 600 |
| `--fps <num>` | Frames per second | 60 |
| `-w, --width <num>` | Video width | 1920 |
| `-h, --height <num>` | Video height | 1080 |
| `--no-circles` | Hide circle outlines | |
| `--no-vectors` | Hide radius vectors | |
| `--no-path` | Hide traced path | |
| `--samples <num>` | Contour sample points | 500 |

### Examples

```bash
# Basic usage
./build/fourier_animation assets/logo.png

# High quality with more epicycles
./build/fourier_animation assets/logo.png -n 200 -f 1200 --fps 60

# 4K output
./build/fourier_animation assets/logo.png -w 3840 -h 2160 -o output_4k.mp4

# Minimal visualization (path only)
./build/fourier_animation assets/logo.png --no-circles --no-vectors
```

## Project Structure

```
fourier_animation/
├── CMakeLists.txt
├── build.sh
├── include/
│   ├── colors.hpp           # Color enum + getColor()
│   ├── fourier.hpp           # FFT complex computations
│   ├── contour_extractor.hpp # OpenCV contour extraction
│   ├── animation.hpp         # Epicycle animation engine
│   └── video_writer.hpp      # FFmpeg/GStreamer wrapper
├── src/
│   ├── main.cpp
│   ├── colors.cpp
│   ├── fourier.cpp
│   ├── contour_extractor.cpp
│   ├── animation.cpp
│   └── video_writer.cpp
├── assets/
│   └── image.png             # Input image
└── output/
    └── (generated videos)
```

## License

MIT License
