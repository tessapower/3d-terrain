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

	cgra::mesh_builder builder;

	int debugging = 0;

	simplified_mesh() { }
	void draw(const glm::mat4& view, const glm::mat4& proj);
	void set_model(const cgra::mesh_builder builder);
	void build(glm::vec2 screenSize);
};