#include "application.hpp"

#include <chrono>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"

void basic_model::draw(const glm::mat4 &view, const glm::mat4 &projection) {
  glm::mat4 model_view = view * model_transform;

  glUseProgram(shader);  // load shader and variables
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

  m_window_size_ = glm::vec2(width, height);
  glViewport(0, 0, width, height);

  // Clear the back-buffer
  glClearColor(0.3f, 0.3f, 0.4f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Enable flags for normal/forward rendering
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  const glm::mat4 projection =
      glm::perspective(1.f, static_cast<float>(width) / height, 0.1f, 1000.f);

  // Update values that affect camera speed
  const auto current_frame = static_cast<float>(glfwGetTime());
  m_delta_time_ = current_frame - m_last_frame_;
  m_last_frame_ = current_frame;

  // Camera
  const auto view = glm::lookAt(m_camera_pos_, m_camera_pos_ + m_camera_front_, m_camera_up_);

  // helpful draw options
  glPolygonMode(GL_FRONT_AND_BACK, (m_show_wireframe_) ? GL_LINE : GL_FILL);

  // draw the model
  m_model_.draw(view, projection);
}

void application::render_gui() {
  ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiSetCond_Once);
  ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiSetCond_Once);
  ImGui::Begin("Options", nullptr);

  ImGui::Text("Application %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

  ImGui::Text("Camera Pitch %.1f", static_cast<double>(m_camera_pitch_));
  ImGui::Text("Camera Yaw %.1f", static_cast<double>(m_camera_yaw_));

  ImGui::Checkbox("Wireframe", &m_show_wireframe_);
  ImGui::SameLine();
  if (ImGui::Button("Screenshot")) cgra::rgba_image::screenshot(true);

  ImGui::End();
}

void application::cursor_pos_cb(const double x_pos, const double y_pos) {
  if (m_first_mouse_) {
    m_mouse_position_ = glm::vec2(x_pos, y_pos);
    m_first_mouse_ = false;
  }

  float x_offset = static_cast<float>(x_pos) - m_mouse_position_.x;
  float y_offset = m_mouse_position_.y - static_cast<float>(y_pos);

  x_offset *= m_sensitivity_;
  y_offset *= m_sensitivity_;

  m_camera_yaw_ += x_offset;
  m_camera_pitch_ += y_offset;

  if (m_camera_pitch_ > m_max_pitch_)
    m_camera_pitch_ = m_max_pitch_;
  if (m_camera_pitch_ < -m_max_pitch_)
    m_camera_pitch_ = -m_max_pitch_;

  m_direction_.x = cos(glm::radians(m_camera_yaw_)) * cos(glm::radians(m_camera_pitch_));
  m_direction_.y = sin(glm::radians(m_camera_pitch_));
  m_direction_.z = sin(glm::radians(m_camera_yaw_)) * cos(glm::radians(m_camera_pitch_));
  m_camera_front_ = glm::normalize(m_direction_);

  // updated mouse position
  m_mouse_position_ = glm::vec2(x_pos, y_pos);
}

void application::mouse_button_cb(const int button, const int action, const int mods) {
  (void)mods;  // currently un-used

  // capture is left-mouse down
  if (button == GLFW_MOUSE_BUTTON_LEFT)
    // only other option is GLFW_RELEASE
    m_left_mouse_down_ = (action == GLFW_PRESS);
}

void application::scroll_cb(const double x_offset, const double y_offset) {
  (void)x_offset;  // currently un-used
  (void)y_offset;
}

void application::key_cb(const int key, const int scan_code, const int action, const int mods) {
  (void)scan_code, (void)action, (void)mods;  // currently un-used
  const float speed = m_camera_speed_ * m_delta_time_;

  switch (key) {
    case GLFW_KEY_W: {
      m_camera_pos_ += speed * m_camera_front_;
      break;
    }
    case GLFW_KEY_S: {
      m_camera_pos_ -= speed * m_camera_front_;
      break;
    }
    case GLFW_KEY_A: {
      m_camera_pos_ -= glm::normalize(glm::cross(m_camera_front_, m_camera_up_)) * speed;
      break;
    }
    case GLFW_KEY_D: {
      m_camera_pos_ += glm::normalize(glm::cross(m_camera_front_, m_camera_up_)) * speed;
      break;
    }
  default: break;
  }
}

void application::char_cb(const unsigned int c) {
  (void)c;  // currently un-used
}
