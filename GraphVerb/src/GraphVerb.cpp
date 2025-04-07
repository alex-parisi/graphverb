#include "GraphVerb.h"
#include "GraphVerbEditor.h"

/**
 * @brief Constructor for the GraphVerb processor.
 */
GraphVerb::GraphVerb() :
    AudioProcessor(
            BusesProperties()
                    .withInput("Input", juce::AudioChannelSet::stereo(), true)
                    .withOutput("Output", juce::AudioChannelSet::stereo(),
                                true)),
    parameters(*this, nullptr, "PARAMETERS", createParameterLayout()),
    spectralAnalyzer(10, 512) {}

/**
 * @brief Prepare the processor for playback.
 * @param sampleRate The sample rate of the audio stream.
 * @param samplesPerBlock The number of samples per block to process.
 */
void GraphVerb::prepareToPlay(double sampleRate, int samplesPerBlock) {
    spectralAnalyzer.reset();
}

/**
 * @brief Check if the processor supports the given bus layout.
 * @param layouts The bus layout to check for support.
 * @return True if the layout is supported, false otherwise.
 */
bool GraphVerb::isBusesLayoutSupported(const BusesLayout &layouts) const {
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

/**
 * @brief Process a block of audio and MIDI data.
 */
void GraphVerb::processBlock(juce::AudioBuffer<float> &buffer,
                             juce::MidiBuffer &midiMessages) {
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    std::vector monoBuffer(numSamples, 0.0f);

    /// Stereo to mono conversion
    if (numChannels >= 2) {
        auto *left = buffer.getReadPointer(0);
        auto *right = buffer.getReadPointer(1);
        for (int i = 0; i < numSamples; ++i)
            monoBuffer[i] = 0.5f * (left[i] + right[i]);
    } else {
        auto *ch = buffer.getReadPointer(0);
        std::copy_n(ch, numSamples, monoBuffer.begin());
    }

    /// Spectral Analysis and Graph Construction
    spectralAnalyzer.pushSamples(monoBuffer.data(), numSamples);
    const auto &magnitudes = spectralAnalyzer.getLatestMagnitudes();
    spectralGraph.buildGraph(magnitudes, static_cast<float>(getSampleRate()),
                             1 << 10);

    /// Community Detection
    /// TODO - turn this into a knob
    constexpr int numClusters = 12;
    const std::vector<int> clusterAssignments =
            CommunityClustering::clusterNodes(spectralGraph.nodes, numClusters);

    /// Compute Average Energy per Cluster
    clusterEnergies.resize(numClusters);
    std::vector clusterCounts(numClusters, 0);
    for (size_t i = 0; i < spectralGraph.nodes.size(); ++i) {
        const int cluster = clusterAssignments[i];
        clusterEnergies[cluster] += spectralGraph.nodes[i].magnitude;
        clusterCounts[cluster]++;
    }
    for (int i = 0; i < numClusters; ++i) {
        clusterEnergies[i] = clusterCounts[i] > 0
                                     ? (clusterEnergies[i] /
                                        static_cast<float>(clusterCounts[i]))
                                     : 0.0f;
    }

    /// Normalize Energy (Prevent clipping)
    const float energySum = std::accumulate(clusterEnergies.begin(),
                                            clusterEnergies.end(), 0.0f);
    if (energySum > 0.0f) {
        for (float &e: clusterEnergies)
            e /= energySum;
    }

    /// Resize Community Reverbs if needed
    if (communityReverbs.size() != static_cast<size_t>(numClusters)) {
        communityReverbs.clear();
        for (int i = 0; i < numClusters; ++i)
            communityReverbs.push_back(std::make_unique<CommunityReverb>());
    }
    for (int i = 0; i < numClusters; ++i)
        communityReverbs[i]->updateParameters(clusterEnergies[i]);

    /// Prepare buffers
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);
    juce::AudioBuffer<float> wetBuffer;
    wetBuffer.makeCopyOf(buffer);
    wetBuffer.clear();
    juce::AudioBuffer<float> tempBuffer;
    tempBuffer.makeCopyOf(buffer);

    /// Per-Cluster Reverb Processing
    for (int i = 0; i < numClusters; ++i) {
        tempBuffer.makeCopyOf(dryBuffer);
        communityReverbs[i]->processBlock(tempBuffer);
        const float weight = clusterEnergies[i];
        for (int ch = 0; ch < wetBuffer.getNumChannels(); ++ch) {
            float *wet = wetBuffer.getWritePointer(ch);
            const float *temp = tempBuffer.getReadPointer(ch);
            for (int s = 0; s < wetBuffer.getNumSamples(); ++s) {
                wet[s] += weight * temp[s];
            }
        }
    }

    /// Dry/Wet mix
    const float dryLevel = *parameters.getRawParameterValue("dryLevel");
    const auto *leftOutput = buffer.getWritePointer(0);
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        float *dry = buffer.getWritePointer(ch);
        const float *wet = wetBuffer.getReadPointer(ch);
        for (int s = 0; s < buffer.getNumSamples(); ++s) {
            const float mixed = dryLevel * dry[s] + (1.0f - dryLevel) * wet[s];
            dry[s] = std::tanh(mixed);
            jassert(std::isfinite(dry[s]));
        }
    }

    /// Collect the final signal after the mix is written
    scopeDataCollector.process(leftOutput,
                               static_cast<size_t>(buffer.getNumSamples()));
}

/**
 * @brief Create the parameter layout for the processor.
 * @return The parameter layout for the processor.
 */
juce::AudioProcessorValueTreeState::ParameterLayout
GraphVerb::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    /// Add the parameters
    layout.add(std::make_unique<juce::AudioParameterBool>("bypass", "Bypass",
                                                          false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
            "dryLevel", "Dry Level", 0.0f, 1.0f, 0.1f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("gain", "Gain", 0.0f,
                                                           1.0f, 0.5f));
    return layout;
}

/**
 * @brief Create an editor for the processor.
 * @return A pointer to the created editor.
 */
juce::AudioProcessorEditor *GraphVerb::createEditor() {
    return new GraphVerbEditor(*this);
}

/**
 * @brief Factory function to create an instance of the PDrum
 * processor.
 * @return A pointer to the created processor instance.
 */
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
    return new GraphVerb();
}
