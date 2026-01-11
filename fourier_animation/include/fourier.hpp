#pragma once

#include <complex>
#include <vector>
#include <opencv2/core.hpp>

namespace fourier {

struct FourierCoefficient {
    int frequency;
    std::complex<double> cn;
    double amplitude;
    double phase;
    cv::Scalar color;
};

// Compute DFT and return coefficients sorted by amplitude
std::vector<FourierCoefficient> computeDFT(
    const std::vector<std::complex<double>>& points,
    int numCircles
);

// Get epicycle positions at time t
std::vector<cv::Point2d> getEpicyclePositions(
    const std::vector<FourierCoefficient>& coefficients,
    double t
);

}
