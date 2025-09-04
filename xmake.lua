set_project("gfx-utils")

add_rules("mode.debug", "mode.release")
add_requires("spdlog", "glm", "glad", "glfw", "stb")
add_requires("imgui", {configs = { glfw = true, opengl3 = true }})

target("gfx-utils-core")
    set_languages("cxx20")
    set_kind("static")
    set_warnings("all", "error", "extra", "pedantic")

    add_includedirs("include", {public = true})
    add_headerfiles("include/(gfx-utils-core/*.h)")
    add_headerfiles("include/(gfx-utils-core/interfaces/*.h)")
    add_files("src/**.cpp")
    add_packages("spdlog", "glm", "glad", "glfw", "imgui", "stb", {public = true})

    if is_plat("windows") then
        add_cxflags("/utf-8", {force = true})
        add_cxxflags("/utf-8", {force = true})
        add_cxxflags("/wd4068", {force = true}) -- for #pragma clang
    end
target_end()

function add_example(example_name, extra_pkgs)
    target("example-" .. example_name)
        set_languages("cxx20")
        set_kind("binary")
        set_default(false)
        set_warnings("all", "error", "extra", "pedantic")

        add_files("examples/" .. example_name .. ".cpp")
        add_deps("gfx-utils-core")

        if extra_pkgs then
            add_packages(table.unpack(extra_pkgs))
        end

        if is_plat("windows") then
            add_cxflags("/utf-8", {force = true})
            add_cxxflags("/utf-8", {force = true})
            add_cxxflags("/wd4068", {force = true})
        end

        after_build(function (target)
            os.cp(target:targetfile(), "bin/examples/")
            os.cp("assets", "bin/examples/")
        end)
    target_end()
end

add_example("create_window")
add_example("mrt")
add_example("compute")
add_example("texture_io")
add_example("post_processing")
