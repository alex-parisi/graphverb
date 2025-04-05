#include "GraphVerbEditor.h"

/**
 * @brief Constructor for the NBandParametricEQEditor class.
 * @param p A reference to the NBandParametricEQ processor that this editor
 * is associated with.
 */
GraphVerbEditor::GraphVerbEditor(GraphVerb &p) :
    AudioProcessorEditor(p), processor(p) {
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
    // juce::Rectangle<int> area = getLocalBounds();
}

/**
 * @brief Timer callback function to update the editor.
 */
void GraphVerbEditor::timerCallback() { repaint(); }
