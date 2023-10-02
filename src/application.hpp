#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "cgra/cgra_mesh.hpp"
#include "opengl.hpp"

// Basic model that holds the shader, mesh and transform for drawing.
// Can be copied and modified for adding in extra information for drawing
// including textures for texture mapping etc.
struct basic_model {
  GLuint shader = 0;
  cgra::gl_mesh mesh;
  glm::vec3 color{0.7f};
  glm::mat4 model_transform{1.0f};
  GLuint texture;

  void draw(const glm::mat4 &view, const glm::mat4 &projection);
};

class application {
 private:
  glm::vec2 m_window_size_;
  GLFWwindow *m_window_;

  // Orbital camera
  float m_pitch_ = 0.86f;
  float m_yaw_ = -0.86f;
  float m_distance_ = 20.0f;

  // last input
  bool m_left_mouse_down_ = false;
  glm::vec2 m_mouse_position_;

  bool m_show_wireframe_ = false;

  basic_model m_model_;

 public:
  // setup
  explicit application(GLFWwindow *);

  // disable copy constructors (for safety)
  application(const application &) = delete;
  application &operator=(const application &) = delete;

  // rendering callbacks (every frame)
  void render();
  void render_gui();

  // input callbacks
  void cursor_pos_cb(double x_pos, double y_pos);
  void mouse_button_cb(int button, int action, int mods);
  void scroll_cb(double x_offset, double y_offset);
  void key_cb(int key, int scan_code, int action, int mods);
  void char_cb(unsigned int c);
};