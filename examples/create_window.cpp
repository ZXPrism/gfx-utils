#include <gfx-utils-core/app.h>
#include <gfx-utils-core/render_pass.h>

#include <glad/glad.h>

int main() {
	using namespace gfxutils;

	auto &app = App::instance();
	app.init("gfx-utils example: Create Window", 800, 600);
	app.set_flag_vsync(true);
	app.set_clear_color({ 0.341f, 0.808f, 0.980f });

	auto default_pass = RenderPass::RenderPassBuilder("default_pass").build();

	app.run([&](float dt [[maybe_unused]]) {
		default_pass.use([&]() {
			default_pass.clear(GL_COLOR_BUFFER_BIT);
		});
	});

	app.shutdown();

	return 0;
}
