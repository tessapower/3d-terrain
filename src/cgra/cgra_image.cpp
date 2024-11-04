#include "cgra/cgra_image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <cassert>
#include <chrono>
#include <iostream>
#include <sstream>

namespace cgra {
  rgba_image::rgba_image(const std::string &file_name) {
    // gl expects image origin at lower left
    stbi_set_flip_vertically_on_load(true);
    unsigned char *raw_stb_data =
        stbi_load(file_name.c_str(), &m_size_.x, &m_size_.y, nullptr, 4);
    if (!raw_stb_data) {
      std::cerr << "Error: Failed to open image " << file_name << '\n';
      throw std::runtime_error("Error: Failed to open image " + file_name);
    }
    data_.assign(raw_stb_data, raw_stb_data + m_size_.x * m_size_.y * 4);
    stbi_image_free(raw_stb_data);
  }

  GLuint rgba_image::upload_texture(const GLenum format, GLuint tex) const {
    // check we have consistent size and data
    assert(m_size_.x * m_size_.y * 4 == data_.size());

    if (!tex) glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_.x);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_.y);
    glTexImage2D(GL_TEXTURE_2D, 0, format, m_size_.x, m_size_.y, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, data_.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    return tex;
  }

  auto rgba_image::write_png(const std::string &filename) const -> void {
    // check we have consistent size and data
    assert(m_size_.x * m_size_.y * 4 == data_.size());
    std::vector<unsigned char> char_data(m_size_.x * m_size_.y * 4, 0);
    std::ostringstream ss;
    ss << filename << ".png";
    if (stbi_write_png(ss.str().c_str(), m_size_.x, m_size_.y, 4,
                       data_.data() + (m_size_.y - 1) * m_size_.x * 4,
                       -m_size_.x * 4)) {
      std::cout << "Wrote image " << ss.str() << '\n';
    } else {
      std::cerr << "Error: Failed to write image " << ss.str() << '\n';
    }
  }

  auto rgba_image::screenshot(const bool write) -> rgba_image {
    using namespace std;
    int w, h;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glfwGetFramebufferSize(glfwGetCurrentContext(), &w, &h);

    rgba_image img(w, h);
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, img.data_.data());

    if (write) {
      ostringstream filename_ss;
      filename_ss << "screenshot_"
                  << (chrono::system_clock::now().time_since_epoch() / 1ms);
      const string filename = filename_ss.str();
      img.write_png(filename);
    }

    return img;
  }
}  // namespace cgra
