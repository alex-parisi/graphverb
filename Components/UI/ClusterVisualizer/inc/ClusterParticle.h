#ifndef CLUSTER_PARTICLE_H
#define CLUSTER_PARTICLE_H

#include <juce_gui_basics/juce_gui_basics.h>

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

#endif //CLUSTER_PARTICLE_H
