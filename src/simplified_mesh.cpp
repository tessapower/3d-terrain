#pragma once

// glm
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// project
#include "cgra/cgra_geometry.hpp"
#include "simplified_mesh.hpp"


using namespace std;
using namespace glm;
using namespace cgra;

vec3 gridToWorldPosition(vec3 topRight, vec3 bottomLeft, vec3 position, vec3 gridSize);
mesh_builder debug_box(vec3 bottomLeft, vec3 size);

/*

Implementing the paper Robust Low-Poly Meshing for General 3D Models by Zhen Chen, Zherong Pan, Kui Wu, Etienne Vouga, Xifeng Gao
https://dl.acm.org/doi/10.1145/3592396

Code Author: Marshall Scott

*/

void simplified_mesh::draw(const glm::mat4& view, const glm::mat4& proj) {

	glUseProgram(shader); // load shader and variables
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(view));
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(vec3(1,1,1)));

	mesh.draw(); // draw
}

void simplified_mesh::set_model(mesh_builder builder) {
	this->builder = builder;
}

void simplified_mesh::build(glm::vec2 screenSize) {

	int nF = 5000; // Target number of triangles

	// TODO: Calculate bounding box for mesh
	vec3 topRight = builder.vertices[0].pos;
	vec3 bottomLeft = builder.vertices[0].pos;

	for (mesh_vertex vertex : builder.vertices) {
		if (vertex.pos.y > topRight.y) {
			topRight.y = vertex.pos.y;
		}
		if (vertex.pos.y < bottomLeft.y) {
			bottomLeft.y = vertex.pos.y;
		}
		if (vertex.pos.x > topRight.x) {
			topRight.x = vertex.pos.x;
		}
		if (vertex.pos.x < bottomLeft.x) {
			bottomLeft.x = vertex.pos.x;
		}
		if (vertex.pos.z > topRight.z) {
			topRight.z = vertex.pos.z;
		}
		if (vertex.pos.z < bottomLeft.z) {
			bottomLeft.z = vertex.pos.z;
		}
	}

	// Bounding cube debugging
	if (debugging == 1) {
		mesh_builder t = debug_box(bottomLeft, topRight - bottomLeft);
		t.append(builder);
		mesh = t.build();
		return;
	}

	// TODO: Project bounding box using projection & view matrix

	// TODO: Calculate input parameter np
	
	vec2 screenSpacePos(700, 500);

	float lp = screenSpacePos.x; // Pixel length of screenspace bounding box
	float l = sqrt(pow(screenSpacePos.x, 2) + pow(screenSpacePos.y, 2)); // Diagonal length of screenspace bounding box

	float np = 500; //l / lp; // the maximum number of pixels that the high-poly mesh’s diagonal could occupy across all potential rendering view
	float d = l / np;

	float voxelEdgeLength = d / sqrt(3);

	// Grid discretization (Mi,d)

	// Calculate grid points
	vec3 gridSize = vec3(
		std::ceil((topRight.x - bottomLeft.x) / voxelEdgeLength),
		std::ceil((topRight.y - bottomLeft.y) / voxelEdgeLength),
			std::ceil((topRight.z - bottomLeft.z) / voxelEdgeLength)
		);

	vector<vector<vector<float>>> G(gridSize.x, vector(gridSize.y, vector(gridSize.z, 9999999999.9f)));

	// Calculate unsigned distance field
	// Compute f(p) for all grid points p in G
	for (int x = 0; x < gridSize.x; x++) {
		for (int y = 0; y < gridSize.y; y++) {
			for (int z = 0; z < gridSize.z; z++) {
				vec3 gridPoint = gridToWorldPosition(topRight, bottomLeft, vec3(x,y,z), gridSize);
				// Loop over all points
				for (auto t : builder.vertices) {
					vec3 val = t.pos - gridPoint;
					G[x][y][z] = std::min(G[x][y][z], abs(val.x) + abs(val.y) + abs(val.z));
				}
			}
		}
	}

	// Visualize unsigned distance field
	if (debugging == 3) {
		mesh_builder debugging;
		//mesh_builder builder;
		for (int x = 0; x < gridSize.x; x++) {
			for (int y = 0; y < gridSize.y; y++) {
				for (int z = 0; z < gridSize.z; z++) {
					vec3 gridPoint = gridToWorldPosition(topRight, bottomLeft, vec3(x, y, z), gridSize);
					float sizeFloat = 0.5 - (G[x][y][z] / 5.0);
					if (sizeFloat < 0) {
						sizeFloat = 0;
					}
					vec3 size(sizeFloat);

					debugging.append(debug_box(gridPoint - (size/2.0f), size));
				}
			}
		}
		debugging.append(builder);
		mesh = debugging.build();
		return;
	}

	/*
	for (int cube : cubes) {

		// TODO: Lookup template case
		// template = templates[cube];

		for (patch : cube_case) {

			// TODO: Compute the iso-points on cube edges

			// TODO: For the quadratic program

			// Solve for feature vertices

		}

	}*/

	// TODO: Run edge flip on output

	// TODO: Run feature filter on output

	// TODO: Run remove interior on output

	// TODO: Store mesh

	mesh = builder.build();

}


float discretization(vec3 p) {
	// q must be an element of the mesh
	vec3 q = vec3(); // TODO: Figure out where this comes from

	vec3 a = p - q;

	return sqrt(dot(a, a));
}

vec3 gridToWorldPosition(vec3 topRight, vec3 bottomLeft, vec3 position, vec3 gridSize) {
	vec3 size = topRight - bottomLeft;

	return bottomLeft + vec3(position.x * (size.x / gridSize.x), position.y * (size.y / gridSize.y), position.z * (size.z / gridSize.z));
}

/// <summary>
/// Feature Vertex Insertion.
/// We use one vertex per local patch with a disk-topology to provide more degrees of freedom to capture sharp features.
/// Its position can be computed by minimizing the distance to patch vertices along the normal directions
/// </summary>
/// <param name="x">Desired position</param>
/// <param name="pi">Iso-Countour vertex</param>
/// <param name="npi">Iso-Countour normal</param>
/// <returns></returns>
float featureVertexInsertion(vec3 x, vec3* p, vec3* np, int len) {
	float argMin = std::numeric_limits<float>::max();

	for (int i = 0; i < len; i++) {
		float v = pow(dot(np[i], x - p[i]), 2);
		argMin = std::min(v, argMin);
	}

	return argMin;
}

/// <summary>
/// Flip flip vertices around for some reason
/// </summary>
/// <param name="input">Input mesh</param>
void edgeFlip(vector<vec3>& input) {
	vector<vec3> Q;

	// BVHTree T;
	// T.build(input) - I think this is another paper we need to implement

	/**
	
	
	for (edge e : input) {
		if (e.oppVs are feature vertices) {
			Q.push_back(e);
		}
	}

	while (Q.size() != 0) {
		edge e = Q.pop_back();

		if (!e.wasFlipped) {
			if (isIntersectionTree(input, T, e)) {
				flipEdge(input, e);
				T.refit(input);
			}
		}
	}

	*/
}

mesh_builder debug_box(vec3 bottomLeft, vec3 size) {

	mesh_builder bounding_box = mesh_builder();
	bounding_box.push_vertex(mesh_vertex{ bottomLeft, vec3(1,0,0) });
	bounding_box.push_vertex(mesh_vertex{ bottomLeft + vec3(size.x, 0, 0), vec3(1,0,0) });
	bounding_box.push_vertex(mesh_vertex{ bottomLeft + vec3(0, 0, size.z), vec3(1,0,0) });
	bounding_box.push_vertex(mesh_vertex{ bottomLeft + vec3(size.x, 0, size.z), vec3(1,0,0) });

	bounding_box.push_vertex(mesh_vertex{ bottomLeft + vec3(0, size.y, 0), vec3(1,0,0) });
	bounding_box.push_vertex(mesh_vertex{ bottomLeft + vec3(size.x, size.y, 0), vec3(1,0,0) });
	bounding_box.push_vertex(mesh_vertex{ bottomLeft + vec3(0, size.y, size.z), vec3(1,0,0) });
	bounding_box.push_vertex(mesh_vertex{ bottomLeft + vec3(size.x, size.y, size.z), vec3(1,0,0) });

	bounding_box.push_indices(
		{
			0, 1, 2, // front face
			1, 3, 2,
			4, 5, 6, // back face
			5, 7, 6,
			0, 1, 4, // bottom face
			1, 5, 4,
			2, 3, 6, // top face
			3, 7, 6,
			0, 2, 4, // left face
			2, 6, 4,
			1, 3, 5, // right face
			3, 7, 5
		}
	);

	return bounding_box;

}