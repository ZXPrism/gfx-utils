#include <gfx-utils-core/vertex_buffer.h>

#include <numeric>

#include <gfx-utils-core/logger.h>
#include <gfx-utils-core/resource_manager.h>

namespace gfxutils {

VertexBuffer::VertexBufferBuilder::VertexBufferBuilder(const std::string &name, const std::vector<float> &data)
    : IBuilder(name)
    , _Data(data) {
}

VertexBuffer::VertexBufferBuilder &VertexBuffer::VertexBufferBuilder::add_attribute(size_t n_bytes) {
	_AttrSizes.push_back(n_bytes);
	return *this;
}

VertexBuffer VertexBuffer::VertexBufferBuilder::_build() const {
	VertexBuffer res;

	res._set_name(_Name);

	res._VAO = ResourceManager::instance().alloc(ResourceType::VAO);
	glBindVertexArray(res._VAO);

	res._VBO = ResourceManager::instance().alloc(ResourceType::VBO);
	glBindBuffer(GL_ARRAY_BUFFER, res._VBO);
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(_Data.size() * sizeof(float)), _Data.data(), GL_STATIC_DRAW);

	size_t n_attrs = _AttrSizes.size();
	size_t stride_bytes = std::accumulate(_AttrSizes.begin(), _AttrSizes.end(), static_cast<size_t>(0)) * sizeof(float);
	for (size_t i = 0, offset_bytes = 0; i < n_attrs; i++) {
		glVertexAttribPointer(static_cast<GLuint>(i),
		                      static_cast<GLint>(_AttrSizes[i]),
		                      GL_FLOAT,
		                      GL_FALSE,
		                      static_cast<GLsizei>(stride_bytes),
		                      reinterpret_cast<const void *>(offset_bytes));
		glEnableVertexAttribArray(static_cast<GLuint>(i));
		offset_bytes += _AttrSizes[i] * sizeof(float);

		g_logger->info("VertexBuffer::VertexBufferBuilder ({}): enabled vertex attribute {} with {} components", _Name, i, _AttrSizes[i]);
	}

	glBindVertexArray(0);

	g_logger->info("VertexBuffer::VertexBufferBuilder ({}): successfully built vertex buffer", _Name);

	res._set_complete();

	return res;
}

void VertexBuffer::use() const {
	glBindVertexArray(_VAO);
}

}  // namespace gfxutils
