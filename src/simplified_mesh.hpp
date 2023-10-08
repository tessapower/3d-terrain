#pragma once

// glm
#include <glm/glm.hpp>

// project
#include "opengl.hpp"
#include <cgra/cgra_mesh.hpp>

// Blender kdtree
#include "KDTree.h"


class simplified_mesh {
public:
	std::vector<std::vector<std::vector<float>>> G;

	GLuint shader = 0;
	cgra::gl_mesh mesh;
	float isolevel = 0.6f;

	cgra::mesh_builder builder;

	int debugging = 0;
	float voxelEdgeLength = 0.01;


	// Bounding box
	glm::vec3 topRight;
	glm::vec3 bottomLeft;

	jk::tree::KDTree<int, 3, 512> tree;

	simplified_mesh() { }
	void draw(const glm::mat4& view, const glm::mat4& proj);
	void set_model(const cgra::mesh_builder builder);
	void build(); // Builds the mesh from the unsigned distance field
	void build_from_model(); // Builds the unsigned distance field from the model, then runs build
};