#include "application.hpp"

#include <imgui.h>

#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <string>

#include "cgra/cgra_basic_model.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"
#include "mesh/mesh_deformation.hpp"
#include "mesh/simplified_mesh.hpp"
#include "terrain/terrain_model.hpp"
#include "trees/trees.hpp"
#include "utils/texture_loader.hpp"

application::application(GLFWwindow *window)
    : m_window_size_(0, 0), m_window_(window), m_mouse_position_(0, 0) {
  // Load shaders
  cgra::shader_builder shader_builder;
  shader_builder.set_shader(
      GL_VERTEX_SHADER,
      CGRA_SRCDIR + std::string("//res//shaders//color_vs.glsl"));
  shader_builder.set_shader(
      GL_FRAGMENT_SHADER,
      CGRA_SRCDIR + std::string("//res//shaders//color_fs.glsl"));
  const GLuint shader = shader_builder.build();

  cgra::shader_builder terrain_shader_builder;
  terrain_shader_builder.set_shader(
      GL_VERTEX_SHADER,
      CGRA_SRCDIR + std::string("//res//shaders//terrain.vs.glsl"));
  terrain_shader_builder.set_shader(
      GL_FRAGMENT_SHADER,
      CGRA_SRCDIR + std::string("//res//shaders//terrain.fs.glsl"));
  const GLuint terrain_shader = terrain_shader_builder.build();

  // Load textures
  glUseProgram(terrain_shader);
  texture_loader tl{};
  tl.load_textures(terrain_shader);

  // create terrain mesh
  m_terrain_.m_shader = terrain_shader;
  m_terrain_.create_terrain(m_use_perlin_);
  m_mesh_deform_.set_model(m_terrain_);
  m_mesh_deform_.deform_mesh(m_terrain_.m_selected_point, m_terrain_.m_is_bump,
                             0.0f, 0.0f);
  m_terrain_ = m_mesh_deform_.get_model();

  glUseProgram(shader);
  // Uncomment to include a voxelized bunny model
  //m_model_bunny_.m_shader = shader;
  //m_model_bunny_.set_model(cgra::load_wavefront_data(
  //    CGRA_SRCDIR + std::string("/res/assets/bunny.obj")));
  //m_model_bunny_.m_iso_level = 0.007f;
  //m_model_bunny_.build_from_model();

  m_clouds_.m_shader = shader;
  m_clouds_.simulate();

  // Uncomment to include a teapot model
  //m_model_.shader = shader;
  //m_model_.mesh = cgra::load_wavefront_data(
  //                    CGRA_SRCDIR + std::string("/res//assets//teapot.obj"))
  //                    .build();
  //m_model_.color = glm::vec3(1.0f, 0.0f, 0.f);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> rand_vertices(
      0, static_cast<int>(m_terrain_.m_builder.get_vertices().size()));
  std::uniform_real_distribution<float> size_tree(3.0f, 7.0f);

  for (auto i = 0; i < m_num_trees_; ++i) {
    tree t;
    t.m_shader = shader;
    m_tree_positions_.push_back(rand_vertices(gen));
    m_trees_.push_back(t);
    m_tree_sizes_.push_back(size_tree(gen));
  }

  m_trees_[0].generate_leaves(5, 500);
  m_trees_[0].generate_tree();
  const auto size = static_cast<int>(m_trees_.size());
  for (auto i = 1; i < size; ++i) {
    m_trees_[i].m_branches = m_trees_[0].m_branches;
    m_trees_[i].m_mesh = m_trees_[0].m_mesh;
    m_trees_[i].m_leaves = m_trees_[0].m_leaves;
  }
}

auto application::render() -> void {
  // Retrieve the window height
  int width, height;
  glfwGetFramebufferSize(m_window_, &width, &height);

  // Set the window size (we do this every time we call render to support
  // resizing)
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
  m_camera_.update(m_delta_time_, m_terrain_);

  const glm::mat4 projection = glm::perspective(
      1.f, static_cast<float>(width) / static_cast<float>(height), 0.1f,
      10000.f);

  glPolygonMode(GL_FRONT_AND_BACK, (m_show_wireframe_) ? GL_LINE : GL_FILL);

  // draw the terrain first to not mess up the other objects!!!!
  m_terrain_.draw(m_camera_.view_matrix(), projection);
  m_mesh_deform_.m_view = m_camera_.view_matrix();
  m_mesh_deform_.m_projection = projection;

  // draw the model
  //m_model_bunny_.draw(glm::scale(glm::translate(m_camera_.view_matrix(),
  //                                              glm::vec3(15.0f, 50.0f, 0.0f)),
  //                               glm::vec3(15.0f)),
  //                    projection);

  m_clouds_.draw(m_camera_.view_matrix(), projection);
  //m_model_.draw(m_camera_.view_matrix(), projection);

  const auto size = static_cast<int>(m_trees_.size());
  for (auto i = 0; i < size; i++) {
    m_trees_[i].draw(m_camera_.view_matrix(), projection);
    auto terrain_vertices =
        m_terrain_.m_builder.get_vertex(m_tree_positions_[i]);

    const glm::mat4 translation_matrix =
        glm::translate(glm::mat4(1.0f), terrain_vertices.pos);
    m_trees_[i].m_model_translate = translation_matrix;
    m_trees_[i].m_model_scale = glm::vec3(m_tree_sizes_[i]);
  }

  m_skybox_.draw(m_camera_.view_matrix(), projection);
}

auto application::render_gui() -> void {
  ImGui::SetNextWindowPos(m_window_pos_);
  ImGui::SetNextWindowSize(m_option_window_dimensions_);
  ImGui::Begin("Options", nullptr);

  ImGui::Text("Application %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

  ImGui::Text("Camera Pitch %.1f", static_cast<double>(m_camera_.m_pitch));
  ImGui::Text("Camera Yaw %.1f", static_cast<double>(m_camera_.m_yaw));

  ImGui::Checkbox("Wireframe", &m_show_wireframe_);
  ImGui::SameLine();
  if (ImGui::Button("Screenshot")) cgra::rgba_image::screenshot(true);

  ImGui::End();

  // Voxel window
  m_window_pos_ = ImVec2(m_window_pos_.x, m_window_pos_.y + m_option_window_dimensions_.y + 5);
  ImGui::SetNextWindowPos(m_window_pos_);
  ImGui::SetNextWindowSize(m_voxel_window_dimensions_);
  ImGui::Begin("Voxel Settings", nullptr);

  if (ImGui::SliderFloat("Voxel Size", &m_voxel_edge_length_, 0.004f, 0.03f)) {
    //m_model_bunny_.m_voxel_edge_length = m_voxel_edge_length_;
    //m_model_bunny_.build_from_model();
  }

  if (ImGui::SliderFloat("Iso Level", &m_iso_level_, 0.01f, 0.1f)) {
    //m_model_bunny_.m_iso_level = m_iso_level_;
    //m_model_bunny_.build_from_model();
  }

  if (ImGui::Checkbox("Normal Smoothing", &m_smoothing_)) {
    //m_model_bunny_.m_smooth_normals = m_smoothing_;
    //m_model_bunny_.build_from_model();
    m_clouds_.mesh.m_smooth_normals = m_smoothing_;
    m_clouds_.mesh.build();
  }

  if (ImGui::Combo(
          "Debugging", reinterpret_cast<int *>(&m_debugging_),
          "None\0Bounding Box\0Voxel Collisions\0Marching Cubes\0Final\0", 5)) {
    //m_model_bunny_.m_debugging = m_debugging_;
    //m_model_bunny_.build_from_model();
    m_clouds_.mesh.m_debugging = m_debugging_;
    m_clouds_.mesh.build();
  }

  if (ImGui::SliderFloat("Cloud threshold", &m_clouds_.cloud_threshold, 0.1f, 0.8f)) {
    m_clouds_.simulate();
  }

  if (ImGui::SliderFloat("Cloud Fade Out", &m_clouds_.fade_out_range, 0.0f, 20.0f)) {
    m_clouds_.simulate();
  }

  ImGui::End();

  // Mesh Editing & Texturing window
  // Setup window
  m_window_pos_ = { m_window_size_.x - m_mesh_window_dimensions_.x - m_starting_position_.x, m_starting_position_.y };
  ImGui::SetNextWindowPos(m_window_pos_);
  ImGui::SetNextWindowSize(m_mesh_window_dimensions_);
  ImGui::Begin("Mesh Editing & Texturing", nullptr);

  if (ImGui::SliderFloat("Radius", &m_terrain_.m_radius, 0, 100, "%.2f"))
    m_mesh_deform_.set_model(m_terrain_);
  if (ImGui::SliderFloat("Strength", &m_terrain_.m_strength, 0, 10, "%.2f"))
    m_mesh_deform_.set_model(m_terrain_);
  if (ImGui::SliderFloat("Grass/Mud Height", &m_terrain_.m_height_change1, -5, 50, "%.2f"))
    m_mesh_deform_.set_model(m_terrain_);
  if (ImGui::SliderFloat("Mud/Rocks Height", &m_terrain_.m_height_change2, -50, 5, "%.2f"))
    m_mesh_deform_.set_model(m_terrain_);
  if (ImGui::SliderFloat("Height Map Scale", &m_terrain_.m_height_scale, 0, 1, "%.2f"))
    m_mesh_deform_.set_model(m_terrain_);

  if (ImGui::RadioButton("Normal Map", (m_terrain_.m_tex == 1))) {
    m_terrain_.m_tex = 1 - m_terrain_.m_tex;
    m_mesh_deform_.set_model(m_terrain_);
  }

  bool not_bump = !m_terrain_.m_is_bump;
  if (ImGui::Checkbox("Raise", &m_terrain_.m_is_bump)) {
    not_bump = !m_terrain_.m_is_bump;
    m_mesh_deform_.set_model(m_terrain_);
  }
  ImGui::SameLine();
  if (ImGui::Checkbox("Excavate", &not_bump)) {
    m_terrain_.m_is_bump = !not_bump;
    m_mesh_deform_.set_model(m_terrain_);
  }
  ImGui::SameLine();
  if (ImGui::Button("Deform")) {
    m_mesh_deform_.deform_mesh(m_terrain_.m_selected_point,
                               m_terrain_.m_is_bump, m_terrain_.m_radius,
                               m_terrain_.m_strength);
    m_terrain_ = m_mesh_deform_.get_model();
  }

  ImGui::SliderInt("Octaves", reinterpret_cast<int *>(&m_terrain_.m_octaves), 1, 10);
  ImGui::SliderFloat("Lacunarity", &m_terrain_.m_lacunarity, 0.0f, 10.0f);
  ImGui::SliderFloat("Persistence", &m_terrain_.m_persistence, 0.0f, 10.0f);
  ImGui::SliderFloat("Height", &m_terrain_.m_height, 0.0f, 1000.0f);
  ImGui::SliderInt("Repeats", reinterpret_cast<int *>(&m_terrain_.m_repeat), 0, 10);
  ImGui::SliderInt("Seed", reinterpret_cast<int *>(&m_terrain_.m_seed), 0, 100);

  bool not_flat = !m_use_perlin_;
  if (ImGui::Checkbox("Perlin", &m_use_perlin_)) not_flat = !m_use_perlin_;

  ImGui::SameLine();
  if (ImGui::Checkbox("Flat", &not_flat)) m_use_perlin_ = !not_flat;

  ImGui::SameLine();
  if (ImGui::Button("Recreate Terrain")) {
    m_terrain_.create_terrain(m_use_perlin_);
    m_mesh_deform_.set_model(m_terrain_);
    m_mesh_deform_.deform_mesh(m_terrain_.m_selected_point,
                               m_terrain_.m_is_bump, 0,
                               0);  // initial computation of TBN, normals
    m_terrain_ = m_mesh_deform_.get_model();
  }

  ImGui::End();

  // Tree Settings window
  m_window_pos_ =
      ImVec2(m_window_pos_.x, m_window_pos_.y + m_mesh_window_dimensions_.y +
                                  m_starting_position_.y);
  ImGui::SetNextWindowPos(m_window_pos_);
  ImGui::SetNextWindowSize(m_tree_window_dimensions_);
  ImGui::Begin("Tree Settings", nullptr);

  if (ImGui::Button("Spooky Mode")) {
    for (auto &tree : m_trees_) {
      tree.m_spooky_mode = !tree.m_spooky_mode;
    }
  }

  ImGui::SameLine();

  if (ImGui::Button("New Tree")) {
    m_trees_[0].generate_leaves(5, 500);
    m_trees_[0].generate_tree();

    const auto size = static_cast<int>(m_trees_.size());
    for (auto i = 1; i < size; i++) {
      m_trees_[i].m_branches = m_trees_[0].m_branches;
      m_trees_[i].m_mesh = m_trees_[0].m_mesh;
      m_trees_[i].m_leaves = m_trees_[0].m_leaves;
    }
  }

  ImGui::SameLine();

  if (ImGui::Button("Print Tree")) {
    m_trees_[0].print_tree();
  }

  ImGui::End();
}

auto application::cursor_pos_cb(const double x_pos, const double y_pos)
    -> void {
  if (m_first_mouse_) {
    m_mouse_position_ =
        glm::vec2(static_cast<float>(x_pos), static_cast<float>(y_pos));
    m_first_mouse_ = false;
  }

  if (m_middle_mouse_down_) {
    const float x_offset = static_cast<float>(x_pos) - m_mouse_position_.x;
    const float y_offset = m_mouse_position_.y - static_cast<float>(y_pos);

    // Update the camera to use the new offsets
    m_camera_.update_angle(x_offset, y_offset);
  }

  m_mouse_position_ =
      glm::vec2(static_cast<float>(x_pos), static_cast<float>(y_pos));
}

auto application::mouse_button_cb(const int button, const int action,
                                  const int mods) -> void {
  (void)mods;

  switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT: {
      // Capture is left-mouse down
      m_left_mouse_down_ = (action == GLFW_PRESS);
      if (m_left_mouse_down_) {
        double x_pos, y_pos;
        glfwGetCursorPos(m_window_, &x_pos, &y_pos);
        m_mesh_deform_.mouse_intersect_mesh(x_pos, y_pos, m_window_size_.x,
                                            m_window_size_.y);
        m_terrain_ = m_mesh_deform_.get_model();
      }
      break;
    }
    case GLFW_MOUSE_BUTTON_MIDDLE: {
      m_middle_mouse_down_ = (action == GLFW_PRESS);
      break;
    }
    default:
      break;
  }
}

auto application::scroll_cb(const double x_offset, const double y_offset)
    -> void {
  (void)x_offset;
  (void)y_offset;
}

auto application::key_cb(const int key, const int scan_code, const int action,
                         const int mods) -> void {
  (void)scan_code, (void)mods;

  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_W: {
        m_camera_.set_direction(camera_movement::forward);
        break;
      }
      case GLFW_KEY_S: {
        m_camera_.set_direction(camera_movement::backward);
        break;
      }
      case GLFW_KEY_A: {
        m_camera_.set_direction(camera_movement::left);
        break;
      }
      case GLFW_KEY_D: {
        m_camera_.set_direction(camera_movement::right);
        break;
      }
      default:
        break;
    }
  } else if (action == GLFW_RELEASE) {
    m_camera_.set_direction(camera_movement::rest);
  }
}

auto application::char_cb(const unsigned int c) -> void { (void)c; }
