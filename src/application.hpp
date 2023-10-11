#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "utils/camera.hpp"
#include "cgra/cgra_basic_model.hpp"
#include "mesh/simplified_mesh.hpp"
#include "clouds/cloud_model.hpp"
#include "utils/opengl.hpp"
#include "mesh/simplified_mesh_debugging.hpp"
#include "mesh/mesh_deformation.hpp"
#include "terrain/terrain_model.hpp"
#include "trees/Tree.hpp"
#include "utils/skybox.hpp"

/**
 * \brief Represents the application which is responsible for rendering the scene and the
 * user interface.
 */
class application {
 private:
  glm::vec2 m_window_size_;
  GLFWwindow *m_window_;

  // Flying Camera
  const glm::vec3 m_camera_pos_   = { -5.0f, 185.0f, 752.0f };
  const glm::vec3 m_camera_front_ = {  0.0f,   0.0f,  -1.0f };
  const glm::vec3 m_camera_up_    = {  0.0f,   1.0f,   0.0f };
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
  simplified_mesh m_model_bunny;

  cloud_model clouds;

  float voxelEdgeLength = 0.01;
  float isolevel = 0.007;
  bool smoothing = true;

  simplified_mesh_debugging debugging = simplified_mesh_debugging::result;

  // end Voxel Settings

  // Mesh editing and texturing
  terrain_model m_terrain;
  mesh_deformation m_mesh_deform;

  //Tree Values
  basic_model m_model;

  int TreeAmount = 35;

  vector<Tree> m_trees;
  vector<int> m_treePositions;
  vector<float> m_treeSizes;

  skybox m_skybox_{};

 public:
  explicit application(GLFWwindow *);

  // The copy and assignment constructors are deleted to prevent multiple
  // instances of the application existing.
  application(const application &) = delete;
  application &operator=(const application &) = delete;

  /**
   * \brief Renders the application to the screen.
   */
  void render();

  /**
   * \brief Renders the user interface components to the screen.
   */
  void render_gui();

  /**
   * \brief A callback function to respond to the cursor moving.
   * \param x_pos The x coordinate of the cursor's position.
   * \param y_pos The y coordinate of the cursor's position.
   */
  void cursor_pos_cb(double x_pos, double y_pos);

  /**
   * \brief A callback function to respond to mouse button clicks.
   */
  void mouse_button_cb(int button, int action, int mods);

  /**
   * \brief A callback function to respond to the scroll wheel moving.
   */
  [[maybe_unused]] void scroll_cb(double x_offset, double y_offset);

  /**
   * \brief A callback function to respond to keys being pressed or released.
   */
  void key_cb(int key, int scan_code, int action, int mods);

  /**
   * \brief A callback function to respond to characters being typed.
   */
  [[maybe_unused]] void char_cb(unsigned int c);
};
