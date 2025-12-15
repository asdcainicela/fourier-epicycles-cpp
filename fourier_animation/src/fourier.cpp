#include "fourier.hpp"
#include "colors.hpp"
#include <algorithm>
#include <cmath>

namespace fourier {

constexpr double PI = 3.14159265358979323846;
constexpr double TWO_PI = 2.0 * PI;

std::vector<FourierCoefficient> computeDFT(
    const std::vector<std::complex<double>>& points,
    int numCircles
) {
    const int N = static_cast<int>(points.size());
    std::vector<FourierCoefficient> coefficients;
    coefficients.reserve(N);
    
    // Compute all frequency components
    for (int n = -N/2; n < N/2; ++n) {
        std::complex<double> sum(0.0, 0.0);
        
        for (int k = 0; k < N; ++k) {
            double angle = -TWO_PI * n * k / N;
            std::complex<double> exp_term(std::cos(angle), std::sin(angle));
            sum += points[k] * exp_term;
        }
        
        sum /= static_cast<double>(N);
        
        FourierCoefficient coef;
        coef.frequency = n;
        coef.cn = sum;
        coef.amplitude = std::abs(sum);
        coef.phase = std::arg(sum);
        coef.color = getRandomColor(static_cast<unsigned int>(n + 10000));
        
        coefficients.push_back(coef);
    }
    
    // Sort by amplitude (largest first) for better visual effect
    std::sort(coefficients.begin(), coefficients.end(),
        [](const FourierCoefficient& a, const FourierCoefficient& b) {
            return a.amplitude > b.amplitude;
        }
    );
    
    // Keep only the requested number of circles
    if (numCircles > 0 && numCircles < static_cast<int>(coefficients.size())) {
        coefficients.resize(numCircles);
    }
    
    return coefficients;
}

std::complex<double> evaluateFourier(
    const std::vector<FourierCoefficient>& coefficients,
    double t
) {
    std::complex<double> result(0.0, 0.0);
    
    for (const auto& coef : coefficients) {
        double angle = coef.frequency * t + coef.phase;
        std::complex<double> rotation(std::cos(angle), std::sin(angle));
        result += coef.amplitude * rotation;
    }
    
    return result;
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

// Cooley-Tukey FFT (for power-of-2 sizes)
std::vector<std::complex<double>> computeFFT(
    const std::vector<std::complex<double>>& points
) {
    const int N = static_cast<int>(points.size());
    
    if (N <= 1) return points;
    
    // Split into even and odd
    std::vector<std::complex<double>> even, odd;
    even.reserve(N / 2);
    odd.reserve(N / 2);
    
    for (int i = 0; i < N; i += 2) {
        even.push_back(points[i]);
        if (i + 1 < N) odd.push_back(points[i + 1]);
    }
    
    // Recursive FFT
    auto fftEven = computeFFT(even);
    auto fftOdd = computeFFT(odd);
    
    // Combine
    std::vector<std::complex<double>> result(N);
    for (int k = 0; k < N / 2; ++k) {
        double angle = -TWO_PI * k / N;
        std::complex<double> w(std::cos(angle), std::sin(angle));
        result[k] = fftEven[k] + w * fftOdd[k];
        result[k + N / 2] = fftEven[k] - w * fftOdd[k];
    }
    
    return result;
}

} // namespace fourier
