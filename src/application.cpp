#include "application.hpp"

#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"
#include "simplified_mesh.hpp"
#include "texture_loader.hpp"
#include "mesh_deformation.hpp"
#include <terrain_model.hpp>
#include <random>


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

  // shek
  glUseProgram(shader);
  texture_loader tl{};
  tl.loadTextures(shader);

  m_terrain.shader = shader;
  m_terrain.createFlatGround();

  m_mesh_deform.setModel(m_terrain);
  m_mesh_deform.deformMesh(m_terrain.selectedPoint, m_terrain.m_isBump, 0, 0); // initial computation of TBN, normals
  m_terrain = m_mesh_deform.getModel();
  // end shek

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

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> randVertices(0, m_terrain.builder.getVertices().size());
  std::uniform_real_distribution<float> sizeTree(3, 7);

  for (int i = 0; i < TreeAmount; i++) {
      Tree T;
      T.m_shader = shader;
      m_treePositions.push_back(randVertices(gen));
	  m_trees.push_back(T);
      m_treeSizes.push_back(sizeTree(gen));
  }
  m_trees[0].generateLeaves(5, 500);
  m_trees[0].generateTree();
  for (int i = 1; i < m_trees.size(); i++) {
      m_trees[i].Branches = m_trees[0].Branches;
      m_trees[i].mesh = m_trees[0].mesh;
      m_trees[i].leaves = m_trees[0].leaves;
  }
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
  m_camera_.update(m_delta_time_);

  const glm::mat4 projection =
      glm::perspective(1.f, static_cast<float>(width) / height, 0.1f, 10000.f);

  glPolygonMode(GL_FRONT_AND_BACK, (m_show_wireframe_) ? GL_LINE : GL_FILL);


  // draw the terrain first to not mess up the other objects!!!!
  m_terrain.draw(m_camera_.view_matrix(), projection);
  m_mesh_deform.m_view = m_camera_.view_matrix();
  m_mesh_deform.m_proj = projection;

  // draw the model
  m_model_bunny.draw(glm::scale(glm::translate(m_camera_.view_matrix(), vec3(15, 0, 0)), vec3(15)), projection);
  
  clouds.draw(m_camera_.view_matrix(), projection);
  m_model_.draw(m_camera_.view_matrix(), projection);

  for (int i = 0; i < m_trees.size(); i++) {
      m_trees[i].draw(m_camera_.view_matrix(), projection);
      mesh_vertex terrainVertices = m_terrain.builder.getVertex(m_treePositions[i]);
      mat4 translationMatrix = glm::translate(mat4(1.0f), terrainVertices.pos);
      m_trees[i].modelTranslate = translationMatrix;
      m_trees[i].modelScale = vec3(m_treeSizes[i]);
  }

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

  ImGui::End();

  // Voxel window
  ImGui::SetNextWindowPos(ImVec2(5, 215), ImGuiSetCond_Once);
  ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiSetCond_Once);
  ImGui::Begin("Voxel Settings", nullptr);

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

  if (ImGui::Combo("Debugging", (int*)(&debugging), "None\0Bounding Box\0Voxel Collisions\0Marching Cubes\0Final\0", 5)) {
    m_model_bunny.debugging = debugging;
    m_model_bunny.build_from_model();
    clouds.mesh.debugging = debugging;
    clouds.mesh.build();
  }

  if (ImGui::SliderFloat("Cloud threshold", &clouds.cloudThreshold, 0.1, 0.8)) {
    clouds.simulate();
  }

  if (ImGui::SliderFloat("Cloud Fade Out", &clouds.fadeOutRange, 0.0, 20.0)) {
    clouds.simulate();
  }


  // finish creating window
  ImGui::End();

  // Mesh Editing & Texturing window
  int width, height;
  glfwGetFramebufferSize(m_window_, &width, &height);
  // setup window
  ImGui::SetNextWindowPos(ImVec2(width - 305, 5), ImGuiSetCond_Once);
  ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiSetCond_Once);
  ImGui::Begin("Mesh Editing & Texturing", 0);

  if (ImGui::SliderFloat("Radius", &m_terrain.m_radius, 0, 100, "%.2f", 2.0f)) m_mesh_deform.setModel(m_terrain);
  if (ImGui::SliderFloat("Strength", &m_terrain.m_strength, 0, 10, "%.2f", 2.0f)) m_mesh_deform.setModel(m_terrain);
  if (ImGui::SliderFloat("Grass/Mud Height", &m_terrain.m_heightChange1, -5, 50, "%.2f", 2.0f)) m_mesh_deform.setModel(m_terrain);
  if (ImGui::SliderFloat("Mud/Rocks Height", &m_terrain.m_heightChange2, -50, 5, "%.2f", 2.0f)) m_mesh_deform.setModel(m_terrain);
  if (ImGui::SliderFloat("Heightmap Scale", &m_terrain.m_heightScale, 0, 1, "%.2f", 2.0f)) m_mesh_deform.setModel(m_terrain);

  if (ImGui::RadioButton("Normal Map", (m_terrain.m_tex == 1) ? true : false)) {
      m_terrain.m_tex = 1 - m_terrain.m_tex;
      m_mesh_deform.setModel(m_terrain);
  }

  bool notBump = !m_terrain.m_isBump;
  if (ImGui::Checkbox("Raise", &m_terrain.m_isBump)) {
      notBump = !m_terrain.m_isBump;
      m_mesh_deform.setModel(m_terrain);
  }
  ImGui::SameLine();
  if (ImGui::Checkbox("Excavate", &notBump)) {
      m_terrain.m_isBump = !notBump;
      m_mesh_deform.setModel(m_terrain);
  }
  ImGui::SameLine();
  if (ImGui::Button("Deform")) {
      m_mesh_deform.deformMesh(m_terrain.selectedPoint, m_terrain.m_isBump, m_terrain.m_radius, m_terrain.m_strength);
      m_terrain = m_mesh_deform.getModel();
  }

  // finish creating window
  ImGui::End();

  // Tree window
  ImGui::SetNextWindowPos(ImVec2(width - 305, 215), ImGuiSetCond_Once);
  ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiSetCond_Once);
  ImGui::Begin("Tree Settings", nullptr);

  if (ImGui::Button("Spooky Mode")) {
      for (int i = 0; i < m_trees.size(); i++) {
          m_trees[i].spookyMode = !m_trees[i].spookyMode;
      }
  }
  if (ImGui::Button("New Tree")) {
      m_trees[0].generateLeaves(5, 500);
      m_trees[0].generateTree();
      for (int i = 1; i < m_trees.size(); i++) {
          m_trees[i].Branches = m_trees[0].Branches;
          m_trees[i].mesh = m_trees[0].mesh;
          m_trees[i].leaves = m_trees[0].leaves;
      }
  }
  if (ImGui::Button("Print Tree")) {
      m_trees[0].printTree();
  }

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
        // Capture is left-mouse down
        m_left_mouse_down_ = (action == GLFW_PRESS);
        if (m_left_mouse_down_) {
            double xpos, ypos;
            glfwGetCursorPos(m_window_, &xpos, &ypos);
            m_mesh_deform.mouseIntersectMesh(xpos, ypos, m_window_size_.x, m_window_size_.y);
            m_terrain = m_mesh_deform.getModel();
        }
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
      default: break;
    }
  } else if (action == GLFW_RELEASE) {
    m_camera_.set_direction(camera_movement::rest);
  }
}

void application::char_cb(const unsigned int c) {
  (void)c;
}