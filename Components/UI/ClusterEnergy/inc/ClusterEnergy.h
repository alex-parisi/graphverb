#ifndef CLUSTER_ENERGY_H
#define CLUSTER_ENERGY_H

#include <juce_gui_basics/juce_gui_basics.h>
#include "GraphVerb.h"

class ClusterEnergyView final : public juce::Component, juce::Timer {
public:
    explicit ClusterEnergyView(GraphVerb &p) : processor(p) {
        startTimerHz(30);
        smoothedEnergies.resize(processor.getClusterEnergies().size(), 0.0f);
    }

    void paint(juce::Graphics &g) override {
        g.fillAll(juce::Colours::transparentBlack);
        const auto bounds = getLocalBounds().reduced(10);
        const int numClusters = static_cast<int>(smoothedEnergies.size());
        const float barWidth = static_cast<float>(bounds.getWidth()) /
                               static_cast<float>(numClusters);

        for (int i = 0; i < numClusters; ++i) {
            const float normHeight =
                    juce::jlimit(0.0f, 1.0f, smoothedEnergies[i]);
            const float barHeight =
                    static_cast<float>(bounds.getHeight()) * normHeight;
            const float barX = static_cast<float>(bounds.getX()) +
                               static_cast<float>(i) * barWidth;
            const float barY =
                    static_cast<float>(bounds.getBottom()) - barHeight;

            g.setColour(juce::Colours::aqua);
            g.fillRect(barX, barY, barWidth - 2, barHeight);
        }
    }

    void timerCallback() override {
        const auto energies = processor.getClusterEnergies();
        constexpr float alpha = 0.2f;
        if (smoothedEnergies.size() != energies.size())
            smoothedEnergies.resize(energies.size(), 0.0f);
        for (size_t i = 0; i < energies.size(); ++i)
            smoothedEnergies[i] =
                    alpha * energies[i] + (1.0f - alpha) * smoothedEnergies[i];
        repaint();
    }

private:
    GraphVerb &processor;
    std::vector<float> smoothedEnergies;
};

#endif // CLUSTER_ENERGY_H
