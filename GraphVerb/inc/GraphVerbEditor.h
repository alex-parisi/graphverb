#ifndef GRAPH_VERB_EDITOR_H
#define GRAPH_VERB_EDITOR_H

#include <juce_audio_basics/juce_audio_basics.h>
#include "ClusterEnergy.h"
#include "GraphVerb.h"
#include "KnobComponent.h"
#include "ScopeComponent.h"

/**
 * @brief Editor class for the GraphVerb processor.
 */
class GraphVerbEditor final : public juce::AudioProcessorEditor, juce::Timer {
public:
    /**
     * @brief Constructor for the GraphVerbEditor.
     */
    explicit GraphVerbEditor(GraphVerb &);

    /**
     * @brief Destructor for the GraphVerbEditor.
     */
    ~GraphVerbEditor() override = default;

    /**
     * @brief Paint the editor's background.
     */
    void paint(juce::Graphics &) override;

    /**
     * @brief Resize the editor.
     */
    void resized() override;

private:
    /** Reference to the GraphVerb processor */
    GraphVerb &processor;

    /** Tooltip window for displaying information */
    juce::TooltipWindow tooltipWindow;

    /** Knob components for various parameters */
    KnobComponent dryLevelKnob;
    KnobComponent gainKnob;

    /** Waveform display for visualizing audio data */
    ScopeComponent<float> scope;

    /** Cluster energy view for visualizing cluster energies */
    ClusterEnergyView clusterEnergy;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GraphVerbEditor)

    /**
     * @brief Timer callback function to update the editor.
     */
    void timerCallback() override;
};

#endif // GRAPH_VERB_EDITOR_H
