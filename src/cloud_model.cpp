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

float cloudFalloff(float x) { return 1.f - ((x - 1.f) * (x - 1.f)); }

void cloud_model::simulate() {

	// Generate volumeric data
	cloudData = vector(size.x, vector(size.y, vector(size.z, 1.0f)));

	const siv::PerlinNoise perlin{ 0 };

	for (int x = 0; x < size.x; x++) {
		for (int y = 0; y < size.y; y++) {
			for (int z = 0; z < size.z; z++) {
				double noise = perlin.octave3D_01((x / noiseScale.x), y / noiseScale.y, z / noiseScale.z, 3);

				// Fade out at top and bottom
				if (y > size.y - fadeOutRange) {
					noise *= cloudFalloff(1. - (y - (size.y - fadeOutRange)) / fadeOutRange);
				}
				else if (y < fadeOutRange) {
					noise *= cloudFalloff(y / fadeOutRange);
				}

				// 1 is not solid, 0 is solid
				cloudData[x][y][z] = noise > cloudThreshold ? 0.0 : 1.0;
			}
		}
	}

	std::cout << "Finished building noise map\n";

	mesh.G = cloudData;
	mesh.topRight = size;
	mesh.bottomLeft = vec3(0.0f);
	mesh.voxelEdgeLength = 1.0;
	mesh.build();
}

void cloud_model::draw(const glm::mat4& view, const glm::mat4 proj) {
	mat4 viewmodel = glm::scale(view, vec3(5.f));
	viewmodel = glm::translate(viewmodel, vec3(size.x / -2.f, 10.f, size.z / -2.f));
	mesh.shader = shader;
	mesh.draw(viewmodel, proj); // draw
}