#include "application.hpp"

#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"
#include "simplified_mesh.hpp"

void basic_model::draw(const glm::mat4 &view, const glm::mat4 &projection) {
  glm::mat4 model_view = view * model_transform;

  // Load shader and variables into GPU
  glUseProgram(shader);
  glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1,
                     false, value_ptr(projection));
  glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false,
                     glm::value_ptr(model_view));
  glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));

  mesh.draw();
}

application::application(GLFWwindow *window) : m_window_(window) {
  cgra::shader_builder sb;
  sb.set_shader(GL_VERTEX_SHADER,
                CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
  sb.set_shader(GL_FRAGMENT_SHADER,
                CGRA_SRCDIR + std::string("//res//shaders//color_frag.glsl"));
  const GLuint shader = sb.build();

	m_model_bunny.shader = shader;
	m_model_bunny.set_model(load_wavefront_data(CGRA_SRCDIR + std::string("/res/assets/bunny.obj")));
	m_model_bunny.isolevel = 0.007;
	m_model_bunny.build_from_model();

	clouds.shader = shader;
	clouds.simulate();

  m_model_.shader = shader;
  m_model_.mesh = cgra::load_wavefront_data(
                      CGRA_SRCDIR + std::string("/res//assets//teapot.obj"))
                      .build();
  m_model_.color = glm::vec3(1.0f, 0.0f, 0.f);
}

void application::render() {
  // Retrieve the window height
  int width, height;
  glfwGetFramebufferSize(m_window_, &width, &height);

  // Set the window size (we do this every time we call render to support resizing)
  m_window_size_ = glm::vec2(width, height);
  glViewport(0, 0, width, height);

  // Clear the back-buffer
  glClearColor(0.3f, 0.3f, 0.4f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Enable flags for normal/forward rendering
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // Update values that affect camera speed
  const auto current_frame = static_cast<float>(glfwGetTime());
  m_delta_time_ = current_frame - m_last_frame_;
  m_last_frame_ = current_frame;

  const glm::mat4 projection =
      glm::perspective(1.f, static_cast<float>(width) / height, 0.1f, 1000.f);

	glPolygonMode(GL_FRONT_AND_BACK, (m_showWireframe) ? GL_LINE : GL_FILL);

	// draw the model
	m_model_bunny.draw(glm::scale(glm::translate(m_camera_.view_matrix(), vec3(0, 0, 0)), vec3(15)), proj);

	clouds.draw(view, proj);
  m_model_.draw(m_camera_.view_matrix(), projection);
}

void application::render_gui() {
  ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiSetCond_Once);
  ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiSetCond_Once);
  ImGui::Begin("Options", nullptr);

  ImGui::Text("Application %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

  ImGui::Text("Camera Pitch %.1f", static_cast<double>(m_camera_.m_pitch));
  ImGui::Text("Camera Yaw %.1f", static_cast<double>(m_camera_.m_yaw));

  ImGui::Checkbox("Wireframe", &m_show_wireframe_);
  ImGui::SameLine();
  if (ImGui::Button("Screenshot")) cgra::rgba_image::screenshot(true);

	// helpful drawing options
	ImGui::Checkbox("Show axis", &m_show_axis);
	ImGui::SameLine();
	ImGui::Checkbox("Show grid", &m_show_grid);
	ImGui::Checkbox("Wireframe", &m_showWireframe);
	ImGui::SameLine();
	if (ImGui::Button("Screenshot")) rgba_image::screenshot(true);

	ImGui::Separator();

	if (ImGui::SliderFloat("Voxel Size", &voxelEdgeLength, 0.004, 0.03)) {
		m_model_bunny.voxelEdgeLength = voxelEdgeLength;
		m_model_bunny.build_from_model();
	}

	if (ImGui::SliderFloat("Isolevel", &isolevel, 0.01, 0.1)) {
		m_model_bunny.isolevel = isolevel;
		m_model_bunny.build_from_model();
	}

	if (ImGui::Checkbox("Normal Smoothing", &smoothing)) {
		m_model_bunny.smoothNormals = smoothing;
		m_model_bunny.build_from_model();
		clouds.mesh.smoothNormals = smoothing;
		clouds.mesh.build();
	}

	if (ImGui::Combo("Debugging", &debugging, "None\0Bounding Box\0Voxel Collisions\0Marching Cubes\0Final\0", 5)) {
		m_model_bunny.debugging = debugging;
		m_model_bunny.build_from_model();
	}

	// finish creating window
	ImGui::End();
}

void application::cursor_pos_cb(const double x_pos, const double y_pos) {
  if (m_first_mouse_) {
    m_mouse_position_ = glm::vec2(x_pos, y_pos);
    m_first_mouse_ = false;
  }

  if (m_middle_mouse_down_) {
    const float x_offset = static_cast<float>(x_pos) - m_mouse_position_.x;
    const float y_offset = m_mouse_position_.y - static_cast<float>(y_pos);

    // Update the camera to use the new offsets
    m_camera_.update_angle(x_offset, y_offset);
  }

  m_mouse_position_ = glm::vec2(x_pos, y_pos);
}

void application::mouse_button_cb(const int button, const int action, const int mods) {
  (void)mods;

  switch(button) {
    case GLFW_MOUSE_BUTTON_LEFT: {
      m_left_mouse_down_ = (action == GLFW_PRESS);
      break;
    }
    case GLFW_MOUSE_BUTTON_MIDDLE: {
      m_middle_mouse_down_ = (action == GLFW_PRESS);
      break;
    }
    default: break;
  }
}

void application::scroll_cb(const double x_offset, const double y_offset) {
  (void)x_offset;
  (void)y_offset;
}

void application::key_cb(const int key, const int scan_code, const int action, const int mods) {
  (void)scan_code, (void)mods;

  switch (key) {
    case GLFW_KEY_W: {
      m_camera_.move(camera_movement::forward, m_delta_time_);
      break;
    }
    case GLFW_KEY_S: {
      m_camera_.move(camera_movement::backward, m_delta_time_);
      break;
    }
    case GLFW_KEY_A: {
      m_camera_.move(camera_movement::left, m_delta_time_);
      break;
    }
    case GLFW_KEY_D: {
      m_camera_.move(camera_movement::right, m_delta_time_);
      break;
    }
    default: break;
  }
}

void application::char_cb(const unsigned int c) {
  (void)c;
}
