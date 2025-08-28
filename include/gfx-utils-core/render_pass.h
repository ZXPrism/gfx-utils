#pragma once

#include <gfx-utils-core/interfaces/build_target.h>
#include <gfx-utils-core/interfaces/builder.h>
#include <gfx-utils-core/texture.h>

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include <glad/glad.h>

namespace gfxutils {

class RenderPass : public IBuildTarget<RenderPass> {
private:
	std::shared_ptr<GLuint> _FBO;
	std::vector<Texture> _ColorAttachments;
	Texture _DepthAttachment;

public:
	class RenderPassBuilder : public IBuilder<RenderPassBuilder, RenderPass> {
	private:
		std::vector<Texture> _ColorAttachments;
		std::vector<bool> _ColorAttachmentClearOptions;
		std::optional<Texture> _DepthAttachment;

	public:
		RenderPassBuilder(const std::string &name);

		RenderPassBuilder &add_color_attachment(const Texture &texture, bool clear_before_use);  // TODO: control clear colors for each attachments...
		RenderPassBuilder &set_depth_attachment(const Texture &texture);

		RenderPass _build();
	};

	void clear(int mask) const;
	void use(const std::function<void()> &callback) const;

private:
	RenderPass() = default;
};

}  // namespace gfxutils
