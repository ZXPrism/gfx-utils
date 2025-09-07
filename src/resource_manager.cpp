#include <gfx-utils-core/resource_manager.h>

#include <gfx-utils-core/logger.h>

#include <glad/glad.h>

namespace gfxutils {

GLuint ResourceManager::alloc(ResourceType type, const std::function<void()> callback) {
	GLuint handle = 0;

	switch (type) {
	case ResourceType::VAO:
		glGenVertexArrays(1, &handle);
		_VAOList.emplace_back(handle, callback);
		g_logger->info("ResourceManager: created VAO {}", handle);
		break;

	case ResourceType::VBO:
	case ResourceType::SSBO:
		glGenBuffers(1, &handle);
		_BufferList.emplace_back(handle, callback);
		g_logger->info("ResourceManager: created buffer {}", handle);
		break;

	case ResourceType::TEXTURE:
		glGenTextures(1, &handle);
		_TextureList.emplace_back(handle, callback);
		g_logger->info("ResourceManager: created texture {}", handle);
		break;

	case ResourceType::VERTEX_SHADER:
		handle = glCreateShader(GL_VERTEX_SHADER);
		_ShaderList.emplace_back(handle, callback);
		g_logger->info("ResourceManager: created vertex shader {}", handle);
		break;

	case ResourceType::FRAGMENT_SHADER:
		handle = glCreateShader(GL_FRAGMENT_SHADER);
		_ShaderList.emplace_back(handle, callback);
		g_logger->info("ResourceManager: created fragment shader {}", handle);
		break;

	case ResourceType::COMPUTE_SHADER:
		handle = glCreateShader(GL_COMPUTE_SHADER);
		_ShaderList.emplace_back(handle, callback);
		g_logger->info("ResourceManager: created compute shader {}", handle);
		break;

	case ResourceType::SHADER_PROGRAM:
		handle = glCreateProgram();
		_ShaderProgramList.emplace_back(handle, callback);
		g_logger->info("ResourceManager: created shader program {}", handle);
		break;

	default:
		break;
	}

	return handle;
}

void ResourceManager::free() {
	// unnecessary to free in inverse order, let the driver handles the rest!
	for (const auto &[handle, callback] : _VAOList) {
		callback();
		glDeleteVertexArrays(1, &handle);
	}
	for (const auto &[handle, callback] : _BufferList) {
		callback();
		glDeleteBuffers(1, &handle);
	}
	for (const auto &[handle, callback] : _TextureList) {
		callback();
		glad_glDeleteTextures(1, &handle);
	}
	for (const auto &[handle, callback] : _ShaderList) {
		callback();
		glDeleteShader(handle);
	}
	for (const auto &[handle, callback] : _ShaderProgramList) {
		callback();
		glDeleteProgram(handle);
	}
}

}  // namespace gfxutils
