#include <gfx-utils-core/app.h>
#include <gfx-utils-core/render_pass.h>
#include <gfx-utils-core/shader_program.h>
#include <gfx-utils-core/vertex_buffer.h>
#include <gfx-utils-core/vertices.h>

#include <glad/glad.h>
#include <imgui.h>

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr const char *WINDOW_TITLE = "Example: texture_io";

int main() {
	using namespace gfxutils;

	auto &app = App::instance();
	app.init(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
	app.set_flag_vsync(true);
	app.set_clear_color({ 0.341f, 0.808f, 0.980f });

	// prepare shaders
	ShaderProgram::ShaderProgramBuilder pp_pass_shader_program_builder("pp_pass_shader_program");
	{
		Shader::ShaderBuilder vs_builder("pp_pass_vs");
		auto vertex_shader = vs_builder
		                         .set_type(ShaderType::VERTEX_SHADER)
		                         .set_source_from_file("assets/texture_io/pp.vert")
		                         .build();

		Shader::ShaderBuilder fs_builder("pp_pass_fs");
		auto fragment_shader = fs_builder
		                           .set_type(ShaderType::FRAGMENT_SHADER)
		                           .set_source_from_file("assets/texture_io/pp.frag")
		                           .build();

		pp_pass_shader_program_builder.add_shader(vertex_shader).add_shader(fragment_shader);
	}
	auto pp_pass_shader_program = pp_pass_shader_program_builder.build();
	pp_pass_shader_program.use();
	pp_pass_shader_program.set_uniform("window_width", WINDOW_WIDTH);
	pp_pass_shader_program.set_uniform("window_height", WINDOW_HEIGHT);

	// pp stands for post-processing
	ShaderProgram::ShaderProgramBuilder default_pass_shader_program_builder("default_pass_shader_program");
	{
		Shader::ShaderBuilder vs_builder("default_pass_vs");
		auto vertex_shader = vs_builder
		                         .set_type(ShaderType::VERTEX_SHADER)
		                         .set_source_from_file("assets/texture_io/default.vert")
		                         .build();

		Shader::ShaderBuilder fs_builder("default_pass_fs");
		auto fragment_shader = fs_builder
		                           .set_type(ShaderType::FRAGMENT_SHADER)
		                           .set_source_from_file("assets/texture_io/default.frag")
		                           .build();

		default_pass_shader_program_builder.add_shader(vertex_shader).add_shader(fragment_shader);
	}
	auto default_pass_shader_program = default_pass_shader_program_builder.build();

	// prepare textures (attachments)
	auto input_texture = Texture::TextureBuilder("input_texture")
	                         .set_data_from_file("assets/texture_io/image.png")
	                         .set_format(GL_RGBA32F)
	                         .build();
	auto albedo = Texture::TextureBuilder("albedo")
	                  .set_size(WINDOW_WIDTH, WINDOW_HEIGHT)
	                  .set_format(GL_RGBA32F)
	                  .build();

	// prepare render passes
	auto pp_pass = RenderPass::RenderPassBuilder("pp_pass")
	                   .add_color_attachment(albedo, false)
	                   .build();
	auto default_pass = RenderPass::RenderPassBuilder("default_pass").build();

	// prepare vertices
	auto quad_vertex_buffer = VertexBuffer::VertexBufferBuilder("fullscreen_quad", g_screen_quad_vertices)
	                              .add_attribute(2)  // position (vec2)
	                              .add_attribute(2)  // texture coordinates (vec2)
	                              .build();

	RenderPassConfig render_pass_config;
	render_pass_config._EnableDepthTest = false;
	render_pass_config._EnableSRGB = false;

	app.run([&](float dt [[maybe_unused]]) {
		pp_pass.use(render_pass_config, [&]() {
			quad_vertex_buffer.use();
			pp_pass_shader_program.use();

			input_texture.use(0);
			pp_pass_shader_program.set_uniform("input_texture_sampler", 0);

			glDrawArrays(GL_TRIANGLES, 0, 6);
		});

		default_pass.use(render_pass_config, [&]() {
			quad_vertex_buffer.use();
			default_pass_shader_program.use();

			albedo.use(0);
			default_pass_shader_program.set_uniform("albedo_sampler", 0);

			glDrawArrays(GL_TRIANGLES, 0, 6);
		});
	});

	albedo.export_to_file("output.png");

	app.shutdown();

	return 0;
}
