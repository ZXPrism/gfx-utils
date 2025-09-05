#include <gfx-utils-core/camera.h>

#include <gfx-utils-core/app.h>
#include <gfx-utils-core/config.h>

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

namespace gfxutils {

Camera::Camera() {
	auto &app = App::instance();

	_Projection = glm::perspective(config::fovy, app.get_aspect_ratio(), 0.1f, 100.0f);

	app.register_on_cursor_pos_func([&](double x_pos, double y_pos) {
		static double prev_x_pos = 0.0;
		static double prev_y_pos = 0.0;

		auto dx = static_cast<float>(x_pos - prev_x_pos);
		auto dy = static_cast<float>(y_pos - prev_y_pos);

		dx *= _MouseSens;
		dy *= _MouseSens;

		_Yaw += dx;
		_Pitch -= dy;

		_Pitch = std::clamp(_Pitch, -89.0f, 89.0f);

		prev_x_pos = x_pos;
		prev_y_pos = y_pos;

		_calc_orientation();
	});

	_calc_orientation();
}

void Camera::_calc_orientation() {
	glm::vec3 new_front;
	new_front.x = glm::cos(glm::radians(_Yaw)) * glm::cos(glm::radians(_Pitch));
	new_front.y = glm::sin(glm::radians(_Pitch));
	new_front.z = glm::sin(glm::radians(_Yaw)) * glm::cos(glm::radians(_Pitch));

	_Front = glm::normalize(new_front);
	_Right = glm::normalize(glm::cross(_Front, config::world_up));
	_View = glm::lookAt(_Pos, _Pos + _Front, config::world_up);
}

const glm::mat4 &Camera::get_view() const {
	return _View;
}

const glm::mat4 &Camera::get_projection() const {
	return _Projection;
}

const glm::vec3 &Camera::get_pos() const {
	return _Pos;
}

void Camera::update(float delta_time) {
	auto &app = App::instance();

	bool moved = false;
	float speed = _MoveSpeed * delta_time;

	if (app.check_key_pressed(GLFW_KEY_W)) {
		moved = true;
		_Pos += speed * _Front;
	}
	if (app.check_key_pressed(GLFW_KEY_S)) {
		moved = true;
		_Pos -= speed * _Front;
	}
	if (app.check_key_pressed(GLFW_KEY_A)) {
		moved = true;
		_Pos -= _Right * speed;
	}
	if (app.check_key_pressed(GLFW_KEY_D)) {
		moved = true;
		_Pos += _Right * speed;
	}

	if (moved) {
		_View = glm::lookAt(_Pos, _Pos + _Front, config::world_up);
	}
}

}  // namespace gfxutils
