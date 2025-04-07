#ifndef CLUSTER_VISUALIZER_H
#define CLUSTER_VISUALIZER_H

#include "ClusterParticle.h"
#include "GraphVerb.h"

/**
 * @brief Class for visualizing clusters in the GraphVerb plugin.
 */
class ClusterVisualizer final : public juce::Component, juce::Timer {
public:
    /**
     * @brief Constructor for the ClusterVisualizer.
     * @param processorRef Reference to the GraphVerb processor.
     */
    explicit ClusterVisualizer(GraphVerb &processorRef);

    /**
     * @brief Paints the component.
     * @param g The graphics context to use for painting.
     */
    void paint(juce::Graphics &g) override;

    /**
     * @brief Resize the component and update the orbit radius of the particles.
     */
    void resized() override;

private:
    /** Reference to the GraphVerb processor */
    GraphVerb &processor;

    /** Vector of particles for the visualizer. */
    std::vector<ClusterParticle> particles;

    /**
     * @brief Initialize the particles for the visualizer.
     */
    void initialiseParticles();

    /**
     * @brief Update the orbit radius of the particles based on the component
     * size.
     */
    void updateOrbitRadius();

    /**
     * @brief Timer callback function to update the visualizer.
     */
    void timerCallback() override;
};

#endif // CLUSTER_VISUALIZER_H
