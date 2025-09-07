#include <gfx-utils-core/shader.h>

#include <gfx-utils-core/config.h>
#include <gfx-utils-core/logger.h>
#include <gfx-utils-core/resource_manager.h>

#include <format>
#include <fstream>
#include <regex>
#include <sstream>

namespace gfxutils {

Shader::ShaderBuilder::ShaderBuilder(const std::string &name)
    : IBuilder(name) {
}

Shader::ShaderBuilder &Shader::ShaderBuilder::set_type(ShaderType shader_type) {
	_ShaderType = shader_type;
	return *this;
}

Shader::ShaderBuilder &Shader::ShaderBuilder::set_source(const std::string &shader_source) {
	_Source = shader_source;
	return *this;
}

Shader::ShaderBuilder &Shader::ShaderBuilder::set_source_from_file(const std::string &source_file_path) {
	std::ifstream fin(source_file_path);
	if (!fin) {
		g_logger->warn("Shader::ShaderBuilder ({}): failed to load shader source file from {}", _Name, source_file_path);
		return *this;
	}

	std::stringstream ssm;
	ssm << fin.rdbuf();
	_Source = ssm.str();
	fin.close();

	return *this;
}

Shader Shader::ShaderBuilder::_build() const {
	Shader res;

	res._set_name(_Name);

	if (_Source.empty()) {
		return res;
	}

	switch (_ShaderType) {
	case ShaderType::VERTEX_SHADER:
		res._ShaderHandle = ResourceManager::instance().alloc(ResourceType::VERTEX_SHADER);
		break;
	case ShaderType::FRAGMENT_SHADER:
		res._ShaderHandle = ResourceManager::instance().alloc(ResourceType::FRAGMENT_SHADER);
		break;
	case ShaderType::COMPUTE_SHADER:
		res._ShaderHandle = ResourceManager::instance().alloc(ResourceType::COMPUTE_SHADER);
		break;
	default:
		g_logger->warn("Shader::ShaderBuilder ({}): unknown shader type, renderer may not work correctly", _Name);
		return res;
	}

	// override shader GLSL version
	std::string shader_src = _Source;
	std::regex glsl_version_pattern(R"(#version\s+(\d+)\s+core)");
	std::smatch glsl_version_match;
	if (std::regex_search(_Source, glsl_version_match, glsl_version_pattern)) {
		int actual_glsl_version = std::stoi(glsl_version_match[1].str());
		int config_glsl_version = std::stoi(std::format("{}{}0", config::opengl_ver_major, config::opengl_ver_minor));

		g_logger->info("Shader::ShaderBuilder ({}): detected GLSL version: '{}'", _Name, actual_glsl_version);

		if (config_glsl_version < actual_glsl_version) {
			g_logger->warn("Shader::ShaderBuilder ({}): config GLSL version '{}' is lower than actual requested GLSL version '{}'",
			               _Name,
			               config_glsl_version,
			               actual_glsl_version);
			g_logger->warn("Shader::ShaderBuilder ({}): for compatibility, the actual requested GLSL version is lowered to the config one; shader may not work correctly", _Name);

			std::string new_version = std::format("#version {} core", config_glsl_version);
			shader_src = std::regex_replace(_Source, glsl_version_pattern, new_version, std::regex_constants::format_first_only);
		}
	}

	const char *const shader_src_cstr = shader_src.c_str();
	glShaderSource(res._ShaderHandle, 1, &shader_src_cstr, nullptr);
	glCompileShader(res._ShaderHandle);

	GLint compile_status;
	glGetShaderiv(res._ShaderHandle, GL_COMPILE_STATUS, &compile_status);
	if (compile_status == 0) {
		static std::array<char, 1024> compile_log;
		glGetShaderInfoLog(res._ShaderHandle, sizeof(compile_log), nullptr, compile_log.data());
		g_logger->warn("Shader::ShaderBuilder ({}): shader compilation failed:\n{}", _Name, compile_log.data());
		return res;
	}

	g_logger->info("Shader::ShaderBuilder ({}): successfully built shader", _Name);

	res._set_complete();

	return res;
}

GLuint Shader::_get_handle() const {
	return _ShaderHandle;
}

}  // namespace gfxutils
