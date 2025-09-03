#include <gfx-utils-core/app.h>
#include <gfx-utils-core/render_pass.h>

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr const char *WINDOW_TITLE = "Example: Create Window";

int main() {
	using namespace gfxutils;

	auto &app = App::instance();
	app.init(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
	app.set_flag_vsync(true);
	app.set_clear_color({ 0.341f, 0.808f, 0.980f });

	auto default_pass = RenderPass::RenderPassBuilder("default_pass").build();

	RenderPassConfig render_pass_config;
	render_pass_config._EnableDepthTest = false;
	render_pass_config._EnableSRGB = false;

	app.run([&](float dt [[maybe_unused]]) {
		default_pass.use(render_pass_config, [&]() {
		});
	});

	app.shutdown();

	return 0;
}
