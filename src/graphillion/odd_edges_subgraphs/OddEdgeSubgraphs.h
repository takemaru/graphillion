#ifndef GRAPHILLION_ODD_EDGE_SUBGRAPH_SPEC_H_
#define GRAPHILLION_ODD_EDGE_SUBGRAPH_SPEC_H_

namespace graphillion {
/**
 * @return A setset that represents the set of subgraphs with odd edges.
 */
setset SearchOddEdgeSubgraphs(const std::vector<edge_t> &edges);
}  // namespace graphillion

#endif
