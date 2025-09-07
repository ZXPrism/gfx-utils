#pragma once

#include <glm/glm.hpp>

namespace gfxutils::config {

constexpr const char *version [[maybe_unused]] = "gfx-utils v0.3.0";

constexpr int opengl_ver_major [[maybe_unused]] = 4;
constexpr int opengl_ver_minor [[maybe_unused]] = 6;
constexpr float fps_lerp_coeff [[maybe_unused]] = 0.5f;

constexpr glm::vec3 world_up [[maybe_unused]]{ 0.0f, 1.0f, 0.0f };
constexpr float fovy [[maybe_unused]] = 45.0f;

}  // namespace gfxutils::config
