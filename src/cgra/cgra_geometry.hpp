#pragma once

#include <glm/gtc/matrix_transform.hpp>

namespace cgra {
  // creates a mesh for a unit sphere (radius of 1)
  // immediately draws the sphere mesh, assuming the shader is set up
  void draw_sphere();

  // creates a mesh for a unit cylinder (radius and height of 1) along the z-axis
  // immediately draws the sphere mesh, assuming the shader is set up
  void draw_cylinder();
  
  // creates a mesh for a unit cone (radius and height of 1) along the z-axis
  // immediately draws the sphere mesh, assuming the shader is set up
  void draw_cone();

  // sets up a shader and draws an axis straight to the current framebuffer
  void draw_axis(const glm::mat4 &view, const glm::mat4 &proj);

  // sets up a shader and draws a grid straight to the current framebuffer
  void draw_grid(const glm::mat4 &view, const glm::mat4 &proj);
}
