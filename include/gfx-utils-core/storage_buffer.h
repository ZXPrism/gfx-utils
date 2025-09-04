#pragma once

#include <gfx-utils-core/interfaces/build_target.h>
#include <gfx-utils-core/interfaces/builder.h>

#include <cstdint>
#include <memory>
#include <string>

#include <glad/glad.h>

namespace gfxutils {

class StorageBuffer : public IBuildTarget<StorageBuffer> {
private:
	std::shared_ptr<GLuint> _StorageBufferHandle;
	size_t _BufferSizeBytes;

public:
	class StorageBufferBuilder : public IBuilder<StorageBufferBuilder, StorageBuffer> {
	private:
		size_t _BufferSizeBytes;

	public:
		StorageBufferBuilder(const std::string &name);

		StorageBufferBuilder &set_size(size_t buffer_size_bytes);

		[[nodiscard]] StorageBuffer _build() const;
	};

	void bind(size_t binding_point) const;
	void set_data(const uint8_t *data, size_t n_bytes);
};

}  // namespace gfxutils
