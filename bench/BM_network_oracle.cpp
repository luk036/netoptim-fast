#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <cstdint>
#include <digraphx_fast/csr_graph.hpp>
#include <netoptim_fast/network_oracle.hpp>
#include <string>
#include <utility>
#include <vector>

using namespace netoptim_fast;
using digraphx_fast::CSRGraph;

struct MockOracle {
    std::vector<double> values;
    auto eval(size_t e, double) const -> double { return values[e]; }
    auto grad(size_t, double) const -> double { return 0.0; }
    void update(double) {}
};

static auto build_graph(size_t n_nodes, int k = 3) -> CSRGraph<double> {
    auto builder = CSRGraph<double>::Builder(static_cast<uint32_t>(n_nodes));
    for (size_t i = 0; i < n_nodes; ++i) {
        for (int d = 1; d <= k; ++d) {
            auto j = (i + static_cast<size_t>(d)) % n_nodes;
            double w = static_cast<double>(((i + 1) * 7 + (j + 1) * 13) % 100 + 1);
            builder.add_edge(static_cast<uint32_t>(i), static_cast<uint32_t>(j), w);
        }
    }
    if (n_nodes > 2) {
        builder.add_edge(0, 1, -5.0);
        builder.add_edge(1, 2, -5.0);
        builder.add_edge(2, 0, -5.0);
    }
    return builder.build();
}

int main() {
    const size_t sizes[] = {20000, 50000, 100000, 200000, 500000, 1000000};

    for (auto n : sizes) {
        auto g = build_graph(n);
        MockOracle oracle;
        oracle.values = g.weights;
        std::vector<double> dist(g.num_nodes, 0.0);
        auto network = NetworkOracle(g, dist, oracle);
        bool found = network.assess_feas(0.0).has_value();
        std::printf("n=%-8zu edges=%-9zu cut=%-3s\n", n, g.num_edges, found ? "yes" : "no");
    }

    ankerl::nanobench::Bench bench;
    bench.title("netoptim-fast NetworkOracle assess_feas (CSR) sweep")
        .unit("op")
        .warmup(1)
        .epochs(3)
        .minEpochIterations(3);

    for (auto n : sizes) {
        auto g = build_graph(n);
        bench.run("assess_feas n=" + std::to_string(n), [&] {
            std::vector<double> d(g.num_nodes, 0.0);
            NetworkOracle net(g, d, MockOracle{g.weights});
            auto cut = net.assess_feas(0.0);
            ankerl::nanobench::doNotOptimizeAway(cut);
        });
    }
}
