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
#include "mesh/simplified_mesh.hpp"

using namespace std;
using namespace glm;
using namespace cgra;

class cloud_model {
public:
	GLuint shader = 0;
	glm::vec3 color{0.7};
	glm::mat4 modelTransform{1.0};
	GLuint texture;

	vec3 noiseScale{ 80.f, 60.f, 80.f };
	float cloudThreshold = 0.66;
	double voxelEdgeLength = 1.0;
	vec3 size{ 300.f, 30.f, 300.f };
	float fadeOutRange = 6.;

	vector<vector<vector<float>>> cloudData;

	simplified_mesh mesh;

	void simulate();
	void draw(const glm::mat4& view, const glm::mat4 proj);
};