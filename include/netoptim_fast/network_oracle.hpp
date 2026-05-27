#pragma once

#include <optional>
#include <utility>
#include <vector>

#include <digraphx_fast/csr_graph.hpp>
#include <digraphx_fast/neg_cycle.hpp>

namespace netoptim_fast {

template <typename T> constexpr auto zeros(const T&) noexcept -> T { return T{}; }

/*!
 * @brief Separation oracle for parametric network problems
 *
 * Checks feasibility by detecting negative cycles using Howard's algorithm.
 * If infeasible, returns a cutting plane (gradient, intercept) for use
 * in ellipsoid or cutting-plane methods.
 *
 * The constraint: u[j] - u[i] <= h(edge, x) for all edges (i,j)
 *
 * @tparam Fn Constraint function with:
 *   - eval(edge_idx, x) -> double (edge weight at point x)
 *   - grad(edge_idx, x) -> Arr (subgradient at point x)
 *   - update(gamma) -> void (optional, for parametric updates)
 */
template <typename Fn> class NetworkOracle {
  public:
    using graph_t = digraphx_fast::CSRGraph<double>;
    using node_t = graph_t::node_t;

  private:
    const graph_t& _graph;
    std::vector<double>& _dist;
    digraphx_fast::NegCycleFinder<graph_t> _finder;
    Fn _fn;
    size_t _max_cycles;

  public:
    NetworkOracle(const graph_t& graph, std::vector<double>& dist, Fn fn,
                  size_t max_cycles = 1)
        : _graph(graph), _dist(dist), _finder(graph), _fn(std::move(fn)),
          _max_cycles(max_cycles) {}

    void update(double gamma) { _fn.update(gamma); }

    /*!
     * @brief Assess feasibility at point x
     *
     * Builds edge weights from Fn::eval at point x, then runs Howard's
     * algorithm. If a negative cycle is found, computes a cutting plane.
     *
     * @tparam Arr Type of gradient vector (e.g., vector<double>, valarray<double>)
     * @param xval Point to assess
     * @return Empty if feasible, else (gradient, intercept) pair
     */
    template <typename Arr>
    auto assess_feas(const Arr& xval) -> std::optional<std::pair<Arr, double>> {
        // Build weight vector for current x
        auto weights = _graph.weights; // copy base weights
        for (size_t e = 0; e < _graph.num_edges; ++e) {
            weights[e] = _fn.eval(e, xval);
        }

        bool found = false;
        std::vector<size_t> cycle_edges;

        found = _finder.howard(_dist, weights,
                               [&](const auto& cycle) { cycle_edges = cycle; }, _max_cycles);

        if (!found) {
            return {};
        }

        auto grad = zeros(xval);
        double fval = 0.0;
        for (auto e : cycle_edges) {
            fval -= _fn.eval(e, xval);
            grad -= _fn.grad(e, xval);
        }
        return {{std::move(grad), fval}};
    }

    template <typename Arr>
    auto operator()(const Arr& xvar) -> std::optional<std::pair<Arr, double>> {
        return assess_feas(xvar);
    }
};

} // namespace netoptim_fast
