#pragma once

#include <glm/glm.hpp>

#include "cgra/cgra_mesh.hpp"
#include "utils/opengl.hpp"

/// Code Author: Shekinah Pratap

/**
 * \brief Class for creating and rendering a terrain model
 */
class terrain_model {
 public:
  GLuint m_shader;
  cgra::gl_mesh m_mesh;
  cgra::mesh_builder m_builder;

  std::vector<std::vector<int>> m_adjacent_faces;

  // variables
  int m_tex = 1;
  cgra::mesh_vertex m_selected_point;
  float m_radius = 25;
  bool m_is_bump = true;
  float m_strength = 7;
  float m_height_change1 = 7;
  float m_height_change2 = -25;
  float m_height_scale = 1;
  int m_grid_size = 200;

  // noise variables
  unsigned int m_seed = 0;
  unsigned int m_octaves = 5;
  float m_lacunarity = 2.0f;
  float m_persistence = 0.5f;
  unsigned int m_repeat = 3;
  float m_height = 200.0f;

  terrain_model() = default;
  auto draw(const glm::mat4& view, const glm::mat4& projection) const -> void;
  void create_terrain(bool perlin);

 private:
  float m_spacing_ = 5.0f;  // spacing between grid points
  int m_type_ = 0;
};
