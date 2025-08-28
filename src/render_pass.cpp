#include <gfx-utils-core/render_pass.h>

#include <gfx-utils-core/logger.h>

namespace gfxutils {

RenderPass::RenderPassBuilder::RenderPassBuilder(const std::string &name)
    : IBuilder(name) {
}

RenderPass::RenderPassBuilder &RenderPass::RenderPassBuilder::add_color_attachment(const Texture &texture, bool clear_before_use, const glm::vec4 &clear_value_rgba) {
	_ColorAttachments.push_back(texture);
	_ColorAttachmentClearFlags.push_back(clear_before_use);
	_ColorAttachmentClearValues.push_back(clear_value_rgba);
	return *this;
}

RenderPass::RenderPassBuilder &RenderPass::RenderPassBuilder::set_depth_attachment(const Texture &texture) {
	_DepthAttachment = texture;
	return *this;
}

RenderPass RenderPass::RenderPassBuilder::_build() {
	RenderPass res;

	size_t n_color_attachments = _ColorAttachments.size();
	res._ColorAttachments = _ColorAttachments;
	res._ColorAttachmentClearFlags = _ColorAttachmentClearFlags;
	res._ColorAttachmentClearValues = _ColorAttachmentClearValues;

	res._IsDefault = !n_color_attachments && !_DepthAttachment.has_value();  // no attachments, fallback to default FBO

	auto fbo_raw_handle = new GLuint(0);
	res._FBO = std::shared_ptr<GLuint>(fbo_raw_handle, [=](GLuint *ptr) {
		if (!res._IsDefault) {  // if fallback, the delete is handled by the window system, no need to manually delete it
			glDeleteFramebuffers(1, ptr);
		}
		delete ptr;
	});

	if (res._IsDefault) {
		return res;
	}

	glGenFramebuffers(1, res._FBO.get());
	glBindFramebuffer(GL_FRAMEBUFFER, *res._FBO);

	std::vector<GLenum> attachments(n_color_attachments);

	for (size_t i = 0; i < n_color_attachments; i++) {
		auto texture_handle = res._ColorAttachments[i]._get_handle();
		attachments[i] = static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i);
		if (res._ColorAttachments[i].is_complete()) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, attachments[i], GL_TEXTURE_2D, texture_handle, 0);
		}
	}

	if (_DepthAttachment.has_value()) {
		res._DepthAttachment = _DepthAttachment;
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, res._DepthAttachment->_get_handle(), 0);
	}

	glDrawBuffers(static_cast<GLsizei>(n_color_attachments), attachments.data());

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		g_logger->warn("RenderPass::RenderPassBuilder ({}): framebuffer is not complete!", _Name);
		return res;
	}

	g_logger->info("RenderPass::RenderPassBuilder ({}): successfully built render pass with {} color attachment(s)", _Name, n_color_attachments);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	res._set_complete();

	return res;
}

void RenderPass::use(bool depth_test, const std::function<void()> &callback) const {
	glBindFramebuffer(GL_FRAMEBUFFER, *_FBO);

	if (_IsDefault) {
		if (depth_test) {
			glEnable(GL_DEPTH_TEST);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		} else {
			glDisable(GL_DEPTH_TEST);
			glClear(GL_COLOR_BUFFER_BIT);
		}

	} else {
		if (_DepthAttachment.has_value()) {
			glClear(GL_DEPTH_BUFFER_BIT);
		}

		size_t n_color_attachments = _ColorAttachments.size();
		for (size_t i = 0; i < n_color_attachments; i++) {
			if (_ColorAttachmentClearFlags[i]) {
				glClearBufferfv(GL_COLOR, static_cast<GLint>(i), &_ColorAttachmentClearValues[i].r);
			}
		}
	}

	callback();
}

}  // namespace gfxutils
