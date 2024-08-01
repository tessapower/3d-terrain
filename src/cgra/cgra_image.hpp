#pragma once

#include <stb_image.h>
#include <stb_image_write.h>

#include <cassert>
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <utils/opengl.hpp>
#include <vector>

#include "glm/glm.hpp"

namespace cgra {
// 4-channel rgba image
struct rgba_image {
  glm::ivec2 m_size;
  std::vector<unsigned char> data;
  glm::vec<2, GLenum> wrap{GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE};

  rgba_image() : m_size(0, 0) {}

  explicit rgba_image(const int w, const int h)
      : m_size(w, h), data(m_size.x * m_size.y * 4, 0) {}

  explicit rgba_image(const glm::ivec2 size)
      : m_size(size), data(m_size.x * m_size.y * 4, 0) {}

  explicit rgba_image(const std::string &file_name) {
    // gl expects image origin at lower left
    stbi_set_flip_vertically_on_load(true);
    unsigned char *raw_stb_data =
        stbi_load(file_name.c_str(), &m_size.x, &m_size.y, nullptr, 4);
    if (!raw_stb_data) {
      std::cerr << "Error: Failed to open image " << file_name << '\n';
      throw std::runtime_error("Error: Failed to open image " + file_name);
    }
    data.assign(raw_stb_data, raw_stb_data + m_size.x * m_size.y * 4);
    stbi_image_free(raw_stb_data);
  }

  // generates and returns a texture object
  [[nodiscard]] GLuint upload_texture(const GLenum format = GL_RGBA8,
                                      GLuint tex = 0) const {
    // check we have consistent size and data
    assert(m_size.x * m_size.y * 4 == data.size());

    if (!tex) glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap.x);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap.y);
    glTexImage2D(GL_TEXTURE_2D, 0, format, m_size.x, m_size.y, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, data.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    return tex;
  }

  // outputs the image to the given filepath and appends ".png"
  auto write_png(const std::string &filename) const -> void {
    // check we have consistent size and data
    assert(m_size.x * m_size.y * 4 == data.size());
    std::vector<unsigned char> char_data(m_size.x * m_size.y * 4, 0);
    std::ostringstream ss;
    ss << filename << ".png";
    if (stbi_write_png(ss.str().c_str(), m_size.x, m_size.y, 4,
                       data.data() + (m_size.y - 1) * m_size.x * 4,
                       -m_size.x * 4)) {
      std::cout << "Wrote image " << ss.str() << '\n';
    } else {
      std::cerr << "Error: Failed to write image " << ss.str() << '\n';
    }
  }

  // creates an image from FB0
  static auto screenshot(const bool write) -> rgba_image {
    using namespace std;
    int w, h;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glfwGetFramebufferSize(glfwGetCurrentContext(), &w, &h);

    rgba_image img(w, h);
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, img.data.data());

    if (write) {
      ostringstream filename_ss;
      filename_ss << "screenshot_"
                  << (chrono::system_clock::now().time_since_epoch() / 1ms);
      const string filename = filename_ss.str();
      img.write_png(filename);
    }

    return img;
  }
};
}  // namespace cgra
