#pragma once

#include "terrain/terrain_model.hpp"
#include "utils/perlin_noise.hpp"
#include <glm/gtc/type_ptr.hpp>

void terrain_model::createTerrain(bool perlin) {
	cgra::mesh_builder mb;
	cgra::mesh_vertex mv;

	Perlin terrain = Perlin(seed, octaves, lacunarity, persistence, repeat);

	// Calculate the total width and length of the grid
	float totalWidth = gridSize * spacing;
	float totalLength = gridSize * spacing;

	// Calculate the offset to center the grid
	float xOffset = -totalWidth / 2.0f;
	float zOffset = -totalLength / 2.0f;

	// Create a 2D array to store adjacent faces for each vertex
	adjacent_faces = std::vector<std::vector<int>>((gridSize + 1) * (gridSize + 1));

	// Generate the grid vertices
	for (int i = 0; i <= gridSize; ++i) {
		for (int j = 0; j <= gridSize; ++j) {
			// Calculate vertex position (x, y, z) with increased spacing and centered
			float x = static_cast<float>(i) * spacing + xOffset;
			float z = static_cast<float>(j) * spacing + zOffset;
			// whether to use perlin or not
			float y = perlin ? (terrain.perlin(x, 0.0, z) * height) - 100 : 0.0f;

			mv.pos = { x, y, z };

			// Normal vectors for flat ground (facing up)
			mv.norm = { 0.0f, 1.0f, 0.0f };

			// Texture coordinates (u, v) can be set as desired
			mv.uv = { static_cast<float>(i) * 10 / gridSize, static_cast<float>(j) * 10 / gridSize };

			mb.push_vertex(mv);
		}
	}

	// Generate triangle indices to create the grid
	for (int i = 0; i < gridSize; ++i) {
		for (int j = 0; j < gridSize; ++j) {
			int k1 = i * (gridSize + 1) + j;
			int k2 = k1 + 1;
			int k3 = (i + 1) * (gridSize + 1) + j;
			int k4 = k3 + 1;

			// First triangle
			mb.push_index(k1);
			mb.push_index(k2);
			mb.push_index(k3);

			// Second triangle
			mb.push_index(k2);
			mb.push_index(k4);
			mb.push_index(k3);

			adjacent_faces[k1].push_back(k1);
			adjacent_faces[k1].push_back(k2);
			adjacent_faces[k1].push_back(k3);
			adjacent_faces[k2].push_back(k1);
			adjacent_faces[k2].push_back(k2);
			adjacent_faces[k2].push_back(k3);
			adjacent_faces[k3].push_back(k1);
			adjacent_faces[k3].push_back(k2);
			adjacent_faces[k3].push_back(k3);

			adjacent_faces[k2].push_back(k2);
			adjacent_faces[k2].push_back(k4);
			adjacent_faces[k2].push_back(k3);
			adjacent_faces[k4].push_back(k2);
			adjacent_faces[k4].push_back(k4);
			adjacent_faces[k4].push_back(k3);
			adjacent_faces[k3].push_back(k2);
			adjacent_faces[k3].push_back(k4);
			adjacent_faces[k3].push_back(k3);
		}
	}

	builder = mb;
	mesh = mb.build();
}

void terrain_model::draw(const glm::mat4& view, const glm::mat4& proj) {
	glUseProgram(shader);
	// Set uniform values
	glUniform3f(glGetUniformLocation(shader, "uCenter"), selectedPoint.pos.x, selectedPoint.pos.y, selectedPoint.pos.z); // Set the center point
	glUniform1f(glGetUniformLocation(shader, "uRadius"), m_radius); // Set the selection radius
	glUniform1f(glGetUniformLocation(shader, "uHeightChange1"), m_heightChange1);
	glUniform1f(glGetUniformLocation(shader, "uHeightChange2"), m_heightChange2);
	glUniform1iv(glGetUniformLocation(shader, "uTex"), 1, &m_tex);
	glUniform1f(glGetUniformLocation(shader, "uHeightScale"), m_heightScale);
	glUniform1iv(glGetUniformLocation(shader, "uType"), 1, &m_type);
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(view));

	mesh.draw(); // draw

	// reset for other objects
	int reset = -1;
	glUniform1iv(glGetUniformLocation(shader, "uType"), 1, &reset);
}