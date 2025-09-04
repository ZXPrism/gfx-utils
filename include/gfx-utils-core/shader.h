#include <memory>
#include <string>

#include <gfx-utils-core/interfaces/build_target.h>
#include <gfx-utils-core/interfaces/builder.h>
#include <gfx-utils-core/shader_types.h>

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace gfxutils {

class Shader : public IBuildTarget<Shader> {
private:
	std::shared_ptr<GLuint> _ShaderHandle;

public:
	class ShaderBuilder : public IBuilder<ShaderBuilder, Shader> {
	private:
		ShaderType _ShaderType = ShaderType::UNKNOWN;
		std::string _Source;

	public:
		ShaderBuilder(const std::string &name);

		ShaderBuilder &set_type(ShaderType shader_type);
		ShaderBuilder &set_source(const std::string &shader_source);
		ShaderBuilder &set_source_from_file(const std::string &source_file_path);

		[[nodiscard]] Shader _build() const;
	};

	[[nodiscard]] GLuint _get_handle() const;
};

}  // namespace gfxutils
