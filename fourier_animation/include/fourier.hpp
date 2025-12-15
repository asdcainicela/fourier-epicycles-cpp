#pragma once

#include <complex>
#include <vector>
#include <opencv2/core.hpp>

namespace fourier {

/**
 * @brief Fourier coefficient structure for epicycles
 */
struct FourierCoefficient {
    int frequency;              // n (harmonic number)
    std::complex<double> cn;    // Complex coefficient
    double amplitude;           // |cn| - radius of circle
    double phase;               // arg(cn) - initial phase
    cv::Scalar color;           // Color for this epicycle
};

/**
 * @brief Compute Discrete Fourier Transform for complex path
 * @param points Vector of complex points from contour
 * @param numCircles Number of harmonics to compute (epicycles)
 * @return Vector of Fourier coefficients sorted by amplitude (largest first)
 */
std::vector<FourierCoefficient> computeDFT(
    const std::vector<std::complex<double>>& points,
    int numCircles
);

/**
 * @brief Compute position at time t using Fourier coefficients
 * @param coefficients Vector of Fourier coefficients
 * @param t Time parameter (0 to 2*PI for one cycle)
 * @return Complex position (x + iy)
 */
std::complex<double> evaluateFourier(
    const std::vector<FourierCoefficient>& coefficients,
    double t
);

/**
 * @brief Get all epicycle positions at time t (for animation)
 * @param coefficients Vector of Fourier coefficients
 * @param t Time parameter
 * @return Vector of points (center positions of each circle)
 */
std::vector<cv::Point2d> getEpicyclePositions(
    const std::vector<FourierCoefficient>& coefficients,
    double t
);

/**
 * @brief Compute FFT using Cooley-Tukey algorithm (optimized)
 * @param points Input complex points (size should be power of 2)
 * @return FFT result
 */
std::vector<std::complex<double>> computeFFT(
    const std::vector<std::complex<double>>& points
);

} // namespace fourier
