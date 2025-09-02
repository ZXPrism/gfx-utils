#include <gfx-utils-core/shader.h>

#include <gfx-utils-core/logger.h>

#include <fstream>
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

	if (_Source.empty()) {
		return res;
	}

	GLuint *raw_shader_handle = new GLuint(0);
	res._ShaderHandle = std::shared_ptr<GLuint>(raw_shader_handle, [](GLuint *ptr) {
		glDeleteShader(*ptr);
		delete ptr;
	});

	switch (_ShaderType) {
	case ShaderType::VERTEX_SHADER:
		*res._ShaderHandle = glCreateShader(GL_VERTEX_SHADER);
		break;
	case ShaderType::FRAGMENT_SHADER:
		*res._ShaderHandle = glCreateShader(GL_FRAGMENT_SHADER);
		break;
	case ShaderType::COMPUTE_SHADER:
		*res._ShaderHandle = glCreateShader(GL_COMPUTE_SHADER);
		break;
	default:
		g_logger->warn("Shader::ShaderBuilder ({}): unknown shader type, renderer may not work correctly", _Name);
		return res;
	}

	const char *const shader_src = _Source.c_str();
	glShaderSource(*res._ShaderHandle, 1, &shader_src, nullptr);
	glCompileShader(*res._ShaderHandle);

	GLint compile_status;
	glGetShaderiv(*res._ShaderHandle, GL_COMPILE_STATUS, &compile_status);
	if (!compile_status) {
		static char compile_log[1024];
		glGetShaderInfoLog(*res._ShaderHandle, sizeof(compile_log), nullptr, compile_log);
		g_logger->warn("Shader::ShaderBuilder ({}): shader compilation failed:\n{}", _Name, compile_log);
		return res;
	}

	res._set_complete();

	return res;
}

GLuint Shader::_get_handle() const {
	return *_ShaderHandle;
}

}  // namespace gfxutils
