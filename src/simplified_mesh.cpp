
// glm
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// project
#include "cgra/cgra_geometry.hpp"
#include "skeleton_model.hpp"


using namespace std;
using namespace glm;
using namespace cgra;

/*

Implementing the paper Robust Low-Poly Meshing for General 3D Models by Zhen Chen, Zherong Pan, Kui Wu, Etienne Vouga, Xifeng Gao
https://dl.acm.org/doi/10.1145/3592396

Code Author: Marshall Scott, Adam Goodyear 

*/

void draw(glm::vec2 screenSize, const glm::mat4& view, const glm::mat4& proj) {

	int nF = 5000; // Target number of triangles

	// TODO: Calculate bounding box for mesh

	// TODO: Project bounding box using projection & view matrix

	// TODO: Calculate input parameter np
	/**
	float lp = screenSpacePos.x; // Pixel length of screenspace bounding box
	float l = sqrt(pow(screenSpacePos.x, 2), pow(screenSpacePos.y, 2)); // Diagonal length of screenspace bounding box
	float np = l / lp; // the maximum number of pixels that the high-poly mesh’s diagonal could occupy across all potential rendering view
	*/

	// TODO: Calculate unsigned distance field

	float voxelEdgeLength = d / sqrt(3);

	// TODO: Grid discretization (Mi,d)

	// TODO: Compute f(p) for all grid points p in G

	for (int cube : cubes) {

		// TODO: Lookup template case
		// template = templates[cube];

		for (patch : cube_case) {

			// TODO: Compute the iso-points on cube edges

			// TODO: For the quadratic program

			// Solve for feature vertices

		}

	}

	// TODO: Run edge flip on output

	// TODO: Run feature filter on output

	// TODO: Run remove interior on output

	// TODO: Store mesh

}


float discretization(vec3 p) {
	// q must be an element of the mesh
	vec3 q(); // TODO: Figure out where this comes from

	vec3 a = p - q;

	return sqrt(dot(a, a));
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
vec3 featureVertexInsertion(vec3 x, vec3* p, vec3* np, int len) {
	float argMin = std::numeric_limits<float>::max();

	for (int i = 0; i < len; i++) {
		float v = pow(dot(np[i], x - p[i]), 2);
		argMin = min(v, argMin);
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