#pragma once

#include <gfx-utils-core/interfaces/build_target.h>
#include <gfx-utils-core/interfaces/builder.h>

#include <glad/glad.h>

#include <memory>
#include <vector>

namespace gfxutils {

class VertexBuffer : public IBuildTarget<VertexBuffer> {
private:
	GLuint _VAO;
	GLuint _VBO;

public:
	class VertexBufferBuilder : public IBuilder<VertexBufferBuilder, VertexBuffer> {
	private:
		const std::vector<float> &_Data;
		std::vector<size_t> _AttrSizes;

	public:
		VertexBufferBuilder(const std::string &name, const std::vector<float> &data);

		VertexBufferBuilder &add_attribute(size_t n_bytes);

		[[nodiscard]] VertexBuffer _build() const;
	};

	void use() const;
};

}  // namespace gfxutils
