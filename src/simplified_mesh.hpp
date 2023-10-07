#pragma once

// glm
#include <glm/glm.hpp>

// project
#include "opengl.hpp"
#include <cgra/cgra_mesh.hpp>

// Blender kdtree
#include "KDTree.h"


class simplified_mesh {
private:
	std::vector<std::vector<std::vector<float>>> G;

public:
	GLuint shader = 0;
	cgra::gl_mesh mesh;
	float isolevel = 0.6f;

	cgra::mesh_builder builder;

	int debugging = 0;
	float voxelEdgeLength = 0.01;

	jk::tree::KDTree<int, 3, 512> tree;

	simplified_mesh() { }
	void draw(const glm::mat4& view, const glm::mat4& proj);
	void set_model(const cgra::mesh_builder builder);
	void build(glm::vec2 screenSize);
};