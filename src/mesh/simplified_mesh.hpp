#ifndef SIMPLIFIED_MESH_HPP
#define SIMPLIFIED_MESH_HPP

#include <cgra/cgra_mesh.hpp>

#include "utils/opengl.hpp"

// Blender KDtree
#include "mesh/simplified_mesh_debugging.hpp"

class simplified_mesh {
 public:
  std::vector<std::vector<std::vector<float>>> m_grid;

  GLuint m_shader = 0;
  cgra::gl_mesh m_mesh;
  float m_iso_level = 0.6f;

  cgra::mesh_builder m_builder;

  simplified_mesh_debugging m_debugging = result;
  float m_voxel_edge_length = 0.01f;
  bool m_smooth_normals = true;

  // Bounding box
  glm::vec3 m_bb_top_right;
  glm::vec3 m_bb_bottom_left;

  simplified_mesh() = default;
  auto draw(const glm::mat4& view, const glm::mat4& proj) const -> void;
  auto set_model(const cgra::mesh_builder& builder) -> void;
  // Builds the mesh from the unsigned distance field
  auto build() -> void;
  // Builds the unsigned distance field from the model, then runs build
  auto build_from_model() -> void;
};

#endif  // SIMPLIFIED_MESH_HPP
