#!/bin/bash
# =============================================================================
# Fourier Animation Build Script for Jetson AGX Orin
# =============================================================================

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

echo -e "${BLUE}=== Fourier Animation Build Script ===${NC}"
echo ""

# Parse arguments
BUILD_TYPE="Release"
CLEAN_BUILD=false
RUN_AFTER_BUILD=false
IMAGE_PATH=""

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --run)
            RUN_AFTER_BUILD=true
            IMAGE_PATH="$2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --debug    Build with debug symbols"
            echo "  --clean    Clean build directory before building"
            echo "  --run <image>  Run after building with specified image"
            echo "  -h, --help     Show this help"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

# Clean build if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf "${BUILD_DIR}"
fi

# Create build directory
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Configure with CMake
echo -e "${GREEN}Configuring with CMake...${NC}"
cmake .. \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_CUDA_ARCHITECTURES=87

# Build
echo ""
echo -e "${GREEN}Building...${NC}"
NUM_CORES=$(nproc)
echo "Using ${NUM_CORES} cores for parallel build"
make -j"${NUM_CORES}"

# Check if build succeeded
if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}=== Build Successful ===${NC}"
    echo "Executable: ${BUILD_DIR}/fourier_animation"
    
    # Run if requested
    if [ "$RUN_AFTER_BUILD" = true ] && [ -n "$IMAGE_PATH" ]; then
        echo ""
        echo -e "${BLUE}Running with image: ${IMAGE_PATH}${NC}"
        ./fourier_animation "$IMAGE_PATH"
    fi
else
    echo ""
    echo -e "${RED}=== Build Failed ===${NC}"
    exit 1
fi

echo ""
echo -e "${BLUE}Usage:${NC}"
echo "  ./build/fourier_animation <image_path> [options]"
echo ""
echo "Example:"
echo "  ./build/fourier_animation assets/image.png -n 150 -f 900 -o output.mp4"
