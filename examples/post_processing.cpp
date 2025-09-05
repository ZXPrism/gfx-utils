#include <gfx-utils-core/app.h>
#include <gfx-utils-core/logger.h>
#include <gfx-utils-core/render_pass.h>
#include <gfx-utils-core/shader_program.h>
#include <gfx-utils-core/vertex_buffer.h>
#include <gfx-utils-core/vertices.h>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include <json/json.h>
#include <filesystem>
#include <format>
#include <fstream>
#include <unordered_set>

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr const char *WINDOW_TITLE = "Example: post_processing";
constexpr const char *CONFIG_FILE_PATH = "assets/post_processing/config.json";
constexpr const char *SHADER_ROOT_PATH = "assets/post_processing/shader";
constexpr const char *INPUT_TEXTURE_FOLDER = "assets/post_processing/input_image";

using namespace gfxutils;

VertexBuffer g_quad_vertex_buffer;

struct PostProcessingStage {
	std::string _EffectName;
	std::vector<ShaderProgram> _ShaderProgramVec;
	std::vector<RenderPass> _RenderPassVec;
	std::vector<Texture> _RenderTargetVec;

	const Texture &execute(const Texture &input) {
		RenderPassConfig render_pass_config;
		render_pass_config._EnableDepthTest = false;
		render_pass_config._EnableSRGB = false;

		for (size_t i = 0; const auto &render_pass : _RenderPassVec) {
			auto &shader_program = _ShaderProgramVec[i];
			render_pass.use(render_pass_config, [&]() {
				g_quad_vertex_buffer.use();
				shader_program.use();
				shader_program.set_uniform("u_input_texture_sampler", 0);
				input.use(0);

				glDrawArrays(GL_TRIANGLES, 0, 6);
			});

			++i;
		}

		return _RenderTargetVec.back();
	}
};

int main() {
	auto &app = App::instance();
	app.init(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
	app.set_flag_vsync(false);
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
		shader_program.set_uniform("u_window_size",
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

	auto load_effects = [&](std::vector<PostProcessingStage> &effect_vec, const std::string &type) -> bool {
		auto &root_node = config_root[type];
		for (auto &effect_node : root_node) {
			PostProcessingStage fx;

			std::string fx_name = effect_node["fx_name"].asString();
			auto &render_passes_node = effect_node["render_passes"];

			g_logger->info("loading {} effect '{}' with {} render pass(s)", type, fx_name, render_passes_node.size());

			if (render_passes_node.empty()) {
				g_logger->error("effect must contain at least 1 render pass!");
				return false;
			}

			fx._EffectName = fx_name;

			for (auto &render_pass_node : render_passes_node) {
				std::string pass_name = render_pass_node["pass_name"].asString();
				g_logger->info("loading render pass '{}'", pass_name);

				// prepare shaders
				std::string vs_path = render_pass_node["vs_path"].asString();
				std::string fs_path = render_pass_node["fs_path"].asString();
				auto shader_program = load_shader(pass_name, vs_path, fs_path);

				fx._ShaderProgramVec.push_back(shader_program);

				// prepare render passes
				auto render_target = Texture::TextureBuilder(std::format("{}/color", pass_name))
				                         .set_size(WINDOW_WIDTH, WINDOW_HEIGHT)
				                         .set_format(GL_RGBA32F)
				                         .build();
				auto render_pass = RenderPass::RenderPassBuilder(pass_name)
				                       .add_color_attachment(render_target, false)
				                       .build();

				fx._RenderTargetVec.push_back(render_target);
				fx._RenderPassVec.push_back(render_pass);
			}

			effect_vec.push_back(fx);
		}
		return true;
	};

	// load AA effects
	std::vector<PostProcessingStage> effect_aa_vec;
	if (!load_effects(effect_aa_vec, "aa")) {
		return -1;
	}

	// load sharpening effects
	std::vector<PostProcessingStage> effect_sharpening_vec;
	if (!load_effects(effect_sharpening_vec, "sharpening")) {
		return -1;
	}

	// prepare default resources (final pass)
	auto default_pass_shader_program = load_shader("default_pass", "default.vert", "default.frag");
	auto default_pass = RenderPass::RenderPassBuilder("default_pass").build();

	// prepare vertices
	g_quad_vertex_buffer = VertexBuffer::VertexBufferBuilder("fullscreen_quad", g_screen_quad_vertices)
	                           .add_attribute(2)  // position (vec2)
	                           .add_attribute(2)  // texture coordinates (vec2)
	                           .build();

	RenderPassConfig render_pass_config;
	render_pass_config._EnableDepthTest = false;
	render_pass_config._EnableSRGB = false;

	// detect images under assets/post_processing/input_image folder
	// here we guarantee at least one image is available, or the program aborts
	int curr_selected_texture = 0;
	std::vector<Texture> input_texture_vec;
	std::vector<std::string> input_texture_names;

	namespace fs = std::filesystem;
	if (!fs::exists(INPUT_TEXTURE_FOLDER) || !fs::is_directory(INPUT_TEXTURE_FOLDER) || fs::is_empty(INPUT_TEXTURE_FOLDER)) {
		g_logger->warn("input texture folder doesn't exist or empty!");
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

	// shader reflector utility function
	auto reflect_shader = [](std::vector<ShaderProgram> &shader_program_vec) {
		static std::unordered_set<std::string> uniform_name_exclude_set{ "u_input_texture_sampler", "u_window_size" };
		for (auto &shader_program : shader_program_vec) {
			if (ImGui::CollapsingHeader(shader_program.get_name().c_str())) {
				shader_program.use();

				auto uniform_info_vec = shader_program.get_all_uniform_info();
				for (auto &uniform_info : uniform_info_vec) {
					if (!uniform_name_exclude_set.contains(uniform_info._Name)) {
						const char *label = uniform_info._Name.c_str();

						switch (uniform_info._Type) {
						case ShaderDataType::INT:
							{
								static int value = 0;
								if (ImGui::InputInt(label, &value)) {
									shader_program.set_uniform(uniform_info._Name, value);
								}
								break;
							}
						case ShaderDataType::FLOAT:
							{
								static float value = 0.0f;
								if (ImGui::InputFloat(label, &value)) {
									shader_program.set_uniform(uniform_info._Name, value);
								}
								break;
							}
						case ShaderDataType::VEC2:
							{
								static glm::vec2 value(0.0f);
								if (ImGui::InputFloat2(label, glm::value_ptr(value))) {
									shader_program.set_uniform(uniform_info._Name, value);
								}
								break;
							}
						case ShaderDataType::VEC3:
							{
								static glm::vec3 value(0.0f);
								if (ImGui::InputFloat3(label, glm::value_ptr(value))) {
									shader_program.set_uniform(uniform_info._Name, value);
								}
								break;
							}
						default:
							break;
						}
					}
				}
			}
		}
	};

	// AA
	int curr_selected_aa = 0;

	// sharpening
	int curr_selected_sharpening = 0;

	// perf
	constexpr int QUERY_FREQ = 10;
	int frame_cnt = 0;  // read gpu query every `QUERY_FREQ` frames
	float cpu_time_average = 0.0f;
	float gpu_time_average = 0.0f;
	GLuint gpu_time_query;

	app.run([&](float dt) {
		ImGui::Begin("Control");
		{
			ImGui::SeparatorText("Profiling");

			// export buttons
			if (curr_selected_sharpening != 0 || curr_selected_aa != 0) {
				if (ImGui::Button("export framebuffer")) {
					if (curr_selected_sharpening != 0) {
						size_t sharpening_id = curr_selected_sharpening - 1;
						effect_sharpening_vec[sharpening_id]._RenderTargetVec.back().export_to_file("output.png");
					} else if (curr_selected_aa != 0) {
						size_t aa_id = curr_selected_aa - 1;
						effect_aa_vec[aa_id]._RenderTargetVec.back().export_to_file("output.png");
					}
				}
			}

			ImGui::Text("CPU time (average): %fs", cpu_time_average);
			ImGui::Text("GPU time (average): %fms", gpu_time_average);

			ImGui::SeparatorText("Basics");

			// input image selection
			{
				std::vector<const char *> image_options;
				for (const auto &name : input_texture_names) {
					image_options.push_back(name.c_str());
				}
				ImGui::Combo("Input Image", &curr_selected_texture, image_options.data(), static_cast<int>(image_options.size()));
			}

			// aa & sharpening selection
			{
				std::vector<const char *> aa_options;
				aa_options.push_back("(none)");
				for (const auto &effect_aa : effect_aa_vec) {
					aa_options.push_back(effect_aa._EffectName.c_str());
				}
				ImGui::Combo("AA", &curr_selected_aa, aa_options.data(), static_cast<int>(aa_options.size()));

				std::vector<const char *> sharpening_options;
				sharpening_options.push_back("(none)");
				for (const auto &effect_sharpening : effect_sharpening_vec) {
					sharpening_options.push_back(effect_sharpening._EffectName.c_str());
				}
				ImGui::Combo("Sharpening", &curr_selected_sharpening, sharpening_options.data(), static_cast<int>(sharpening_options.size()));
			}
		}

		ImGui::SeparatorText("Shader Configs");

		// gpu time measurement start
		if (frame_cnt == 0) {
			glGenQueries(1, &gpu_time_query);
			glBeginQuery(GL_TIME_ELAPSED, gpu_time_query);
		}

		if (curr_selected_aa != 0) {
			size_t aa_id = curr_selected_aa - 1;
			effect_aa_vec[aa_id].execute(input_texture_vec[curr_selected_texture]);

			reflect_shader(effect_aa_vec[aa_id]._ShaderProgramVec);
		}

		if (curr_selected_sharpening != 0) {
			size_t sharpening_id = curr_selected_sharpening - 1;
			if (curr_selected_aa == 0) {
				effect_sharpening_vec[sharpening_id].execute(input_texture_vec[curr_selected_texture]);
			} else {
				size_t aa_id = curr_selected_aa - 1;
				effect_sharpening_vec[sharpening_id].execute(effect_aa_vec[aa_id]._RenderTargetVec.back());
			}

			reflect_shader(effect_sharpening_vec[sharpening_id]._ShaderProgramVec);
		}

		ImGui::End();

		default_pass.use(render_pass_config, [&]() {
			g_quad_vertex_buffer.use();
			default_pass_shader_program.use();

			if (curr_selected_sharpening != 0) {
				size_t sharpening_id = curr_selected_sharpening - 1;
				effect_sharpening_vec[sharpening_id]._RenderTargetVec.back().use(0);
			} else if (curr_selected_aa != 0) {
				size_t aa_id = curr_selected_aa - 1;
				effect_aa_vec[aa_id]._RenderTargetVec.back().use(0);
			} else {
				input_texture_vec[curr_selected_texture].use(0);
			}
			default_pass_shader_program.set_uniform("u_input_texture_sampler", 0);

			glDrawArrays(GL_TRIANGLES, 0, 6);
		});

		if (frame_cnt == 0) {
			glEndQuery(GL_TIME_ELAPSED);
		}

		// gpu time measurement end
		if (frame_cnt == QUERY_FREQ) {
			GLuint64 result = 0;
			glGetQueryObjectui64v(gpu_time_query, GL_QUERY_RESULT, &result);
			gpu_time_average = (gpu_time_average + static_cast<float>(result) / 1e6f) * 0.5f;

			frame_cnt = 0;
		} else {
			++frame_cnt;
		}

		cpu_time_average = (cpu_time_average + dt) * 0.5f;
	});

	glDeleteQueries(1, &gpu_time_query);

	app.shutdown();

	return 0;
}
