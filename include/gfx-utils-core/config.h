#pragma once

#include <glm/glm.hpp>

namespace gfxutils {

namespace config {

constexpr int opengl_ver_major [[maybe_unused]] = 4;
constexpr int opengl_ver_minor [[maybe_unused]] = 6;
constexpr const char *glsl_ver [[maybe_unused]] = "#version 460";  // for imgui init only
constexpr float fps_lerp_coeff [[maybe_unused]] = 0.5f;

constexpr glm::vec3 world_up [[maybe_unused]]{ 0.0f, 1.0f, 0.0f };
constexpr float fovy [[maybe_unused]] = 45.0f;

}  // namespace config

}  // namespace gfxutils
