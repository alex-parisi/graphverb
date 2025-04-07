#include "CommunityClustering.h"
#include <random>
#include <vector>

/**
 * @brief Cluster the nodes into k communities using a simple k-means
 * algorithm.
 *
 * @param nodes Vector of GraphNode from your spectral graph.
 * @param k Number of clusters (communities) to form.
 * @param maxIterations Maximum iterations for convergence.
 * @return A vector of cluster assignments corresponding to each node.
 */
std::vector<int>
CommunityClustering::clusterNodes(const std::vector<GraphNode> &nodes,
                                  const int k, const int maxIterations) {
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
                std::random_device dev;
                std::mt19937 rng(dev());
                std::uniform_int_distribution dist(0, n - 1);
                const int idx = dist(rng);
                newCentroids[j] = {nodes[idx].frequency, nodes[idx].magnitude};
            }
        }

        centroids = newCentroids;
        iterations++;
    }

    return assignments;
}

/**
 *@brief Calculate the squared distance between a node and a centroid.
 */
float CommunityClustering::distanceSquared(const GraphNode &node,
                                           const Centroid &centroid) {
    const float logF_node = std::log(node.frequency + 1e-6f);
    const float logF_centroid = std::log(centroid.frequency + 1e-6f);

    const float dB_node = 20.0f * std::log10(node.magnitude + 1e-6f);
    const float dB_centroid = 20.0f * std::log10(centroid.magnitude + 1e-6f);

    const float df = logF_node - logF_centroid;
    const float dm = dB_node - dB_centroid;

    return df * df + dm * dm;
}
