#pragma once

// glm
#include <glm/glm.hpp>

// project
#include "opengl.hpp"
#include <cgra/cgra_mesh.hpp>

// Octree
#include "octree.h"

class simplified_mesh {
private:
	std::vector<std::vector<std::vector<float>>> G;
	OrthoTree::OctreePoint octree;
	std::vector<OrthoTree::Point3D> octreePoints;

public:
	GLuint shader = 0;
	cgra::gl_mesh mesh;
	float isolevel = 0.6f;

	cgra::mesh_builder builder;

	int debugging = 0;

	simplified_mesh() { }
	void draw(const glm::mat4& view, const glm::mat4& proj);
	void set_model(const cgra::mesh_builder builder);
	void build(glm::vec2 screenSize);
};