#pragma once

// glm
#include <glm/glm.hpp>

// project
#include "utils/opengl.hpp"
#include "cgra/cgra_mesh.hpp"


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

	// noise variables
	unsigned int seed = 0;
	unsigned int octaves = 5;
	float lacunarity = 2.0f;
	float persistence = 0.5f;
	unsigned int repeat = 3;
	float height = 200.0f;

	terrain_model() { }
	void draw(const glm::mat4& view, const glm::mat4& proj);
	void createFlatGround();
	void createTerrain();

private:
	float spacing = 5.0f;  // spacing between grid points
	int m_type = 0;
};