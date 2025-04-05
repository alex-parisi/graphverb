#include "GraphVerb.h"
#include "GraphVerbEditor.h"

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
