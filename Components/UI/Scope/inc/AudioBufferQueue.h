#ifndef AUDIO_BUFFER_QUEUE_H
#define AUDIO_BUFFER_QUEUE_H

#include <array>
#include <juce_audio_basics/juce_audio_basics.h>

/**
 * @brief AudioBufferQueue class
 * @tparam T Type of the audio samples (e.g., float, double)
 */
template<typename T>
class AudioBufferQueue {
public:
    /** Order of the FFT */
    static constexpr size_t order = 9;

    /** Size of the buffer */
    static constexpr size_t bufferSize = 1U << order;

    /** Number of buffers */
    static constexpr size_t numBuffers = 10;

    /**
     * @brief Push data into the queue.
     * @param dataToPush Pointer to the data to be pushed into the queue.
     * @param numSamples Number of samples to push into the queue.
     */
    void push(const T *dataToPush, const size_t numSamples) {
        jassert(numSamples <= bufferSize);
        int start1, size1, start2, size2;
        abstractFifo.prepareToWrite(1, start1, size1, start2, size2);
        jassert(size1 <= 1);
        jassert(size2 == 0);
        if (size1 > 0)
            juce::FloatVectorOperations::copy(
                    buffers[static_cast<size_t>(start1)].data(), dataToPush,
                    static_cast<int>(juce::jmin(bufferSize, numSamples)));
        abstractFifo.finishedWrite(size1);
    }

    /**
     * @brief Pop data from the queue.
     * @param outputBuffer Pointer to the buffer where the popped data will be
     * stored.
     */
    void pop(T *outputBuffer) {
        int start1, size1, start2, size2;
        abstractFifo.prepareToRead(1, start1, size1, start2, size2);
        jassert(size1 <= 1);
        jassert(size2 == 0);
        if (size1 > 0)
            juce::FloatVectorOperations::copy(
                    outputBuffer, buffers[static_cast<size_t>(start1)].data(),
                    static_cast<int>(bufferSize));
        abstractFifo.finishedRead(size1);
    }

private:
    /** FIFO implementation to manage the buffers */
    juce::AbstractFifo abstractFifo{numBuffers};

    /** Buffers to hold the audio samples */
    std::array<std::array<T, bufferSize>, numBuffers> buffers;
};

#endif // AUDIO_BUFFER_QUEUE_H
