#pragma once

// glm
#include <glm/glm.hpp>

// project
#include "opengl.hpp"
#include <cgra/cgra_mesh.hpp>

class simplified_mesh {
public:
	GLuint shader = 0;
	cgra::gl_mesh mesh;

	simplified_mesh() { }
	void draw(const glm::mat4& view, const glm::mat4& proj);
	void build(glm::vec2 screenSize, const cgra::mesh_builder builder);
};