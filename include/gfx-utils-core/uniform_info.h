#pragma once

#include <gfx-utils-core/shader_types.h>

#include <string>

namespace gfxutils {

struct UniformInfo {
	std::string _Name;
	ShaderDataType _Type;
};

}  // namespace gfxutils
