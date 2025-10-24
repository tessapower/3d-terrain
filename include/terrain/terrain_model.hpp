#ifndef TERRAIN_MODEL_HPP
#define TERRAIN_MODEL_HPP

#include <glm/glm.hpp>
#include <atomic>
#include <thread>
#include <mutex>

#include "cgra/cgra_mesh.hpp"
#include "utils/opengl.hpp"
#include "utils/aabb_tree.hpp"

/// Code Author(s): Shekinah Pratap, Tessa Power

/**
 * \brief Class for creating and rendering a terrain model
 */
class terrain_model {
 public:
  GLuint m_shader{};
  cgra::gl_mesh m_mesh;
  cgra::mesh_builder m_builder;

  aabb_tree m_aabb_tree;
  std::atomic<bool> aabb_rebuilding{false};  // Track if rebuild is in progress
  std::thread aabb_rebuild_thread;           // Background thread
  std::mutex aabb_mutex;                     // Protect tree access

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
  float m_spacing = 5.0f;
  float m_box_depth = 100.0f;  // Box depth for terrain mesh

  // noise variables
  unsigned int m_seed = 0;
  unsigned int m_octaves = 5;
  float m_lacunarity = 2.0f;
  float m_persistence = 0.5f;
  unsigned int m_repeat = 0;  // Disable repeat - it causes harsh tiling on large terrains
  float m_height = 200.0f;

  terrain_model() = default;
  ~terrain_model() {
    // Ensure thread is joined before destruction
    if (aabb_rebuild_thread.joinable()) {
      aabb_rebuild_thread.join();
    }
  }

  auto draw(const glm::mat4& view, const glm::mat4& projection) const -> void;
  auto create_terrain(bool use_perlin) -> void;

  auto build_aabb_tree() -> void;
  auto build_aabb_tree_async() -> void;
  auto wait_for_aabb_rebuild() -> void;

 private:
  int m_type_ = 0;
};

#endif  // TERRAIN_MODEL_HPP
