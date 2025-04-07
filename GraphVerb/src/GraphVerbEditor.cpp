#include "GraphVerbEditor.h"

/**
 * @brief Constructor for the NBandParametricEQEditor class.
 * @param p A reference to the NBandParametricEQ processor that this editor
 * is associated with.
 */
GraphVerbEditor::GraphVerbEditor(GraphVerb &p) :
    AudioProcessorEditor(p), processor(p), scope(p.getAudioBufferQueue()),
    clusterEnergy(p), clusterVisualizer(p),
    livelinessKnob(p.getParameters(), "liveliness", "Liveliness"),
    gainKnob(p.getParameters(), "gain", "Gain"),
    bypassButton(p.getParameters(), "bypass", "Bypass"),
    invertButton(p.getParameters(), "invert", "Invert") {

    addAndMakeVisible(livelinessKnob);
    addAndMakeVisible(gainKnob);
    addAndMakeVisible(scope);
    addAndMakeVisible(clusterEnergy);
    addAndMakeVisible(clusterVisualizer);
    addAndMakeVisible(bypassButton);
    addAndMakeVisible(invertButton);

    setSize(300, 400);
    setResizable(true, true);
    setResizeLimits(300, 400, 1000, 600);
    startTimerHz(60);
}

/**
 * @brief Paint the editor's background.
 * @param g The graphics context used for painting.
 */
void GraphVerbEditor::paint(juce::Graphics &g) {
    g.fillAll(getLookAndFeel().findColour(
            juce::ResizableWindow::backgroundColourId));
}

/**
 * @brief Resize the editor.
 */
void GraphVerbEditor::resized() {
    juce::Rectangle<int> area = getLocalBounds();

    juce::Rectangle<int> controlArea = area.removeFromBottom(100);
    livelinessKnob.setBounds(
            controlArea.removeFromLeft(controlArea.getWidth() / 4));
    gainKnob.setBounds(controlArea.removeFromLeft(controlArea.getWidth() / 3));
    bypassButton.setBounds(
            controlArea.removeFromLeft(controlArea.getWidth() / 2));
    invertButton.setBounds(
            controlArea.removeFromLeft(controlArea.getWidth() / 1));

    const juce::Rectangle<int> scopeArea =
            area.removeFromBottom(area.getHeight() / 3);
    scope.setBounds(scopeArea.reduced(10));

    const juce::Rectangle<int> clusterArea =
            area.removeFromBottom(area.getHeight() / 2);
    clusterEnergy.setBounds(clusterArea.reduced(10));

    const juce::Rectangle<int> nodeArea = area;
    clusterVisualizer.setBounds(nodeArea.reduced(10));
}

/**
 * @brief Timer callback function to update the editor.
 */
void GraphVerbEditor::timerCallback() { repaint(); }
