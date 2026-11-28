add_rules("mode.debug", "mode.release", "mode.coverage")
add_requires("doctest", "fmt", "nanobench", "spdlog")

set_languages("c++20")
add_includedirs("include", "../digraphx-fast/include", "../ellalgo-cpp/include")

if is_mode("release") then
    set_optimize("fastest")
end

if is_plat("windows") then
    add_cxflags("/EHsc /utf-8 /W4 /WX /wd4702", {force = true})
end

-- ellalgo-cpp cutting-plane driver lives there
local ellalgo_dir = path.join(os.projectdir(), "../ellalgo-cpp")
local ellalgo_inc = path.join(ellalgo_dir, "include")

target("EllAlgo")
    set_kind("static")
    add_includedirs(ellalgo_inc, {public = true})
    add_files(path.join(ellalgo_dir, "source/*.cpp"))
    add_packages("fmt", "spdlog")
    set_group("Dependencies")

target("test_netoptim_fast")
    add_deps("EllAlgo")
    add_files("test/source/*.cpp")
    add_packages("doctest")
    add_tests("default")

target("BM_network_oracle")
    add_files("bench/BM_network_oracle.cpp")
    add_packages("nanobench", "fmt")
