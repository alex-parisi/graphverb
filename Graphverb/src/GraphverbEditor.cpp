#include "GraphverbEditor.h"

/**
 * @brief Constructor for the NBandParametricEQEditor class.
 * @param p A reference to the NBandParametricEQ processor that this editor
 * is associated with.
 */
GraphverbEditor::GraphverbEditor(Graphverb &p) :
    AudioProcessorEditor(p), processor(p), scope(p.getAudioBufferQueue()),
    clusterEnergy(p), clusterVisualizer(p),
    livelinessKnob(p.getParameters(), "liveliness", "Liveliness"),
    gainKnob(p.getParameters(), "gain", "Gain"),
    intensityKnob(p.getParameters(), "intensity", "Intensity"),
    bypassButton(p.getParameters(), "bypass", "Bypass"),
    expandButton(p.getParameters(), "expand", "Expand") {

    addAndMakeVisible(livelinessKnob);
    addAndMakeVisible(gainKnob);
    addAndMakeVisible(intensityKnob);
    addAndMakeVisible(scope);
    addAndMakeVisible(clusterEnergy);
    addAndMakeVisible(clusterVisualizer);
    addAndMakeVisible(bypassButton);
    addAndMakeVisible(expandButton);

    setSize(450, 200);
    setResizable(true, true);
    setResizeLimits(450, 200, 600, 400);
    startTimerHz(60);
}

/**
 * @brief Paint the editor's background.
 * @param g The graphics context used for painting.
 */
void GraphverbEditor::paint(juce::Graphics &g) {
    g.fillAll(getLookAndFeel().findColour(
            juce::ResizableWindow::backgroundColourId));
}

/**
 * @brief Resize the editor.
 */
void GraphverbEditor::resized() {
    juce::Rectangle<int> area = getLocalBounds();

    juce::Rectangle<int> controlArea = area.removeFromBottom(100);
    livelinessKnob.setBounds(
            controlArea.removeFromLeft(controlArea.getWidth() / 5));
    intensityKnob.setBounds(
            controlArea.removeFromLeft(controlArea.getWidth() / 4));
    gainKnob.setBounds(controlArea.removeFromLeft(controlArea.getWidth() / 3));
    expandButton.setBounds(
            controlArea.removeFromLeft(controlArea.getWidth() / 2));
    bypassButton.setBounds(
            controlArea.removeFromLeft(controlArea.getWidth() / 1));

    juce::Rectangle<int> clusterArea = area;
    const juce::Rectangle<int> clusterVisualizerArea =
            clusterArea.removeFromRight(static_cast<int>(
                    static_cast<float>(clusterArea.getWidth()) * 0.75f));
    scope.setBounds(clusterVisualizerArea.reduced(10));
    clusterEnergy.setBounds(clusterVisualizerArea.reduced(10));
    clusterVisualizer.setBounds(clusterArea.reduced(10));
}

/**
 * @brief Timer callback function to update the editor.
 */
void GraphverbEditor::timerCallback() { repaint(); }
