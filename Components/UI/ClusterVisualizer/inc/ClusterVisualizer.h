#ifndef CLUSTER_VISUALIZER_H
#define CLUSTER_VISUALIZER_H

#include "GraphVerb.h"

struct ClusterParticle {
    juce::Point<float> position;
    juce::Colour baseColor;
    float radius = 10.0f;
    float energy = 0.0f;
    float angle = 0.0f;
    float orbitRadius = 80.0f;
};

class ClusterVisualizer final : public juce::Component, juce::Timer {
public:
    explicit ClusterVisualizer(GraphVerb &processorRef) :
        processor(processorRef) {
        initialiseParticles();
        startTimerHz(60); // 60fps
    }

    void paint(juce::Graphics &g) override {
        g.fillAll(juce::Colours::black);
        const auto bounds = getLocalBounds().toFloat();
        const auto center = bounds.getCentre();
        const auto maxSize =
                juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;

        for (const auto &p: particles) {
            float visEnergy = std::pow(p.energy, 0.6f); // visual bias boost
            const float size = juce::jmap(visEnergy, 0.0f, 1.0f, 5.0f, 40.0f);
            const auto color = p.baseColor.withAlpha(
                    juce::jmap(visEnergy, 0.0f, 1.0f, 0.05f, 0.9f));

            const float safeRadius =
                    juce::jmin(p.orbitRadius, maxSize - size / 2.0f);

            juce::Point<float> orbPos =
                    center + juce::Point<float>(std::cos(p.angle) * safeRadius,
                                                std::sin(p.angle) * safeRadius);

            g.setColour(color);
            g.fillEllipse(orbPos.x - size / 2.0f, orbPos.y - size / 2.0f, size,
                          size);
        }
    }

    void resized() override { updateOrbitRadius(); }

private:
    GraphVerb &processor;
    std::vector<ClusterParticle> particles;

    void initialiseParticles() {
        constexpr int numParticles = 12; // match your cluster count
        particles.clear();

        for (int i = 0; i < numParticles; ++i) {
            const float angle = juce::MathConstants<float>::twoPi *
                                static_cast<float>(i) / numParticles;
            ClusterParticle p;
            p.baseColor = juce::Colour::fromHSV(
                    static_cast<float>(i) / numParticles, 0.9f, 0.9f, 1.0f);
            p.angle = angle;
            particles.push_back(p);
        }

        updateOrbitRadius();
    }

    void updateOrbitRadius() {
        constexpr float margin = 10.0f; // ensures particles don’t hit the edges
        const float maxRadius = juce::jmin(static_cast<float>(getWidth()),
                                           static_cast<float>(getHeight())) /
                                        2.0f -
                                margin;

        for (auto &p: particles)
            p.orbitRadius =
                    maxRadius *
                    (0.8f + 0.2f * juce::Random::getSystemRandom().nextFloat());
    }

    void timerCallback() override {
        const auto &energies = processor.getClusterEnergies();
        const int count = juce::jmin(static_cast<int>(energies.size()),
                                     static_cast<int>(particles.size()));

        for (int i = 0; i < count; ++i) {
            float boosted = std::log10(1.0f + 9.0f * energies[i]); // log boost
            particles[i].energy +=
                    0.1f * (boosted - particles[i].energy); // smooth
            particles[i].angle += 0.002f + 0.001f * particles[i].energy;
        }

        repaint();
    }
};

#endif // CLUSTER_VISUALIZER_H
