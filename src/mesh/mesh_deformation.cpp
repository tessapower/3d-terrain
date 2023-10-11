#pragma once

#include "mesh_deformation.hpp"

#include "utils/intersections.hpp"

void mesh_deformation::setModel(terrain_model& m) {
	model = m;
}

void mesh_deformation::deformMesh(const cgra::mesh_vertex& center, bool isBump, double deformationRadius, double maxDeformationStrength) {
	for (cgra::mesh_vertex& vertex : model.builder.vertices) {
		// Calculate the distance between the vertex and the center point.
		double distance = std::sqrt(
			(vertex.pos.x - center.pos.x) * (vertex.pos.x - center.pos.x) +
			//(vertex.pos.y - center.pos.y) * (vertex.pos.y - center.pos.y) +
			(vertex.pos.z - center.pos.z) * (vertex.pos.z - center.pos.z)
		);

		// Calculate a normalized strength based on distance (closer points get stronger deformations)
		double normalizedStrength = std::exp(-distance * distance / (deformationRadius * deformationRadius * 0.33));

		// Ensure that the strength is in the range [0, 1]
		normalizedStrength = std::max(0.0, std::min(1.0, normalizedStrength));

		// Scale the deformation strength based on the normalized strength
		double deformationStrength = maxDeformationStrength * normalizedStrength;

		// Apply the deformation based on deformation type (bump or ditch)
		double displacement = isBump ? deformationStrength : -deformationStrength;

		// Update the vertex position.
		vertex.pos.y += displacement;

		// Clear the vertex normal for the next computation
		vertex.norm = { 0.0, 0.0, 0.0 };
	}

	// Recompute vertex normals
	computeVertexNormals();

	// Recompute TBN
	computeTBN();

	model.mesh = model.builder.build();
}

void mesh_deformation::computeTBN() {
	// Recompute TBN
	for (int i = 0; i < model.gridSize; ++i) {
		for (int j = 0; j < model.gridSize; ++j) {
			int k1 = i * (model.gridSize + 1) + j;
			int k2 = k1 + 1;
			int k3 = (i + 1) * (model.gridSize + 1) + j;
			int k4 = k3 + 1;

			calculateTBN(model.builder, true, k1, k2, k3, k4);
			calculateTBN(model.builder, false, k1, k2, k3, k4);
		}
	}
}

void mesh_deformation::calculateTBN(cgra::mesh_builder& mb, bool topLeft, int k1, int k2, int k3, int k4) {
	// triangles positions
	glm::vec3 pos1 = mb.vertices.at(k1).pos;
	glm::vec3 pos2 = mb.vertices.at(k3).pos;
	glm::vec3 pos3 = mb.vertices.at(k2).pos;
	glm::vec3 pos4 = mb.vertices.at(k4).pos;

	// uv positions
	glm::vec2 uv1 = mb.vertices.at(k1).uv;
	glm::vec2 uv2 = mb.vertices.at(k3).uv;
	glm::vec2 uv3 = mb.vertices.at(k2).uv;
	glm::vec2 uv4 = mb.vertices.at(k4).uv;

	glm::vec3 edge1, edge2;
	glm::vec2 deltaUV1, deltaUV2;
	if (topLeft) {
		// calculate tangent and bitangent for top left triangle
		edge1 = pos2 - pos1;
		edge2 = pos3 - pos1;

		// delta UVs
		deltaUV1 = uv2 - uv1;
		deltaUV2 = uv3 - uv1;
	}
	else {
		// calculate tangent and bitangent for bot right triangle
		edge1 = pos3 - pos4;
		edge2 = pos2 - pos4;

		// delta UVs
		deltaUV1 = uv3 - uv4;
		deltaUV2 = uv2 - uv4;
	}

	glm::vec3 tangent, bitangent;
	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

	bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

	// normalize tangent and bitangent
	tangent = normalize(tangent);
	bitangent = normalize(bitangent);

	if (topLeft) {
		// store tangent and bitangent in vertex attributes for top left triangle
		mb.vertices.at(k1).tang = tangent;
		mb.vertices.at(k2).tang = tangent;
		mb.vertices.at(k3).tang = tangent;

		mb.vertices.at(k1).bitang = bitangent;
		mb.vertices.at(k2).bitang = bitangent;
		mb.vertices.at(k3).bitang = bitangent;
	}
	else {
		// store tangent and bitangent in vertex attributes for bot right triangle
		mb.vertices.at(k2).tang = tangent;
		mb.vertices.at(k4).tang = tangent;
		mb.vertices.at(k3).tang = tangent;

		mb.vertices.at(k2).bitang = bitangent;
		mb.vertices.at(k4).bitang = bitangent;
		mb.vertices.at(k3).bitang = bitangent;
	}
}

terrain_model mesh_deformation::getModel() {
	return model;
}

glm::vec3 mesh_deformation::calculateFaceNormal(const glm::vec3& vertex1, const glm::vec3& vertex2, const glm::vec3& vertex3) {
	// Calculate two edge vectors of the triangle
	glm::vec3 edge1 = vertex2 - vertex1;
	glm::vec3 edge2 = vertex3 - vertex1;

	// Calculate the cross product of the two edge vectors to get the face normal
	glm::vec3 faceNormal = cross(edge1, edge2);

	// Normalize the face normal to ensure it has a unit length
	faceNormal = normalize(faceNormal);

	return faceNormal;
}

void mesh_deformation::computeVertexNormals() {
	for (int vertexIndex = 0; vertexIndex < model.builder.vertices.size(); ++vertexIndex) {
		if (model.adjacent_faces[vertexIndex].size() == 0) continue;

		glm::vec3 newNormal = { 0.0f, 0.0f, 0.0f };

		// Iterate through adjacent faces of the current vertex
		for (int i = 0; i < model.adjacent_faces[vertexIndex].size(); i += 3) {
			// Get the vertices of the adjacent face (assuming that each face stores its vertex indices)
			const glm::vec3& vertex1 = model.builder.vertices[model.adjacent_faces[vertexIndex].at(i)].pos;
			const glm::vec3& vertex2 = model.builder.vertices[model.adjacent_faces[vertexIndex].at(i + 1)].pos;
			const glm::vec3& vertex3 = model.builder.vertices[model.adjacent_faces[vertexIndex].at(i + 2)].pos;

			// Calculate the face normal using the vertices
			glm::vec3 faceNormal = calculateFaceNormal(vertex1, vertex2, vertex3);

			// Accumulate the face normal to compute the new vertex normal
			newNormal += faceNormal;
		}

		// Normalize the computed vertex normal
		newNormal = normalize(newNormal);

		// Update the vertex normal
		model.builder.vertices[vertexIndex].norm = newNormal;
	}
}

void mesh_deformation::mouseIntersectMesh(double xpos, double ypos, double windowsize_x, double windowsize_y) {
	// Convert screen coordinates to normalized device coordinates (NDC)
	float ndcX = (2.0f * xpos) / windowsize_x - 1.0f;
	float ndcY = 1.0f - (2.0f * ypos) / windowsize_y;

	// Assume ndcX and ndcY are the NDC coordinates you want to convert to world coordinates
	glm::vec4 nearPoint(ndcX, ndcY, -1.0f, 1.0f); // Near point in NDC space (z = -1)
	glm::vec4 farPoint(ndcX, ndcY, 1.0f, 1.0f);   // Far point in NDC space (z = 1)

	// Inverse transformations to get ray origin and direction in world space
	glm::mat4 invProjection = glm::inverse(m_proj);
	glm::mat4 invView = glm::inverse(m_view);

	// Unproject NDC coordinates to view coordinates
	glm::vec4 nearPointInView = invProjection * nearPoint;
	glm::vec4 farPointInView = invProjection * farPoint;

	// Divide by w to get homogeneous coordinates
	nearPointInView /= nearPointInView.w;
	farPointInView /= farPointInView.w;

	// Unproject view coordinates to world coordinates
	glm::vec3 rayOriginWorld = glm::vec3(invView * nearPointInView);
	glm::vec3 rayEndWorld = glm::vec3(invView * farPointInView);

	// Calculate ray direction
	glm::vec3 rayDirection = glm::normalize(rayEndWorld - rayOriginWorld);

	// Iterate through your mesh vertices and test for intersection
	for (int i = 0; i < model.builder.vertices.size(); ++i) {
		cgra::mesh_vertex vertex = model.builder.vertices[i];

		// Check if the ray intersects with the vertex
		if (ray_intersects_vertex(rayOriginWorld, rayDirection, i, model)) {
			// You've clicked on this vertex, store its index
			model.selectedPoint = model.builder.vertices.at(i);
			break; // Exit the loop since you've found the first intersection
		}
	}
}
