#include "terrain/terrain_model.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "utils/perlin_noise.hpp"

auto terrain_model::create_terrain(bool use_perlin) -> void {
  cgra::mesh_builder mb;
  cgra::mesh_vertex mv;

  auto terrain =
      perlin(m_seed, m_octaves, m_lacunarity, m_persistence, m_repeat);

  // Calculate the total width and length of the grid
  const float total_width = m_spacing * static_cast<float>(m_grid_size);
  const float total_length = m_spacing * static_cast<float>(m_grid_size);

  // Calculate the offset to center the grid
  const float x_offset = -total_width / 2.0f;
  const float z_offset = -total_length / 2.0f;

  // Box depth (how deep the box extends below the terrain)
  // This value determines the "thickness" of the terrain mesh, creating walls
  // that prevent the camera from going through the bottom
  const float box_depth = m_box_depth;

  // Create a 2D array to store adjacent faces for each vertex (only for top face)
  m_adjacent_faces =
      std::vector<std::vector<int>>((m_grid_size + 1) * (m_grid_size + 1));

  // Generate the TOP grid vertices
  for (auto i = 0; i <= m_grid_size; ++i) {
    for (auto j = 0; j <= m_grid_size; ++j) {
      // Calculate vertex position (x, y, z) with increased spacing and centered
      float x = static_cast<float>(i) * m_spacing + x_offset;
      float z = static_cast<float>(j) * m_spacing + z_offset;
      
      // Generate Perlin noise height
      // terrain.generate_perlin returns [0, 1], so we map it to [-m_height/2, m_height/2]
      float y = 0.0f;
      if (use_perlin) {
        float noise_value = terrain.generate_perlin(x, 0.0f, z);
        // Map from [0, 1] to [-m_height/2, m_height/2]
        y = (noise_value * m_height) - (m_height / 2.0f);
      }

      mv.pos = {x, y, z};

      // Normal vectors for flat ground (facing up)
      mv.norm = {0.0f, 1.0f, 0.0f};

      // Texture coordinates (u, v) can be set as desired
      mv.uv = {static_cast<float>(i) * 10.0f / static_cast<float>(m_grid_size),
               static_cast<float>(j) * 10.0f / static_cast<float>(m_grid_size)};

      mb.push_vertex(mv);
    }
  }

  const GLuint top_vertices_count = (m_grid_size + 1) * (m_grid_size + 1);

  // Generate the BOTTOM grid vertices (same x, z positions but lower y)
  for (auto i = 0; i <= m_grid_size; ++i) {
    for (auto j = 0; j <= m_grid_size; ++j) {
      float x = static_cast<float>(i) * m_spacing + x_offset;
      float z = static_cast<float>(j) * m_spacing + z_offset;
      
      float y = -box_depth;
      if (use_perlin) {
        float noise_value = terrain.generate_perlin(x, 0.0f, z);
        y = (noise_value * m_height) - (m_height / 2.0f) - box_depth;
      }

      mv.pos = {x, y, z};

      // Normal vectors for bottom (facing down)
      mv.norm = {0.0f, -1.0f, 0.0f};

      // Texture coordinates
      mv.uv = {static_cast<float>(i) * 10.0f / static_cast<float>(m_grid_size),
               static_cast<float>(j) * 10.0f / static_cast<float>(m_grid_size)};

      mb.push_vertex(mv);
    }
  }

  // Store base vertex count for side vertices
  const GLuint base_vertex_count = mb.m_vertices.size();

  // Generate vertices for SIDE WALLS with proper normals
  // Front side (z = z_offset, normal pointing in -z direction)
  for (auto i = 0; i <= m_grid_size; ++i) {
    float x = static_cast<float>(i) * m_spacing + x_offset;
    float z = z_offset;
    
    float y_top = 0.0f;
    if (use_perlin) {
      float noise_value = terrain.generate_perlin(x, 0.0f, z);
      y_top = (noise_value * m_height) - (m_height / 2.0f);
    }
    float y_bottom = use_perlin ? y_top - box_depth : -box_depth;

    // Top vertex of front side
    mv.pos = {x, y_top, z};
    mv.norm = {0.0f, 0.0f, -1.0f};  // Front face normal
    mv.uv = {static_cast<float>(i) / static_cast<float>(m_grid_size), 0.0f};
    mb.push_vertex(mv);

    // Bottom vertex of front side
    mv.pos = {x, y_bottom, z};
    mv.norm = {0.0f, 0.0f, -1.0f};  // Front face normal
    mv.uv = {static_cast<float>(i) / static_cast<float>(m_grid_size), 1.0f};
    mb.push_vertex(mv);
  }

  const GLuint front_side_start = base_vertex_count;

  // Back side (z = z_offset + total_length, normal pointing in +z direction)
  for (auto i = 0; i <= m_grid_size; ++i) {
    float x = static_cast<float>(i) * m_spacing + x_offset;
    float z = z_offset + total_length;
    
    float y_top = 0.0f;
    if (use_perlin) {
      float noise_value = terrain.generate_perlin(x, 0.0f, z);
      y_top = (noise_value * m_height) - (m_height / 2.0f);
    }
    float y_bottom = use_perlin ? y_top - box_depth : -box_depth;

    // Top vertex of back side
    mv.pos = {x, y_top, z};
    mv.norm = {0.0f, 0.0f, 1.0f};  // Back face normal
    mv.uv = {static_cast<float>(i) / static_cast<float>(m_grid_size), 0.0f};
    mb.push_vertex(mv);

    // Bottom vertex of back side
    mv.pos = {x, y_bottom, z};
    mv.norm = {0.0f, 0.0f, 1.0f};  // Back face normal
    mv.uv = {static_cast<float>(i) / static_cast<float>(m_grid_size), 1.0f};
    mb.push_vertex(mv);
  }

  const GLuint back_side_start = front_side_start + (m_grid_size + 1) * 2;

  // Left side (x = x_offset, normal pointing in -x direction)
  for (auto j = 0; j <= m_grid_size; ++j) {
    float x = x_offset;
    float z = static_cast<float>(j) * m_spacing + z_offset;
    
    float y_top = 0.0f;
    if (use_perlin) {
      float noise_value = terrain.generate_perlin(x, 0.0f, z);
      y_top = (noise_value * m_height) - (m_height / 2.0f);
    }
    float y_bottom = use_perlin ? y_top - box_depth : -box_depth;

    // Top vertex of left side
    mv.pos = {x, y_top, z};
    mv.norm = {-1.0f, 0.0f, 0.0f};  // Left face normal
    mv.uv = {static_cast<float>(j) / static_cast<float>(m_grid_size), 0.0f};
    mb.push_vertex(mv);

    // Bottom vertex of left side
    mv.pos = {x, y_bottom, z};
    mv.norm = {-1.0f, 0.0f, 0.0f};  // Left face normal
    mv.uv = {static_cast<float>(j) / static_cast<float>(m_grid_size), 1.0f};
    mb.push_vertex(mv);
  }

  const GLuint left_side_start = back_side_start + (m_grid_size + 1) * 2;

  // Right side (x = x_offset + total_width, normal pointing in +x direction)
  for (auto j = 0; j <= m_grid_size; ++j) {
    float x = x_offset + total_width;
    float z = static_cast<float>(j) * m_spacing + z_offset;
    
    float y_top = 0.0f;
    if (use_perlin) {
      float noise_value = terrain.generate_perlin(x, 0.0f, z);
      y_top = (noise_value * m_height) - (m_height / 2.0f);
    }
    float y_bottom = use_perlin ? y_top - box_depth : -box_depth;

    // Top vertex of right side
    mv.pos = {x, y_top, z};
    mv.norm = {1.0f, 0.0f, 0.0f};  // Right face normal
    mv.uv = {static_cast<float>(j) / static_cast<float>(m_grid_size), 0.0f};
    mb.push_vertex(mv);

    // Bottom vertex of right side
    mv.pos = {x, y_bottom, z};
    mv.norm = {1.0f, 0.0f, 0.0f};  // Right face normal
    mv.uv = {static_cast<float>(j) / static_cast<float>(m_grid_size), 1.0f};
    mb.push_vertex(mv);
  }

  const GLuint right_side_start = left_side_start + (m_grid_size + 1) * 2;

  // Generate triangle indices for the TOP face (only these are interactable)
  for (auto i = 0; i < m_grid_size; ++i) {
    for (auto j = 0; j < m_grid_size; ++j) {
      const GLuint k1 = i * (m_grid_size + 1) + j;
      const GLuint k2 = k1 + 1;
      const GLuint k3 = (i + 1) * (m_grid_size + 1) + j;
      const GLuint k4 = k3 + 1;

      mb.push_indices({k1, k2, k3});  // First triangle
      mb.push_indices({k2, k4, k3}); // Second triangle

      // Helper lambda to add triangle to adjacent faces (ONLY for top face)
      auto add_triangle = [&](int v1, int v2, int v3) {
        m_adjacent_faces[v1].insert(m_adjacent_faces[v1].end(), {v1, v2, v3});
        m_adjacent_faces[v2].insert(m_adjacent_faces[v2].end(), {v1, v2, v3});
        m_adjacent_faces[v3].insert(m_adjacent_faces[v3].end(), {v1, v2, v3});
      };

      add_triangle(k1, k2, k3);
      add_triangle(k2, k4, k3);
    }
  }

  // Generate triangle indices for the BOTTOM face (not interactable, no adjacent faces)
  for (auto i = 0; i < m_grid_size; ++i) {
    for (auto j = 0; j < m_grid_size; ++j) {
      const GLuint k1 = top_vertices_count + i * (m_grid_size + 1) + j;
      const GLuint k2 = k1 + 1;
      const GLuint k3 = top_vertices_count + (i + 1) * (m_grid_size + 1) + j;
      const GLuint k4 = k3 + 1;

      // Reverse winding order for bottom face (so normals face down/out)
      mb.push_indices({k1, k3, k2});  // First triangle
      mb.push_indices({k2, k3, k4}); // Second triangle
    }
  }

  // Generate SIDE WALLS indices using the dedicated side vertices
  // Front side
  for (auto i = 0; i < m_grid_size; ++i) {
    GLuint top_left = front_side_start + i * 2;
    GLuint bottom_left = top_left + 1;
    GLuint top_right = front_side_start + (i + 1) * 2;
    GLuint bottom_right = top_right + 1;

    mb.push_indices({top_left, bottom_left, top_right});
    mb.push_indices({top_right, bottom_left, bottom_right});
  }

  // Back side
  for (auto i = 0; i < m_grid_size; ++i) {
    GLuint top_left = back_side_start + i * 2;
    GLuint bottom_left = top_left + 1;
    GLuint top_right = back_side_start + (i + 1) * 2;
    GLuint bottom_right = top_right + 1;

    mb.push_indices({top_left, top_right, bottom_left});
    mb.push_indices({top_right, bottom_right, bottom_left});
  }

  // Left side
  for (auto j = 0; j < m_grid_size; ++j) {
    GLuint top_left = left_side_start + j * 2;
    GLuint bottom_left = top_left + 1;
    GLuint top_right = left_side_start + (j + 1) * 2;
    GLuint bottom_right = top_right + 1;

    mb.push_indices({top_left, top_right, bottom_left});
    mb.push_indices({top_right, bottom_right, bottom_left});
  }

  // Right side
  for (auto j = 0; j < m_grid_size; ++j) {
    GLuint top_left = right_side_start + j * 2;
    GLuint bottom_left = top_left + 1;
    GLuint top_right = right_side_start + (j + 1) * 2;
    GLuint bottom_right = top_right + 1;

    mb.push_indices({top_left, bottom_left, top_right});
    mb.push_indices({top_right, bottom_left, bottom_right});
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

void terrain_model::build_aabb_tree() {
  // Collect all vertex positions
  std::vector<glm::vec3> positions;
  positions.reserve(m_builder.m_vertices.size());

  for (const auto& vertex : m_builder.m_vertices) {
    positions.push_back(vertex.pos);
  }

  // Flatten m_adjacent_faces into a single index list
  std::vector<unsigned int> flat_indices;

  for (const auto& vertex_faces : m_adjacent_faces) {
    for (unsigned int idx : vertex_faces) {
      flat_indices.push_back(idx);
    }
  }

  // Build the AABB tree with flat indices
  m_aabb_tree.build(positions, flat_indices);

  std::cout << "Built AABB tree with " << flat_indices.size() / 3
            << " triangles" << std::endl;
}

void terrain_model::build_aabb_tree_async() {
  // If already rebuilding, don't start another
  if (aabb_rebuilding.load()) {
    return;
  }

  // Join previous thread if it exists
  if (aabb_rebuild_thread.joinable()) {
    aabb_rebuild_thread.join();
  }

  // Start background rebuild
  aabb_rebuilding.store(true);

  aabb_rebuild_thread = std::thread([this]() {
    std::cout << "Starting async AABB tree rebuild..." << std::endl;

    // Collect vertex positions
    std::vector<glm::vec3> positions;
    positions.reserve(m_builder.m_vertices.size());

    for (const auto& vertex : m_builder.m_vertices) {
      positions.push_back(vertex.pos);
    }

    // Flatten indices
    std::vector<unsigned int> flat_indices;
    for (const auto& vertex_faces : m_adjacent_faces) {
      for (unsigned int idx : vertex_faces) {
        flat_indices.push_back(idx);
      }
    }

    // Build new tree (this is the slow part, happens in background)
    aabb_tree new_tree;
    new_tree.build(positions, flat_indices);

    // Swap in the new tree (fast, lock protected)
    {
      std::lock_guard<std::mutex> lock(aabb_mutex);
      m_aabb_tree = std::move(new_tree);
    }

    std::cout << "Async AABB tree rebuild complete! ("
              << flat_indices.size() / 3 << " triangles)" << std::endl;

    aabb_rebuilding.store(false);
  });
}

void terrain_model::wait_for_aabb_rebuild() {
  if (aabb_rebuild_thread.joinable()) {
    aabb_rebuild_thread.join();
  }
}
