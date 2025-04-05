#ifndef GRAPH_VERB_H
#define GRAPH_VERB_H

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_processors/juce_audio_processors.h>

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
        /// Retrieve the number of samples and channels.
        const int numSamples = buffer.getNumSamples();
        const int numChannels = buffer.getNumChannels();

        /// Create a temporary buffer for the mono signal.
        std::vector<float> monoBuffer(numSamples, 0.0f);

        /// If stereo, average the channels; if mono, use the existing channel.
        if (numChannels >= 2) {
            auto *leftChannel = buffer.getReadPointer(0);
            auto *rightChannel = buffer.getReadPointer(1);
            for (int i = 0; i < numSamples; ++i)
                monoBuffer[i] = 0.5f * (leftChannel[i] + rightChannel[i]);
        } else {
            /// If only one channel exists, copy it directly.
            auto *channelData = buffer.getReadPointer(0);
            std::copy_n(channelData, numSamples, monoBuffer.begin());
        }

        /// Push the combined mono samples into the spectral analyzer.
        spectralAnalyzer.pushSamples(monoBuffer.data(), numSamples);

        /// Retrieve the latest magnitude spectrum for further processing.
        const auto &magnitudes = spectralAnalyzer.getLatestMagnitudes();
        /// Use 'magnitudes' for further processing like graph construction or
        /// UI updates.
        spectralGraph.buildGraph(magnitudes,
                                 static_cast<float>(getSampleRate()), 1 << 10);

        /// Pass the audio through unchanged (or process it as needed).
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
     * @brief Gets the value tree state for the parameters.
     * @return A reference to the AudioProcessorValueTreeState object.
     */
    juce::AudioProcessorValueTreeState &getParameters() { return parameters; }

private:
    /** Audio processor value tree state for managing parameters. */
    juce::AudioProcessorValueTreeState parameters;

    /** Spectral analyzer for performing the STFT */
    SpectralAnalyzer spectralAnalyzer;

    /** Spectral graph for storing the graph structure */
    SpectralGraph spectralGraph;

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
                "dryLevel", "Dry Level", 0.0f, 1.0f, 0.5f));

        return layout;
    }


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GraphVerb)
};

#endif // GRAPH_VERB_H
