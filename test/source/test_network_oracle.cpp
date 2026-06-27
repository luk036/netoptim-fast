#include <doctest/doctest.h>

#include <cmath>
#include <digraphx_fast/csr_graph.hpp>
#include <memory>
#include <netoptim_fast/network_oracle.hpp>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace netoptim_fast;

namespace {

    struct SharedState {
        double t = 0.0;
    };

    struct MockOracle {
        std::shared_ptr<SharedState> state;
        std::vector<double> values;  // indexed by edge_idx
        std::vector<double> grads;   // indexed by edge_idx

        MockOracle() : state(std::make_shared<SharedState>()) {}

        auto eval(size_t e, double /*x*/) const -> double { return values[e]; }
        auto grad(size_t e, double /*x*/) const -> double { return grads[e]; }
        void update(double gamma) { state->t = gamma; }
    };

}  // namespace

TEST_CASE("Test NetworkOracle assess_feas no negative cycle") {
    auto builder = digraphx_fast::CSRGraph<double>::Builder(3);
    builder.add_edge(0, 1, 0.0);
    builder.add_edge(1, 2, 0.0);
    builder.add_edge(2, 0, 0.0);
    auto graph = builder.build();

    MockOracle oracle;
    oracle.values = {1.0, 1.0, 1.0};  // all positive — no negative cycle
    oracle.grads = {1.0, 1.0, 1.0};

    std::vector<double> dist(graph.num_nodes, 0.0);
    auto network = NetworkOracle(graph, dist, oracle);

    double x = 0.0;
    auto cut = network.assess_feas(x);
    CHECK_FALSE(cut.has_value());
}

TEST_CASE("Test NetworkOracle assess_feas with negative cycle") {
    auto builder = digraphx_fast::CSRGraph<double>::Builder(3);
    builder.add_edge(0, 1, 0.0);
    builder.add_edge(1, 2, 0.0);
    builder.add_edge(2, 0, 0.0);
    auto graph = builder.build();

    MockOracle oracle;
    oracle.values = {1.0, 1.0, -3.0};  // 1+1-3 = -1 < 0 → negative cycle
    oracle.grads = {1.0, 1.0, -1.0};

    std::vector<double> dist(graph.num_nodes, 0.0);
    auto network = NetworkOracle(graph, dist, oracle);

    double x = 0.0;
    auto cut = network.assess_feas(x);
    CHECK(cut.has_value());

    auto& [g, f] = *cut;
    // CSR edges: 0->1, 1->2, 2->0
    // Cycle found depends on iteration, verify a cut is returned
    CHECK(f > 0.0);
}

TEST_CASE("Test NetworkOracle update") {
    auto builder = digraphx_fast::CSRGraph<double>::Builder(3);
    builder.add_edge(0, 1, 0.0);
    builder.add_edge(1, 2, 0.0);
    builder.add_edge(2, 0, 0.0);
    auto graph = builder.build();

    MockOracle oracle;
    oracle.values = {1.0, 1.0, 1.0};
    oracle.grads = {1.0, 1.0, 1.0};

    std::vector<double> dist(graph.num_nodes, 0.0);
    auto network = NetworkOracle(graph, dist, oracle);

    network.update(42.0);
    CHECK_EQ(oracle.state->t, 42.0);
}

TEST_CASE("Test NetworkOracle max_cycles default") {
    // Graph with one negative cycle
    auto builder = digraphx_fast::CSRGraph<double>::Builder(3);
    builder.add_edge(0, 1, 0.0);
    builder.add_edge(1, 2, 0.0);
    builder.add_edge(2, 0, -3.0);
    auto graph = builder.build();

    MockOracle oracle;
    oracle.values = {1.0, 1.0, -3.0};
    oracle.grads = {0.0, 0.0, 0.0};

    std::vector<double> dist(graph.num_nodes, 0.0);
    auto network = NetworkOracle(graph, dist, oracle);

    double x = 0.0;
    auto cut = network.assess_feas(x);
    // Default max_cycles=1 should find one cut
    CHECK(cut.has_value());
}
