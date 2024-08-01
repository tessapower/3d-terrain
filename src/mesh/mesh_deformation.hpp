#ifndef MESH_DEFORMATION_HPP
#define MESH_DEFORMATION_HPP

#include <cgra/cgra_mesh.hpp>
#include <terrain/terrain_model.hpp>

/*
 * Based on the SIGGRAPH 2004 Technical Paper:
 * Mesh editing with poisson-based gradient field manipulation (Yizhou Yu, Kun
 * Zhou, Dong Xu, Xiaohan Shi, Hujun Bao, Baining Guo, Heung-Yeung Shum)
 * https://dl.acm.org/doi/10.1145/1015706.1015774
 *
 * Created with the help of the Normal Mapping tutorial from:
 * https://learnopengl.com/Advanced-Lighting/Normal-Mapping
 *
 * Code Author: Shekinah Pratap
 *
 */

class mesh_deformation {
 public:
  glm::mat4 m_view;
  glm::mat4 m_projection;

  mesh_deformation() = default;

  auto set_model(const terrain_model& m) -> void;

  auto deform_mesh(const cgra::mesh_vertex& center, bool is_bump,
                   float deformation_radius, float max_deformation_strength)
      -> void;

  static auto calculate_face_normal(const glm::vec3& vertex1, const glm::vec3& vertex2,
                                    const glm::vec3& vertex3) -> glm::vec3;

  auto compute_vertex_normals() -> void;

  auto mouse_intersect_mesh(double x_pos, double y_pos, double window_size_x,
                            double window_size_y) -> void;

  auto compute_tbn() -> void;

  static auto calculate_tbn(cgra::mesh_builder& mb, bool top_left, int k1, int k2,
                            int k3, int k4) -> void;

  auto get_model() -> terrain_model;

 private:
  terrain_model m_model_;
};

#endif  // MESH_DEFORMATION_HPP
