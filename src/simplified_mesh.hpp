#pragma once

// glm
#include <glm/glm.hpp>

// project
#include "opengl.hpp"
#include "cgra/cgra_mesh.hpp"

class simplified_mesh {
public:
	cgra_mesh mesh;

	simplified_mesh() { }
	void draw(glm::vec2 screenSize, const glm::mat4& view, const glm::mat4& proj);
};