#include "fourier.hpp"
#include <kissfft.hh>
#include <algorithm>
#include <cmath>
#include <random>

namespace fourier {

std::vector<FourierCoefficient> computeDFT(
    const std::vector<std::complex<double>>& points,
    int numCircles
) {
    const int N = static_cast<int>(points.size());
    if (N == 0) return {};
    
    // Use KissFFT for the transform
    kissfft<double> fft(N, false);  // false = forward transform
    
    std::vector<std::complex<double>> fftResult(N);
    fft.transform(points.data(), fftResult.data());
    
    // Convert FFT result to FourierCoefficients
    std::vector<FourierCoefficient> coefficients;
    coefficients.reserve(N);
    
    // Random generator for colors
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 255);
    
    for (int i = 0; i < N; ++i) {
        // Convert index to frequency (-N/2 to N/2)
        int n = (i < N/2) ? i : i - N;
        
        FourierCoefficient coef;
        coef.frequency = n;
        coef.cn = fftResult[i] / static_cast<double>(N);  // Normalize
        coef.amplitude = std::abs(coef.cn);
        coef.phase = std::arg(coef.cn);
        coef.color = cv::Scalar(dist(rng), dist(rng), dist(rng));
        
        coefficients.push_back(coef);
    }
    
    // Sort by amplitude (largest first)
    std::sort(coefficients.begin(), coefficients.end(),
        [](const FourierCoefficient& a, const FourierCoefficient& b) {
            return a.amplitude > b.amplitude;
        }
    );
    
    // Keep only requested number of circles
    if (numCircles > 0 && numCircles < static_cast<int>(coefficients.size())) {
        coefficients.resize(numCircles);
    }
    
    return coefficients;
}


std::vector<cv::Point2d> getEpicyclePositions(
    const std::vector<FourierCoefficient>& coefficients,
    double t
) {
    std::vector<cv::Point2d> positions;
    positions.reserve(coefficients.size() + 1);
    
    std::complex<double> current(0.0, 0.0);
    positions.push_back(cv::Point2d(current.real(), current.imag()));
    
    for (const auto& coef : coefficients) {
        double angle = coef.frequency * t + coef.phase;
        std::complex<double> rotation(std::cos(angle), std::sin(angle));
        current += coef.amplitude * rotation;
        positions.push_back(cv::Point2d(current.real(), current.imag()));
    }
    
    return positions;
}

}
