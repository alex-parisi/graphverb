#ifndef SPECTRAL_GRAPH_H
#define SPECTRAL_GRAPH_H

#include <algorithm>
#include <cmath>
#include <vector>

/**
 * @brief Structure to represent a node in the spectral graph.
 */
struct GraphNode {
    int index;
    float frequency;
    float magnitude;
};

/**
 * @brief Structure to represent an edge in the spectral graph.
 */
struct GraphEdge {
    int nodeA;
    int nodeB;
    float weight;
};

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
    void buildGraph(const std::vector<float> &magnitudes,
                    const float sampleRate, const int fftSize) {
        nodes.clear();
        edges.clear();
        const int numNodes = static_cast<int>(magnitudes.size());
        /// Compute the frequency resolution: each bin covers
        /// (sampleRate/fftSize) Hz.
        const float binResolution = sampleRate / static_cast<float>(fftSize);

        /// Create nodes for each frequency bin.
        for (int i = 0; i < numNodes; ++i) {
            GraphNode node{};
            node.index = i;
            node.frequency = static_cast<float>(i) * binResolution;
            node.magnitude = magnitudes[i];
            nodes.push_back(node);
        }

        /// Create edges based on spectral proximity.
        /// Connect each node to its immediate neighbor.
        for (int i = 0; i < numNodes - 1; ++i) {
            GraphEdge edge{};
            edge.nodeA = i;
            edge.nodeB = i + 1;
            /// Weight based on similarity: lower difference means higher
            /// weight.
            const float diff =
                    std::abs(nodes[i].magnitude - nodes[i + 1].magnitude);
            edge.weight = std::exp(-diff);
            edges.push_back(edge);
        }

        /// Create edges based on harmonic relations.
        /// For each node, attempt to link to nodes that represent harmonic
        /// multiples. Here we limit harmonics to, say, 2x, 3x, and 4x the
        /// frequency.
        for (int i = 1; i < numNodes; ++i) {
            const float baseFreq = nodes[i].frequency;
            for (int h = 2; h <= 4; ++h) {
                const float targetFreq = baseFreq * static_cast<float>(h);
                /// Find the closest bin index for the target harmonic.
                if (const int targetIndex = static_cast<int>(
                            std::lround(targetFreq / binResolution));
                    targetIndex < numNodes) {
                    GraphEdge edge{};
                    edge.nodeA = i;
                    edge.nodeB = targetIndex;
                    /// Weight decays with the frequency difference from the
                    /// exact harmonic.
                    const float freqDiff =
                            std::abs(nodes[targetIndex].frequency - targetFreq);
                    edge.weight = std::exp(-freqDiff);
                    edges.push_back(edge);
                }
            }
        }

        /// 4. (Optional) Additional edges based on energy pattern similarity
        /// could be added here. This might involve maintaining a history of
        /// magnitudes for each node and computing a correlation metric between
        /// the energy envelopes of different nodes.
    }
};

#endif // SPECTRAL_GRAPH_H
