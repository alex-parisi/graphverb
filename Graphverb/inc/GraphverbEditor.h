#ifndef GRAPH_VERB_EDITOR_H
#define GRAPH_VERB_EDITOR_H

#include <juce_audio_basics/juce_audio_basics.h>
#include "ClusterEnergy.h"
#include "ClusterVisualizer.h"
#include "Graphverb.h"
#include "KnobComponent.h"
#include "ButtonComponent.h"
#include "ScopeComponent.h"

/**
 * @brief Editor class for the GraphVerb processor.
 */
class GraphverbEditor final : public juce::AudioProcessorEditor, juce::Timer {
public:
    /**
     * @brief Constructor for the GraphVerbEditor.
     */
    explicit GraphverbEditor(Graphverb &);

    /**
     * @brief Destructor for the GraphVerbEditor.
     */
    ~GraphverbEditor() override = default;

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
    Graphverb &processor;

    /** Tooltip window for displaying information */
    juce::TooltipWindow tooltipWindow;

    /** Knob components for various parameters */
    KnobComponent livelinessKnob;
    KnobComponent gainKnob;
    KnobComponent intensityKnob;

    /** Button components for interactivity */
    ButtonComponent bypassButton;
    ButtonComponent expandButton;

    /** Waveform display for visualizing audio data */
    ScopeComponent<float> scope;

    /** Cluster energy view for visualizing cluster energies */
    ClusterEnergyView clusterEnergy;

    /** Cluster visualizer for displaying the graph structure */
    ClusterVisualizer clusterVisualizer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GraphverbEditor)

    /**
     * @brief Timer callback function to update the editor.
     */
    void timerCallback() override;
};

#endif // GRAPH_VERB_EDITOR_H
