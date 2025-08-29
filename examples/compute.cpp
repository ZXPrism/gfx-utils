#include <gfx-utils-core/app.h>
#include <gfx-utils-core/render_pass.h>
#include <gfx-utils-core/shader_program.h>
#include <gfx-utils-core/storage_buffer.h>
#include <gfx-utils-core/vertex_buffer.h>
#include <gfx-utils-core/vertices.h>

constexpr int WINDOW_WIDTH = 960;
constexpr int WINDOW_HEIGHT = 960;
constexpr const char *WINDOW_TITLE = "Example: Compute";

int main() {
	using namespace gfxutils;

	auto &app = App::instance();
	app.init(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
	app.set_flag_vsync(true);
	app.set_clear_color({ 0.341f, 0.808f, 0.980f });

	ShaderProgram::ShaderProgramBuilder compute_shader_program_builder("color_pass_shader_program");
	{
		Shader::ShaderBuilder cs_builder("compute_shader");
		auto compute_shader = cs_builder
		                          .set_type(ShaderType::COMPUTE_SHADER)
		                          .set_source_from_file("assets/compute/compute.comp")
		                          .build();

		compute_shader_program_builder.add_shader(compute_shader);
	}
	auto compute_shader_program = compute_shader_program_builder.build();
	compute_shader_program.use();
	compute_shader_program.set_uniform("window_width", WINDOW_WIDTH);
	compute_shader_program.set_uniform("window_height", WINDOW_HEIGHT);

	ShaderProgram::ShaderProgramBuilder default_pass_shader_program_builder("default_pass_shader_program");
	{
		Shader::ShaderBuilder vs_builder("default_vs");
		auto vertex_shader = vs_builder
		                         .set_type(ShaderType::VERTEX_SHADER)
		                         .set_source_from_file("assets/compute/default.vert")
		                         .build();

		Shader::ShaderBuilder fs_builder("default_fs");
		auto fragment_shader = fs_builder
		                           .set_type(ShaderType::FRAGMENT_SHADER)
		                           .set_source_from_file("assets/compute/default.frag")
		                           .build();

		default_pass_shader_program_builder.add_shader(vertex_shader).add_shader(fragment_shader);
	}
	auto default_pass_shader_program = default_pass_shader_program_builder.build();
	default_pass_shader_program.use();
	default_pass_shader_program.set_uniform("window_width", WINDOW_WIDTH);

	auto default_pass = RenderPass::RenderPassBuilder("default_pass").build();

	auto quad_vertex_buffer = VertexBuffer::VertexBufferBuilder("fullscreen_quad", g_screen_quad_vertices)
	                              .add_attribute(2)  // position (vec2)
	                              .add_attribute(2)  // texture coordinates (vec2)
	                              .build();
	auto storage_buffer = StorageBuffer::StorageBufferBuilder("compute_input_buffer")
	                          .set_size(WINDOW_WIDTH * WINDOW_HEIGHT * 16)
	                          .build();

	float time = 0.0f;
	app.run([&](float dt [[maybe_unused]]) {
		default_pass.use(false, [&]() {
			compute_shader_program.use();
			compute_shader_program.set_uniform("time", time * 0.5f);
			storage_buffer.bind(0);
			glDispatchCompute(static_cast<GLuint>(WINDOW_WIDTH >> 4), static_cast<GLuint>(WINDOW_HEIGHT >> 4), 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

			default_pass_shader_program.use();
			storage_buffer.bind(0);
			quad_vertex_buffer.use();
			glDrawArrays(GL_TRIANGLES, 0, 6);

			time += dt;
		});
	});

	app.shutdown();

	return 0;
}
