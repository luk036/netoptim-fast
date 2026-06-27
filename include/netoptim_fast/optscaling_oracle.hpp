/**
 * @file optscaling_oracle.hpp
 * @brief Optimal Matrix Scaling oracle (Orlin & Rothblum 1985) — fast version
 *
 * Finds diagonal scaling factors minimizing pi/psi via a parametric network
 * oracle using Howard's algorithm for negative cycle detection.
 */

#pragma once

#include <algorithm>
#include <tuple>
#include <utility>
#include <valarray>
#include <vector>

#include "network_oracle.hpp"

namespace netoptim_fast {

    /**
     * @brief Optimal Matrix Scaling oracle (Orlin & Rothblum 1985)
     * @details Finds diagonal scaling factors u_i to minimize pi/psi.
     * The oracle converts this to a parametric network problem where
     * each edge (i,j) has cost pair (log|a_ij|, log|a_ji|).
     * @tparam Fn Cost function: (edge_idx) -> pair<double,double> = (a_ij, a_ji)
     */
    template <typename Fn> class OptScalingOracle {
        using Vec = std::valarray<double>;
        using Cut = std::pair<Vec, double>;

        /** @brief Ratio constraint evaluator
         * @details For each edge, computes min(pi - a_ji, a_ij - psi) where
         * x = (pi, psi) in log scale.
         * @f[
         *     f_e(x) = \min(\pi - a_{ji},\; a_{ij} - \psi), \quad x = (\pi, \psi)
         * @f]
         *
         * @dot
         *   digraph ratio_fast {
         *     bgcolor="transparent";
         *     node [shape=box, style=filled, fillcolor="#d4e6f1"];
         *     input [label="Edge idx\nx = (pi, psi)", fillcolor="#a9cce3"];
         *     compute [label="min(pi - a_ji,\na_ij - psi)"];
         *     output [label="Constraint\nvalue", fillcolor="#7fb3d8"];
         *     input -> compute -> output;
         *   }
         * @enddot */
        class Ratio {
            Fn _get_cost;

          public:
            explicit Ratio(Fn get_cost) : _get_cost(std::move(get_cost)) {}

            auto eval(size_t edge_idx, const Vec& x) const -> double {
                auto [aij, aji] = _get_cost(edge_idx);
                return std::min(x[0] - aji, aij - x[1]);
            }

            auto grad(size_t edge_idx, const Vec& x) const -> Vec {
                auto [aij, aji] = _get_cost(edge_idx);
                if (x[0] - aji < aij - x[1]) {
                    return Vec{1.0, 0.0};
                }
                return Vec{0.0, -1.0};
            }

            void update(double /*gamma*/) const {
                // no-op: ratio constraints don't depend on gamma
            }
        };

        NetworkOracle<Ratio> _network;

      public:
        OptScalingOracle(const digraphx_fast::CSRGraph<double>& graph, std::vector<double>& dist,
                         Fn get_cost)
            : _network(graph, dist, Ratio(std::move(get_cost))) {}

        /** @brief Assess optimality at point x
         * @details First checks feasibility via the network oracle. If feasible,
         * computes objective s = pi - psi and compares with best-so-far gamma.
         * @f[
         *     s = \pi - \psi, \quad f_j = s - \gamma
         * @f]
         * @dot
         *   digraph assess_optim_fast {
         *     rankdir=LR; bgcolor="transparent";
         *     node [shape=box, style=filled, fillcolor="#d4e6f1"];
         *     x [label="x = (pi, psi)", fillcolor="#a9cce3"];
         *     feas [label="Check\nfeasibility", shape=diamond, fillcolor="#f9e79f"];
         *     cut [label="Return\ncutting plane", fillcolor="#fadbd8"];
         *     obj [label="s = pi - psi\nfj = s - gamma"];
         *     improve [label="Improve:\ngamma = s", fillcolor="#d5f5e3"];
         *     opt_cut [label="Return\n(1,-1), 0", fillcolor="#7fb3d8"];
         *     done [label="Return\n(1,-1), fj", fillcolor="#7fb3d8"];
         *     x -> feas;
         *     feas -> cut [label="infeasible", color="#e74c3c"];
         *     feas -> obj [label="feasible", color="#27ae60"];
         *     obj -> improve [label="fj < 0", color="#27ae60"];
         *     obj -> done [label="fj >= 0", color="#e74c3c"];
         *     improve -> opt_cut;
         *   }
         * @enddot
         *
         * @param[in] x (pi, psi) in log scale
         * @param[in,out] gamma best-so-far optimal value (updated if improved)
         * @return (cut, shrunk) — shrunk=true if gamma was updated */
        auto assess_optim(const Vec& x, double& gamma) -> std::tuple<Cut, bool> {
            auto cut = _network.assess_feas(x);
            if (cut) {
                return {*cut, false};
            }

            double s = x[0] - x[1];
            double fj = s - gamma;
            if (fj < 0.0) {
                gamma = s;
                return {{Vec{1.0, -1.0}, 0.0}, true};
            }
            return {{Vec{1.0, -1.0}, fj}, false};
        }

        auto operator()(const Vec& x, double& gamma) -> std::tuple<Cut, bool> {
            return assess_optim(x, gamma);
        }
    };

}  // namespace netoptim_fast
