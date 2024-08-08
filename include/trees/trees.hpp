#ifndef TREES_HPP
#define TREES_HPP

#include <glm/glm.hpp>
#include <vector>

#include "cgra/cgra_mesh.hpp"
#include "utils/opengl.hpp"

/// Code Author: Adam Goodyear
///
/// Implementing the paper Modeling Trees with a Space Colonization Algorithm by
/// Adam Runions, Brendan Lane, and Przemyslaw Prusinkiewicz:
/// http://algorithmicbotany.org/papers/colonization.egwnp2007.large.pdf

// Branch structure for storing info on branches and generating/resetting them
struct branch {
  int m_id;
  glm::vec3 m_start;
  glm::vec3 m_end;
  glm::vec3 m_direction;
  // direction copy so branch can return to original
  glm::vec3 m_direction_original;
  int m_parent;
  std::vector<int> children;
  std::vector<int> vertices;
  float count = 0.0f;

  // Constructor
  branch(const glm::vec3& start, const glm::vec3& end,
         const glm::vec3& direction, int parent = 0, const int id = 0)
      : m_id(id),
        m_start(start),
        m_end(end),
        m_direction(direction),
        m_direction_original(direction),
        m_parent(parent) {}

  // resets branch position and attractors
  void reset() {
    m_direction = m_direction_original;
    count = 0.0f;
  }
};

// leaf structure for storing info on leaves for foliage gen
struct leaf {
  glm::vec3 m_leaf_color;
  glm::vec3 m_leaf_size;
  glm::vec3 m_leaf_pos;

  // constructor
  leaf(const glm::vec3& leaf_color, const glm::vec3& leaf_size,
       const glm::vec3& leaf_pos)
      : m_leaf_color(leaf_color),
        m_leaf_size(leaf_size),
        m_leaf_pos(leaf_pos) {}
};

// Class for generating and creating Trees
class tree {
 public:
  tree() = default;

  // variables and lists
  GLuint m_shader = 0;
  glm::mat4 m_model_translate{1.0};
  glm::vec3 m_model_scale{1.0f};
  bool m_spooky_mode = false;
  cgra::gl_mesh m_mesh;
  std::vector<leaf> m_leaves;
  std::vector<glm::vec3> m_leaf_positions;
  std::vector<glm::vec3> m_reached_leaves;
  std::vector<branch> m_branches;

  // Functions
  auto draw(const glm::mat4& view, const glm::mat4& projection) const -> void;
  auto generate_leaves(float radius, int leaves) -> void;
  auto generate_tree() -> void;
  auto print_tree() const -> void;
  auto draw_branches_nodes(int current_branch, const glm::mat4& view,
                           const glm::mat4& projection) const -> void;
  static auto process(float x) -> float;
  auto draw_tree(const glm::mat4& view, const glm::mat4& projection) const
      -> void;
  auto generate_mesh() -> void;

 private:
  glm::vec3 start_position_{0.0f};
  float m_branch_length_ = 0.5f;
  float m_attraction_range_ = 1.5f;
  float m_kill_range_ = 0.5f;
  float m_randomness_factor_ = 0.1f;

  auto generate_branches() -> void;
  auto grow() -> bool;
  static auto generate_random_leaf_position(float radius) noexcept -> glm::vec3;
  auto draw_leaves(const glm::mat4& view, const glm::mat4& projection) const
      -> void;
  auto draw_foliage(const glm::mat4& view) const -> void;
  auto calculate_foliage() -> void;
};

#endif  // TREES_HPP
