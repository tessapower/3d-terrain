#ifndef SKYBOX_HPP
#define SKYBOX_HPP

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
/// https://polyhaven.com/a/kloofendal_43d_clear_puresky (converted using
/// https://matheowis.github.io/HDRI-to-CubeMap/)

/**
 * \brief Represents a skybox.
 */
class skybox {
  std::vector<glm::vec3> m_vertices_ = {
      // Right (+x)
      {-1.0f, 1.0f, -1.0f},
      {-1.0f, -1.0f, -1.0f},
      {1.0f, -1.0f, -1.0f},
      {1.0f, -1.0f, -1.0f},
      {1.0f, 1.0f, -1.0f},
      {-1.0f, 1.0f, -1.0f},
      // Left (-x)
      {-1.0f, -1.0f, 1.0f},
      {-1.0f, -1.0f, -1.0f},
      {-1.0f, 1.0f, -1.0f},
      {-1.0f, 1.0f, -1.0f},
      {-1.0f, 1.0f, 1.0f},
      {-1.0f, -1.0f, 1.0f},
      // Top (+y)
      {1.0f, -1.0f, -1.0f},
      {1.0f, -1.0f, 1.0f},
      {1.0f, 1.0f, 1.0f},
      {1.0f, 1.0f, 1.0f},
      {1.0f, 1.0f, -1.0f},
      {1.0f, -1.0f, -1.0f},
      // Bottom (-y)
      {-1.0f, 1.0f, 1.0f},
      {-1.0f, -1.0f, 1.0f},
      {1.0f, 1.0f, 1.0f},
      {1.0f, 1.0f, 1.0f},
      {1.0f, -1.0f, 1.0f},
      {-1.0f, -1.0f, 1.0f},
      // Back (+z)
      {-1.0f, 1.0f, -1.0f},
      {1.0f, 1.0f, -1.0f},
      {1.0f, 1.0f, 1.0f},
      {1.0f, 1.0f, 1.0f},
      {-1.0f, 1.0f, 1.0f},
      {-1.0f, 1.0f, -1.0f},
      // Front (-z)
      {-1.0f, -1.0f, -1.0f},
      {-1.0f, -1.0f, 1.0f},
      {1.0f, -1.0f, -1.0f},
      {1.0f, -1.0f, -1.0f},
      {-1.0f, -1.0f, 1.0f},
      {1.0f, -1.0f, 1.0f}};

  // Texture files
  std::vector<std::string> m_textures_ = {
      std::string{"res/textures/skybox/right.jpg"},
      std::string{"res/textures/skybox/left.jpg"},
      std::string{"res/textures/skybox/top.jpg"},
      std::string{"res/textures/skybox/bottom.jpg"},
      std::string{"res/textures/skybox/back.jpg"},
      std::string{"res/textures/skybox/front.jpg"}};

  GLuint m_shader_ = 0;
  GLuint m_texture_;
  cgra::gl_mesh m_mesh_;

 public:
  skybox() noexcept;
  auto draw(const glm::mat4& view, const glm::mat4& projection) const noexcept
      -> void;

 private:
  [[nodiscard]] static auto load_shaders() noexcept -> GLuint;
  [[nodiscard]] auto load_cube_map() const noexcept -> GLuint;
  [[nodiscard]] auto load_mesh() const noexcept -> cgra::gl_mesh;
};

#endif  // SKYBOX_HPP
