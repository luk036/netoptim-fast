add_rules("mode.debug", "mode.release", "mode.coverage")
add_requires("doctest", {alias = "doctest"})
set_languages("c++20")

if is_mode("release") then
    set_optimize("fastest")
end

if is_plat("windows") then
    add_cxflags("/EHsc /utf-8 /W4 /WX", { force = true })
end

target("test_netoptim_fast")
    set_kind("binary")
    add_includedirs("../digraphx-fast/include", {public = true})
    add_includedirs("include", {public = true})
    add_files("test/source/*.cpp")
    add_packages("doctest")
    add_tests("default")

target("BM_network_oracle")
    set_kind("binary")
    add_includedirs("../digraphx-fast/include", {public = true})
    add_includedirs("include", {public = true})
    add_files("bench/BM_network_oracle.cpp")
