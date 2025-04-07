#ifndef COMMUNITY_REVERB_H
#define COMMUNITY_REVERB_H

#include <juce_dsp/juce_dsp.h>

/**
 * @brief CommunityReverb class for applying a reverb effect based on community
 * clustering.
 */
struct CommunityReverb {
    /** Reverb processor from JUCE DSP module. */
    juce::dsp::Reverb reverb;

    /** Reverb parameters. */
    juce::dsp::Reverb::Parameters params;

    /**
     * @brief Constructor for the CommunityReverb class.
     */
    CommunityReverb() {
        /// Initialize with some default reverb parameters.
        params.roomSize = 0.5f;
        params.damping = 0.5f;
        params.wetLevel = 0.33f;
        params.dryLevel = 0.4f;
        params.width = 1.0f;
        params.freezeMode = 0.0f;
        reverb.setParameters(params);
    }

    /**
     * @brief Update reverb parameters based on the average energy of the
     * community.
     *
     * @param avgEnergy Average magnitude (energy) for the community.
     * @param inverted If true, the parameters are inverted.
     * @param intensity Intensity of the reverb effect.
     */
    void updateParameters(const float avgEnergy, const bool inverted,
                          const float intensity) {
        /// TODO - new plugin with N bandpass filters that get their own reverbs
        if (inverted) {
            params.roomSize = juce::jlimit(0.0f, 1.0f, avgEnergy);
        } else {
            params.roomSize = juce::jlimit(0.0f, 1.0f, 1.0f - avgEnergy);
        }
        params.wetLevel = juce::jlimit(0.0f, 1.0f, intensity);
        reverb.setParameters(params);
    }

    /**
     * @brief Process the provided audio block through the reverb.
     *
     * @tparam SampleType The type of the audio sample.
     * @param buffer The audio buffer to process.
     */
    template<typename SampleType>
    void processBlock(juce::AudioBuffer<SampleType> &buffer) {
        juce::dsp::AudioBlock<SampleType> block(buffer);
        juce::dsp::ProcessContextReplacing<SampleType> context(block);
        reverb.process(context);
    }
};

#endif // COMMUNITY_REVERB_H
