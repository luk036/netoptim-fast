#include <doctest/doctest.h>

#include <cmath>
#include <limits>
#include <utility>
#include <valarray>
#include <vector>

#include <digraphx_fast/csr_graph.hpp>
#include <netoptim_fast/optscaling_oracle.hpp>

using namespace netoptim_fast;

TEST_CASE("Test OptScalingOracle assess_optim") {
    using CSR = digraphx_fast::CSRGraph<double>;

    // 3-node graph with cost pairs
    auto builder = CSR::Builder(3);
    // Node 0 -> 1: cost pair (log22, log125)
    // Node 1 -> 0: cost pair (log125, log22)
    // Node 0 -> 2: cost pair (log10, log10)
    // Node 2 -> 0: cost pair (log10, log10)
    // Node 1 -> 2: cost pair (log10, log10)
    // Node 2 -> 1: cost pair (log10, log10)
    double log10 = std::log(10.0);
    double log22 = std::log(22.0);
    double log125 = std::log(125.0);

    // Use a simple graph where edge index maps to cost pairs
    // We'll store costs in a vector indexed by edge_idx
    builder.add_edge(0, 1, 0.0); // edge 0
    builder.add_edge(1, 0, 0.0); // edge 1
    builder.add_edge(0, 2, 0.0); // edge 2
    builder.add_edge(2, 0, 0.0); // edge 3
    builder.add_edge(1, 2, 0.0); // edge 4
    builder.add_edge(2, 1, 0.0); // edge 5
    auto graph = builder.build();

    // Cost pairs indexed by edge_idx in the order edges were added
    std::vector<std::pair<double, double>> costs = {
        {log22, log125}, // edge 0: 0->1
        {log125, log22}, // edge 1: 1->0
        {log10, log10},  // edge 2: 0->2
        {log10, log10},  // edge 3: 2->0
        {log10, log10},  // edge 4: 1->2
        {log10, log10},  // edge 5: 2->1
    };

    auto get_cost = [&costs](size_t e) -> std::pair<double, double> { return costs[e]; };

    std::valarray<double> x{log125, log10};
    std::vector<double> dist(graph.num_nodes, 0.0);

    auto oracle = OptScalingOracle(graph, dist, get_cost);
    double gamma = std::numeric_limits<double>::infinity();

    auto [cut, shrunk] = oracle.assess_optim(x, gamma);
    CHECK(shrunk);
    CHECK(gamma < std::numeric_limits<double>::infinity());
}
