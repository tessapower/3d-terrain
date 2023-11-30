#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "mesh/simplified_mesh.hpp"

class cloud_model {
 public:
  GLuint m_shader = 0;
  glm::vec3 m_color{0.7f};
  glm::mat4 m_model_transform{1.0f};
  GLuint m_texture;

  glm::vec3 noise_scale{80.0f, 60.0f, 80.0f};
  float cloud_threshold = 0.66f;
  double voxel_edge_length = 1.0;
  glm::vec3 size{300.f, 30.f, 300.f};
  float fade_out_range = 6.0f;

  std::vector<std::vector<std::vector<float>>> cloud_data;

  simplified_mesh mesh;

  auto simulate() -> void;
  auto draw(const glm::mat4& view, const glm::mat4& projection) -> void;
};
