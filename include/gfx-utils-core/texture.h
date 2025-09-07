#pragma once

#include <gfx-utils-core/interfaces/build_target.h>
#include <gfx-utils-core/interfaces/builder.h>
#include <gfx-utils-core/interfaces/exportable_resource.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <glad/glad.h>

namespace gfxutils {

enum class TextureFormat {
	RGB16F,
	RGB32F,
	// todo
};

struct TextureInfo {
	size_t _Width;
	size_t _Height;
	GLenum _InternalFormat;
	GLenum _CPUFormat;
	GLenum _CPUCompType;
	GLint _Filter;
};

class Texture : public IBuildTarget<Texture>,
                public IExportableResource<Texture> {
private:
	GLuint _TextureHandle;
	TextureInfo _Info;

public:
	class TextureBuilder : public IBuilder<TextureBuilder, Texture> {
	private:
		std::vector<uint8_t> _Data;
		bool _IsSizeSet = false;
		bool _IsDataSet = false;
		bool _IsFormatSet = false;
		bool _IsFilterSet = false;

		TextureInfo _Info;

	public:
		TextureBuilder(const std::string &name);

		TextureBuilder &set_size(size_t width, size_t height);
		TextureBuilder &set_format(GLenum internal_format, GLenum cpu_format = GL_RGBA, GLenum cpu_comp_type = GL_UNSIGNED_BYTE);
		TextureBuilder &set_filter(GLint filter);
		TextureBuilder &set_data(const std::vector<uint8_t> &data);
		TextureBuilder &set_data_from_file(const std::string &file_path);

		[[nodiscard]] Texture _build() const;
	};

	void use(size_t texture_unit) const;

	[[nodiscard]] GLuint _get_handle() const;
	void _export_to_file(const std::string &file_path) const;
};

}  // namespace gfxutils
