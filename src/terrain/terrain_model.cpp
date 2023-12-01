#include "terrain/terrain_model.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "utils/perlin_noise.hpp"

auto terrain_model::create_terrain(bool perlin) -> void {
  cgra::mesh_builder mb;
  cgra::mesh_vertex mv;

  auto terrain =
      perlin(m_seed, m_octaves, m_lacunarity, m_persistence, m_repeat);

  // Calculate the total width and length of the grid
  float total_width = m_spacing_ * static_cast<float>(m_grid_size);
  float total_length = m_spacing_ * static_cast<float>(m_grid_size);

  // Calculate the offset to center the grid
  float x_offset = -total_width / 2.0f;
  float z_offset = -total_length / 2.0f;

  // Create a 2D array to store adjacent faces for each vertex
  m_adjacent_faces =
      std::vector<std::vector<int>>((m_grid_size + 1) * (m_grid_size + 1));

  // Generate the grid vertices
  for (int i = 0; i <= m_grid_size; ++i) {
    for (int j = 0; j <= m_grid_size; ++j) {
      // Calculate vertex position (x, y, z) with increased spacing and centered
      float x = static_cast<float>(i) * m_spacing_ + x_offset;
      float z = static_cast<float>(j) * m_spacing_ + z_offset;
      // whether to use perlin or not
      float y = perlin ? (terrain.generate_perlin(x, 0.0, z) * m_height) -
                             (m_height / 2)
                       : 0.0f;

      mv.pos = {x, y, z};

      // Normal vectors for flat ground (facing up)
      mv.norm = {0.0f, 1.0f, 0.0f};

      // Texture coordinates (u, v) can be set as desired
      mv.uv = {static_cast<float>(i) * 10.0f / static_cast<float>(m_grid_size),
               static_cast<float>(j) * 10.0f / static_cast<float>(m_grid_size)};

      mb.push_vertex(mv);
    }
  }

  // Generate triangle indices to create the grid
  for (int i = 0; i < m_grid_size; ++i) {
    for (int j = 0; j < m_grid_size; ++j) {
      int k1 = i * (m_grid_size + 1) + j;
      int k2 = k1 + 1;
      int k3 = (i + 1) * (m_grid_size + 1) + j;
      int k4 = k3 + 1;

      // First triangle
      mb.push_index(k1);
      mb.push_index(k2);
      mb.push_index(k3);

      // Second triangle
      mb.push_index(k2);
      mb.push_index(k4);
      mb.push_index(k3);

      m_adjacent_faces[k1].push_back(k1);
      m_adjacent_faces[k1].push_back(k2);
      m_adjacent_faces[k1].push_back(k3);
      m_adjacent_faces[k2].push_back(k1);
      m_adjacent_faces[k2].push_back(k2);
      m_adjacent_faces[k2].push_back(k3);
      m_adjacent_faces[k3].push_back(k1);
      m_adjacent_faces[k3].push_back(k2);
      m_adjacent_faces[k3].push_back(k3);

      m_adjacent_faces[k2].push_back(k2);
      m_adjacent_faces[k2].push_back(k4);
      m_adjacent_faces[k2].push_back(k3);
      m_adjacent_faces[k4].push_back(k2);
      m_adjacent_faces[k4].push_back(k4);
      m_adjacent_faces[k4].push_back(k3);
      m_adjacent_faces[k3].push_back(k2);
      m_adjacent_faces[k3].push_back(k4);
      m_adjacent_faces[k3].push_back(k3);
    }
  }

  m_builder = mb;
  m_mesh = mb.build();
}

auto terrain_model::draw(const glm::mat4& view,
                         const glm::mat4& projection) const -> void {
  glUseProgram(m_shader);
  // Set uniform values
  glUniform3f(glGetUniformLocation(m_shader, "uCenter"), m_selected_point.pos.x,
              m_selected_point.pos.y,
              m_selected_point.pos.z);  // Set the center point
  glUniform1f(glGetUniformLocation(m_shader, "uRadius"),
              m_radius);  // Set the selection radius
  glUniform1f(glGetUniformLocation(m_shader, "uHeightChange1"),
              m_height_change1);
  glUniform1f(glGetUniformLocation(m_shader, "uHeightChange2"),
              m_height_change2);
  glUniform1iv(glGetUniformLocation(m_shader, "uTex"), 1, &m_tex);
  glUniform1f(glGetUniformLocation(m_shader, "uHeightScale"), m_height_scale);
  glUniform1iv(glGetUniformLocation(m_shader, "uType"), 1, &m_type_);
  glUniformMatrix4fv(glGetUniformLocation(m_shader, "uProjectionMatrix"), 1,
                     false, value_ptr(projection));
  glUniformMatrix4fv(glGetUniformLocation(m_shader, "uModelViewMatrix"), 1,
                     false, value_ptr(view));

  m_mesh.draw();  // draw

  // reset for other objects
  constexpr int reset = -1;
  glUniform1iv(glGetUniformLocation(m_shader, "uType"), 1, &reset);
}
