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
    end
target_end()

target("example-create-window")
    set_languages("cxx20")
    set_kind("binary")
    set_default(false)
    set_warnings("all", "error", "extra", "pedantic")

    add_files("examples/create_window.cpp")
    add_deps("gfx-utils-core")

    if is_plat("windows") then
        add_cxflags("/utf-8", {force = true})
        add_cxxflags("/utf-8", {force = true})
    end

    after_build(function (target)
        os.cp(target:targetfile(), "bin/examples/")
    end)
target_end()
