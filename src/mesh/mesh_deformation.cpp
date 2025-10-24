#include "mesh/mesh_deformation.hpp"
#include "utils/intersections.hpp"

auto mesh_deformation::initialize() -> void {
  // Calculate the number of top face vertices
  const GLuint top_vertices_count = (m_model_->m_grid_size + 1) * (m_model_->m_grid_size + 1);
  
  // Only clear normals for top face vertices that will be recomputed
  for (size_t i = 0; i < top_vertices_count; ++i) {
    m_model_->m_builder.m_vertices[i].norm = {0.0f, 0.0f, 0.0f};
  }

  // Recompute vertex normals for top face
  compute_vertex_normals();

  // Recompute TBN for top face
  recompute_tbn();

  // Destroy if mesh exists
  if (m_model_->m_mesh.vao != 0) m_model_->m_mesh.destroy();
  // Rebuild mesh
  m_model_->m_mesh = m_model_->m_builder.build();
}

auto mesh_deformation::set_model(const terrain_model& m) -> void {
  m_model_ = const_cast<terrain_model*>(&m);
}

auto mesh_deformation::deform_mesh(const cgra::mesh_vertex& center,
                                   const bool is_bump,
                                   const float deformation_radius,
                                   const float max_deformation_strength)
    -> void {
  std::cout << "=== Deforming mesh ===" << std::endl;
  std::cout << "Center: (" << center.pos.x << ", " << center.pos.y << ", "
            << center.pos.z << ")" << std::endl;

  int vertices_affected = 0;
  float max_displacement = 0.0f;
  int sample_vertex_idx = -1;
  float sample_vertex_y_before = 0.0f;

  // Precalculate radius squared for faster distance checks
  const float radius_sq = deformation_radius * deformation_radius;

  // Calculate the index offset for bottom vertices
  const GLuint top_vertices_count = (m_model_->m_grid_size + 1) * (m_model_->m_grid_size + 1);

  // ONLY process top face vertices for deformation
  for (size_t idx = 0; idx < top_vertices_count; ++idx) {
    cgra::mesh_vertex& v = m_model_->m_builder.m_vertices[idx];

    // Calculate SQUARED distance (faster - no sqrt needed yet)
    const float dist_sq = (v.pos.x - center.pos.x) * (v.pos.x - center.pos.x) +
                          (v.pos.z - center.pos.z) * (v.pos.z - center.pos.z);

    // SKIP vertices outside the deformation radius
    if (dist_sq > radius_sq) {
      continue;
    }

    // Now calculate actual distance only for vertices we're modifying
    const float distance = std::sqrt(dist_sq);

    // Calculate a normalized strength based on distance
    float normalized_strength =
        std::exp(-dist_sq / (radius_sq * 0.33f));  // Use dist_sq directly

    // Ensure that the strength is in the range [0, 1]
    normalized_strength = std::max(0.0f, std::min(1.0f, normalized_strength));

    // Scale the deformation strength
    const float deformation_strength =
        max_deformation_strength * normalized_strength;

    // Apply the deformation
    const float displacement =
        is_bump ? deformation_strength : -deformation_strength;

    if (std::abs(displacement) > 0.01f) {
      if (vertices_affected == 0) {
        sample_vertex_idx = idx;
        sample_vertex_y_before = v.pos.y;
      }
      vertices_affected++;
      max_displacement = std::max(max_displacement, std::abs(displacement));
    }

    // Update the vertex position
    v.pos.y += displacement;

    // Get corresponding bottom vertex position
    const float bottom_y = m_model_->m_builder.m_vertices[idx + top_vertices_count].pos.y;
    // Ensure top vertex stays above bottom vertex (with small margin)
    const float min_y = bottom_y + 0.1f;  // 0.1 unit margin
    if (v.pos.y < min_y) {
      v.pos.y = min_y;
    }

    // Clear the normal for this affected vertex - will be recomputed
    v.norm = {0.0f, 0.0f, 0.0f};
  }

  std::cout << "Vertices affected: " << vertices_affected << std::endl;
  std::cout << "Max displacement: " << max_displacement << std::endl;

  if (sample_vertex_idx >= 0) {
    float sample_vertex_y_after =
        m_model_->m_builder.m_vertices[sample_vertex_idx].pos.y;
    std::cout << "Sample vertex [" << sample_vertex_idx
              << "] Y: " << sample_vertex_y_before << " -> "
              << sample_vertex_y_after << std::endl;
  }

  // Recompute normals for affected area (with slightly larger radius to catch neighboring vertices)
  compute_vertex_normals_partial(center.pos, deformation_radius * 1.2f);

  // Recompute TBN only for affected area
  recompute_tbn_partial(center.pos, deformation_radius);

  if (m_model_->m_mesh.vao != 0) {
    glBindBuffer(GL_ARRAY_BUFFER, m_model_->m_mesh.vbo);
    glBufferSubData(
        GL_ARRAY_BUFFER, 0,
        m_model_->m_builder.m_vertices.size() * sizeof(cgra::mesh_vertex),
        m_model_->m_builder.m_vertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  } else {
    m_model_->m_mesh = m_model_->m_builder.build();
  }

  m_model_->build_aabb_tree_async();
  std::cout << "Mesh updated!" << std::endl;
}

auto mesh_deformation::recompute_tbn() -> void {
  for (auto i = 0; i < m_model_->m_grid_size; ++i) {
    for (auto j = 0; j < m_model_->m_grid_size; ++j) {
      const int k1 = i * (m_model_->m_grid_size + 1) + j;
      const int k2 = k1 + 1;
      const int k3 = (i + 1) * (m_model_->m_grid_size + 1) + j;
      const int k4 = k3 + 1;

      calculate_tbn(m_model_->m_builder, true, k1, k2, k3, k4);
      calculate_tbn(m_model_->m_builder, false, k1, k2, k3, k4);
    }
  }
}

auto mesh_deformation::recompute_tbn_partial(const glm::vec3& center,
                                             float radius) -> void {
  // Calculate grid bounds for affected area
  const float grid_cell_size = m_model_->m_spacing;
  const int grid_size = m_model_->m_grid_size;

  // Convert world position to grid coordinates
  const float total_width = grid_cell_size * static_cast<float>(grid_size);
  const float x_offset = -total_width / 2.0f;
  const float z_offset = -total_width / 2.0f;

  const int center_i = static_cast<int>((center.x - x_offset) / grid_cell_size);
  const int center_j = static_cast<int>((center.z - z_offset) / grid_cell_size);

  // Calculate grid radius
  const int grid_radius = static_cast<int>(std::ceil(radius / grid_cell_size)) + 1;

  const int i_min = glm::max(0, center_i - grid_radius);
  const int i_max = glm::min(grid_size - 1, center_i + grid_radius);
  const int j_min = glm::max(0, center_j - grid_radius);
  const int j_max = glm::min(grid_size - 1, center_j + grid_radius);

  // Multi-threaded TBN computation
  const int num_threads = std::thread::hardware_concurrency();
  std::vector<std::thread> threads;

  int rows_per_thread = (i_max - i_min + 1) / num_threads;

  for (auto t = 0; t < num_threads; ++t) {
    int start_i = i_min + t * rows_per_thread;
    int end_i = (t == num_threads - 1) ? i_max : start_i + rows_per_thread - 1;

    threads.emplace_back([this, start_i, end_i, j_min, j_max, grid_size]() {
      for (auto i = start_i; i <= end_i; ++i) {
        for (auto j = j_min; j <= j_max; ++j) {
          const int k1 = i * (grid_size + 1) + j;
          const int k2 = k1 + 1;
          const int k3 = (i + 1) * (grid_size + 1) + j;
          const int k4 = k3 + 1;

          calculate_tbn(m_model_->m_builder, true, k1, k2, k3, k4);
          calculate_tbn(m_model_->m_builder, false, k1, k2, k3, k4);
        }
      }
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }
}

auto mesh_deformation::calculate_tbn(cgra::mesh_builder& mb, bool top_left,
                                     int k1, int k2, int k3, int k4) -> void {
  // triangles positions
  glm::vec3 pos1 = mb.m_vertices.at(k1).pos;
  glm::vec3 pos2 = mb.m_vertices.at(k3).pos;
  glm::vec3 pos3 = mb.m_vertices.at(k2).pos;
  glm::vec3 pos4 = mb.m_vertices.at(k4).pos;

  // uv positions
  glm::vec2 uv1 = mb.m_vertices.at(k1).uv;
  glm::vec2 uv2 = mb.m_vertices.at(k3).uv;
  glm::vec2 uv3 = mb.m_vertices.at(k2).uv;
  glm::vec2 uv4 = mb.m_vertices.at(k4).uv;

  glm::vec3 edge1, edge2;
  glm::vec2 delta_uv1, delta_uv2;
  if (top_left) {
    // calculate tangent and bitangent for top left triangle
    edge1 = pos2 - pos1;
    edge2 = pos3 - pos1;

    // delta UVs
    delta_uv1 = uv2 - uv1;
    delta_uv2 = uv3 - uv1;
  } else {
    // calculate tangent and bitangent for bot right triangle
    edge1 = pos3 - pos4;
    edge2 = pos2 - pos4;

    // delta UVs
    delta_uv1 = uv3 - uv4;
    delta_uv2 = uv2 - uv4;
  }

  glm::vec3 tangent{};
  glm::vec3 bitangent{};
  
  float denominator = delta_uv1.x * delta_uv2.y - delta_uv2.x * delta_uv1.y;
  
  // Avoid division by zero or very small denominators
  if (abs(denominator) < 0.0001f) {
    // Use the triangle's edges as fallback for tangent/bitangent
    tangent = glm::normalize(edge1);
    bitangent = glm::normalize(edge2);
  } else {
    float f = 1.0f / denominator;

    tangent.x = f * (delta_uv2.y * edge1.x - delta_uv1.y * edge2.x);
    tangent.y = f * (delta_uv2.y * edge1.y - delta_uv1.y * edge2.y);
    tangent.z = f * (delta_uv2.y * edge1.z - delta_uv1.y * edge2.z);

    bitangent.x = f * (-delta_uv2.x * edge1.x + delta_uv1.x * edge2.x);
    bitangent.y = f * (-delta_uv2.x * edge1.y + delta_uv1.x * edge2.y);
    bitangent.z = f * (-delta_uv2.x * edge1.z + delta_uv1.x * edge2.z);

    // Safety check and normalize tangent and bitangent
    float tangent_length = glm::length(tangent);
    float bitangent_length = glm::length(bitangent);
    
    if (tangent_length > 0.0001f) {
      tangent = tangent / tangent_length;
    } else {
      tangent = glm::vec3(1.0f, 0.0f, 0.0f); // Default tangent
    }
    
    if (bitangent_length > 0.0001f) {
      bitangent = bitangent / bitangent_length;
    } else {
      bitangent = glm::vec3(0.0f, 0.0f, 1.0f); // Default bitangent
    }
  }

  if (top_left) {
    // store tangent and bitangent in vertex attributes for top left triangle
    mb.m_vertices.at(k1).tang = tangent;
    mb.m_vertices.at(k2).tang = tangent;
    mb.m_vertices.at(k3).tang = tangent;

    mb.m_vertices.at(k1).bitang = bitangent;
    mb.m_vertices.at(k2).bitang = bitangent;
    mb.m_vertices.at(k3).bitang = bitangent;
  } else {
    // store tangent and bitangent in vertex attributes for bot right triangle
    mb.m_vertices.at(k2).tang = tangent;
    mb.m_vertices.at(k4).tang = tangent;
    mb.m_vertices.at(k3).tang = tangent;

    mb.m_vertices.at(k2).bitang = bitangent;
    mb.m_vertices.at(k4).bitang = bitangent;
    mb.m_vertices.at(k3).bitang = bitangent;
  }
}

auto mesh_deformation::calculate_face_normal(const glm::vec3& vertex1,
                                             const glm::vec3& vertex2,
                                             const glm::vec3& vertex3)
    -> glm::vec3 {
  // Calculate two edge vectors of the triangle
  const glm::vec3 edge_1 = vertex2 - vertex1;
  const glm::vec3 edge_2 = vertex3 - vertex1;

  // Calculate the cross product of the two edge vectors to get the face normal
  glm::vec3 face_normal = cross(edge_1, edge_2);

  // Check if the normal is valid (non-zero length) before normalizing
  float length_sq = glm::dot(face_normal, face_normal);
  if (length_sq > 0.0001f) {
    // Normalize the face normal to ensure it has a unit length
    face_normal = normalize(face_normal);
  } else {
    // Degenerate triangle (zero area), return a default upward normal
    face_normal = glm::vec3(0.0f, 1.0f, 0.0f);
  }

  return face_normal;
}

auto mesh_deformation::compute_vertex_normals() -> void {
  // Calculate the number of top face vertices
  const GLuint top_vertices_count = (m_model_->m_grid_size + 1) * (m_model_->m_grid_size + 1);
  
  // Only compute normals for top face vertices (bottom and sides have fixed normals)
  for (size_t v_idx = 0; v_idx < top_vertices_count; ++v_idx) {
    // Check if vertex has adjacent faces
    if (v_idx >= m_model_->m_adjacent_faces.size() || 
        m_model_->m_adjacent_faces[v_idx].empty()) {
      // No adjacent faces - use default upward normal for top face
      m_model_->m_builder.m_vertices[v_idx].norm = glm::vec3(0.0f, 1.0f, 0.0f);
      continue;
    }

    glm::vec3 new_normal = {0.0f, 0.0f, 0.0f};
    int valid_face_count = 0;

    // Iterate through adjacent faces of the current vertex
    for (size_t i = 0; i < m_model_->m_adjacent_faces[v_idx].size(); i += 3) {
      // Get the vertices of the adjacent face (assuming that each face stores
      // its vertex indices)
      const glm::vec3& vertex1 =
          m_model_->m_builder.m_vertices[m_model_->m_adjacent_faces[v_idx].at(i)]
              .pos;
      const glm::vec3& vertex2 =
          m_model_->m_builder
              .m_vertices[m_model_->m_adjacent_faces[v_idx].at(i + 1)]
              .pos;
      const glm::vec3& vertex3 =
          m_model_->m_builder
              .m_vertices[m_model_->m_adjacent_faces[v_idx].at(i + 2)]
              .pos;

      // Calculate the face normal using the vertices
      const glm::vec3 face_normal =
          calculate_face_normal(vertex1, vertex2, vertex3);

      // Only accumulate if the face normal is valid (not zero)
      float face_normal_length_sq = glm::dot(face_normal, face_normal);
      if (face_normal_length_sq > 0.0001f) {
        new_normal += face_normal;
        valid_face_count++;
      }
    }

    // Check if normal is valid before normalizing
    float normal_length_sq = glm::dot(new_normal, new_normal);
    if (normal_length_sq > 0.0001f && valid_face_count > 0) {
      new_normal = normalize(new_normal);
      
      // Final safety check: ensure normalized normal is still valid
      if (glm::length(new_normal) > 0.9f) {
        m_model_->m_builder.m_vertices[v_idx].norm = new_normal;
      } else {
        // If somehow still invalid, default to upward normal
        m_model_->m_builder.m_vertices[v_idx].norm = glm::vec3(0.0f, 1.0f, 0.0f);
      }
    } else {
      // Fallback to upward-facing normal if calculation fails
      m_model_->m_builder.m_vertices[v_idx].norm = glm::vec3(0.0f, 1.0f, 0.0f);
    }
  }
}


auto mesh_deformation::compute_vertex_normals_partial(const glm::vec3& center,
                                                      float radius) -> void {
  float radius_squared = radius * radius;  // Match the deformation radius exactly
  
  // Calculate the number of top face vertices
  const GLuint top_vertices_count = (m_model_->m_grid_size + 1) * (m_model_->m_grid_size + 1);

  // Only compute normals for top face vertices
  for (size_t v_idx = 0; v_idx < top_vertices_count; ++v_idx) {
    auto& vertex = m_model_->m_builder.m_vertices[v_idx];

    // Skip vertices far from deformation
    float dist_sq = (vertex.pos.x - center.x) * (vertex.pos.x - center.x) +
                    (vertex.pos.z - center.z) * (vertex.pos.z - center.z);
    if (dist_sq > radius_squared) continue;

    if (v_idx >= m_model_->m_adjacent_faces.size() || 
        m_model_->m_adjacent_faces[v_idx].empty()) {
      // No adjacent faces - use default upward normal
      vertex.norm = glm::vec3(0.0f, 1.0f, 0.0f);
      continue;
    }

    glm::vec3 new_normal = {0.0f, 0.0f, 0.0f};
    int valid_face_count = 0;

    for (size_t i = 0; i < m_model_->m_adjacent_faces[v_idx].size(); i += 3) {
      const glm::vec3& vertex1 =
          m_model_->m_builder
              .m_vertices[m_model_->m_adjacent_faces[v_idx].at(i)]
              .pos;
      const glm::vec3& vertex2 =
          m_model_->m_builder
              .m_vertices[m_model_->m_adjacent_faces[v_idx].at(i + 1)]
              .pos;
      const glm::vec3& vertex3 =
          m_model_->m_builder
              .m_vertices[m_model_->m_adjacent_faces[v_idx].at(i + 2)]
              .pos;

      const glm::vec3 face_normal =
          calculate_face_normal(vertex1, vertex2, vertex3);
      
      // Only accumulate if the face normal is valid
      float face_normal_length_sq = glm::dot(face_normal, face_normal);
      if (face_normal_length_sq > 0.0001f) {
        new_normal += face_normal;
        valid_face_count++;
      }
    }

    // Check if normal is valid before normalizing
    float normal_length_sq = glm::dot(new_normal, new_normal);
    if (normal_length_sq > 0.0001f && valid_face_count > 0) {
      new_normal = normalize(new_normal);
      
      // Final safety check: ensure normalized normal is still valid
      if (glm::length(new_normal) > 0.9f) {
        vertex.norm = new_normal;
      } else {
        // If somehow still invalid, default to upward normal
        vertex.norm = glm::vec3(0.0f, 1.0f, 0.0f);
      }
    } else {
      // Fallback to upward-facing normal if calculation fails
      vertex.norm = glm::vec3(0.0f, 1.0f, 0.0f);
    }
  }
}

// TODO: check if this signature actually needs to use doubles
void mesh_deformation::mouse_intersect_mesh(double x_pos, double y_pos,
                                            double window_size_x,
                                            double window_size_y) {
  // Convert screen coordinates to normalized device coordinates (NDC)
  float ndc_x = 2.0f * x_pos / window_size_x - 1.0f;
  float ndc_y = 1.0f - (2.0f * y_pos) / window_size_y;

  // Assume ndc_x and ndc_y are the NDC coordinates you want
  // to convert to world coordinates
  // Near point in NDC space (z = -1)
  const glm::vec4 near_point(ndc_x, ndc_y, -1.0f, 1.0f);
  // Far point in NDC space (z = 1)
  const glm::vec4 far_point(ndc_x, ndc_y, 1.0f, 1.0f);

  // Inverse transformations to get ray origin and direction in world space
  const glm::mat4 inv_projection = glm::inverse(m_projection);
  const glm::mat4 inv_view = glm::inverse(m_view);

  // Undo projection of NDC coordinates to view coordinates
  glm::vec4 near_point_in_view = inv_projection * near_point;
  glm::vec4 far_point_in_view = inv_projection * far_point;

  // Divide by w to get homogeneous coordinates
  near_point_in_view /= near_point_in_view.w;
  far_point_in_view /= far_point_in_view.w;

  // Undo projection of view coordinates to world coordinates
  const auto ray_origin_world = glm::vec3(inv_view * near_point_in_view);
  const auto ray_end_world = glm::vec3(inv_view * far_point_in_view);

  // Calculate ray direction
  const glm::vec3 ray_direction = glm::normalize(ray_end_world - ray_origin_world);

  // Use fast AABB tree intersection (with lock to prevent race conditions)
  cgra::mesh_vertex hit_vertex;

  {
    std::lock_guard<std::mutex> lock(m_model_->aabb_mutex);

    if (ray_intersects_mesh_fast(ray_origin_world, ray_direction, *m_model_,
                                 hit_vertex)) {
      m_model_->m_selected_point = hit_vertex;
    }
  }

  if (m_model_->aabb_rebuilding.load()) {
    std::cout << "(Using old AABB tree - rebuild in progress)" << std::endl;
  }
}
