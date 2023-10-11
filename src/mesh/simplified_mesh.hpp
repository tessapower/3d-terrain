#pragma once

// glm
#include <glm/glm.hpp>

// project
#include "utils/opengl.hpp"
#include <cgra/cgra_mesh.hpp>

// Blender kdtree
#include "mesh/simplified_mesh_debugging.hpp"

class simplified_mesh {
public:
	std::vector<std::vector<std::vector<float>>> G;

	GLuint shader = 0;
	cgra::gl_mesh mesh;
	float isolevel = 0.6f;

	cgra::mesh_builder builder;

	simplified_mesh_debugging debugging = simplified_mesh_debugging::result;
	float voxelEdgeLength = 0.01;
	bool smoothNormals = true;

	// Bounding box
	glm::vec3 topRight;
	glm::vec3 bottomLeft;

	simplified_mesh() { }
	void draw(const glm::mat4& view, const glm::mat4& proj);
	void set_model(const cgra::mesh_builder builder);
	void build(); // Builds the mesh from the unsigned distance field
	void build_from_model(); // Builds the unsigned distance field from the model, then runs build
};