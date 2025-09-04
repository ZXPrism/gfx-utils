#include <gfx-utils-core/storage_buffer.h>

#include <gfx-utils-core/logger.h>

namespace gfxutils {

StorageBuffer::StorageBufferBuilder::StorageBufferBuilder(const std::string &name)
    : IBuilder(name) {
}

StorageBuffer::StorageBufferBuilder &StorageBuffer::StorageBufferBuilder::set_size(size_t buffer_size_bytes) {
	_BufferSizeBytes = buffer_size_bytes;
	return *this;
}

StorageBuffer StorageBuffer::StorageBufferBuilder::_build() const {
	StorageBuffer res;

	res._set_name(_Name);

	res._BufferSizeBytes = _BufferSizeBytes;

	auto *storage_buffer_raw_handle = new GLuint(0);
	res._StorageBufferHandle = std::shared_ptr<GLuint>(storage_buffer_raw_handle, [&](GLuint *ptr) {
		glDeleteBuffers(1, ptr);
		delete ptr;
	});

	glGenBuffers(1, res._StorageBufferHandle.get());
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, *res._StorageBufferHandle);
	// TODO: evaluate the usage on perf effects and consider if it's necessary to expose it
	glBufferData(GL_SHADER_STORAGE_BUFFER, _BufferSizeBytes, nullptr, GL_DYNAMIC_COPY);

	g_logger->info("StorageBuffer::StorageBufferBuilder ({}): successfully built storage buffer", _Name);

	res._set_complete();

	return res;
}

void StorageBuffer::bind(size_t binding_point) const {
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, static_cast<GLuint>(binding_point), *_StorageBufferHandle);
}

void StorageBuffer::set_data(const uint8_t *data, size_t n_bytes) {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, *_StorageBufferHandle);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, static_cast<GLsizeiptr>(n_bytes), data);
}

}  // namespace gfxutils
