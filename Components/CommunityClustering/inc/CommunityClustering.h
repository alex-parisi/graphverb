#ifndef COMMUNITY_CLUSTERING_H
#define COMMUNITY_CLUSTERING_H

#include <algorithm>
#include <cstdlib>
#include <vector>
#include "GraphNode.h"

/**
 * @brief A simple structure to represent a node in the graph.
 * This structure contains the frequency and magnitude of the node.
 */
struct Centroid {
    float frequency;
    float magnitude;
};

class CommunityClustering {
public:
    /**
     * @brief Cluster the nodes into k communities using a simple k-means
     * algorithm.
     *
     * @param nodes Vector of GraphNode from your spectral graph.
     * @param k Number of clusters (communities) to form.
     * @param maxIterations Maximum iterations for convergence.
     * @return A vector of cluster assignments corresponding to each node.
     */
    static std::vector<int> clusterNodes(const std::vector<GraphNode> &nodes,
                                         const int k,
                                         const int maxIterations = 100) {
        const int n = static_cast<int>(nodes.size());
        std::vector<int> assignments(n, 0);
        if (n == 0 || k <= 0)
            return assignments;
        /// Initialize centroids by picking k nodes (or using a more random
        /// approach)
        std::vector<Centroid> centroids;
        centroids.reserve(k);
        for (int i = 0; i < k; ++i) {
            const int idx = i % n;
            centroids.push_back({nodes[idx].frequency, nodes[idx].magnitude});
        }

        bool changed = true;
        int iterations = 0;
        while (changed && iterations < maxIterations) {
            changed = false;
            /// Assignment step: assign each node to the nearest centroid.
            for (int i = 0; i < n; ++i) {
                int bestCluster = 0;
                float bestDistance = distanceSquared(nodes[i], centroids[0]);
                for (int j = 1; j < k; ++j) {
                    const float d = distanceSquared(nodes[i], centroids[j]);
                    if (d < bestDistance) {
                        bestDistance = d;
                        bestCluster = j;
                    }
                }
                if (assignments[i] != bestCluster) {
                    assignments[i] = bestCluster;
                    changed = true;
                }
            }

            /// Update step: recompute centroids.
            std::vector<Centroid> newCentroids(k, {0.0f, 0.0f});
            std::vector<int> counts(k, 0);

            for (int i = 0; i < n; ++i) {
                const int cluster = assignments[i];
                newCentroids[cluster].frequency += nodes[i].frequency;
                newCentroids[cluster].magnitude += nodes[i].magnitude;
                counts[cluster]++;
            }

            for (int j = 0; j < k; ++j) {
                if (counts[j] > 0) {
                    newCentroids[j].frequency /= static_cast<float>(counts[j]);
                    newCentroids[j].magnitude /= static_cast<float>(counts[j]);
                } else {
                    /// If a centroid loses all its points, reinitialize it
                    /// randomly.
                    /// TODO - this should be a better random choice
                    const int idx = rand() % n;
                    newCentroids[j] = {nodes[idx].frequency,
                                       nodes[idx].magnitude};
                }
            }

            centroids = newCentroids;
            iterations++;
        }

        return assignments;
    }

private:
    /// TODO - use log spacing?
    static float distanceSquared(const GraphNode &node,
                                 const Centroid &centroid) {
        const float df = node.frequency - centroid.frequency;
        const float dm = node.magnitude - centroid.magnitude;
        return df * df + dm * dm;
    }
};

#endif // COMMUNITY_CLUSTERING_H
