#include <netoptim_fast/version.h>

#include <iostream>

auto main() -> int {
    const auto ok = (NETOPTIMFAST_VERSION_MAJOR >= 1);
    std::cout << "netoptim_fast installed test: version " << NETOPTIMFAST_VERSION << "\n";
    return ok ? 0 : 1;
}
