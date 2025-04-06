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

/**
 * @brief Audio processor for the GraphVerb plugin.
 */
class GraphVerb final : public juce::AudioProcessor {
public:
    /**
     * @brief Constructor for the GraphVerb processor.
     */
    GraphVerb() :
        AudioProcessor(BusesProperties()
                               .withInput("Input",
                                          juce::AudioChannelSet::stereo(), true)
                               .withOutput("Output",
                                           juce::AudioChannelSet::stereo(),
                                           true)),
        parameters(*this, nullptr, "PARAMETERS", createParameterLayout()),
        spectralAnalyzer(10, 512) {}

    /**
     * @brief Destructor for the GraphVerb processor.
     */
    ~GraphVerb() override = default;

    /**
     * @brief Prepare the processor for playback.
     * @param sampleRate The sample rate of the audio stream.
     * @param samplesPerBlock The number of samples per block to process.
     */
    void prepareToPlay(double sampleRate, int samplesPerBlock) override {
        spectralAnalyzer.reset();
    }

    /**
     * @brief Release any resources used by the processor.
     */
    void releaseResources() override {}

    /**
     * @brief Check if the processor supports the given bus layout.
     * @param layouts The bus layout to check for support.
     * @return True if the layout is supported, false otherwise.
     */
    bool isBusesLayoutSupported(const BusesLayout &layouts) const override {
        if (layouts.getMainOutputChannelSet() !=
                    juce::AudioChannelSet::mono() &&
            layouts.getMainOutputChannelSet() !=
                    juce::AudioChannelSet::stereo())
            return false;

        return true;
    }

    /**
     * @brief Process a block of audio and MIDI data.
     */
    void processBlock(juce::AudioBuffer<float> &buffer,
                      juce::MidiBuffer &midiMessages) override {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = buffer.getNumChannels();
        std::vector<float> monoBuffer(numSamples, 0.0f);

        // --- Stereo to Mono Conversion ---
        if (numChannels >= 2) {
            auto *left = buffer.getReadPointer(0);
            auto *right = buffer.getReadPointer(1);
            for (int i = 0; i < numSamples; ++i)
                monoBuffer[i] = 0.5f * (left[i] + right[i]);
        } else {
            auto *ch = buffer.getReadPointer(0);
            std::copy_n(ch, numSamples, monoBuffer.begin());
        }

        // --- Spectral Analysis and Graph Construction ---
        spectralAnalyzer.pushSamples(monoBuffer.data(), numSamples);
        const auto &magnitudes = spectralAnalyzer.getLatestMagnitudes();
        spectralGraph.buildGraph(magnitudes,
                                 static_cast<float>(getSampleRate()), 1 << 10);

        // --- Community Detection ---
        constexpr int numClusters = 12;
        const std::vector<int> clusterAssignments =
                CommunityClustering::clusterNodes(spectralGraph.nodes,
                                                  numClusters);

        // --- Compute Average Energy per Cluster ---
        clusterEnergies.resize(numClusters);
        std::vector<int> clusterCounts(numClusters, 0);
        for (size_t i = 0; i < spectralGraph.nodes.size(); ++i) {
            int cluster = clusterAssignments[i];
            clusterEnergies[cluster] += spectralGraph.nodes[i].magnitude;
            clusterCounts[cluster]++;
        }
        for (int i = 0; i < numClusters; ++i) {
            clusterEnergies[i] =
                    clusterCounts[i] > 0
                            ? (clusterEnergies[i] /
                               static_cast<float>(clusterCounts[i]))
                            : 0.0f;
        }

        // --- Normalize Energy (Prevent clipping) ---
        float energySum = std::accumulate(clusterEnergies.begin(),
                                          clusterEnergies.end(), 0.0f);
        if (energySum > 0.0f) {
            for (float &e: clusterEnergies)
                e /= energySum;
        }

        // --- Resize Community Reverbs if needed ---
        if (communityReverbs.size() != static_cast<size_t>(numClusters)) {
            communityReverbs.clear();
            for (int i = 0; i < numClusters; ++i)
                communityReverbs.push_back(std::make_unique<CommunityReverb>());
        }

        for (int i = 0; i < numClusters; ++i)
            communityReverbs[i]->updateParameters(clusterEnergies[i]);

        // --- Prepare Buffers ---
        juce::AudioBuffer<float> dryBuffer;
        dryBuffer.makeCopyOf(buffer); // Save pristine dry input

        juce::AudioBuffer<float> wetBuffer;
        wetBuffer.makeCopyOf(buffer);
        wetBuffer.clear(); // Start accumulation from silence

        juce::AudioBuffer<float> tempBuffer;
        tempBuffer.makeCopyOf(buffer);

        // --- Per-Cluster Reverb Processing ---
        for (int i = 0; i < numClusters; ++i) {
            tempBuffer.makeCopyOf(dryBuffer); // Always start with dry input
            communityReverbs[i]->processBlock(tempBuffer);
            float weight = clusterEnergies[i];

            for (int ch = 0; ch < wetBuffer.getNumChannels(); ++ch) {
                float *wet = wetBuffer.getWritePointer(ch);
                const float *temp = tempBuffer.getReadPointer(ch);
                for (int s = 0; s < wetBuffer.getNumSamples(); ++s) {
                    wet[s] += weight * temp[s];
                }
            }
        }

        // --- Dry/Wet Mix ---
        float dryLevel = *parameters.getRawParameterValue("dryLevel");

        auto *leftOutput = buffer.getWritePointer(0);
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            float *dry = buffer.getWritePointer(ch);
            const float *wet = wetBuffer.getReadPointer(ch);
            for (int s = 0; s < buffer.getNumSamples(); ++s) {
                float mixed = dryLevel * dry[s] + (1.0f - dryLevel) * wet[s];
                dry[s] = std::tanh(mixed); // soft clipping
                jassert(std::isfinite(dry[s]));
            }
        }

        // 👇 Collect the final signal *after* the mix is written
        scopeDataCollector.process(leftOutput,
                                   static_cast<size_t>(buffer.getNumSamples()));
    }

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
    const juce::String getName() const override { return "GraphVerb"; }

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

    /**
     * @brief Create the parameter layout for the processor.
     * @return The parameter layout for the processor.
     */
    static juce::AudioProcessorValueTreeState::ParameterLayout
    createParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;

        /// Add the parameters
        layout.add(std::make_unique<juce::AudioParameterBool>("bypass",
                                                              "Bypass", false));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
                "dryLevel", "Dry Level", 0.0f, 1.0f, 0.1f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
                "gain", "Gain", 0.0f, 1.0f, 0.5f));

        return layout;
    }


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GraphVerb)
};

#endif // GRAPH_VERB_H
