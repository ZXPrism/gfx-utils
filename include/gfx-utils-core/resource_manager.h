#pragma once

#include <gfx-utils-core/interfaces/singleton.h>

#include <glad/glad.h>

#include <functional>
#include <utility>
#include <vector>

namespace gfxutils {

enum class ResourceType {
	VAO,
	VBO,
	SSBO,
	TEXTURE,
	VERTEX_SHADER,
	FRAGMENT_SHADER,
	COMPUTE_SHADER,
	SHADER_PROGRAM
};

class ResourceManager : public Singleton<ResourceManager> {
private:
	std::vector<std::pair<GLuint, std::function<void()>>> _VAOList;
	std::vector<std::pair<GLuint, std::function<void()>>> _BufferList;
	std::vector<std::pair<GLuint, std::function<void()>>> _TextureList;
	std::vector<std::pair<GLuint, std::function<void()>>> _ShaderList;
	std::vector<std::pair<GLuint, std::function<void()>>> _ShaderProgramList;

public:
	// NOTE: the callback is called *right before* the resource is deleted
	[[nodiscard]] GLuint alloc(ResourceType type, const std::function<void()> callback = []() {});
	void free();
};

}  // namespace gfxutils
