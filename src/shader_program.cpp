#include <gfx-utils-core/shader_program.h>

#include <gfx-utils-core/logger.h>
#include <gfx-utils-core/resource_manager.h>

#include <array>
#include <format>

namespace gfxutils {

void ShaderProgram::use() const {
	glUseProgram(_Program);
}

void ShaderProgram::set_uniform(const std::string &name, int scalar) {
	glUniform1i(get_uniform_location(name), scalar);
}

void ShaderProgram::set_uniform(const std::string &name, float scalar) {
	glUniform1f(get_uniform_location(name), scalar);
}

void ShaderProgram::set_uniform(const std::string &name, const glm::mat4 &matrix) {
	glUniformMatrix4fv(get_uniform_location(name), 1, GL_FALSE, &matrix[0][0]);
}

void ShaderProgram::set_uniform(const std::string &name, const glm::vec2 &vector) {
	glUniform2fv(get_uniform_location(name), 1, &vector[0]);
}

void ShaderProgram::set_uniform(const std::string &name, const glm::vec3 &vector) {
	glUniform3fv(get_uniform_location(name), 1, &vector[0]);
}

GLint ShaderProgram::get_uniform_location(const std::string &name) {
	return _MapUniformNameToLocation[name];
}

std::vector<UniformInfo> ShaderProgram::get_all_uniform_info() const {
	return _UniformInfoVec;
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

	res._Program = ResourceManager::instance().alloc(ResourceType::SHADER_PROGRAM);

	for (const auto &shader : _Shaders) {
		if (shader.is_complete()) {
			glAttachShader(res._Program, shader._get_handle());
		}
	}
	glLinkProgram(res._Program);

	GLint link_status;
	glGetProgramiv(res._Program, GL_LINK_STATUS, &link_status);
	if (link_status == 0) {
		static std::array<char, 1024> link_log;
		glGetProgramInfoLog(res._Program, sizeof(link_log), nullptr, link_log.data());
		g_logger->warn("ShaderProgram::ShaderProgramBuilder ({}): program link failed:\n{}", _Name, link_log.data());
		return res;
	}

	// detect all uniforms and cache them
	GLint n_uniforms = 0;
	glGetProgramiv(res._Program, GL_ACTIVE_UNIFORMS, &n_uniforms);

	GLint max_uniform_name_length = 0;
	glGetProgramiv(res._Program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_uniform_name_length);

	std::vector<char> uniform_name_buffer(max_uniform_name_length);
	for (GLint i = 0; i < n_uniforms; i++) {
		GLsizei uniform_name_length = 0;
		GLint uniform_size = 0;
		GLenum uniform_type = 0;
		glGetActiveUniform(res._Program, i, max_uniform_name_length, &uniform_name_length, &uniform_size, &uniform_type, uniform_name_buffer.data());

		for (GLint j = 0; j < uniform_size; j++) {
			std::string uniform_name_str(uniform_name_buffer.data(), uniform_name_length);
			if (uniform_size != 1) {
				uniform_name_str += std::format("[{}]", j);
			}
			res._MapUniformNameToLocation[uniform_name_str] = glGetUniformLocation(res._Program, uniform_name_str.c_str());

			UniformInfo uniform_info;
			uniform_info._Name = uniform_name_str;
			switch (uniform_type) {
			case GL_INT:
				uniform_info._Type = ShaderDataType::INT;
				break;
			case GL_FLOAT:
				uniform_info._Type = ShaderDataType::FLOAT;
				break;
			case GL_FLOAT_VEC2:
				uniform_info._Type = ShaderDataType::VEC2;
				break;
			case GL_FLOAT_VEC3:
				uniform_info._Type = ShaderDataType::VEC3;
				break;
			case GL_FLOAT_VEC4:
				uniform_info._Type = ShaderDataType::VEC4;
				break;
			case GL_FLOAT_MAT2:
				uniform_info._Type = ShaderDataType::MAT2;
				break;
			case GL_FLOAT_MAT3:
				uniform_info._Type = ShaderDataType::MAT3;
				break;
			case GL_FLOAT_MAT4:
				uniform_info._Type = ShaderDataType::MAT4;
				break;
			case GL_SAMPLER_2D:
				uniform_info._Type = ShaderDataType::SAMPLER_2D;
				break;
			default:  // other types will be implemented if needed
				g_logger->warn("ShaderProgram::ShaderProgramBuilder ({}): detected unsupported uniform '{}' with type '{}'", _Name, uniform_name_str, uniform_type);
				break;
			}
			res._UniformInfoVec.push_back(uniform_info);

			// TODO: format the type rather than using use raw integers
			g_logger->info("ShaderProgram::ShaderProgramBuilder ({}): detected uniform '{}' with type '{}'", _Name, uniform_name_str, uniform_type);
		}
	}

	g_logger->info("ShaderProgram::ShaderProgramBuilder ({}): successfully built shader program", _Name);

	res._set_complete();

	return res;
}

}  // namespace gfxutils
