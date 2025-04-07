#include "ClusterVisualizer.h"

/**
 * @brief Constructor for the ClusterVisualizer.
 * @param processorRef Reference to the GraphVerb processor.
 */
ClusterVisualizer::ClusterVisualizer(GraphVerb &processorRef) :
    processor(processorRef) {
    initialiseParticles();
    startTimerHz(60);
}

/**
 * @brief Paints the component.
 * @param g The graphics context to use for painting.
 */
void ClusterVisualizer::paint(juce::Graphics &g) {
    g.fillAll(juce::Colours::transparentBlack);
    const auto bounds = getLocalBounds().toFloat();
    const auto center = bounds.getCentre();
    const float maxSize =
            juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    for (const auto &p: particles) {
        const float visEnergy = std::pow(p.energy, 0.6f);
        const float size = juce::jmap(visEnergy, 0.0f, 1.0f, 10.0f, 40.0f);
        const auto color = p.baseColor.withAlpha(
                juce::jmap(visEnergy, 0.0f, 1.0f, 0.05f, 0.9f));
        const float safeRadius =
                juce::jmin(p.orbitRadius, maxSize - size / 2.0f);
        const juce::Point<float> orbPos =
                center + juce::Point<float>(std::cos(p.angle) * safeRadius,
                                            std::sin(p.angle) * safeRadius);
        g.setColour(color);
        g.fillEllipse(orbPos.x - size / 2.0f, orbPos.y - size / 2.0f, size,
                      size);
    }
}

/**
 * @brief Resize the component and update the orbit radius of the particles.
 */
void ClusterVisualizer::resized() { updateOrbitRadius(); }

/**
 * @brief Initialize the particles for the visualizer.
 */
void ClusterVisualizer::initialiseParticles() {
    /// TODO - use a knob value
    constexpr int numParticles = 12;
    particles.clear();
    for (int i = 0; i < numParticles; ++i) {
        const float angle = juce::MathConstants<float>::twoPi *
                            static_cast<float>(i) / numParticles;
        ClusterParticle p;
        /// Set the color
        p.baseColor = juce::Colour::fromHSV(
                static_cast<float>(i) / numParticles, 0.9f, 0.9f, 1.0f);
        p.angle = angle;
        /// Mess with the particle's speed:
        p.angularVelocity = 0.005f;

        particles.push_back(p);
    }
    updateOrbitRadius();
}

/**
 * @brief Update the orbit radius of the particles based on the component
 * size.
 */
void ClusterVisualizer::updateOrbitRadius() {
    constexpr float margin = 10.0f; /// Margin around the edges
    const float maxRadius =
            static_cast<float>(juce::jmin(getWidth(), getHeight())) / 2.0f -
            margin;
    for (auto &p: particles)
        p.orbitRadius =
                maxRadius *
                (0.8f + 0.2f * juce::Random::getSystemRandom().nextFloat());
}

/**
 * @brief Timer callback function to update the visualizer.
 */
void ClusterVisualizer::timerCallback() {
    const auto &energies = processor.getClusterEnergies();
    const int count = juce::jmin(static_cast<int>(energies.size()),
                                 static_cast<int>(particles.size()));
    for (int i = 0; i < count; ++i) {
        const float boosted = std::log10(1.0f + 9.0f * energies[i]);
        particles[i].energy += 0.1f * (boosted - particles[i].energy);
        /// Orbit at unique velocity
        particles[i].angle += particles[i].angularVelocity;
        if (particles[i].angle > juce::MathConstants<float>::twoPi)
            particles[i].angle -= juce::MathConstants<float>::twoPi;
    }
    repaint();
}
