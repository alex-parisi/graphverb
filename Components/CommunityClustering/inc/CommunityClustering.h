#ifndef COMMUNITY_CLUSTERING_H
#define COMMUNITY_CLUSTERING_H

#include <vector>
#include "Centroid.h"
#include "GraphNode.h"

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
                                         int k, int maxIterations = 100);

private:
    /// TODO - use log spacing?
    /**
     *@brief Calculate the squared distance between a node and a centroid.
     */
    static float distanceSquared(const GraphNode &node,
                                 const Centroid &centroid);
};

#endif // COMMUNITY_CLUSTERING_H
