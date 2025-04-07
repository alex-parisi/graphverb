#ifndef CLUSTER_ENERGY_H
#define CLUSTER_ENERGY_H

#include <juce_gui_basics/juce_gui_basics.h>
#include "Graphverb.h"

/**
 * @brief A class that visualizes the cluster energies of a GraphVerb processor.
 */
class ClusterEnergyView final : public juce::Component, juce::Timer {
public:
    /**
     * @brief Constructs a ClusterEnergyView object.
     * @param p The GraphVerb processor to visualize.
     */
    explicit ClusterEnergyView(Graphverb &p);

    /**
     * @brief Paints the component.
     * @param g The graphics context to paint on.
     */
    void paint(juce::Graphics &g) override;

    /**
     * @brief Called when the timer expires.
     */
    void timerCallback() override;

private:
    /** Reference to the GraphVerb processor. */
    Graphverb &processor;

    /** Vector to hold the smoothed cluster energies. */
    std::vector<float> smoothedEnergies;
};

#endif // CLUSTER_ENERGY_H
