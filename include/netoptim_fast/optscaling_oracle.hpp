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
     * x = (pi, psi) in log scale. */
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
    OptScalingOracle(const digraphx_fast::CSRGraph<double>& graph,
                     std::vector<double>& dist, Fn get_cost)
        : _network(graph, dist, Ratio(std::move(get_cost))) {}

    /** @brief Assess optimality at point x
     * @details First checks feasibility via the network oracle. If feasible,
     * computes objective s = pi - psi and compares with best-so-far gamma.
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

} // namespace netoptim_fast
