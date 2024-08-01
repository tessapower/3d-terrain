#ifndef CGRA_GEOMETRY_HPP
#define CGRA_GEOMETRY_HPP

#include <glm/glm.hpp>

namespace cgra {
// creates a mesh for a unit sphere (radius of 1)
// immediately draws the sphere mesh, assuming the shader is set up
auto draw_sphere() -> void;

// creates a mesh for a unit cylinder (radius and height of 1) along the z-axis
// immediately draws the sphere mesh, assuming the shader is set up
auto draw_cylinder() -> void;

// creates a mesh for a unit cone (radius and height of 1) along the z-axis
// immediately draws the sphere mesh, assuming the shader is set up
auto draw_cone() -> void;

// sets up a shader and draws an axis straight to the current framebuffer
auto draw_axis(const glm::mat4& view, const glm::mat4& proj) -> void;

// sets up a shader and draws a grid straight to the current framebuffer
auto draw_grid(const glm::mat4& view, const glm::mat4& proj) -> void;
}  // namespace cgra

#endif  // CGRA_GEOMETRY_HPP
