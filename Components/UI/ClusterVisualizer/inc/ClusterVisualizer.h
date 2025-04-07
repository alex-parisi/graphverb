#ifndef CLUSTER_VISUALIZER_H
#define CLUSTER_VISUALIZER_H

#include "GraphVerb.h"

/**
 * @brief Struct representing a particle in the cluster visualizer.
 */
struct ClusterParticle {
    juce::Point<float> position;
    juce::Colour baseColor;
    float radius = 10.0f;
    float energy = 0.0f;
    float angle = 0.0f;
    float orbitRadius = 80.0f;
    float angularVelocity = 0.01f;
};

/**
 * @brief Class for visualizing clusters in the GraphVerb plugin.
 */
class ClusterVisualizer final : public juce::Component, juce::Timer {
public:
    /**
     * @brief Constructor for the ClusterVisualizer.
     * @param processorRef Reference to the GraphVerb processor.
     */
    explicit ClusterVisualizer(GraphVerb &processorRef) :
        processor(processorRef) {
        initialiseParticles();
        startTimerHz(60);
    }

    /**
     * @brief Paints the component.
     * @param g The graphics context to use for painting.
     */
    void paint(juce::Graphics &g) override {
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
    void resized() override { updateOrbitRadius(); }

private:
    /** Reference to the GraphVerb processor */
    GraphVerb &processor;

    /** Vector of particles for the visualizer. */
    std::vector<ClusterParticle> particles;

    /**
     * @brief Initialize the particles for the visualizer.
     */
    void initialiseParticles() {
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
    void updateOrbitRadius() {
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
    void timerCallback() override {
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
};

#endif // CLUSTER_VISUALIZER_H
