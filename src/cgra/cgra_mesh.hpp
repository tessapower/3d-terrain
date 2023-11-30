#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include <utils/opengl.hpp>
#include <vector>

namespace cgra {
// A data structure for holding buffer IDs and other information related to
// drawing. Also has a helper functions for drawing the mesh and deleting the gl
// buffers.
//
// location 1 : positions (vec3)
// location 2 : normals (vec3)
// location 3 : uv (vec2)
struct gl_mesh {
  GLuint vao = 0;
  GLuint vbo = 0;
  GLuint ibo = 0;
  GLenum mode = 0;      // mode to draw in, eg: GL_TRIANGLES
  int index_count = 0;  // how many indices to draw (no primitives)

  // calls the draw function on mesh data
  auto draw() const -> void;

  // deletes the gl buffers (cleans up all the data)
  auto destroy() const -> void;
};

struct mesh_vertex {
  glm::vec3 pos{0};
  glm::vec3 norm{0};
  glm::vec2 uv{0};
  glm::vec3 tang{0};
  glm::vec3 bitang{0};
};

// Mesh builder object used to create an mesh by taking vertex and index
// information and uploading them to OpenGL.
struct mesh_builder {
  GLenum m_mode = GL_TRIANGLES;
  std::vector<mesh_vertex> m_vertices;
  std::vector<unsigned int> m_indices;

  mesh_builder() = default;

  explicit mesh_builder(const GLenum mode) : m_mode(mode) {}

  template <size_t N, size_t M>
  explicit mesh_builder(const mesh_vertex (&vertex_data)[N],
                        const mesh_vertex (&idx_data)[M],
                        const GLenum mode = GL_TRIANGLES)
      : m_mode(mode),
        m_vertices(vertex_data, vertex_data + N),
        m_indices(idx_data, idx_data + M) {}

  auto push_vertex(const mesh_vertex& v) -> GLuint {
    auto size = m_vertices.size();
    assert(size == static_cast<decltype(size)>(static_cast<GLuint>(size)));
    m_vertices.push_back(v);

    return static_cast<GLuint>(size);
  }

  auto push_index(const GLuint i) -> void { m_indices.push_back(i); }

  auto push_indices(const std::initializer_list<GLuint> indices) -> void {
    m_indices.insert(m_indices.end(), indices);
  }

  [[nodiscard]] auto get_vertex(const int val) const -> mesh_vertex {
    return m_vertices[val];
  }

  auto get_vertices() -> std::vector<mesh_vertex> { return m_vertices; }

  auto append(const mesh_builder& builder) -> void {
    for (const unsigned int i : builder.m_indices) {
      m_indices.push_back(m_vertices.size() + i);
    }

    for (const auto& v : builder.m_vertices) {
      m_vertices.push_back(v);
    }
  }

  [[nodiscard]] auto build() const -> gl_mesh;

  auto print() const -> void {
    std::cout << "pos" << std::endl;
    for (const auto [pos, norm, uv, tang, bitang] : m_vertices) {
      std::cout << pos.x << ", " << pos.y << ", " << pos.z << ", " << norm.x
                << ", " << norm.y << ", " << norm.z << ", " << uv.x << ", "
                << uv.y << ", " << std::endl;
    }
    std::cout << "idx" << std::endl;
    for (const int i : m_indices) {
      std::cout << i << ", ";
    }
    std::cout << std::endl;
  }
};

}  // namespace cgra
