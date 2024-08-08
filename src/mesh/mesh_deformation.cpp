#include "mesh/mesh_deformation.hpp"

#include "utils/intersections.hpp"

auto mesh_deformation::set_model(const terrain_model& m) -> void {
  m_model_ = m;
}

auto mesh_deformation::deform_mesh(const cgra::mesh_vertex& center,
                                   const bool is_bump,
                                   const float deformation_radius,
                                   const float max_deformation_strength)
    -> void {
  for (cgra::mesh_vertex& v : m_model_.m_builder.m_vertices) {
    // Calculate the distance between the vertex and the center point.
    const float distance = std::sqrt(
        (v.pos.x - center.pos.x) * (v.pos.x - center.pos.x) +
        //(vertex.pos.y - center.pos.y) * (vertex.pos.y - center.pos.y) +
        (v.pos.z - center.pos.z) * (v.pos.z - center.pos.z));

    // Calculate a normalized strength based on distance (closer points get
    // stronger deformations)
    float normalized_strength =
        std::exp(-distance * distance /
                 (deformation_radius * deformation_radius * 0.33f));

    // Ensure that the strength is in the range [0, 1]
    normalized_strength = std::max(0.0f, std::min(1.0f, normalized_strength));

    // Scale the deformation strength based on the normalized strength
    const float deformation_strength =
        max_deformation_strength * normalized_strength;

    // Apply the deformation based on deformation type (bump or ditch)
    const float displacement =
        is_bump ? deformation_strength : -deformation_strength;

    // Update the vertex position.
    v.pos.y += displacement;

    // Clear the vertex normal for the next computation
    v.norm = {0.0f, 0.0f, 0.0f};
  }

  // Recompute vertex normals
  compute_vertex_normals();

  // Recompute TBN
  compute_tbn();

  m_model_.m_mesh = m_model_.m_builder.build();
}

auto mesh_deformation::compute_tbn() -> void {
  // Recompute TBN
  for (int i = 0; i < m_model_.m_grid_size; ++i) {
    for (int j = 0; j < m_model_.m_grid_size; ++j) {
      const int k1 = i * (m_model_.m_grid_size + 1) + j;
      const int k2 = k1 + 1;
      const int k3 = (i + 1) * (m_model_.m_grid_size + 1) + j;
      const int k4 = k3 + 1;

      calculate_tbn(m_model_.m_builder, true, k1, k2, k3, k4);
      calculate_tbn(m_model_.m_builder, false, k1, k2, k3, k4);
    }
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

  glm::vec3 tangent;
  glm::vec3 bitangent;
  float f = 1.0f / (delta_uv1.x * delta_uv2.y - delta_uv2.x * delta_uv1.y);

  tangent.x = f * (delta_uv2.y * edge1.x - delta_uv1.y * edge2.x);
  tangent.y = f * (delta_uv2.y * edge1.y - delta_uv1.y * edge2.y);
  tangent.z = f * (delta_uv2.y * edge1.z - delta_uv1.y * edge2.z);

  bitangent.x = f * (-delta_uv2.x * edge1.x + delta_uv1.x * edge2.x);
  bitangent.y = f * (-delta_uv2.x * edge1.y + delta_uv1.x * edge2.y);
  bitangent.z = f * (-delta_uv2.x * edge1.z + delta_uv1.x * edge2.z);

  // normalize tangent and bitangent
  tangent = normalize(tangent);
  bitangent = normalize(bitangent);

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

auto mesh_deformation::get_model() -> terrain_model { return m_model_; }

auto mesh_deformation::calculate_face_normal(const glm::vec3& vertex1,
                                             const glm::vec3& vertex2,
                                             const glm::vec3& vertex3)
    -> glm::vec3 {
  // Calculate two edge vectors of the triangle
  const glm::vec3 edge_1 = vertex2 - vertex1;
  const glm::vec3 edge_2 = vertex3 - vertex1;

  // Calculate the cross product of the two edge vectors to get the face normal
  glm::vec3 face_normal = cross(edge_1, edge_2);

  // Normalize the face normal to ensure it has a unit length
  face_normal = normalize(face_normal);

  return face_normal;
}

auto mesh_deformation::compute_vertex_normals() -> void {
  for (size_t v_idx = 0; v_idx < m_model_.m_builder.m_vertices.size();
       ++v_idx) {
    if (m_model_.m_adjacent_faces[v_idx].empty()) continue;

    glm::vec3 new_normal = {0.0f, 0.0f, 0.0f};

    // Iterate through adjacent faces of the current vertex
    for (size_t i = 0; i < m_model_.m_adjacent_faces[v_idx].size(); i += 3) {
      // Get the vertices of the adjacent face (assuming that each face stores
      // its vertex indices)
      const glm::vec3& vertex1 =
          m_model_.m_builder.m_vertices[m_model_.m_adjacent_faces[v_idx].at(i)]
              .pos;
      const glm::vec3& vertex2 =
          m_model_.m_builder
              .m_vertices[m_model_.m_adjacent_faces[v_idx].at(i + 1)]
              .pos;
      const glm::vec3& vertex3 =
          m_model_.m_builder
              .m_vertices[m_model_.m_adjacent_faces[v_idx].at(i + 2)]
              .pos;

      // Calculate the face normal using the vertices
      const glm::vec3 face_normal =
          calculate_face_normal(vertex1, vertex2, vertex3);

      // Accumulate the face normal to compute the new vertex normal
      new_normal += face_normal;
    }

    // Normalize the computed vertex normal
    new_normal = normalize(new_normal);

    // Update the vertex normal
    m_model_.m_builder.m_vertices[v_idx].norm = new_normal;
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
  glm::vec4 near_point(ndc_x, ndc_y, -1.0f, 1.0f);
  // Far point in NDC space (z = 1)
  glm::vec4 far_point(ndc_x, ndc_y, 1.0f, 1.0f);

  // Inverse transformations to get ray origin and direction in world space
  glm::mat4 inv_projection = glm::inverse(m_projection);
  glm::mat4 inv_view = glm::inverse(m_view);

  // Undo projection of NDC coordinates to view coordinates
  glm::vec4 near_point_in_view = inv_projection * near_point;
  glm::vec4 far_point_in_view = inv_projection * far_point;

  // Divide by w to get homogeneous coordinates
  near_point_in_view /= near_point_in_view.w;
  far_point_in_view /= far_point_in_view.w;

  // Undo projection of view coordinates to world coordinates
  auto ray_origin_world = glm::vec3(inv_view * near_point_in_view);
  auto ray_end_world = glm::vec3(inv_view * far_point_in_view);

  // Calculate ray direction
  glm::vec3 ray_direction = glm::normalize(ray_end_world - ray_origin_world);

  // Iterate through your mesh vertices and test for intersection
  for (size_t i = 0; i < m_model_.m_builder.m_vertices.size(); ++i) {
    cgra::mesh_vertex vertex = m_model_.m_builder.m_vertices[i];

    // Check if the ray intersects with the vertex
    if (ray_intersects_vertex(ray_origin_world, ray_direction, static_cast<int>(i), m_model_)) {
      // You've clicked on this vertex, store its index
      m_model_.m_selected_point = m_model_.m_builder.m_vertices.at(i);
      break;  // Exit the loop since you've found the first intersection
    }
  }
}
