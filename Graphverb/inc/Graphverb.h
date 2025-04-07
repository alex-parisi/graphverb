#ifndef GRAPH_VERB_H
#define GRAPH_VERB_H

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include "AudioBufferQueue.h"
#include "CommunityClustering.h"
#include "CommunityReverb.h"
#include "ScopeDataCollector.h"
#include "SpectralAnalyzer.h"
#include "SpectralGraph.h"
#include "ThreadSafeQueue.h"

/**
 * @brief Audio processor for the Graphverb plugin.
 */
class Graphverb final : public juce::AudioProcessor {
public:
    /**
     * @brief Constructor for the Graphverb processor.
     */
    Graphverb();

    /**
     * @brief Destructor for the Graphverb processor.
     */
    ~Graphverb() override = default;

    /**
     * @brief Prepare the processor for playback.
     * @param sampleRate The sample rate of the audio stream.
     * @param samplesPerBlock The number of samples per block to process.
     */
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    /**
     * @brief Release any resources used by the processor.
     */
    void releaseResources() override;

    /**
     * @brief Check if the processor supports the given bus layout.
     * @param layouts The bus layout to check for support.
     * @return True if the layout is supported, false otherwise.
     */
    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    /**
     * @brief Process a block of audio and MIDI data.
     */
    void processBlock(juce::AudioBuffer<float> &buffer,
                      juce::MidiBuffer &midiMessages) override;

    /**
     * @brief Create an editor for the processor.
     * @return A pointer to the created editor.
     */
    juce::AudioProcessorEditor *createEditor() override;

    /**
     * @brief Check if the processor has an editor.
     * @return True if the processor has an editor, false otherwise.
     */
    bool hasEditor() const override { return true; }

    /**
     * @brief Get the name of the processor.
     * @return The name of the processor as a string.
     */
    const juce::String getName() const override { return "Graphverb"; }

    /**
     * @brief Check if the processor accepts MIDI input.
     * @return True if the processor accepts MIDI input, false otherwise.
     */
    bool acceptsMidi() const override { return false; }

    /**
     * @brief Check if the processor produces MIDI output.
     * @return True if the processor produces MIDI output, false otherwise.
     */
    bool producesMidi() const override { return false; }

    /**
     * @brief Get the tail length in seconds.
     * @return The tail length in seconds. For this processor, it is 0.0
     */
    double getTailLengthSeconds() const override { return 0.0; }

    /**
     * @brief Get the number of programs supported by the processor.
     * @return The number of programs supported. For this processor, it is 1.
     */
    int getNumPrograms() override { return 1; }

    /**
     * @brief Get the current program index.
     * @return The current program index. For this processor, it is always 0.
     */
    int getCurrentProgram() override { return 0; }

    /**
     * @brief Set the current program index.
     */
    void setCurrentProgram(int) override {}

    /**
     * @brief Get the name of a specific program.
     * @return The name of the program. For this processor, it returns an empty
     */
    const juce::String getProgramName(int) override { return {}; }

    /**
     * @brief Change the name of a specific program.
     */
    void changeProgramName(int, const juce::String &) override {}

    /**
     * @brief Get the state information of the processor.
     */
    void getStateInformation(juce::MemoryBlock &) override {}

    /**
     * @brief Set the state information of the processor.
     */
    void setStateInformation(const void *, int) override {}

    /**
     * @brief Get the audio buffer queue.
     * @return A reference to the audio buffer queue used for scope data
     */
    AudioBufferQueue<float> &getAudioBufferQueue() noexcept {
        return audioBufferQueue;
    }

    /**
     * @brief Gets the value tree state for the parameters.
     * @return A reference to the AudioProcessorValueTreeState object.
     */
    juce::AudioProcessorValueTreeState &getParameters() { return parameters; }

    /**
     * @brief Get the cluster energies.
     * @return A reference to the vector of cluster energies.
     */
    const std::vector<float> &getClusterEnergies() const {
        return clusterEnergies;
    }

private:
    /** Audio processor value tree state for managing parameters. */
    juce::AudioProcessorValueTreeState parameters;

    /** Spectral analyzer for performing the STFT */
    SpectralAnalyzer spectralAnalyzer;

    /** Spectral graph for storing the graph structure */
    SpectralGraph spectralGraph;

    /** Community clustering algorithm for clustering nodes */
    CommunityClustering clustering;

    /** Vector to store the average energy of each cluster */
    std::vector<float> clusterEnergies;

    /** Community reverb instances for each cluster */
    std::vector<std::unique_ptr<CommunityReverb>> communityReverbs;

    /** Buffer for visualizing audio data. */
    AudioBufferQueue<float> audioBufferQueue{};

    /** Scope collector for visualizing audio data. */
    ScopeDataCollector<float> scopeDataCollector{audioBufferQueue};

    /** Thread-safe queue for passing audio data to the analysis thread. */
    ThreadSafeQueue<float> analysisInputQueue;

    /** Thread for performing spectral analysis */
    std::thread analysisThread;

    /** Flag to indicate if the analysis thread should exit */
    std::atomic<bool> threadShouldExit;

    /** The latest cluster energies from the analysis thread */
    std::vector<float> latestClusterEnergies;

    /** Mutex for synchronizing access to the cluster energies */
    std::mutex energyMutex;

    /**
     * @brief Create the parameter layout for the processor.
     * @return The parameter layout for the processor.
     */
    static juce::AudioProcessorValueTreeState::ParameterLayout
    createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Graphverb)
};

#endif // GRAPH_VERB_H
