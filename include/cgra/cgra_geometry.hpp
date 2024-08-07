#ifndef CGRA_GEOMETRY_HPP
#define CGRA_GEOMETRY_HPP

#include <glm/glm.hpp>

namespace cgra {
/**
 * \brief Creates a mesh for a unit sphere (radius of 1) and
 * immediately draws the sphere mesh, assuming the shader is set up.
 */
auto draw_sphere() -> void;

/**
 * \brief Creates a mesh for a unit cylinder (radius and height of 1) along the
 * z-axis immediately draws the sphere mesh, assuming the shader is set up.
 */
auto draw_cylinder() -> void;

/**
 * \brief Creates a mesh for a unit cone (radius and height of 1) along the
 * z-axis and immediately draws the sphere mesh, assuming the shader is set up.
 */
auto draw_cone() -> void;

/**
 * \brief Sets up a shader and draws an axis straight to the current
 * framebuffer.
 */
auto draw_axis(const glm::mat4& view, const glm::mat4& proj) -> void;

/**
 * \brief Sets up a shader and draws a grid straight to the current framebuffer.
 */
auto draw_grid(const glm::mat4& view, const glm::mat4& proj) -> void;
}  // namespace cgra

#endif  // CGRA_GEOMETRY_HPP
