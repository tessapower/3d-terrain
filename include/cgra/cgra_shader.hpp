#ifndef CGRA_SHADER_HPP
#define CGRA_SHADER_HPP

#include <map>
#include <memory>
#include <string>
#include <utils/opengl.hpp>

namespace cgra {
class shader_builder {
  std::map<GLenum, std::shared_ptr<gl_object>> m_shaders_;

 public:
  shader_builder() = default;
  auto set_shader(GLenum type, const std::string& file_name) -> void;
  auto set_shader_source(GLenum type, const std::string& shader_source) -> void;

  [[nodiscard]] auto build(GLuint program = 0) const -> GLuint;
};
}  // namespace cgra

#endif  // CGRA_SHADER_HPP
