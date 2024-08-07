#ifndef CGRA_OPENGL_HPP
#define CGRA_OPENGL_HPP

/// OpenGL header
/// Set up to include everything necessary for OpenGL functionality in one place

// include glew.h before (instead of) gl.h, or anything that includes gl.h
// glew.h replaces gl.h and sets up OpenGL functions in a cross-platform manner
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace cgra {
// helper function that draws an empty OpenGL object
// can be used for shaders that do all the work
inline void draw_dummy(const unsigned instances = 1) {
  static GLuint vao = 0;
  if (vao == 0) {
    glGenVertexArrays(1, &vao);
  }
  glBindVertexArray(vao);
  glDrawArraysInstanced(GL_POINTS, 0, 1, instances);
  glBindVertexArray(0);
}

// gl_object is a helper class that wraps around a GLuint
// object id for OpenGL. Does not allow copying (can't be
// owned by more than one thing) and de-allocates the object
// when destroyed (easy cleanup). Can be used for VAO, VBO,
// textures and framebuffers etc.
class gl_object {
 public:
  using destroyer_t = void(APIENTRY *)(GLsizei, const GLuint *);

 private:
  GLuint m_id_ = 0;
  destroyer_t m_dtor_;

  auto destroy() noexcept -> void {
    if (m_id_) {
      m_dtor_(1, &m_id_);
      m_id_ = 0;
    }
  }

 public:
  // empty object
  gl_object() = default;

  // explicit ctor
  // takes an existing OpenGL object identifier and a pointer to a function
  // that will de-allocated the identifier on destruction
  gl_object(const GLuint id, const destroyer_t dtor_)
      : m_id_(id), m_dtor_(dtor_) {}

  // remove copy constructors
  gl_object(const gl_object &) = delete;
  gl_object &operator=(const gl_object &) = delete;

  // define move constructors
  gl_object(gl_object &&other) noexcept {
    m_id_ = other.m_id_;
    m_dtor_ = other.m_dtor_;
    other.m_id_ = 0;
  }

  auto operator=(gl_object &&other) noexcept -> gl_object & {
    destroy();
    m_id_ = other.m_id_;
    m_dtor_ = other.m_dtor_;
    other.m_id_ = 0;
    return *this;
  }

  // implicit GLuint converter
  // returns the OpenGL identifier for this object
  operator GLuint() const noexcept { return m_id_; }

  // explicit boolean converter
  // true IFF identifier is not zero
  explicit operator bool() const noexcept { return m_id_; }

  // true IFF identifier is zero
  auto operator!() const noexcept -> bool { return !m_id_; }

  // destructor
  ~gl_object() { destroy(); }

  // returns a gl_object with an OpenGL buffer identifier
  static auto gen_buffer() -> gl_object {
    GLuint o;
    glGenBuffers(1, &o);
    return {o, glDeleteBuffers};
  }

  // returns a gl_object with an OpenGL vertex array identifier
  static auto gen_vertex_array() -> gl_object {
    GLuint o;
    glGenVertexArrays(1, &o);
    return {o, glDeleteVertexArrays};
  }

  // returns a gl_object with an OpenGL texture identifier
  static auto gen_texture() -> gl_object {
    GLuint o;
    glGenTextures(1, &o);
    return {o, glDeleteTextures};
  }

  // returns a gl_object with an OpenGL framebuffer identifier
  static auto gen_framebuffer() -> gl_object {
    GLuint o;
    glGenFramebuffers(1, &o);
    return {o, glDeleteFramebuffers};
  }

  // returns a gl_object with an OpenGL shader identifier
  static auto gen_shader(const GLenum type) -> gl_object {
    GLuint o = glCreateShader(type);
    return {o, [](GLsizei, const GLuint *o) { glDeleteShader(*o); }};
  }

  // returns a gl_object with an OpenGL shader program identifier
  static auto gen_program() -> gl_object {
    GLuint o = glCreateProgram();
    return {o, [](GLsizei, const GLuint *o) { glDeleteProgram(*o); }};
  }
};
}  // namespace cgra

#endif  // CGRA_OPENGL_HPP
