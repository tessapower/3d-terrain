#ifndef INTERSECTIONS_HPP
#define INTERSECTIONS_HPP

#include "cgra/cgra_mesh.hpp"
#include "glm/glm.hpp"
#include "terrain/terrain_model.hpp"

constexpr float epsilon = 0.00001f;

/**
 * \brief Returns whether the given ray starting at the given origin and going
 * in the given direction intersects with the triangle formed by the given
 * vertices.
 */
inline auto ray_intersects_triangle(const glm::vec3& ray_origin,
                                    const glm::vec3& ray_direction,
                                    const glm::vec3& v0, const glm::vec3& v1,
                                    const glm::vec3& v2) -> bool
{
  // Based on the Moeller-Trumbore intersection algorithm:
  // https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm

  // Calculate edge vectors of the triangle
  const glm::vec3 edge1 = v1 - v0;
  const glm::vec3 edge2 = v2 - v0;

  // Calculate the determinant of the matrix formed by the direction vector and
  // edge vectors
  const glm::vec3 h = glm::cross(ray_direction, edge2);
  const float a = glm::dot(edge1, h);

  // Check if the ray and triangle are parallel
  if (a > -epsilon && a < epsilon) return false;

  const float f = 1.0f / a;
  const glm::vec3 s = ray_origin - v0;
  const float u = f * glm::dot(s, h);

  if (u < 0.0f || u > 1.0f) return false;

  const glm::vec3 q = glm::cross(s, edge1);

  if (const float v = f * glm::dot(ray_direction, q); v < 0.0f || u + v > 1.0f)
    return false;

  // Calculate the intersection distance along the ray
  const float t = f * dot(edge2, q);

  // Check if the intersection point is in front of the ray's origin
  // If true, the ray intersects with the triangle, and t is the intersection
  // distance
  return t > epsilon;
}

/**
 * \brief Returns whether the ray starting from the given origin and going in
 * the given direction intersects with the vertex found at given index on the
 * given model.
 */
inline auto ray_intersects_vertex(const glm::vec3& ray_origin,
                                  const glm::vec3& ray_direction,
                                  const int vertex_idx,
                                  const terrain_model& model) -> bool
{
  // Iterate through the adjacent faces of the vertex
  for (size_t i = 0; i < model.m_adjacent_faces[vertex_idx].size(); i += 3) {
    // Get the vertices of the triangle formed by the current adjacent faces
    const glm::vec3& v0 =
        model.m_builder.m_vertices[model.m_adjacent_faces[vertex_idx][i]].pos;
    const glm::vec3& v1 =
        model.m_builder.m_vertices[model.m_adjacent_faces[vertex_idx][i + 1]]
            .pos;
    const glm::vec3& v2 =
        model.m_builder.m_vertices[model.m_adjacent_faces[vertex_idx][i + 2]]
            .pos;

    // Check if the ray intersects with the triangle
    if (ray_intersects_triangle(ray_origin, ray_direction, v0, v1, v2))
      return true;
  }

  return false; // No intersection
}

/**
 * \brief Fast ray-mesh intersection using AABB tree acceleration.
 * Returns the closest hit and the vertex from the terrain model.
 */
inline auto ray_intersects_mesh_fast(const glm::vec3& ray_origin,
                                     const glm::vec3& ray_direction,
                                     const terrain_model& model,
                                     cgra::mesh_vertex& hit_vertex) -> bool {
  // Query AABB tree for candidate triangles
  auto candidates = model.m_aabb_tree.query_ray(ray_origin, ray_direction);

  if (candidates.empty()) {
    return false;
  }

  float closest_t = std::numeric_limits<float>::max();
  bool found_hit = false;
  glm::vec3 closest_hit_point;

  // Test only candidate triangles with Moeller-Trumbore
  const auto& triangles = model.m_aabb_tree.get_triangles();

  for (unsigned int tri_idx : candidates) {
    if (tri_idx >= triangles.size()) continue;

    const auto& tri = triangles[tri_idx];

    // The triangle struct stores actual vertex positions
    if (ray_intersects_triangle(ray_origin, ray_direction, tri.v0, tri.v1,
                                tri.v2)) {
      // Calculate distance to determine closest hit
      glm::vec3 center = (tri.v0 + tri.v1 + tri.v2) / 3.0f;
      float t = glm::length(center - ray_origin);

      if (t < closest_t) {
        closest_t = t;
        closest_hit_point = center;
        found_hit = true;
      }
    }
  }

  if (found_hit) {
    // Find the closest vertex in the terrain model to the hit point
    float min_dist = std::numeric_limits<float>::max();
    int closest_vertex_idx = 0;

    for (size_t i = 0; i < model.m_builder.m_vertices.size(); ++i) {
      float dist =
          glm::length(model.m_builder.m_vertices[i].pos - closest_hit_point);
      if (dist < min_dist) {
        min_dist = dist;
        closest_vertex_idx = i;
      }
    }

    hit_vertex = model.m_builder.m_vertices[closest_vertex_idx];
    return true;
  }

  return false;
}

#endif  // INTERSECTIONS_HPP
