#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <glm/glm.hpp>

#include "cgra/cgra_basic_model.hpp"
#include "clouds/cloud_model.hpp"
#include "imgui.h"
#include "mesh/mesh_deformation.hpp"
#include "mesh/simplified_mesh.hpp"
#include "mesh/simplified_mesh_debugging.hpp"
#include "terrain/terrain_model.hpp"
#include "trees/trees.hpp"
#include "utils/camera.hpp"
#include "utils/opengl.hpp"
#include "utils/skybox.hpp"

/**
 * \brief Represents the application which is responsible for rendering the
 * scene and the user interface.
 */
class application {
  glm::vec2 m_window_size_;
  GLFWwindow *m_window_;

  // Window Constants
  const ImVec2 m_starting_position_ = {5.0f, 5.0f};
  const ImVec2 m_option_window_dimensions_ = {345.0f, 110.0f};
  const ImVec2 m_voxel_window_dimensions_ = {345.0f, 170.0f};
  const ImVec2 m_mesh_window_dimensions_ = {350.0f, 355.0f};
  const ImVec2 m_tree_window_dimensions_ = {350.0f, 55.0f};
  ImVec2 m_window_pos_ = m_starting_position_;

  // Flying Camera
  glm::vec3 m_camera_pos_ = {-5.0f, 185.0f, 752.0f};
  glm::vec3 m_camera_front_ = {0.0f, 0.0f, -1.0f};
  glm::vec3 m_camera_up_ = {0.0f, 1.0f, 0.0f};
  camera m_camera_{m_camera_pos_, m_camera_up_, m_camera_front_};
  glm::vec3 m_direction_ = glm::vec3(0.0f);

  // Mouse Input
  bool m_left_mouse_down_ = false;
  bool m_middle_mouse_down_ = false;
  bool m_first_mouse_ = true;
  glm::vec2 m_mouse_position_;

  // Time Between Frames
  float m_delta_time_ = 0.0f;
  float m_last_frame_ = 0.0f;

  bool m_show_wireframe_ = false;
  basic_model m_model_;

  // Voxel settings
  // geometry
  simplified_mesh m_model_bunny_;

  cloud_model m_clouds_;

  float m_voxel_edge_length_ = 0.01f;
  float m_iso_level_ = 0.007f;
  bool m_smoothing_ = true;

  simplified_mesh_debugging m_debugging_ = simplified_mesh_debugging::result;

  // Mesh editing and texturing
  terrain_model m_terrain_;
  mesh_deformation m_mesh_deform_;
  bool m_use_perlin_ = true;

  // Tree Values
  int m_num_trees_ = 35;

  std::pmr::vector<tree> m_trees_;
  std::pmr::vector<int> m_tree_positions_;
  std::pmr::vector<float> m_tree_sizes_;

  skybox m_skybox_{};

 public:
  explicit application(GLFWwindow *);
  ~application() = default;

  // The copy, assignment, move, and move assignment constructors are deleted to
  // prevent multiple instances of the application existing.
  application(const application &) = delete;
  application &operator=(const application &) = delete;
  application(application &&other) = delete;
  application &operator=(application &&other) = delete;

  /**
   * \brief Renders the application to the screen.
   */
  auto render() -> void;

  /**
   * \brief Renders the user interface components to the screen.
   */
  auto render_gui() -> void;

  /**
   * \brief A callback function to respond to the cursor moving.
   * \param x_pos The x coordinate of the cursor's position.
   * \param y_pos The y coordinate of the cursor's position.
   */
  auto cursor_pos_cb(double x_pos, double y_pos) -> void;

  /**
   * \brief A callback function to respond to mouse button clicks.
   */
  auto mouse_button_cb(int button, int action, int mods) -> void;

  /**
   * \brief A callback function to respond to the scroll wheel moving.
   */
  [[maybe_unused]] static auto scroll_cb(double x_offset, double y_offset)
      -> void;

  /**
   * \brief A callback function to respond to keys being pressed or released.
   */
  auto key_cb(int key, int scan_code, int action, int mods) -> void;

  /**
   * \brief A callback function to respond to characters being typed.
   */
  [[maybe_unused]] static auto char_cb(unsigned int c) -> void;
};

#endif  // APPLICATION_HPP
