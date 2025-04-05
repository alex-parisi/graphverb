#include "SpectralAnalyzer.h"

#include <cmath>

/**
 * @brief Constructor for the SpectralAnalyzer.
 *
 * @param fftOrder The FFT order (e.g., 10 for 1024 samples).
 * @param hopSizeIn The hop size (number of samples to advance after each
 * FFT frame). A common default is fftSize/2 for 50% overlap.
 */
SpectralAnalyzer::SpectralAnalyzer(const int fftOrder, const int hopSizeIn) :
    fftOrder(fftOrder), fftSize(1 << fftOrder),
    hopSize(hopSizeIn > 0 ? hopSizeIn : (1 << fftOrder) / 2), fifoFill(0),
    fft(fftOrder) {
    /// Initialize the Hann window to reduce spectral leakage.
    window.resize(fftSize);
    juce::dsp::WindowingFunction<float>::fillWindowingTables(
            window.data(), fftSize, juce::dsp::WindowingFunction<float>::hann);
    /// Allocate FIFO buffer for the incoming time-domain samples.
    fifoBuffer.resize(fftSize, 0.0f);
    /// Allocate buffer for the FFT result (frequency domain).
    frequencyDomainBuffer.resize(2 * fftSize, 0.0f);
}

/**
 * @brief Push new audio samples into the analyzer.
 *
 * This method can be called from your plugin's processBlock() and will
 * handle the accumulation of samples until a full FFT frame is ready.
 *
 * @param input Pointer to incoming audio samples.
 * @param numSamples Number of samples in the input buffer.
 */
void SpectralAnalyzer::pushSamples(const float *input, const int numSamples) {
    int index = 0;
    while (index < numSamples) {
        /// Determine how many samples can be copied into the FIFO buffer.
        const int samplesToCopy =
                std::min(numSamples - index, fftSize - fifoFill);
        std::copy_n(input + index, samplesToCopy,
                    fifoBuffer.begin() + fifoFill);
        fifoFill += samplesToCopy;
        index += samplesToCopy;
        /// If we have filled the FIFO buffer with fftSize samples, process
        /// the frame.
        if (fifoFill == fftSize) {
            processFrame();
            /// Shift the buffer left by hopSize samples to prepare for the
            /// next frame.
            std::copy(fifoBuffer.begin() + hopSize, fifoBuffer.end(),
                      fifoBuffer.begin());
            fifoFill -= hopSize;
        }
    }
}

/**
 * @brief Reset the analyzer state.
 */
void SpectralAnalyzer::reset() {
    fifoFill = 0;
    std::ranges::fill(fifoBuffer, 0.0f);
    std::ranges::fill(frequencyDomainBuffer, 0.0f);
    latestMagnitudes.clear();
}

/**
 * @brief Process a full FFT frame using the data in the FIFO buffer.
 *
 * This method applies the window function, performs the FFT, and computes
 * the magnitude for each frequency bin.
 */
void SpectralAnalyzer::processFrame() {
    /// Create a temporary frame and apply the window.
    std::vector<float> frame(fifoBuffer.begin(), fifoBuffer.begin() + fftSize);
    for (int i = 0; i < fftSize; ++i)
        frame[i] *= window[i];
    /// Copy windowed data to the frequency domain buffer.
    std::copy_n(frame.begin(), fftSize, frequencyDomainBuffer.begin());
    /// Perform an in-place FFT. This uses a real-only FFT transform.
    fft.performRealOnlyForwardTransform(frequencyDomainBuffer.data(), true);
    /// Compute magnitudes for the FFT bins.
    latestMagnitudes = computeMagnitudes();
}

/**
 * @brief Compute the magnitude spectrum from the FFT result.
 *
 * @return A vector containing the magnitude for each frequency bin.
 */
std::vector<float> SpectralAnalyzer::computeMagnitudes() const {
    std::vector<float> magnitudes(fftSize / 2);
    /// DC component
    magnitudes[0] = std::abs(frequencyDomainBuffer[0]);
    for (int i = 1; i < fftSize / 2; ++i) {
        const float real = frequencyDomainBuffer[2 * i];
        const float imag = frequencyDomainBuffer[2 * i + 1];
        magnitudes[i] = std::sqrt(real * real + imag * imag);
    }
    return magnitudes;
}
