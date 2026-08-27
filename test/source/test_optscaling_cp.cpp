#include <doctest/doctest.h>

#include <cmath>
#include <cstdint>
#include <digraphx_fast/csr_graph.hpp>
#include <limits>
#include <netoptim_fast/optscaling_oracle.hpp>
#include <utility>
#include <valarray>
#include <vector>

#include <ellalgo/cutting_plane.hpp>  // for cutting_plane_optim
#include <ellalgo/ell.hpp>            // for Ell

namespace {

    using CSR = digraphx_fast::CSRGraph<double>;

    // Mirrors py/netoptim/tests/test_optscaling.py::create_fixed_graph.
    // 17 edges, grouped by source node (CSR insertion order):
    //   node 0: ->2, ->3, ->4   node 1: ->1, ->2, ->3, ->4
    //   node 2: ->0, ->1, ->2   node 3: ->0, ->1, ->4
    //   node 4: ->0, ->1, ->3, ->4
    auto create_fixed_graph() -> std::pair<CSR, std::vector<std::pair<double, double>>> {
        const auto log10 = std::log(10.0);
        const auto log11 = std::log(11.0);
        const auto log12 = std::log(12.0);
        const auto log13 = std::log(13.0);
        const auto log14 = std::log(14.0);
        const auto log15 = std::log(15.0);
        const auto log16 = std::log(16.0);
        const auto log17 = std::log(17.0);
        const auto log18 = std::log(18.0);
        const auto log19 = std::log(19.0);
        const auto log20 = std::log(20.0);
        const auto log21 = std::log(21.0);
        const auto log22 = std::log(22.0);
        const auto log23 = std::log(23.0);
        const auto log24 = std::log(24.0);
        const auto log125 = std::log(125.0);

        auto builder = CSR::Builder(5);
        builder.add_edge(0, 2, 0.0);  // edge 0
        builder.add_edge(0, 3, 0.0);  // edge 1
        builder.add_edge(0, 4, 0.0);  // edge 2
        builder.add_edge(1, 1, 0.0);  // edge 3
        builder.add_edge(1, 2, 0.0);  // edge 4
        builder.add_edge(1, 3, 0.0);  // edge 5
        builder.add_edge(1, 4, 0.0);  // edge 6
        builder.add_edge(2, 0, 0.0);  // edge 7
        builder.add_edge(2, 1, 0.0);  // edge 8
        builder.add_edge(2, 2, 0.0);  // edge 9
        builder.add_edge(3, 0, 0.0);  // edge 10
        builder.add_edge(3, 1, 0.0);  // edge 11
        builder.add_edge(3, 4, 0.0);  // edge 12
        builder.add_edge(4, 0, 0.0);  // edge 13
        builder.add_edge(4, 1, 0.0);  // edge 14
        builder.add_edge(4, 3, 0.0);  // edge 15
        builder.add_edge(4, 4, 0.0);  // edge 16

        std::vector<std::pair<double, double>> costs = {
            {log22, log125},  // edge 0: 0->2
            {log16, log18},   // edge 1: 0->3
            {log15, log11},   // edge 2: 0->4
            {log10, log10},   // edge 3: 1->1
            {log20, log19},   // edge 4: 1->2
            {log14, log12},   // edge 5: 1->3
            {100.0, log21},   // edge 6: 1->4
            {log125, log22},  // edge 7: 2->0
            {log19, log20},   // edge 8: 2->1
            {log13, log13},   // edge 9: 2->2
            {log18, log16},   // edge 10: 3->0
            {log12, log14},   // edge 11: 3->1
            {log24, log23},   // edge 12: 3->4
            {log11, log15},   // edge 13: 4->0
            {log21, -100.0},  // edge 14: 4->1
            {log23, log24},   // edge 15: 4->3
            {log17, log17},   // edge 16: 4->4
        };
        return {builder.build(), costs};
    }

}  // namespace

TEST_CASE("OptScalingOracle fixed graph via cutting_plane_optim") {
    auto [graph, costs] = create_fixed_graph();
    auto get_cost = [&costs](size_t e) -> std::pair<double, double> { return costs[e]; };

    // Mirrors py/netoptim/tests/test_optscaling.py::test_optscaling (fixed graph branch):
    // xinit = [cmax, cmin] = [log(125), log(10)], t = cmax - cmin,
    // ellip = Ell(200 * t, xinit), gamma = inf
    const auto log10 = std::log(10.0);
    const auto log125 = std::log(125.0);

    const auto xinit = std::valarray<double>{log125, log10};
    const auto t = log125 - log10;
    auto ellip = Ell{200.0 * t, xinit};

    std::vector<double> dist(graph.num_nodes, 0.0);
    auto omega = netoptim_fast::OptScalingOracle(graph, dist, get_cost);

    auto gamma = std::numeric_limits<double>::infinity();
    const auto [xbest, num_iters] = cutting_plane_optim(omega, ellip, gamma);

    REQUIRE_NE(xbest.size(), 0U);
    CHECK_LE(num_iters, 2000U);
}
