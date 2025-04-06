//
// Created by Alex Parisi on 3/28/25.
//

#ifndef SCOPE_DATA_COLLECTOR_H
#define SCOPE_DATA_COLLECTOR_H

#include <array>
#include "AudioBufferQueue.h"

/**
 * @brief ScopeDataCollector class that collects audio samples for a scope
 * component.
 * @tparam SampleType The type of audio samples to collect (e.g., float,
 * double).
 */
template<typename SampleType>
class ScopeDataCollector {
public:
    /**
     * @brief Constructor for the ScopeDataCollector.
     * @param queueToUse The audio buffer queue to use for storing collected
     * samples.
     */
    explicit ScopeDataCollector(AudioBufferQueue<SampleType> &queueToUse) :
        audioBufferQueue(queueToUse) {}

    /**
     * @brief Process a block of audio samples.
     * @param data Pointer to the audio samples to process.
     * @param numSamples The number of samples to process.
     */
    void process(const SampleType* data, size_t numSamples) {
        while (numSamples > 0) {
            const auto samplesToCopy = std::min(buffer.size() - numCollected, numSamples);
            std::copy(data, data + samplesToCopy, buffer.begin() + numCollected);

            numCollected += samplesToCopy;
            data += samplesToCopy;
            numSamples -= samplesToCopy;

            if (numCollected == buffer.size()) {
                audioBufferQueue.push(buffer.data(), buffer.size());
                numCollected = 0;
            }
        }
    }

private:
    /** Reference to the audio buffer queue where collected samples are
     * stored. */
    AudioBufferQueue<SampleType> &audioBufferQueue;

    /** Buffer to hold the collected samples. The size of the buffer is
     * determined by the bufferSize defined in the AudioBufferQueue. */
    std::array<SampleType, AudioBufferQueue<SampleType>::bufferSize> buffer;

    /** Number of samples collected in the current buffer. */
    size_t numCollected{};

    /** Previous sample value to detect trigger level crossing. */
    SampleType prevSample = SampleType(100);

    /** Trigger level for detecting the start of a new collection
     * cycle. This is set to a small positive value to avoid noise. */
    static constexpr auto triggerLevel = SampleType(0.05);

    /** State of the collector, indicating whether it is waiting for
     * a trigger or currently collecting samples. */
    enum class State {
        waitingForTrigger,
        collecting
    }

    /** Current state of the collector. It starts in the waiting
     * for trigger state. */
    state{State::waitingForTrigger};
};

#endif // SCOPE_DATA_COLLECTOR_H
