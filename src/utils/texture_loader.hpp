#pragma once

#include "opengl.hpp"

/// Code Author: Shekinah Pratap

/*
 * Loads in the following textures:
 * https://3dtextures.me/2018/01/05/grass-001-2/
 * https://3dtextures.me/2020/01/31/mud-001/
 * https://3dtextures.me/2022/03/03/rock-044/
 * https://3dtextures.me/2020/09/08/stylized-leaves-002/
 * https://3dtextures.me/2021/05/05/bark-006-2/
 */
class texture_loader {
 public:
  texture_loader() = default;
  auto load_textures(GLuint shader) -> void;
  auto bind_textures_to_shader(GLuint shader) -> void;
  static auto bind_texture_uniform(GLuint shader, const char* uniform_name,
                                   GLenum texture_unit, GLuint texture_handle)
      -> void;

 private:
  GLuint m_texture_1_;
  GLuint m_texture_2_;
  GLuint m_texture_3_;
  GLuint m_normal_texture_1_;
  GLuint m_normal_texture_2_;
  GLuint m_normal_texture_3_;
  GLuint m_height_texture_1_;
  GLuint m_height_texture_2_;
  GLuint m_height_texture_3_;
  GLuint m_leaves_texture_;
  GLuint m_leaves_height_;
  GLuint m_bark_texture_;
};
