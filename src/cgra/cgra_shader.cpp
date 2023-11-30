#include "cgra_shader.hpp"

#include <fstream>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <utils/opengl.hpp>
#include <vector>

// forward declaration
class shader_error : public std::runtime_error {
 public:
  explicit shader_error(const std::string &what = "Generic shader error.")
      : std::runtime_error(what) {}
};

class shader_type_error final : public shader_error {
 public:
  explicit shader_type_error(const std::string &what = "Bad shader type.")
      : shader_error(what) {}
};

class shader_compile_error final : public shader_error {
 public:
  explicit shader_compile_error(
      const std::string &what = "Shader compilation failed.")
      : shader_error(what) {}
};

class shader_link_error final : public shader_error {
 public:
  explicit shader_link_error(
      const std::string &what = "Shader program linking failed.")
      : shader_error(what) {}
};

auto print_shader_info_log(const GLuint obj) -> void {
  int info_log_length = 0;
  int chars_written = 0;
  glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &info_log_length);
  if (info_log_length > 1) {
    std::vector<char> info_log(info_log_length);
    glGetShaderInfoLog(obj, info_log_length, &chars_written, info_log.data());
    std::cout << "CGRA Shader : "
              << "SHADER :\n"
              << info_log.data() << std::endl;
  }
}

auto print_program_info_log(const GLuint obj) -> void {
  int info_log_length = 0;
  int chars_written = 0;
  glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &info_log_length);
  if (info_log_length > 1) {
    std::vector<char> info_log(info_log_length);
    glGetProgramInfoLog(obj, info_log_length, &chars_written, info_log.data());
    std::cout << "CGRA Shader : "
              << "PROGRAM :\n"
              << info_log.data() << std::endl;
  }
}

namespace cgra {
auto shader_builder::set_shader(const GLenum type, const std::string &file_name)
    -> void {
  const std::ifstream file_stream(file_name);

  if (!file_stream) {
    std::cerr << "Error: Could not locate and open file " << file_name
              << std::endl;
    throw std::runtime_error("Error: Could not locate and open file " +
                             file_name);
  }

  std::stringstream buffer;
  buffer << file_stream.rdbuf();

  try {
    set_shader_source(type, buffer.str());
  } catch ([[maybe_unused]] shader_compile_error &e) {
    std::cerr << "Error: Could not compile " << file_name << std::endl;
    throw;
  }
}

auto shader_builder::set_shader_source(const GLenum type,
                                       const std::string &shader_source)
    -> void {
  // same as GLint shader = glCreateShader(type);
  gl_object shader = gl_object::gen_shader(type);

  // cgra specific extra (allows different shaders to be defined in a single
  // source) Start of CGRA addition
  const auto get_define = [](const GLenum s_type) {
    switch (s_type) {
      case GL_VERTEX_SHADER:
        return "_VERTEX_";
      case GL_GEOMETRY_SHADER:
        return "_GEOMETRY_";
      case GL_TESS_CONTROL_SHADER:
        return "_TESS_CONTROL_";
      case GL_TESS_EVALUATION_SHADER:
        return "_TESS_EVALUATION_";
      case GL_FRAGMENT_SHADER:
        return "_FRAGMENT_";
      default:
        return "_INVALID_SHADER_TYPE_";
    }
  };

  std::istringstream iss(shader_source);
  std::ostringstream oss;
  while (iss) {
    std::string line;
    std::getline(iss, line);
    oss << line << std::endl;
    if (line.find("#version") < line.find("//")) break;
  }
  oss << "#define " << get_define(type) << std::endl;
  oss << iss.rdbuf();
  const std::string final_source = oss.str();
  // End of CGRA addition

  // upload and compile the shader
  const char *text_c = final_source.c_str();
  glShaderSource(shader, 1, &text_c, nullptr);
  glCompileShader(shader);

  // check compilation status
  GLint compile_status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
  print_shader_info_log(shader);  // print warnings and errors
  if (!compile_status) throw shader_compile_error();

  m_shaders_[type] = std::make_shared<gl_object>(std::move(shader));
}

auto shader_builder::build(GLuint program) const -> GLuint {
  // if the program exists get attached shaders and detach them
  if (program) {
    int shader_count = 0;
    glGetProgramiv(program, GL_ATTACHED_SHADERS, &shader_count);

    if (shader_count > 0) {
      std::vector<GLuint> attached_shaders(shader_count);
      int actual_shader_count = 0;
      glGetAttachedShaders(program, shader_count, &actual_shader_count,
                           attached_shaders.data());
      for (int i = 0; i < actual_shader_count; i++) {
        glDetachShader(program, attached_shaders[i]);
      }
    }
  } else {
    program = glCreateProgram();
  }

  // attach shaders
  for (const auto &snd : m_shaders_ | std::views::values) {
    glAttachShader(program, *(snd));
  }

  // link the program
  glLinkProgram(program);

  // check link status
  GLint link_status;
  glGetProgramiv(program, GL_LINK_STATUS, &link_status);
  print_program_info_log(program);  // print warnings and errors
  if (!link_status) throw shader_link_error();

  return program;
}
}  // namespace cgra
