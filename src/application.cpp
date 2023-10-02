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
  glm::mat4 model_view = view * modelTransform;

  glUseProgram(shader);  // load shader and variables
  glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1,
                     false, value_ptr(projection));
  glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false,
                     glm::value_ptr(model_view));
  glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));

  mesh.draw();
}

Application::Application(GLFWwindow *window) : m_window_(window) {
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

void Application::render() {
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

  // Camera
  const glm::mat4 view =
      glm::translate(glm::mat4(1), glm::vec3(0, 0, -m_distance_)) *
      glm::rotate(glm::mat4(1), m_pitch_, glm::vec3(1, 0, 0)) *
      glm::rotate(glm::mat4(1), m_yaw_, glm::vec3(0, 1, 0));

  // helpful draw options
  glPolygonMode(GL_FRONT_AND_BACK, (m_show_wireframe_) ? GL_LINE : GL_FILL);

  // draw the model
  m_model_.draw(view, projection);
}

void Application::render_gui() {
  // setup window
  ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiSetCond_Once);
  ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiSetCond_Once);
  ImGui::Begin("Options", nullptr);

  // display current camera parameters
  ImGui::Text("Application %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

  // helpful drawing options
  ImGui::Checkbox("Wireframe", &m_show_wireframe_);
  ImGui::SameLine();
  if (ImGui::Button("Screenshot")) cgra::rgba_image::screenshot(true);

  // finish creating window
  ImGui::End();
}

void Application::cursor_pos_cb(double x_pos, double y_pos) {
  if (m_left_mouse_down_) {
    const glm::vec2 wh_size = m_window_size_ / 2.0f;

    // clamp the pitch to [-pi/2, pi/2]
    m_pitch_ += static_cast<float>(
        acos(glm::clamp((m_mouse_position_.y - wh_size.y) / wh_size.y, -1.0f,
                        1.0f)) -
        acos(glm::clamp((static_cast<float>(y_pos) - wh_size.y) / wh_size.y,
                        -1.0f, 1.0f)));
    m_pitch_ = static_cast<float>(
        glm::clamp(m_pitch_, -glm::pi<float>() / 2, glm::pi<float>() / 2));

    // wrap the yaw to [-pi, pi]
    m_yaw_ += static_cast<float>(
        acos(glm::clamp((m_mouse_position_.x - wh_size.x) / wh_size.x, -1.0f,
                        1.0f)) -
        acos(glm::clamp((static_cast<float>(x_pos) - wh_size.x) / wh_size.x,
                        -1.0f, 1.0f)));
    if (m_yaw_ > glm::pi<float>())
      m_yaw_ -= static_cast<float>(2 * glm::pi<float>());
    else if (m_yaw_ < -glm::pi<float>())
      m_yaw_ += static_cast<float>(2 * glm::pi<float>());
  }

  // updated mouse position
  m_mouse_position_ = glm::vec2(x_pos, y_pos);
}

void Application::mouse_button_cb(const int button, const int action,
                                  const int mods) {
  (void)mods;  // currently un-used

  // capture is left-mouse down
  if (button == GLFW_MOUSE_BUTTON_LEFT)
    m_left_mouse_down_ =
        (action == GLFW_PRESS);  // only other option is GLFW_RELEASE
}

void Application::scroll_cb(const double x_offset, const double y_offset) {
  (void)x_offset;  // currently un-used
  m_distance_ *= glm::pow(1.1f, -y_offset);
}

void Application::key_cb(const int key, const int scan_code, const int action,
                         const int mods) {
  (void)key, (void)scan_code, (void)action, (void)mods;  // currently un-used
}

void Application::char_cb(const unsigned int c) {
  (void)c;  // currently un-used
}
