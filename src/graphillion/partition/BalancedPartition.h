#include "graphillion/setset.h"
#include "graphillion/type.h"

using weight_t = uint32_t;

namespace graphillion {

setset SearchRatioPartitions(const std::vector<edge_t> &edges,
                             const std::map<std::string, weight_t> &weight_list,
                             double ratio, uint32_t lower, uint32_t upper,
                             int k);

setset SearchWeightPartitions(
    const std::vector<edge_t> &edges,
    const std::map<std::string, weight_t> &weight_list, weight_t lower,
    weight_t upper, int k);

/**
 * @brief enumerate graph partitions with constraints of lower bound/upper bound/disparity.
 *
 * @param edges       edge set.
 * @param weight_list list of the vertex weights.
 * @param ratio       the maximam value of the allowed disparity.
 * @param lower/upper the minimum/maximum sum of the weights in
 *                    a connected component.
 * @param k           the number of connected components.
 */
setset SearchBalancedPartitions(const std::vector<edge_t> &edges,
                           const std::map<std::string, weight_t> &weight_list,
                           double ratio, weight_t lower, weight_t upper, int k);

}  // namespace graphillion
