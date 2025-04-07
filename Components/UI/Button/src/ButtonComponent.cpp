#include "ButtonComponent.h"

/**
 * @brief Constructor for the ButtonComponent.
 * @param stateRef Reference to the AudioProcessorValueTreeState.
 * @param paramID The ID of the parameter this button is associated with.
 * @param displayText The text to display on the button.
 */
ButtonComponent::ButtonComponent(juce::AudioProcessorValueTreeState &stateRef,
                                 juce::String paramID,
                                 const juce::String &displayText) :
    params(stateRef), parameterID(std::move(paramID)) {
    /// Ensure the parameter exists and is boolean
    if (auto *p = params.getParameter(parameterID)) {
        bypassParam = dynamic_cast<juce::AudioParameterBool *>(p);
        jassert(bypassParam != nullptr);
        params.addParameterListener(parameterID, this);
    }
    title.setText(displayText, juce::dontSendNotification);
    title.setJustificationType(juce::Justification::centred);
    title.setFont(juce::Font(juce::FontOptions(14.0f, juce::Font::bold)));
    addAndMakeVisible(title);
    setOpaque(false);
    setInterceptsMouseClicks(true, false);
}

/**
 * @brief Destructor for the ButtonComponent.
 */
ButtonComponent::~ButtonComponent() {
    params.removeParameterListener(parameterID, this);
}

/**
 * @brief Paint the button.
 * @param g The graphics context used for painting.
 */
void ButtonComponent::paint(juce::Graphics &g) {
    constexpr float borderThickness = 2.0f;
    auto localBounds = getLocalBounds();

    /// Centered Text
    title.setBounds(localBounds.removeFromTop(20));

    const juce::Rectangle bounds(
            static_cast<float>(localBounds.getX()) +
                    static_cast<float>(localBounds.getWidth()) / 4.0f,
            static_cast<float>(localBounds.getY()) +
                    static_cast<float>(localBounds.getHeight()) / 4.0f,
            static_cast<float>(localBounds.getWidth()) / 2.0f,
            static_cast<float>(localBounds.getHeight()) / 2.0f);
    const auto insetBounds = bounds.reduced(borderThickness * 0.5f);

    /// Rectangle with rounded corners
    g.setColour(isBypassed() ? juce::Colours::lightgrey
                             : juce::Colours::darkgrey);
    g.fillRoundedRectangle(insetBounds, cornerRadius);
    g.setColour(juce::Colours::black);
    g.drawRoundedRectangle(insetBounds, cornerRadius, borderThickness);

    /// Red circle indicator
    if (isBypassed()) {
        constexpr float r = 6.0f;
        const float x = bounds.getRight() - r * 2 - 4.0f;
        const float y = bounds.getBottom() - r * 2 - 4.0f;
        g.setColour(juce::Colours::red);
        g.fillEllipse(x, y, r * 2, r * 2);
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.drawEllipse(x, y, r * 2, r * 2, 1.0f);
    }
}

/**
 * @brief Handle mouse down events.
 */
void ButtonComponent::mouseUp(const juce::MouseEvent &) {
    if (bypassParam != nullptr) {
        bypassParam->beginChangeGesture();
        bypassParam->setValueNotifyingHost(bypassParam->get() ? 0.0f : 1.0f);
        bypassParam->endChangeGesture();
    }
}

/**
 * @brief Resize the button to maintain a square shape.
 */
void ButtonComponent::resized() {
    /// Maintain square shape
    const int size = std::min(getWidth(), getHeight());
    setSize(size, size);
}

/**
 * @brief Check if the button is bypassed.
 * @return True if the button is bypassed, false otherwise.
 */
bool ButtonComponent::isBypassed() const {
    return bypassParam != nullptr && bypassParam->get();
}

/**
 * @brief Called when the parameter value changes.
 */
void ButtonComponent::parameterChanged(const juce::String &, float) {
    juce::MessageManager::callAsync([this]() { repaint(); });
}
