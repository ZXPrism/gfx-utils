#pragma once

#include <gfx-utils-core/interfaces/build_target.h>
#include <gfx-utils-core/interfaces/builder.h>

#include <memory>
#include <string>
#include <vector>

#include <glad/glad.h>

namespace gfxutils {

class StorageBuffer : public IBuildTarget<StorageBuffer> {
private:
	std::shared_ptr<GLuint> _StorageBufferHandle;

public:
	class StorageBufferBuilder : public IBuilder<StorageBufferBuilder, StorageBuffer> {
	private:
		;

	public:
		StorageBufferBuilder(const std::string &name);

		StorageBuffer _build();
	};
};

}  // namespace gfxutils
