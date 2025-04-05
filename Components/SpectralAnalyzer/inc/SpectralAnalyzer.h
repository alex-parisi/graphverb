#ifndef SPECTRAL_ANALYZER_H
#define SPECTRAL_ANALYZER_H

#include <algorithm>
#include <juce_dsp/juce_dsp.h>
#include <vector>

class SpectralAnalyzer {
public:
    /**
     * @brief Constructor for the SpectralAnalyzer.
     *
     * @param fftOrder The FFT order (e.g., 10 for 1024 samples).
     * @param hopSizeIn The hop size (number of samples to advance after each
     * FFT frame). A common default is fftSize/2 for 50% overlap.
     */
    explicit SpectralAnalyzer(int fftOrder, int hopSizeIn = -1);

    /**
     * @brief Push new audio samples into the analyzer.
     *
     * This method can be called from your plugin's processBlock() and will
     * handle the accumulation of samples until a full FFT frame is ready.
     *
     * @param input Pointer to incoming audio samples.
     * @param numSamples Number of samples in the input buffer.
     */
    void pushSamples(const float *input, int numSamples);

    /**
     * @brief Retrieve the magnitudes of the latest FFT frame.
     *
     * @return A const reference to a vector of magnitudes (one per frequency
     * bin).
     */
    [[nodiscard]] const std::vector<float> &getLatestMagnitudes() const {
        return latestMagnitudes;
    }

    /**
     * @brief Reset the analyzer state.
     */
    void reset();

private:
    /** FFT order (e.g., 10 for 1024 samples). */
    int fftOrder;

    /** FFT size (number of samples in the FFT frame). */
    int fftSize;

    /** Hop size (number of samples to advance after each FFT frame). */
    int hopSize;

    /** FIFO buffer for incoming audio samples. */
    int fifoFill;

    /** FFT object for performing the FFT. */
    juce::dsp::FFT fft;

    /** Window function to reduce spectral leakage. */
    std::vector<float> window;

    /** Time-domain samples buffer. */
    std::vector<float> fifoBuffer;

    /** Frequency-domain samples buffer. */
    std::vector<float> frequencyDomainBuffer;

    /** Latest magnitudes of the FFT frame. */
    std::vector<float> latestMagnitudes;

    /**
     * @brief Process a full FFT frame using the data in the FIFO buffer.
     *
     * This method applies the window function, performs the FFT, and computes
     * the magnitude for each frequency bin.
     */
    void processFrame();

    /**
     * @brief Compute the magnitude spectrum from the FFT result.
     *
     * @return A vector containing the magnitude for each frequency bin.
     */
    [[nodiscard]] std::vector<float> computeMagnitudes() const;
};

#endif // SPECTRAL_ANALYZER_H
