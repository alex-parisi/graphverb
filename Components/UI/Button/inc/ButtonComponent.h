#ifndef BUTTON_COMPONENT_H
#define BUTTON_COMPONENT_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * @brief A custom button component for toggling a parameter.
 */
class ButtonComponent final : public juce::Component,
                              juce::AudioProcessorValueTreeState::Listener {
public:
    /**
     * @brief Constructor for the ButtonComponent.
     * @param stateRef Reference to the AudioProcessorValueTreeState.
     * @param paramID The ID of the parameter this button is associated with.
     * @param displayText The text to display on the button.
     */
    ButtonComponent(juce::AudioProcessorValueTreeState &stateRef,
                    juce::String paramID, const juce::String& displayText);

    /**
     * @brief Destructor for the ButtonComponent.
     */
    ~ButtonComponent() override;

    /**
     * @brief Paint the button.
     * @param g The graphics context used for painting.
     */
    void paint(juce::Graphics &g) override;

    /**
     * @brief Handle mouse down events.
     */
    void mouseUp(const juce::MouseEvent &) override;

    /**
     * @brief Resize the button to maintain a square shape.
     */
    void resized() override;

private:
    /** Reference to the AudioProcessorValueTreeState */
    juce::AudioProcessorValueTreeState &params;

    /** The ID of the parameter this button is associated with */
    juce::String parameterID;

    /** The title of the button */
    juce::Label title;

    /** Pointer to the bypass parameter */
    juce::AudioParameterBool *bypassParam = nullptr;

    /** The corner radius for the button */
    float cornerRadius = 10.0f;

    /**
     * @brief Check if the button is bypassed.
     * @return True if the button is bypassed, false otherwise.
     */
    bool isBypassed() const;

    /**
     * @brief Called when the parameter value changes.
     */
    void parameterChanged(const juce::String &, float) override;
};

#endif // BUTTON_COMPONENT_H
