#include "skybox.hpp"

#include <stb_image.h>

#include "cgra/cgra_shader.hpp"
#include "glm/gtc/type_ptr.hpp"

skybox::skybox() noexcept {
  m_shader_ = load_shaders();
  m_texture_ = load_cube_map();
  m_mesh_ = load_mesh();
}

GLuint skybox::load_shaders() const noexcept {
  cgra::shader_builder sb;
  sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//skybox.vs.glsl"));
  sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//skybox.fs.glsl"));

  return sb.build();
}

GLuint skybox::load_cube_map() const noexcept {
  GLuint texture_id;
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

  int width, height, nr_channels;
  for (unsigned i = 0; i < m_textures_.size(); i++) {
    if (unsigned char* data = stbi_load(m_textures_[i].c_str(), &width, &height, &nr_channels, 0)) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    } else {
      std::cout << "Cube map texture failed to load at path: " << m_textures_[i] << std::endl;
      stbi_image_free(data);
    }
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return texture_id;
}

cgra::gl_mesh skybox::load_mesh() const noexcept {
  cgra::mesh_builder mb;

  for (int i = 0; i < 36; ++i) {
    mb.push_vertex({m_vertices_[i], glm::vec3{0.0f}, glm::vec3{0.0f}, glm::vec3{0.0f}});
    mb.push_index(i);
  }

  return mb.build();
}

void skybox::draw(const glm::mat4& view, const glm::mat4& projection) noexcept {
  glDepthFunc(GL_LEQUAL);
  glUseProgram(m_shader_);
  // Remove translation from the view matrix
  const auto adjusted_view = glm::mat4(glm::mat3(view));

  glUniformMatrix4fv(glGetUniformLocation(m_shader_, "uModelViewMatrix"), 1, false, glm::value_ptr(adjusted_view));
  glUniformMatrix4fv(glGetUniformLocation(m_shader_, "uProjectionMatrix"), 1, false, glm::value_ptr(projection));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture_);
  m_mesh_.draw();

  glDepthFunc(GL_LESS);
}
