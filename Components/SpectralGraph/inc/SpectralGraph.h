#ifndef SPECTRAL_GRAPH_H
#define SPECTRAL_GRAPH_H

#include <vector>
#include "GraphEdge.h"
#include "GraphNode.h"

/**
 * @brief Class to represent a spectral graph built from FFT magnitudes.
 */
class SpectralGraph {
public:
    /** Nodes in the graph. */
    std::vector<GraphNode> nodes;

    /** Edges in the graph. */
    std::vector<GraphEdge> edges;

    /**
     * @brief Builds the graph from a vector of FFT magnitudes.
     *
     * @param magnitudes FFT magnitude spectrum (size should be fftSize/2).
     * @param sampleRate The audio sample rate.
     * @param fftSize The FFT size that was used.
     */
    void buildGraph(const std::vector<float> &magnitudes, float sampleRate,
                    int fftSize);
};

#endif // SPECTRAL_GRAPH_H
