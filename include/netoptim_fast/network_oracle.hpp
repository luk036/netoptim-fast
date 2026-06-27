/**
 * @file network_oracle.hpp
 * @brief Separation oracle for parametric network problems (fast version)
 *
 * Uses Howard's algorithm for negative cycle detection to assess feasibility
 * and generate cutting planes for use in ellipsoid or cutting-plane methods.
 */

#pragma once

#include <digraphx_fast/csr_graph.hpp>
#include <digraphx_fast/neg_cycle.hpp>
#include <optional>
#include <utility>
#include <vector>

namespace netoptim_fast {

    template <typename T> constexpr auto zeros(const T&) noexcept -> T { return T{}; }

    /**
     * @brief Separation oracle for parametric network problems
     * @details Checks feasibility by detecting negative cycles using Howard's algorithm.
     * If infeasible, returns a cutting plane (gradient, intercept) for use
     * in ellipsoid or cutting-plane methods.
     * @tparam Fn Constraint function with eval, grad, and optional update */
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
        NetworkOracle(const graph_t& graph, std::vector<double>& dist, Fn fn, size_t max_cycles = 1)
            : _graph(graph),
              _dist(dist),
              _finder(graph),
              _fn(std::move(fn)),
              _max_cycles(max_cycles) {}

        void update(double gamma) { _fn.update(gamma); }

        /** @brief Assess feasibility at point x
         * @details Builds edge weights from Fn::eval at point x, then runs Howard's
         * algorithm. If a negative cycle is found, computes a cutting plane.
         * @f[
         *     g = -\sum_{e\in C} \nabla f_e(x), \quad f = -\sum_{e\in C} f_e(x)
         * @f]
         *
         * @dot
         *   digraph oracle_fast {
         *     rankdir=LR; bgcolor="transparent";
         *     node [shape=box, style=filled, fillcolor="#d4e6f1"];
         *     xval [label="Input x", fillcolor="#a9cce3"];
         *     build [label="Build weights\nw[e] = fn.eval(e, x)"];
         *     howard [label="Howard's neg\ncycle detection", shape=diamond, fillcolor="#f9e79f"];
         *     feas [label="Return\nempty", fillcolor="#d5f5e3"];
         *     cut [label="Compute\n(g, f) from\ncycle", fillcolor="#fadbd8"];
         *     done [label="Return\n(g, f)", fillcolor="#7fb3d8"];
         *     xval -> build -> howard;
         *     howard -> feas [label="feasible", color="#27ae60"];
         *     howard -> cut [label="infeasible", color="#e74c3c"];
         *     cut -> done;
         *   }
         * @enddot
         *
         * @tparam Arr Type of gradient vector
         * @param[in] xval Point to assess
         * @return Empty if feasible, else (gradient, intercept) pair */
        template <typename Arr> auto assess_feas(const Arr& xval)
            -> std::optional<std::pair<Arr, double>> {
            // Build weight vector for current x
            auto weights = _graph.weights;  // copy base weights
            for (size_t e = 0; e < _graph.num_edges; ++e) {
                weights[e] = _fn.eval(e, xval);
            }

            bool found = false;
            std::vector<size_t> cycle_edges;

            found = _finder.howard(
                _dist, weights, [&](const auto& cycle) { cycle_edges = cycle; }, _max_cycles);

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

        template <typename Arr> auto operator()(const Arr& xvar)
            -> std::optional<std::pair<Arr, double>> {
            return assess_feas(xvar);
        }
    };

}  // namespace netoptim_fast
