#pragma once
#include "GL/glew.h"
#include "cgra_mesh.hpp"

/**
 * \brief Basic model that holds the shader, mesh and transform for drawing.
 */
struct basic_model {
  GLuint shader = 0;
  cgra::gl_mesh mesh;
  glm::vec3 color{0.7f};
  glm::mat4 model_transform{1.0f};
  GLuint texture{};

  auto draw(const glm::mat4& view, const glm::mat4& projection) -> void {
    glm::mat4 model_view = view * model_transform;

    // Load shader and variables into GPU
    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1,
                       false, value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1,
                       false, glm::value_ptr(model_view));
    glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));

    mesh.draw();
  }
};
