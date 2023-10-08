#pragma once

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// project
#include "application.hpp"
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"
#include "simplified_mesh.hpp"

#include "cloud_model.hpp"
#include "PerlinNoise.hpp"


using namespace std;
using namespace cgra;
using namespace glm;

void cloud_model::simulate() {

	// Generate volumeric data
	cloudData = vector(size.x, vector(size.y, vector(size.z, 1.0f)));

	const siv::PerlinNoise perlin{ 0 };

	double fadeOutRange = 15;

	for (int x = 0; x < size.x; x++) {
		for (int y = 0; y < size.y; y++) {
			for (int z = 0; z < size.z; z++) {
				double noise = perlin.octave3D_01((x / noiseScale.x), y / noiseScale.y, z / noiseScale.z, 3);

				// Fade out at top and bottom
				if (y > size.y - fadeOutRange) {
					noise += (y - size.y + fadeOutRange) / fadeOutRange;
				}
				else if (y < fadeOutRange) {
					noise += (fadeOutRange - y) / fadeOutRange;
				}

				// Fade out x axis
				if (x > size.x - fadeOutRange) {
					noise += (x - size.x + fadeOutRange) / fadeOutRange;
				}
				else if (x < fadeOutRange) {
					noise += (fadeOutRange - x) / fadeOutRange;
				}

				// Fade out z axis
				if (z > size.z - fadeOutRange) {
					noise += (z - size.z + fadeOutRange) / fadeOutRange;
				}
				else if (z < fadeOutRange) {
					noise += (fadeOutRange - z) / fadeOutRange;
				}

				// 0 is solid, 1 is not solid
				cloudData[x][y][z] = noise > cloudThreshold ? 0.0 : 1.0;
			}
		}
	}

	mesh.G = cloudData;
	mesh.topRight = size;
	mesh.bottomLeft = vec3(0.0f);
	mesh.voxelEdgeLength = 1.0;
	mesh.build();
}

void cloud_model::draw(const glm::mat4& view, const glm::mat4 proj) {
	mesh.shader = shader;
	mesh.draw(view, proj); // draw
}