#include "Graphverb.h"
#include "GraphverbEditor.h"

/**
 * @brief Constructor for the GraphVerb processor.
 */
Graphverb::Graphverb() :
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
void Graphverb::prepareToPlay(double sampleRate, int samplesPerBlock) {
    spectralAnalyzer.reset();

    threadShouldExit = false;
    analysisThread = std::thread([this, sampleRate] {
        std::vector<float> inputBuffer;
        while (!threadShouldExit.load()) {
            if (this->analysisInputQueue.pop(inputBuffer)) {
                /// Run spectral + graph + clustering
                spectralAnalyzer.pushSamples(
                        inputBuffer.data(),
                        static_cast<int>(inputBuffer.size()));
                const auto &magnitudes = spectralAnalyzer.getLatestMagnitudes();
                spectralGraph.buildGraph(magnitudes,
                                         static_cast<float>(sampleRate), 1024);
                const std::vector<int> clusterAssignments =
                        CommunityClustering::clusterNodes(spectralGraph.nodes,
                                                          12);
                std::vector newEnergies(12, 0.0f);
                std::vector clusterCounts(12, 0);
                for (size_t i = 0; i < spectralGraph.nodes.size(); ++i) {
                    const int cluster = clusterAssignments[i];
                    newEnergies[cluster] += spectralGraph.nodes[i].magnitude;
                    clusterCounts[cluster]++;
                }
                for (int i = 0; i < 12; ++i)
                    newEnergies[i] =
                            clusterCounts[i] > 0
                                    ? (newEnergies[i] /
                                       static_cast<float>(clusterCounts[i]))
                                    : 0.0f;
                /// Normalize
                if (const float energySum = std::accumulate(
                            newEnergies.begin(), newEnergies.end(), 0.0f);
                    energySum > 0.0f) {
                    for (float &e: newEnergies)
                        e /= energySum;
                }
                /// Store atomically
                {
                    std::lock_guard lock(energyMutex);
                    latestClusterEnergies = std::move(newEnergies);
                }
            } else {
                /// avoid busy loop
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        }
    });
}

/**
 * @brief Release any resources used by the processor.
 */
void Graphverb::releaseResources() {
    threadShouldExit = true;
    if (analysisThread.joinable())
        analysisThread.join();
}

/**
 * @brief Check if the processor supports the given bus layout.
 * @param layouts The bus layout to check for support.
 * @return True if the layout is supported, false otherwise.
 */
bool Graphverb::isBusesLayoutSupported(const BusesLayout &layouts) const {
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

/**
 * @brief Process a block of audio and MIDI data.
 */
void Graphverb::processBlock(juce::AudioBuffer<float> &buffer,
                             juce::MidiBuffer &) {
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    std::vector monoBuffer(numSamples, 0.0f);

    /// Stereo to mono conversion
    if (numChannels >= 2) {
        const float *left = buffer.getReadPointer(0);
        const float *right = buffer.getReadPointer(1);
        for (int i = 0; i < numSamples; ++i)
            monoBuffer[i] = 0.5f * (left[i] + right[i]);
    } else {
        const float *ch = buffer.getReadPointer(0);
        std::copy_n(ch, numSamples, monoBuffer.begin());
    }

    /// Send monoBuffer to background thread for analysis
    analysisInputQueue.push(monoBuffer);

    /// Safely copy the latest energies from background thread
    {
        std::lock_guard lock(energyMutex);
        if (!latestClusterEnergies.empty()) {
            clusterEnergies = latestClusterEnergies;
        }
    }

    /// Resize reverbs if needed
    constexpr int numClusters = 12;
    if (communityReverbs.size() != numClusters) {
        communityReverbs.clear();
        for (int i = 0; i < numClusters; ++i)
            communityReverbs.push_back(std::make_unique<CommunityReverb>());
    }
    /// Update reverb parameters
    const float intensity = *parameters.getRawParameterValue("intensity");
    for (int i = 0; i < numClusters; ++i) {
        const bool expand = *parameters.getRawParameterValue("expand") >= 0.5f;
        communityReverbs[i]->updateParameters(
                (i < clusterEnergies.size() ? clusterEnergies[i] : 0.0f),
                expand, intensity);
    }

    /// Prepare buffers
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);
    juce::AudioBuffer<float> wetBuffer;
    wetBuffer.makeCopyOf(buffer);
    wetBuffer.clear();
    juce::AudioBuffer<float> tempBuffer;
    tempBuffer.makeCopyOf(buffer);

    /// Apply per-cluster reverbs
    if (*parameters.getRawParameterValue("bypass") < 0.5f) {
        for (int i = 0; i < numClusters; ++i) {
            tempBuffer.makeCopyOf(dryBuffer);
            communityReverbs[i]->processBlock(tempBuffer);
            const float weight =
                    (i < clusterEnergies.size()) ? clusterEnergies[i] : 0.0f;

            for (int ch = 0; ch < wetBuffer.getNumChannels(); ++ch) {
                float *wet = wetBuffer.getWritePointer(ch);
                const float *temp = tempBuffer.getReadPointer(ch);
                for (int s = 0; s < wetBuffer.getNumSamples(); ++s) {
                    wet[s] += weight * temp[s];
                }
            }
        }
    } else {
        wetBuffer.makeCopyOf(dryBuffer);
    }

    /// Mix dry/wet and apply gain
    const float liveliness = *parameters.getRawParameterValue("liveliness");
    const float dryLevel = 1.0f - liveliness;
    const float gain = *parameters.getRawParameterValue("gain");
    const float dB = juce::jmap(gain, 0.0f, 1.0f, -60.0f, 12.0f);
    const float linearGain = juce::Decibels::decibelsToGain(dB);
    const float *leftOutput = buffer.getWritePointer(0);
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        float *out = buffer.getWritePointer(ch);
        const float *wet = wetBuffer.getReadPointer(ch);
        for (int s = 0; s < buffer.getNumSamples(); ++s) {
            float mixed = dryLevel * out[s] + (1.0f - dryLevel) * wet[s];
            mixed *= linearGain;
            out[s] = std::tanh(mixed);
            jassert(std::isfinite(out[s]));
        }
    }
    /// Collect signal for scope
    scopeDataCollector.process(leftOutput, static_cast<size_t>(numSamples));
}

/**
 * @brief Create the parameter layout for the processor.
 * @return The parameter layout for the processor.
 */
juce::AudioProcessorValueTreeState::ParameterLayout
Graphverb::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    /// Add the parameters
    layout.add(std::make_unique<juce::AudioParameterBool>("bypass", "Bypass",
                                                          false));
    layout.add(std::make_unique<juce::AudioParameterBool>("expand", "Expand",
                                                          false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
            "liveliness", "Liveliness", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("gain", "Gain", 0.0f,
                                                           1.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
            "intensity", "Intensity", 0.0f, 1.0f, 0.8f));
    return layout;
}

/**
 * @brief Create an editor for the processor.
 * @return A pointer to the created editor.
 */
juce::AudioProcessorEditor *Graphverb::createEditor() {
    return new GraphverbEditor(*this);
}

/**
 * @brief Factory function to create an instance of the PDrum
 * processor.
 * @return A pointer to the created processor instance.
 */
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
    return new Graphverb();
}
