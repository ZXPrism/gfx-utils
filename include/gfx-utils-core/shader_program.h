#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <gfx-utils-core/interfaces/build_target.h>
#include <gfx-utils-core/interfaces/builder.h>
#include <gfx-utils-core/shader.h>
#include <gfx-utils-core/shader_types.h>
#include <gfx-utils-core/uniform_info.h>

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace gfxutils {

class ShaderProgram : public IBuildTarget<ShaderProgram> {
private:
	std::shared_ptr<GLuint> _Program;
	std::unordered_map<std::string, GLint> _MapUniformNameToLocation;
	std::vector<UniformInfo> _UniformInfoVec;

public:
	class ShaderProgramBuilder : public IBuilder<ShaderProgramBuilder, ShaderProgram> {
	private:
		std::vector<Shader> _Shaders;

	public:
		ShaderProgramBuilder(const std::string &name);

		ShaderProgramBuilder &add_shader(const Shader &shader);

		[[nodiscard]] ShaderProgram _build() const;
	};

	void set_uniform(const std::string &name, int scalar);
	void set_uniform(const std::string &name, float scalar);
	void set_uniform(const std::string &name, const glm::mat4 &matrix);
	void set_uniform(const std::string &name, const glm::vec2 &vector);
	void set_uniform(const std::string &name, const glm::vec3 &vector);
	GLint get_uniform_location(const std::string &name);
	std::vector<UniformInfo> get_all_uniform_info() const;

	void use() const;
};

}  // namespace gfxutils
