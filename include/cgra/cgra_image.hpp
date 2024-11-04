#ifndef CGRA_IMAGE_HPP
#define CGRA_IMAGE_HPP

#include <chrono>
#include <string>
#include <utils/opengl.hpp>
#include <vector>

#include <glm/glm.hpp>

namespace cgra {
// 4-channel rgba image
class rgba_image {
  glm::ivec2 m_size_;
  std::vector<unsigned char> data_;
  glm::vec<2, GLenum> wrap_{GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE};

public:
  rgba_image() : m_size_(0, 0) {}

  explicit rgba_image(const int w, const int h)
      : m_size_(w, h),
        data_(static_cast<unsigned char>(m_size_.x * m_size_.y * 4), 0) {}

  explicit rgba_image(const glm::ivec2 size)
      : m_size_(size),
        data_(static_cast<unsigned char>(m_size_.x * m_size_.y * 4), 0) {}

  explicit rgba_image(const std::string &file_name);

  // generates and returns a texture object
  [[nodiscard]] GLuint upload_texture(GLenum format = GL_RGBA8,
                                      GLuint tex = 0) const;

  // outputs the image to the given filepath and appends ".png"
  auto write_png(const std::string &filename) const -> void;

  // creates an image from FB0
  static auto screenshot(bool write) -> rgba_image;
};
}  // namespace cgra

#endif  // CGRA_IMAGE_HPP
