#include <gfx-utils-core/app.h>
#include <gfx-utils-core/render_pass.h>
#include <gfx-utils-core/shader_program.h>
#include <gfx-utils-core/vertex_buffer.h>
#include <gfx-utils-core/vertices.h>

#include <glad/glad.h>
#include <imgui.h>

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr const char *WINDOW_TITLE = "Example: MRT";

int main() {
	using namespace gfxutils;

	auto &app = App::instance();
	app.init(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
	app.set_flag_vsync(true);
	app.set_clear_color({ 0.341f, 0.808f, 0.980f });

	// prepare shaders
	ShaderProgram::ShaderProgramBuilder color_pass_shader_program_builder("color_pass_shader_program");
	{
		Shader::ShaderBuilder vs_builder("color_pass_vs");
		auto vertex_shader = vs_builder
		                         .set_type(ShaderType::VERTEX_SHADER)
		                         .set_source_from_file("assets/mrt/color.vert")
		                         .build();

		Shader::ShaderBuilder fs_builder("color_pass_fs");
		auto fragment_shader = fs_builder
		                           .set_type(ShaderType::FRAGMENT_SHADER)
		                           .set_source_from_file("assets/mrt/color.frag")
		                           .build();

		color_pass_shader_program_builder.add_shader(vertex_shader).add_shader(fragment_shader);
	}
	auto color_pass_shader_program = color_pass_shader_program_builder.build();

	ShaderProgram::ShaderProgramBuilder default_pass_shader_program_builder("default_pass_shader_program");
	{
		Shader::ShaderBuilder vs_builder("default_vs");
		auto vertex_shader = vs_builder
		                         .set_type(ShaderType::VERTEX_SHADER)
		                         .set_source_from_file("assets/mrt/default.vert")
		                         .build();

		Shader::ShaderBuilder fs_builder("default_fs");
		auto fragment_shader = fs_builder
		                           .set_type(ShaderType::FRAGMENT_SHADER)
		                           .set_source_from_file("assets/mrt/default.frag")
		                           .build();
		default_pass_shader_program_builder.add_shader(vertex_shader).add_shader(fragment_shader);
	}
	auto default_pass_shader_program = default_pass_shader_program_builder.build();

	// prepare textures (attachments)
	auto color_r = Texture::TextureBuilder("color_r")
	                   .set_size(WINDOW_WIDTH, WINDOW_HEIGHT)
	                   .set_format(GL_RGBA32F)
	                   .build();
	auto color_g = Texture::TextureBuilder("color_g")
	                   .set_size(WINDOW_WIDTH, WINDOW_HEIGHT)
	                   .set_format(GL_RGBA32F)
	                   .build();
	auto color_b = Texture::TextureBuilder("color_b")
	                   .set_size(WINDOW_WIDTH, WINDOW_HEIGHT)
	                   .set_format(GL_RGBA32F)
	                   .build();

	// prepare render passes
	auto color_pass = RenderPass::RenderPassBuilder("color_pass")
	                      .add_color_attachment(color_r, true, { 1.0, 0.0, 0.0, 1.0 })
	                      .add_color_attachment(color_g, true, { 0.0, 1.0, 0.0, 1.0 })
	                      .add_color_attachment(color_b, true, { 0.0, 0.0, 1.0, 1.0 })
	                      .build();
	auto default_pass = RenderPass::RenderPassBuilder("default_pass").build();

	auto quad_vertex_buffer = VertexBuffer::VertexBufferBuilder("fullscreen_quad", g_screen_quad_vertices)
	                              .add_attribute(2)  // position (vec2)
	                              .add_attribute(2)  // texture coordinates (vec2)
	                              .build();

	int color_option = 0;
	app.run([&](float dt [[maybe_unused]]) {
		ImGui::Begin("Control", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
		{
			const char *items[] = { "R", "G", "B" };
			ImGui::Combo("Color", &color_option, items, IM_ARRAYSIZE(items));
		}
		ImGui::End();

		color_pass.use([&]() {
			app.set_flag_depth_test(false);

			quad_vertex_buffer.use();
			color_pass_shader_program.use();

			glDrawArrays(GL_TRIANGLES, 0, 6);
		});

		default_pass.use([&]() {
			app.set_flag_depth_test(false);

			quad_vertex_buffer.use();
			default_pass_shader_program.use();
			default_pass_shader_program.set_uniform("color_r_sampler", 0);
			default_pass_shader_program.set_uniform("color_g_sampler", 1);
			default_pass_shader_program.set_uniform("color_b_sampler", 2);
			default_pass_shader_program.set_uniform("option", color_option);
			color_r.use(0);
			color_g.use(1);
			color_b.use(2);

			glDrawArrays(GL_TRIANGLES, 0, 6);
		});
	});

	app.shutdown();

	return 0;
}
