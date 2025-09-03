#include <gfx-utils-core/shader_program.h>

#include <gfx-utils-core/logger.h>

namespace gfxutils {

void ShaderProgram::use() const {
	glUseProgram(*_Program);
}

void ShaderProgram::set_uniform(const std::string &name, int scalar) {
	glUniform1i(get_uniform_location(name), scalar);
}

void ShaderProgram::set_uniform(const std::string &name, float scalar) {
	glUniform1f(get_uniform_location(name), scalar);
}

void ShaderProgram::set_uniform(const std::string &name, const glm::mat4 &matrix) {
	glUniformMatrix4fv(get_uniform_location(name), 1, false, &matrix[0][0]);
}

void ShaderProgram::set_uniform(const std::string &name, const glm::vec2 &vector) {
	glUniform2fv(get_uniform_location(name), 1, &vector[0]);
}

void ShaderProgram::set_uniform(const std::string &name, const glm::vec3 &vector) {
	glUniform3fv(get_uniform_location(name), 1, &vector[0]);
}

GLint ShaderProgram::get_uniform_location(const std::string &name) {
	auto iter = _MapUniformNameToLocation.find(name);
	if (iter != _MapUniformNameToLocation.end()) {
		return iter->second;
	} else {
		GLint loc = glGetUniformLocation(*_Program, name.c_str());
		_MapUniformNameToLocation[name] = loc;
		return loc;
	}
}

ShaderProgram::ShaderProgramBuilder::ShaderProgramBuilder(const std::string &name)
    : IBuilder(name) {
}

ShaderProgram::ShaderProgramBuilder &ShaderProgram::ShaderProgramBuilder::add_shader(const Shader &shader) {
	_Shaders.push_back(shader);
	return *this;
}

ShaderProgram ShaderProgram::ShaderProgramBuilder::_build() const {
	ShaderProgram res;

	res._set_name(_Name);

	GLuint *raw_program_handle = new GLuint(glCreateProgram());
	res._Program = std::shared_ptr<GLuint>(raw_program_handle, [](GLuint *ptr) {
		glDeleteProgram(*ptr);
		delete ptr;
	});

	for (const auto &shader : _Shaders) {
		if (shader.is_complete()) {
			glAttachShader(*res._Program, shader._get_handle());
		}
	}
	glLinkProgram(*res._Program);

	GLint link_status;
	glGetProgramiv(*res._Program, GL_LINK_STATUS, &link_status);
	if (!link_status) {
		static char link_log[1024];
		glGetProgramInfoLog(*res._Program, sizeof(link_log), nullptr, link_log);
		g_logger->warn("ShaderProgram::ShaderProgramBuilder ({}): program link failed:\n{}", _Name, link_log);
		return res;
	}

	g_logger->info("ShaderProgram::ShaderProgramBuilder ({}): successfully built shader program", _Name);

	res._set_complete();

	return res;
}

}  // namespace gfxutils
