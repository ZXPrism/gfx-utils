#include <gfx-utils-core/app.h>
#include <gfx-utils-core/logger.h>
#include <gfx-utils-core/render_pass.h>
#include <gfx-utils-core/shader_program.h>
#include <gfx-utils-core/vertex_buffer.h>
#include <gfx-utils-core/vertices.h>

#include <glad/glad.h>
#include <imgui.h>

#include <filesystem>
#include <format>

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr const char *WINDOW_TITLE = "Example: post_processing";

int main() {
	using namespace gfxutils;

	auto &app = App::instance();
	app.init(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
	app.set_flag_vsync(true);
	app.set_clear_color({ 0.341f, 0.808f, 0.980f });

	auto load_shader = [](const std::string &pass_name, const std::string &shader_file_name) {
		ShaderProgram::ShaderProgramBuilder shader_program_builder(std::format("{}_pass_shader_program", pass_name));
		Shader::ShaderBuilder vs_builder(std::format("{}_pass_vs", pass_name));
		auto vertex_shader = vs_builder
		                         .set_type(ShaderType::VERTEX_SHADER)
		                         .set_source_from_file(std::format("assets/post_processing/{}.vert", shader_file_name))
		                         .build();

		Shader::ShaderBuilder fs_builder(std::format("{}_pass_fs", pass_name));
		auto fragment_shader = fs_builder
		                           .set_type(ShaderType::FRAGMENT_SHADER)
		                           .set_source_from_file(std::format("assets/post_processing/{}.frag", shader_file_name))
		                           .build();

		shader_program_builder.add_shader(vertex_shader).add_shader(fragment_shader);

		auto shader_program = shader_program_builder.build();
		shader_program.use();
		shader_program.set_uniform("window_size",
		                           glm::vec2(static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)));

		return shader_program;
	};

	// prepare shaders
	auto pp_pass_shader_program = load_shader("pp", "pp_box_filter");
	auto default_pass_shader_program = load_shader("default", "default");

	// prepare textures (attachments)
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

	// detect images under assets/post_processing/input_image folder
	int curr_selected_texture = 0;
	const char *input_texture_folder = "assets/post_processing/input_image";
	std::vector<Texture> input_texture_vec;
	std::vector<std::string> input_texture_names;

	namespace fs = std::filesystem;
	if (!fs::exists(input_texture_folder) || !fs::is_directory(input_texture_folder)) {
		g_logger->warn("input texture folder doesn't exist!");
	} else {
		for (const auto &entry : fs::directory_iterator(input_texture_folder)) {
			if (!entry.is_regular_file()) {
				continue;
			}

			auto ext = entry.path().extension().string();
			for (auto &ch : ext) {
				ch |= 32;
			}

			if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
			    ext == ".bmp" || ext == ".tga") {
				auto image_path = entry.path();
				auto texture = Texture::TextureBuilder(image_path.stem().string())
				                   .set_data_from_file(image_path.string())
				                   .set_format(GL_RGBA32F)  // here we assume the input image is all in linear color space (for now)
				                   .build();
				input_texture_vec.push_back(texture);
				input_texture_names.push_back(image_path.filename().string());
			}
		}
	}

	// style
	int current_selected_style = 0;
	const char *style_options[] = { "Sobel", "Laplcian" };

	// AA
	int current_selected_AA = 0;
	const char *aa_options[] = { "None", "Box Filter" };

	// sharpening
	int current_selected_sharpening = 0;
	const char *sharpening_options[] = { "None", "Unsharp Masking" };

	app.run([&](float dt [[maybe_unused]]) {
		ImGui::Begin("Control");
		{
			// input image selection
			{
				std::vector<const char *> image_options;
				if (input_texture_vec.empty()) {
					image_options.push_back("(no available images)");
				} else {
					for (const auto &name : input_texture_names) {
						image_options.push_back(name.c_str());
					}
				}
				ImGui::Combo("Input Image", &curr_selected_texture, image_options.data(), static_cast<int>(image_options.size()));
			}

			// mode selection (stylistic / enhancement)
			{
				static int current_selected_mode = 0;
				static const char *mode_options[] = { "Stylistic", "Enhancement" };
				ImGui::Combo("Mode", &current_selected_mode, mode_options, 2);

				if (current_selected_mode == 0) {
					ImGui::Combo("Style", &current_selected_style, style_options, static_cast<int>(sizeof(style_options) / sizeof(const char *)));
				} else {
					ImGui::Combo("AA", &current_selected_AA, aa_options, static_cast<int>(sizeof(aa_options) / sizeof(const char *)));
					ImGui::Combo("Sharpening", &current_selected_sharpening, sharpening_options, static_cast<int>(sizeof(sharpening_options) / sizeof(const char *)));
				}
			}
		}
		ImGui::End();

		pp_pass.use(render_pass_config, [&]() {
			quad_vertex_buffer.use();
			pp_pass_shader_program.use();

			if (!input_texture_vec.empty()) {
				input_texture_vec[curr_selected_texture].use(0);
				pp_pass_shader_program.set_uniform("input_texture_sampler", 0);
			}

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

	app.shutdown();

	return 0;
}
