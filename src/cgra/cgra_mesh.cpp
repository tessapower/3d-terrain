#include "cgra_mesh.hpp"

namespace cgra {
auto gl_mesh::draw() const -> void {
  if (vao == 0) return;

  // bind our VAO which sets up all our buffers and data for us
  glBindVertexArray(vao);
  // tell OpenGL to draw our VAO using the draw mode and how many vertices to
  // render
  glDrawElements(mode, index_count, GL_UNSIGNED_INT, nullptr);
}

auto gl_mesh::destroy() const -> void {
  // delete the data buffers
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ibo);
}

auto mesh_builder::build() const -> gl_mesh {
  gl_mesh m;
  // VAO stores information about how the buffers are set up
  glGenVertexArrays(1, &m.vao);
  glGenBuffers(1, &m.vbo);  // VBO stores the vertex data
  glGenBuffers(1, &m.ibo);  // IBO stores the indices that make up primitives

  // VAO
  glBindVertexArray(m.vao);

  // VBO (single buffer, interleaved)
  glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
  // upload ALL the vertex data in one buffer
  glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(mesh_vertex),
               m_vertices.data(), GL_STATIC_DRAW);

  // this buffer will use location=0 when we use our VAO
  glEnableVertexAttribArray(0);
  // tell OpenGL how to treat data in location=0 - the data is treated in lots
  // of 3 (3 floats = vec3)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex),
                        reinterpret_cast<void*>((offsetof(mesh_vertex, pos))));

  // do the same thing for Normals but bind it to location=1
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex),
                        reinterpret_cast<void*>((offsetof(mesh_vertex, norm))));

  // do the same thing for UVs but bind it to location=2 - the data is treated
  // in lots of 2 (2 floats = vec2)
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex),
                        reinterpret_cast<void*>((offsetof(mesh_vertex, uv))));

  // do the same thing for Tangents but bind it to location=3
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex),
                        reinterpret_cast<void*>((offsetof(mesh_vertex, tang))));

  // do the same thing for Bitangents but bind it to location=4
  glEnableVertexAttribArray(4);
  glVertexAttribPointer(
      4, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex),
      reinterpret_cast<void*>((offsetof(mesh_vertex, bitang))));

  // IBO
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ibo);
  // upload the indices for drawing primitives
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * m_indices.size(),
               m_indices.data(), GL_STATIC_DRAW);

  // set the index count and draw modes
  m.index_count = m_indices.size();
  m.mode = m_mode;

  // clean up by binding VAO 0 (good practice)
  glBindVertexArray(0);

  return m;
}
}  // namespace cgra
