#include <gfx-utils-core/app.h>
#include <gfx-utils-core/logger.h>
#include <gfx-utils-core/render_pass.h>
#include <gfx-utils-core/shader_program.h>
#include <gfx-utils-core/vertex_buffer.h>
#include <gfx-utils-core/vertices.h>

#include <glad/glad.h>
#include <imgui.h>

#include <json/json.h>
#include <filesystem>
#include <format>
#include <fstream>
#include <unordered_map>

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr const char *WINDOW_TITLE = "Example: post_processing";
constexpr const char *CONFIG_FILE_PATH = "assets/post_processing/config.json";
constexpr const char *SHADER_ROOT_PATH = "assets/post_processing/shader";
constexpr const char *INPUT_TEXTURE_FOLDER = "assets/post_processing/input_image";

int main() {
	using namespace gfxutils;

	auto &app = App::instance();
	app.init(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
	app.set_flag_vsync(true);
	app.set_clear_color({ 0.341f, 0.808f, 0.980f });

	auto load_shader = [](const std::string &pass_name, const std::string &vs_path, const std::string &fs_path) {
		ShaderProgram::ShaderProgramBuilder shader_program_builder(std::format("{}_shader_program", pass_name));
		Shader::ShaderBuilder vs_builder(std::format("{}_vs", pass_name));
		auto vertex_shader = vs_builder
		                         .set_type(ShaderType::VERTEX_SHADER)
		                         .set_source_from_file(std::format("{}/{}", SHADER_ROOT_PATH, vs_path))
		                         .build();

		Shader::ShaderBuilder fs_builder(std::format("{}_fs", pass_name));
		auto fragment_shader = fs_builder
		                           .set_type(ShaderType::FRAGMENT_SHADER)
		                           .set_source_from_file(std::format("{}/{}", SHADER_ROOT_PATH, fs_path))
		                           .build();

		shader_program_builder.add_shader(vertex_shader).add_shader(fragment_shader);

		auto shader_program = shader_program_builder.build();
		shader_program.use();
		shader_program.set_uniform("window_size",
		                           glm::vec2(static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)));

		return shader_program;
	};

	// load config file
	std::ifstream config_file_in(CONFIG_FILE_PATH);
	if (!config_file_in) {
		g_logger->error("config file does not exist");
		return -1;
	}
	Json::Value config_root;
	config_file_in >> config_root;
	config_file_in.close();

	// load AA effects
	std::vector<std::string> fx_name_vec_aa;
	std::vector<std::vector<ShaderProgram>> fx_shader_vec_aa;
	std::vector<std::vector<RenderPass>> fx_render_pass_vec_aa;
	std::vector<Texture> fx_render_target_vec_aa;

	auto &aa_root_node = config_root["aa"];
	for (auto &aa_effects_node : aa_root_node) {
		std::string fx_name = aa_effects_node["fx_name"].asString();
		auto &render_passes_node = aa_effects_node["render_passes"];

		fx_name_vec_aa.push_back(fx_name);

		g_logger->info("loading AA effect '{}' with {} render pass(s)", fx_name, render_passes_node.size());

		for (auto &render_pass_node : render_passes_node) {
			std::string pass_name = render_pass_node["pass_name"].asString();
			g_logger->info("loading render pass {}", pass_name);

			// prepare shaders
			std::string vs_path = render_pass_node["vs_path"].asString();
			std::string fs_path = render_pass_node["fs_path"].asString();
			auto shader_program = load_shader(pass_name, vs_path, fs_path);

			auto &shader_vec = fx_shader_vec_aa.emplace_back();
			shader_vec.push_back(shader_program);

			// prepare render passes
			auto render_target = Texture::TextureBuilder("color")
			                         .set_size(WINDOW_WIDTH, WINDOW_HEIGHT)
			                         .set_format(GL_RGBA32F)
			                         .build();
			auto render_pass = RenderPass::RenderPassBuilder(pass_name)
			                       .add_color_attachment(render_target, false)
			                       .build();

			fx_render_target_vec_aa.push_back(render_target);
			auto &render_pass_vec = fx_render_pass_vec_aa.emplace_back();
			render_pass_vec.push_back(render_pass);
		}
	}

	// prepare default resources (final pass)
	auto default_pass_shader_program = load_shader("default_pass", "default.vert", "default.frag");
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
	std::vector<Texture> input_texture_vec;
	std::vector<std::string> input_texture_names;

	namespace fs = std::filesystem;
	if (!fs::exists(INPUT_TEXTURE_FOLDER) || !fs::is_directory(INPUT_TEXTURE_FOLDER)) {
		g_logger->warn("input texture folder doesn't exist!");
	} else {
		for (const auto &entry : fs::directory_iterator(INPUT_TEXTURE_FOLDER)) {
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
	int curr_selected_style = 0;

	// AA
	int curr_selected_aa = 0;

	// sharpening
	int curr_selected_sharpening = 0;

	app.run([&](float dt [[maybe_unused]]) {
		ImGui::Begin("Control");
		{
			// export button
			if (curr_selected_style == 0 && curr_selected_aa == 0 && curr_selected_sharpening == 0) {
				if (!input_texture_vec.empty()) {
					if (ImGui::Button("export framebuffer")) {
						input_texture_vec[curr_selected_texture].export_to_file("output.png");
					}
				}
			} else {
				if (ImGui::Button("export framebuffer")) {
					fx_render_target_vec_aa[curr_selected_aa - 1].export_to_file("output.png");
				}
			}

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
				static int curr_selected_mode = 0;
				ImGui::RadioButton("Stylistic", &curr_selected_mode, 0);
				ImGui::SameLine();
				ImGui::RadioButton("Enhancement", &curr_selected_mode, 1);

				if (curr_selected_mode == 0) {
					// ImGui::Combo("Stylistic", &current_selected_style, style_options, static_cast<int>(sizeof(style_options) / sizeof(const char *)));
				} else {
					std::vector<const char *> aa_options;
					aa_options.push_back("(none)");
					for (const auto &aa_names : fx_name_vec_aa) {
						aa_options.push_back(aa_names.c_str());
					}
					ImGui::Combo("AA", &curr_selected_aa, aa_options.data(), static_cast<int>(aa_options.size()));
					//  ImGui::Combo("Sharpening", &current_selected_sharpening, sharpening_options, static_cast<int>(sizeof(sharpening_options) / sizeof(const char *)));
				}
			}
		}
		ImGui::End();

		if (curr_selected_aa != 0) {
			size_t fx_id = curr_selected_aa - 1;
			for (size_t i = 0; const auto &render_pass : fx_render_pass_vec_aa[fx_id]) {
				auto &shader_program = fx_shader_vec_aa[fx_id][i];
				render_pass.use(render_pass_config, [&]() {
					quad_vertex_buffer.use();
					shader_program.use();
					shader_program.set_uniform("input_texture_sampler", 0);

					if (i == 0) {
						if (!input_texture_vec.empty()) {
							input_texture_vec[curr_selected_texture].use(0);
						}
					} else {
						fx_render_target_vec_aa[i].use(0);
					}

					glDrawArrays(GL_TRIANGLES, 0, 6);
				});

				++i;
			}
		}

		default_pass.use(render_pass_config, [&]() {
			quad_vertex_buffer.use();
			default_pass_shader_program.use();

			// if no style, AA or sharpening, directly use the input image
			if (curr_selected_style == 0 && curr_selected_aa == 0 && curr_selected_sharpening == 0) {
				if (!input_texture_vec.empty()) {
					input_texture_vec[curr_selected_texture].use(0);
				}
			} else {  // otherwise, use previous output texture (for simplicity, currently only use AA)
				fx_render_target_vec_aa[curr_selected_aa - 1].use(0);
			}
			default_pass_shader_program.set_uniform("input_texture_sampler", 0);

			glDrawArrays(GL_TRIANGLES, 0, 6);
		});
	});

	app.shutdown();

	return 0;
}
