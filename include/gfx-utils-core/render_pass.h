#pragma once

#include <gfx-utils-core/interfaces/build_target.h>
#include <gfx-utils-core/interfaces/builder.h>
#include <gfx-utils-core/render_pass_config.h>
#include <gfx-utils-core/texture.h>

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace gfxutils {

class RenderPass : public IBuildTarget<RenderPass> {
private:
	bool _IsDefault;  // i.e. use default FBO

	std::shared_ptr<GLuint> _FBO;
	std::vector<Texture> _ColorAttachments;
	std::vector<bool> _ColorAttachmentClearFlags;
	std::vector<glm::vec4> _ColorAttachmentClearValues;
	std::optional<Texture> _DepthAttachment;

public:
	class RenderPassBuilder : public IBuilder<RenderPassBuilder, RenderPass> {
	private:
		std::vector<Texture> _ColorAttachments;
		std::vector<bool> _ColorAttachmentClearFlags;
		std::vector<glm::vec4> _ColorAttachmentClearValues;
		std::optional<Texture> _DepthAttachment;

	public:
		RenderPassBuilder(const std::string &name);

		// NOTE: current only support float types
		// NOTE: if attachment's one component has less than 4 floats, only clear_value_rgba.r is used to clear it
		RenderPassBuilder &add_color_attachment(const Texture &texture, bool clear_before_use, const glm::vec4 &clear_value_rgba = {});
		RenderPassBuilder &set_depth_attachment(const Texture &texture);

		[[nodiscard]] RenderPass _build() const;
	};

	// NOTE: depth_test flag is ONLY meaningful when writing to default FBO
	// depth attachment(if any) will always be cleared regardless of this flag
	void use(const RenderPassConfig &render_pass_config, const std::function<void()> &callback) const;

private:
	RenderPass() = default;
};

}  // namespace gfxutils
