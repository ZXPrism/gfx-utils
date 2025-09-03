#include <gfx-utils-core/texture.h>

#include <gfx-utils-core/logger.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>
#pragma clang diagnostic pop

namespace gfxutils {

Texture::TextureBuilder::TextureBuilder(const std::string &name)
    : IBuilder(name) {
}

Texture::TextureBuilder &Texture::TextureBuilder::set_size(size_t width, size_t height) {
	_Info._Width = width;
	_Info._Height = height;
	_IsSizeSet = true;
	return *this;
}

Texture::TextureBuilder &Texture::TextureBuilder::set_format(GLenum internal_format, GLenum cpu_format, GLenum cpu_comp_type) {
	_Info._InternalFormat = internal_format;
	_Info._CPUFormat = cpu_format;
	_Info._CPUCompType = cpu_comp_type;
	_IsFormatSet = true;
	return *this;
}

Texture::TextureBuilder &Texture::TextureBuilder::set_data(const std::vector<uint8_t> &data) {
	if (_IsSizeSet && data.size() == _Info._Width * _Info._Height) {
		_Data = data;
		_IsDataSet = true;
	} else {
		g_logger->warn("Texture({}): texture size is not set or mismatchs with input data size", _Name);
	}
	return *this;
}

Texture::TextureBuilder &Texture::TextureBuilder::set_data_from_file(const std::string &file_path) {
	int width = 0, height = 0, n_channels_actual = 0;

	stbi_set_flip_vertically_on_load(true);
	// the 4th argument `req_comp` == 4: force in RGBA 4 channel format
	uint8_t *data = stbi_load(file_path.c_str(), &width, &height, &n_channels_actual, 4);
	if (!data) {
		g_logger->warn("Texture({}): failed to load texture from {}, maybe the path is incorrect, or the file is corrupted", _Name, file_path);
		return *this;
	}

	g_logger->info("Texture({}): loaded texture from {}", _Name, file_path);
	g_logger->info("Texture({}): texture info: width = {}, height = {}, n_channels = {}",
	               _Name,
	               width,
	               height,
	               n_channels_actual);

	_Info._Width = static_cast<size_t>(width);
	_Info._Height = static_cast<size_t>(height);
	_Data.assign(data, data + _Info._Width * _Info._Height * 4);

	stbi_image_free(data);

	_IsDataSet = true;
	_IsSizeSet = true;

	return *this;
}

Texture Texture::TextureBuilder::_build() const {
	Texture res;

	if (!_IsSizeSet) {
		g_logger->warn("Texture({}): texture size is not set: texture won't be built", _Name);
		return res;
	}

	if (!_IsFormatSet) {
		g_logger->warn("Texture({}): texture format is not set: texture won't be built", _Name);
		return res;
	}

	res._Info = _Info;

	auto texture_raw_handle = new GLuint(0);
	res._TextureHandle = std::shared_ptr<GLuint>(texture_raw_handle, [&](GLuint *ptr) {
		glDeleteTextures(1, ptr);
		delete ptr;
	});

	const uint8_t *data_ptr = (_IsDataSet ? _Data.data() : nullptr);

	glGenTextures(1, res._TextureHandle.get());
	glBindTexture(GL_TEXTURE_2D, *res._TextureHandle);
	glTexImage2D(GL_TEXTURE_2D,
	             0,
	             _Info._InternalFormat,
	             static_cast<GLsizei>(_Info._Width),
	             static_cast<GLsizei>(_Info._Height),
	             0,
	             _Info._CPUFormat,
	             _Info._CPUCompType,
	             data_ptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	g_logger->info("Texture::TextureBuilder ({}): successfully built texture", _Name);

	res._set_complete();

	return res;
}

void Texture::use(size_t texture_unit) {
	glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + texture_unit));
	glBindTexture(GL_TEXTURE_2D, *_TextureHandle);
}

GLuint Texture::_get_handle() const {
	return *_TextureHandle;
}

void Texture::_export_to_file(const std::string &file_path [[maybe_unused]]) const {
	std::vector<uint8_t> data(_Info._Width * _Info._Height * 4);

	glBindTexture(GL_TEXTURE_2D, *_TextureHandle);
	glGetTexImage(GL_TEXTURE_2D, 0, _Info._CPUFormat, _Info._CPUCompType, data.data());

	stbi_flip_vertically_on_write(true);
	stbi_write_png(file_path.c_str(),
	               static_cast<int>(_Info._Width),
	               static_cast<int>(_Info._Height),
	               4,
	               data.data(),
	               static_cast<int>(_Info._Width * 4));

	g_logger->info("Texture: saved texture to {}", file_path);
}

}  // namespace gfxutils
