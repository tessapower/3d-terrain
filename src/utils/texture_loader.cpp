#include "texture_loader.hpp"

#include "cgra/cgra_image.hpp"

auto texture_loader::load_textures(GLuint shader) -> void {
  // Load texture data
  cgra::rgba_image texture1_data(
      CGRA_SRCDIR + std::string(R"(\res\textures\Grass_001_COLOR.jpg)"));
  cgra::rgba_image texture2_data(
      CGRA_SRCDIR + std::string(R"(\res\textures\Mud_001_baseColor.jpg)"));
  cgra::rgba_image texture3_data(
      CGRA_SRCDIR + std::string(R"(\res\textures\Rock_044_BaseColor.jpg)"));

  // Load normal maps
  cgra::rgba_image norm_texture1_data(
      CGRA_SRCDIR + std::string(R"(\res\textures\Grass_001_NORM.jpg)"));
  cgra::rgba_image norm_texture2_data(
      CGRA_SRCDIR + std::string(R"(\res\textures\Mud_001_normal.jpg)"));
  cgra::rgba_image norm_texture3_data(
      CGRA_SRCDIR + std::string(R"(\res\textures\Rock_044_Normal.jpg)"));

  // Load height maps
  cgra::rgba_image height_texture1_data(
      CGRA_SRCDIR + std::string(R"(\res\textures\Grass_001_DISP.png)"));
  cgra::rgba_image height_texture2_data(
      CGRA_SRCDIR + std::string(R"(\res\textures\Mud_001_height.png)"));
  cgra::rgba_image height_texture3_data(
      CGRA_SRCDIR + std::string(R"(\res\textures\Rock_044_Height.png)"));

  // Load leaves textures
  cgra::rgba_image leaves_data(
      CGRA_SRCDIR +
      std::string(R"(\res\textures\Stylized_Leaves_002_basecolor.jpg)"));
  cgra::rgba_image height_leaves_data(
      CGRA_SRCDIR +
      std::string(R"(\res\textures\Stylized_Leaves_002_height.png)"));

  // Load bark texture
  cgra::rgba_image bark_data(
      CGRA_SRCDIR + std::string(R"(\res\textures\Bark_06_basecolor.jpg)"));

  // Upload textures and store handles
  m_texture_1_ = texture1_data.upload_texture();
  m_texture_2_ = texture2_data.upload_texture();
  m_texture_3_ = texture3_data.upload_texture();
  m_normal_texture_1_ = norm_texture1_data.upload_texture();
  m_normal_texture_2_ = norm_texture2_data.upload_texture();
  m_normal_texture_3_ = norm_texture3_data.upload_texture();
  m_height_texture_1_ = height_texture1_data.upload_texture();
  m_height_texture_2_ = height_texture2_data.upload_texture();
  m_height_texture_3_ = height_texture3_data.upload_texture();
  m_leaves_texture_ = leaves_data.upload_texture();
  m_leaves_height_ = height_leaves_data.upload_texture();
  m_bark_texture_ = bark_data.upload_texture();

  bind_textures_to_shader(shader);
}

auto texture_loader::bind_textures_to_shader(const GLuint shader) -> void {
  // Bind texture color uniforms
  bind_texture_uniform(shader, "uTexture1", GL_TEXTURE0, m_texture_1_);
  bind_texture_uniform(shader, "uTexture2", GL_TEXTURE1, m_texture_2_);
  bind_texture_uniform(shader, "uTexture3", GL_TEXTURE2, m_texture_3_);

  // Bind texture normal map uniforms
  bind_texture_uniform(shader, "uNormTex1", GL_TEXTURE3, m_normal_texture_1_);
  bind_texture_uniform(shader, "uNormTex2", GL_TEXTURE4, m_normal_texture_2_);
  bind_texture_uniform(shader, "uNormTex3", GL_TEXTURE5, m_normal_texture_3_);

  // Bind height map uniforms
  bind_texture_uniform(shader, "uHeightTexture1", GL_TEXTURE6,
                       m_height_texture_1_);
  bind_texture_uniform(shader, "uHeightTexture2", GL_TEXTURE7,
                       m_height_texture_2_);
  bind_texture_uniform(shader, "uHeightTexture3", GL_TEXTURE8,
                       m_height_texture_3_);

  // Bind leaves texture uniforms
  bind_texture_uniform(shader, "uLeavesTexture", GL_TEXTURE9,
                       m_leaves_texture_);
  bind_texture_uniform(shader, "uLeavesHeight", GL_TEXTURE10, m_leaves_height_);

  // Bind bark texture uniform
  bind_texture_uniform(shader, "uBarkTexture", GL_TEXTURE11, m_bark_texture_);
}

auto texture_loader::bind_texture_uniform(const GLuint shader,
                                          const char* uniform_name,
                                          const GLenum texture_unit,
                                          const GLuint texture_handle) -> void {
  glActiveTexture(texture_unit);
  glBindTexture(GL_TEXTURE_2D, texture_handle);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glUniform1i(glGetUniformLocation(shader, uniform_name),
              texture_unit - GL_TEXTURE0);
}
