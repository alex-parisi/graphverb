#ifndef SCOPE_COMPONENT_H
#define SCOPE_COMPONENT_H

#include <array>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_graphics/juce_graphics.h>
#include "AudioBufferQueue.h"

/**
 * @brief ScopeComponent class that displays an oscilloscope and spectrum
 * analyzer.
 * @tparam SampleType The type of audio samples to display (e.g., float,
 * double).
 */
template<typename SampleType>
class ScopeComponent final : public juce::Component, juce::Timer {
public:
    /** Type alias for the audio buffer queue. */
    using Queue = AudioBufferQueue<SampleType>;

    /**
     * @brief Constructor for the ScopeComponent.
     * @param queueToUse The audio buffer queue to use for displaying samples.
     */
    explicit ScopeComponent(Queue &queueToUse) : audioBufferQueue(queueToUse) {
        sampleData.fill(SampleType(0));
        setFramesPerSecond(30);
    }

    /**
     * @brief Set the frames per second for the timer.
     * @param framesPerSecond The number of frames per second to set for the
     * timer.
     */
    void setFramesPerSecond(const int framesPerSecond) {
        jassert(framesPerSecond > 0 && framesPerSecond < 1000);
        startTimerHz(framesPerSecond);
    }

    /**
     * @brief Paint the component.
     * @param g The graphics context to use for painting.
     */
    void paint(juce::Graphics &g) override {
        g.fillAll(juce::Colours::transparentBlack);
        g.setColour(juce::Colours::white);
        const auto area = getLocalBounds();
        auto h = static_cast<SampleType>(area.getHeight());
        auto w = static_cast<SampleType>(area.getWidth());
        // Oscilloscope
        // auto scopeRect = juce::Rectangle<SampleType>{SampleType(0),
        //                                              SampleType(0), w, h / 2};
        // plot(sampleData.data(), sampleData.size(), g, scopeRect, false,
        //      SampleType(1), h / 4);
        // Spectrum
        auto spectrumRect =
                juce::Rectangle<SampleType>{SampleType(0), SampleType(0), w, h};
        plot(spectrumData.data(), spectrumData.size() / 4, g, spectrumRect,
             true);
    }

    /**
     * @brief Resized the component.
     * This function is called when the component is resized. It can be used
     * to adjust the layout of child components or perform other resizing tasks.
     */
    void resized() override {}

private:
    /** Reference to the audio buffer queue where collected samples are
     * stored. */
    Queue &audioBufferQueue;

    /** Buffer to hold the collected samples. The size of the buffer is
     * determined by the buffer size of the audio buffer queue. */
    std::array<SampleType, Queue::bufferSize> sampleData;

    /** FFT object for performing the Fast Fourier Transform. */
    juce::dsp::FFT fft{Queue::order};

    /** Windowing function for the FFT. */
    using WindowFun = juce::dsp::WindowingFunction<SampleType>;

    /** Windowing function instance. */
    WindowFun windowFun{static_cast<size_t>(fft.getSize()), WindowFun::hann};

    /** Buffer to hold the spectrum data. The size is twice the FFT size
     * because the FFT returns complex numbers (real and imaginary parts). */
    std::array<SampleType, 2 * Queue::bufferSize> spectrumData;

    std::array<SampleType, Queue::bufferSize> smoothedSpectrum;

    /**
     * @brief Timer callback function that is called at each timer tick.
     * This function processes the audio samples, performs the FFT, and
     * updates the spectrum data for display.
     */
    void timerCallback() override {
        audioBufferQueue.pop(sampleData.data());
        juce::FloatVectorOperations::copy(spectrumData.data(),
                                          sampleData.data(),
                                          static_cast<int>(sampleData.size()));
        auto fftSize = static_cast<size_t>(fft.getSize());
        jassert(spectrumData.size() == 2 * fftSize);
        windowFun.multiplyWithWindowingTable(spectrumData.data(), fftSize);
        fft.performFrequencyOnlyForwardTransform(spectrumData.data());
        static constexpr auto mindB = SampleType(-160);
        static constexpr auto maxdB = SampleType(0);
        for (size_t i = 0; i < fftSize; ++i) {
            const auto db = juce::Decibels::gainToDecibels(spectrumData[i]) -
                            juce::Decibels::gainToDecibels(SampleType(fftSize));
            const auto mapped =
                    juce::jmap(juce::jlimit(mindB, maxdB, db), mindB, maxdB,
                               SampleType(0), SampleType(1));
            // Smooth over time
            smoothedSpectrum[i] = 0.8f * smoothedSpectrum[i] + 0.2f * mapped;
        }
        std::copy(smoothedSpectrum.begin(), smoothedSpectrum.begin() + fftSize,
                  spectrumData.begin());
        repaint();
    }

    /**
     * @brief Plot the audio samples on the oscilloscope.
     * @param data Pointer to the audio samples to plot.
     * @param numSamples The number of samples to plot.
     * @param g The graphics context to use for drawing.
     * @param rect The rectangle area where the samples will be plotted.
     * @param scaler The scaler to apply to the samples for scaling the plot.
     * @param offset The offset to apply to the samples for vertical
     * positioning.
     * @param useLogX Whether to use logarithmic scaling for the x-axis.
     */
    static void plot(const SampleType *data, const size_t numSamples,
                     const juce::Graphics &g, juce::Rectangle<SampleType> rect,
                     const bool useLogX = false,
                     SampleType scaler = SampleType(1),
                     SampleType offset = SampleType(0)) {
        auto w = rect.getWidth();
        auto h = rect.getHeight();
        auto right = rect.getRight();
        auto center = rect.getBottom() - offset;
        auto gain = h * scaler;

        auto xStart = right - w;
        auto xEnd = right;

        std::function<SampleType(SampleType)> xPosition;

        if (useLogX) {
            auto minFreq = SampleType(1); // avoid log(0)
            auto maxFreq = static_cast<SampleType>(numSamples);
            auto minLog = std::log10(minFreq);
            auto maxLog = std::log10(maxFreq);

            xPosition = [&](SampleType index) {
                auto freq = juce::jmap(index, SampleType(0), maxFreq, minFreq,
                                       maxFreq);
                auto logFreq = std::log10(freq);
                return juce::jmap(logFreq, minLog, maxLog, xStart, xEnd);
            };
        } else {
            xPosition = [&](SampleType index) {
                return juce::jmap(index, SampleType(0),
                                  SampleType(numSamples - 1), xStart, xEnd);
            };
        }

        for (size_t i = 1; i < numSamples; ++i) {
            auto x1 = xPosition(static_cast<SampleType>(i - 1));
            auto x2 = xPosition(static_cast<SampleType>(i));
            auto y1 = center - gain * data[i - 1];
            auto y2 = center - gain * data[i];

            g.drawLine({x1, y1, x2, y2});
        }
    }
};

#endif // SCOPE_COMPONENT_H
