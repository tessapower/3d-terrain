#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

#include "cgra/cgra_mesh.hpp"

/// Code Author: Tessa Power
///
/// Created with the help of the Cubemaps tutorial from:
/// https://learnopengl.com/Advanced-OpenGL/Cubemaps
///
/// Skybox files:
/// https://learnopengl.com/img/textures/skybox.zip
/// https://polyhaven.com/a/kloofendal_43d_clear_puresky (converted using https://matheowis.github.io/HDRI-to-CubeMap/)

/**
 * \brief Represents a skybox.
 */
class skybox {
  const std::vector<glm::vec3> m_vertices_ = {
      // Right (+x)
      {-1.0f,  1.0f, -1.0f},
      {-1.0f, -1.0f, -1.0f},
      { 1.0f, -1.0f, -1.0f},
      { 1.0f, -1.0f, -1.0f},
      { 1.0f,  1.0f, -1.0f},
      {-1.0f,  1.0f, -1.0f},
      // Left (-x)
      {-1.0f, -1.0f,  1.0f},
      {-1.0f, -1.0f, -1.0f},
      {-1.0f,  1.0f, -1.0f},
      {-1.0f,  1.0f, -1.0f},
      {-1.0f,  1.0f,  1.0f},
      {-1.0f, -1.0f,  1.0f},
      // Top (+y)
      {1.0f, -1.0f, -1.0f},
      {1.0f, -1.0f,  1.0f},
      {1.0f,  1.0f,  1.0f},
      {1.0f,  1.0f,  1.0f},
      {1.0f,  1.0f, -1.0f},
      {1.0f, -1.0f, -1.0f},
      // Bottom (-y)
      {-1.0f,  1.0f, 1.0f},
      {-1.0f, -1.0f, 1.0f},
      { 1.0f,  1.0f, 1.0f},
      { 1.0f,  1.0f, 1.0f},
      { 1.0f, -1.0f, 1.0f},
      {-1.0f, -1.0f, 1.0f},
      // Back (+z)
      {-1.0f, 1.0f, -1.0f},
      { 1.0f, 1.0f, -1.0f},
      { 1.0f, 1.0f,  1.0f},
      { 1.0f, 1.0f,  1.0f},
      {-1.0f, 1.0f,  1.0f},
      {-1.0f, 1.0f, -1.0f},
      // Front (-z)
      {-1.0f, -1.0f, -1.0f},
      {-1.0f, -1.0f,  1.0f},
      { 1.0f, -1.0f, -1.0f},
      { 1.0f, -1.0f, -1.0f},
      {-1.0f, -1.0f,  1.0f},
      { 1.0f, -1.0f,  1.0f}
  };

  // Texture files
  const std::vector<std::string> m_textures_ = {
      CGRA_SRCDIR + std::string{"/res/textures/skybox/right.jpg"},
      CGRA_SRCDIR + std::string{"/res/textures/skybox/left.jpg"},
      CGRA_SRCDIR + std::string{"/res/textures/skybox/top.jpg"},
      CGRA_SRCDIR + std::string{"/res/textures/skybox/bottom.jpg"},
      CGRA_SRCDIR + std::string{"/res/textures/skybox/back.jpg"},
      CGRA_SRCDIR + std::string{"/res/textures/skybox/front.jpg"}
  };

  GLuint m_shader_ = 0;
  GLuint m_texture_;
  cgra::gl_mesh m_mesh_;

 public:
  skybox() noexcept;
  void draw(const glm::mat4& view, const glm::mat4& projection) noexcept;

 private:
  [[nodiscard]] GLuint load_shaders() const noexcept;
  [[nodiscard]] GLuint load_cube_map() const noexcept;
  [[nodiscard]] cgra::gl_mesh load_mesh() const noexcept;
};
