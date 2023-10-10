#pragma once

// glm
#include <glm/glm.hpp>

// project
#include "opengl.hpp"
#include <cgra/cgra_mesh.hpp>


class terrain_model {
public:
	GLuint shader;
	cgra::gl_mesh mesh;
	cgra::mesh_builder builder;

	std::vector<std::vector<int>> adjacent_faces;

	// variables
	int m_tex = 1;
	cgra::mesh_vertex selectedPoint;
	float m_radius = 25;
	bool m_isBump = true;
	float m_strength = 7;
	float m_heightChange1 = 7;
	float m_heightChange2 = -25;
	float m_heightScale = 1;
	int gridSize = 200;

	terrain_model() { }
	void draw(const glm::mat4& view, const glm::mat4& proj);
	void createFlatGround();
	void createTerrain();

private:
	float spacing = 5.0f;  // spacing between grid points
	int m_type = 0;
};