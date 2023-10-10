#pragma once

#define GLM_ENABLE_EXPERIMENTAL 1

// glm
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>

// project
#include "cgra/cgra_geometry.hpp"
#include "simplified_mesh.hpp"
#include <chrono>
#include <unordered_map>
#include <array>
#include <queue>

using namespace std;
using namespace glm;
using namespace cgra;

vec3 grid_to_world_position(vec3 topRight, vec3 bottomLeft, vec3 position, vec3 gridSize);
vec3 world_to_grid_position(vec3 topRight, vec3 bottomLeft, vec3 position, vec3 gridSize);
mesh_builder debug_box(vec3 bottomLeft, vec3 size);

/*

Implementing the paper Robust Low-Poly Meshing for General 3D Models by Zhen Chen, Zherong Pan, Kui Wu, Etienne Vouga, Xifeng Gao
https://dl.acm.org/doi/10.1145/3592396

Code Author: Marshall Scott

*/

// Pre-computed tables taken from https://github.com/BorisTheBrave/mc-dc/blob/a165b326849d8814fb03c963ad33a9faf6cc6dea/marching_cubes_3d.py

vector<vector<vector<int>>> TRI_TABLE = {
	{},
	{{8, 0, 3}},
	{{1, 0, 9}},
	{{8, 1, 3}, {8, 9, 1}},
	{{10, 2, 1}},
	{{8, 0, 3}, {1, 10, 2}},
	{{9, 2, 0}, {9, 10, 2}},
	{{3, 8, 2}, {2, 8, 10}, {10, 8, 9}},
	{{3, 2, 11}},
	{{0, 2, 8}, {2, 11, 8}},
	{{1, 0, 9}, {2, 11, 3}},
	{{2, 9, 1}, {11, 9, 2}, {8, 9, 11}},
	{{3, 10, 11}, {3, 1, 10}},
	{{1, 10, 0}, {0, 10, 8}, {8, 10, 11}},
	{{0, 11, 3}, {9, 11, 0}, {10, 11, 9}},
	{{8, 9, 11}, {11, 9, 10}},
	{{7, 4, 8}},
	{{3, 7, 0}, {7, 4, 0}},
	{{7, 4, 8}, {9, 1, 0}},
	{{9, 1, 4}, {4, 1, 7}, {7, 1, 3}},
	{{7, 4, 8}, {2, 1, 10}},
	{{4, 3, 7}, {4, 0, 3}, {2, 1, 10}},
	{{2, 0, 10}, {0, 9, 10}, {7, 4, 8}},
	{{9, 10, 4}, {4, 10, 3}, {3, 10, 2}, {4, 3, 7}},
	{{4, 8, 7}, {3, 2, 11}},
	{{7, 4, 11}, {11, 4, 2}, {2, 4, 0}},
	{{1, 0, 9}, {2, 11, 3}, {8, 7, 4}},
	{{2, 11, 1}, {1, 11, 9}, {9, 11, 7}, {9, 7, 4}},
	{{10, 11, 1}, {11, 3, 1}, {4, 8, 7}},
	{{4, 0, 7}, {7, 0, 10}, {0, 1, 10}, {7, 10, 11}},
	{{7, 4, 8}, {0, 11, 3}, {9, 11, 0}, {10, 11, 9}},
	{{4, 11, 7}, {9, 11, 4}, {10, 11, 9}},
	{{9, 4, 5}},
	{{9, 4, 5}, {0, 3, 8}},
	{{0, 5, 1}, {0, 4, 5}},
	{{4, 3, 8}, {5, 3, 4}, {1, 3, 5}},
	{{5, 9, 4}, {10, 2, 1}},
	{{8, 0, 3}, {1, 10, 2}, {4, 5, 9}},
	{{10, 4, 5}, {2, 4, 10}, {0, 4, 2}},
	{{3, 10, 2}, {8, 10, 3}, {5, 10, 8}, {4, 5, 8}},
	{{9, 4, 5}, {11, 3, 2}},
	{{11, 0, 2}, {11, 8, 0}, {9, 4, 5}},
	{{5, 1, 4}, {1, 0, 4}, {11, 3, 2}},
	{{5, 1, 4}, {4, 1, 11}, {1, 2, 11}, {4, 11, 8}},
	{{3, 10, 11}, {3, 1, 10}, {5, 9, 4}},
	{{9, 4, 5}, {1, 10, 0}, {0, 10, 8}, {8, 10, 11}},
	{{5, 0, 4}, {11, 0, 5}, {11, 3, 0}, {10, 11, 5}},
	{{5, 10, 4}, {4, 10, 8}, {8, 10, 11}},
	{{9, 7, 5}, {9, 8, 7}},
	{{0, 5, 9}, {3, 5, 0}, {7, 5, 3}},
	{{8, 7, 0}, {0, 7, 1}, {1, 7, 5}},
	{{7, 5, 3}, {3, 5, 1}},
	{{7, 5, 8}, {5, 9, 8}, {2, 1, 10}},
	{{10, 2, 1}, {0, 5, 9}, {3, 5, 0}, {7, 5, 3}},
	{{8, 2, 0}, {5, 2, 8}, {10, 2, 5}, {7, 5, 8}},
	{{2, 3, 10}, {10, 3, 5}, {5, 3, 7}},
	{{9, 7, 5}, {9, 8, 7}, {11, 3, 2}},
	{{0, 2, 9}, {9, 2, 7}, {7, 2, 11}, {9, 7, 5}},
	{{3, 2, 11}, {8, 7, 0}, {0, 7, 1}, {1, 7, 5}},
	{{11, 1, 2}, {7, 1, 11}, {5, 1, 7}},
	{{3, 1, 11}, {11, 1, 10}, {8, 7, 9}, {9, 7, 5}},
	{{11, 7, 0}, {7, 5, 0}, {5, 9, 0}, {10, 11, 0}, {1, 10, 0}},
	{{0, 5, 10}, {0, 7, 5}, {0, 8, 7}, {0, 10, 11}, {0, 11, 3}},
	{{10, 11, 5}, {11, 7, 5}},
	{{5, 6, 10}},
	{{8, 0, 3}, {10, 5, 6}},
	{{0, 9, 1}, {5, 6, 10}},
	{{8, 1, 3}, {8, 9, 1}, {10, 5, 6}},
	{{1, 6, 2}, {1, 5, 6}},
	{{6, 2, 5}, {2, 1, 5}, {8, 0, 3}},
	{{5, 6, 9}, {9, 6, 0}, {0, 6, 2}},
	{{5, 8, 9}, {2, 8, 5}, {3, 8, 2}, {6, 2, 5}},
	{{3, 2, 11}, {10, 5, 6}},
	{{0, 2, 8}, {2, 11, 8}, {5, 6, 10}},
	{{3, 2, 11}, {0, 9, 1}, {10, 5, 6}},
	{{5, 6, 10}, {2, 9, 1}, {11, 9, 2}, {8, 9, 11}},
	{{11, 3, 6}, {6, 3, 5}, {5, 3, 1}},
	{{11, 8, 6}, {6, 8, 1}, {1, 8, 0}, {6, 1, 5}},
	{{5, 0, 9}, {6, 0, 5}, {3, 0, 6}, {11, 3, 6}},
	{{6, 9, 5}, {11, 9, 6}, {8, 9, 11}},
	{{7, 4, 8}, {6, 10, 5}},
	{{3, 7, 0}, {7, 4, 0}, {10, 5, 6}},
	{{7, 4, 8}, {6, 10, 5}, {9, 1, 0}},
	{{5, 6, 10}, {9, 1, 4}, {4, 1, 7}, {7, 1, 3}},
	{{1, 6, 2}, {1, 5, 6}, {7, 4, 8}},
	{{6, 1, 5}, {2, 1, 6}, {0, 7, 4}, {3, 7, 0}},
	{{4, 8, 7}, {5, 6, 9}, {9, 6, 0}, {0, 6, 2}},
	{{2, 3, 9}, {3, 7, 9}, {7, 4, 9}, {6, 2, 9}, {5, 6, 9}},
	{{2, 11, 3}, {7, 4, 8}, {10, 5, 6}},
	{{6, 10, 5}, {7, 4, 11}, {11, 4, 2}, {2, 4, 0}},
	{{1, 0, 9}, {8, 7, 4}, {3, 2, 11}, {5, 6, 10}},
	{{1, 2, 9}, {9, 2, 11}, {9, 11, 4}, {4, 11, 7}, {5, 6, 10}},
	{{7, 4, 8}, {11, 3, 6}, {6, 3, 5}, {5, 3, 1}},
	{{11, 0, 1}, {11, 4, 0}, {11, 7, 4}, {11, 1, 5}, {11, 5, 6}},
	{{6, 9, 5}, {0, 9, 6}, {11, 0, 6}, {3, 0, 11}, {4, 8, 7}},
	{{5, 6, 9}, {9, 6, 11}, {9, 11, 7}, {9, 7, 4}},
	{{4, 10, 9}, {4, 6, 10}},
	{{10, 4, 6}, {10, 9, 4}, {8, 0, 3}},
	{{1, 0, 10}, {10, 0, 6}, {6, 0, 4}},
	{{8, 1, 3}, {6, 1, 8}, {6, 10, 1}, {4, 6, 8}},
	{{9, 2, 1}, {4, 2, 9}, {6, 2, 4}},
	{{3, 8, 0}, {9, 2, 1}, {4, 2, 9}, {6, 2, 4}},
	{{0, 4, 2}, {2, 4, 6}},
	{{8, 2, 3}, {4, 2, 8}, {6, 2, 4}},
	{{4, 10, 9}, {4, 6, 10}, {2, 11, 3}},
	{{11, 8, 2}, {2, 8, 0}, {6, 10, 4}, {4, 10, 9}},
	{{2, 11, 3}, {1, 0, 10}, {10, 0, 6}, {6, 0, 4}},
	{{8, 4, 1}, {4, 6, 1}, {6, 10, 1}, {11, 8, 1}, {2, 11, 1}},
	{{3, 1, 11}, {11, 1, 4}, {1, 9, 4}, {11, 4, 6}},
	{{6, 11, 1}, {11, 8, 1}, {8, 0, 1}, {4, 6, 1}, {9, 4, 1}},
	{{3, 0, 11}, {11, 0, 6}, {6, 0, 4}},
	{{4, 11, 8}, {4, 6, 11}},
	{{6, 8, 7}, {10, 8, 6}, {9, 8, 10}},
	{{3, 7, 0}, {0, 7, 10}, {7, 6, 10}, {0, 10, 9}},
	{{1, 6, 10}, {0, 6, 1}, {7, 6, 0}, {8, 7, 0}},
	{{10, 1, 6}, {6, 1, 7}, {7, 1, 3}},
	{{9, 8, 1}, {1, 8, 6}, {6, 8, 7}, {1, 6, 2}},
	{{9, 7, 6}, {9, 3, 7}, {9, 0, 3}, {9, 6, 2}, {9, 2, 1}},
	{{7, 6, 8}, {8, 6, 0}, {0, 6, 2}},
	{{3, 6, 2}, {3, 7, 6}},
	{{3, 2, 11}, {6, 8, 7}, {10, 8, 6}, {9, 8, 10}},
	{{7, 9, 0}, {7, 10, 9}, {7, 6, 10}, {7, 0, 2}, {7, 2, 11}},
	{{0, 10, 1}, {6, 10, 0}, {8, 6, 0}, {7, 6, 8}, {2, 11, 3}},
	{{1, 6, 10}, {7, 6, 1}, {11, 7, 1}, {2, 11, 1}},
	{{1, 9, 6}, {9, 8, 6}, {8, 7, 6}, {3, 1, 6}, {11, 3, 6}},
	{{9, 0, 1}, {11, 7, 6}},
	{{0, 11, 3}, {6, 11, 0}, {7, 6, 0}, {8, 7, 0}},
	{{7, 6, 11}},
	{{11, 6, 7}},
	{{3, 8, 0}, {11, 6, 7}},
	{{1, 0, 9}, {6, 7, 11}},
	{{1, 3, 9}, {3, 8, 9}, {6, 7, 11}},
	{{10, 2, 1}, {6, 7, 11}},
	{{10, 2, 1}, {3, 8, 0}, {6, 7, 11}},
	{{9, 2, 0}, {9, 10, 2}, {11, 6, 7}},
	{{11, 6, 7}, {3, 8, 2}, {2, 8, 10}, {10, 8, 9}},
	{{2, 6, 3}, {6, 7, 3}},
	{{8, 6, 7}, {0, 6, 8}, {2, 6, 0}},
	{{7, 2, 6}, {7, 3, 2}, {1, 0, 9}},
	{{8, 9, 7}, {7, 9, 2}, {2, 9, 1}, {7, 2, 6}},
	{{6, 1, 10}, {7, 1, 6}, {3, 1, 7}},
	{{8, 0, 7}, {7, 0, 6}, {6, 0, 1}, {6, 1, 10}},
	{{7, 3, 6}, {6, 3, 9}, {3, 0, 9}, {6, 9, 10}},
	{{7, 8, 6}, {6, 8, 10}, {10, 8, 9}},
	{{8, 11, 4}, {11, 6, 4}},
	{{11, 0, 3}, {6, 0, 11}, {4, 0, 6}},
	{{6, 4, 11}, {4, 8, 11}, {1, 0, 9}},
	{{1, 3, 9}, {9, 3, 6}, {3, 11, 6}, {9, 6, 4}},
	{{8, 11, 4}, {11, 6, 4}, {1, 10, 2}},
	{{1, 10, 2}, {11, 0, 3}, {6, 0, 11}, {4, 0, 6}},
	{{2, 9, 10}, {0, 9, 2}, {4, 11, 6}, {8, 11, 4}},
	{{3, 4, 9}, {3, 6, 4}, {3, 11, 6}, {3, 9, 10}, {3, 10, 2}},
	{{3, 2, 8}, {8, 2, 4}, {4, 2, 6}},
	{{2, 4, 0}, {6, 4, 2}},
	{{0, 9, 1}, {3, 2, 8}, {8, 2, 4}, {4, 2, 6}},
	{{1, 2, 9}, {9, 2, 4}, {4, 2, 6}},
	{{10, 3, 1}, {4, 3, 10}, {4, 8, 3}, {6, 4, 10}},
	{{10, 0, 1}, {6, 0, 10}, {4, 0, 6}},
	{{3, 10, 6}, {3, 9, 10}, {3, 0, 9}, {3, 6, 4}, {3, 4, 8}},
	{{9, 10, 4}, {10, 6, 4}},
	{{9, 4, 5}, {7, 11, 6}},
	{{9, 4, 5}, {7, 11, 6}, {0, 3, 8}},
	{{0, 5, 1}, {0, 4, 5}, {6, 7, 11}},
	{{11, 6, 7}, {4, 3, 8}, {5, 3, 4}, {1, 3, 5}},
	{{1, 10, 2}, {9, 4, 5}, {6, 7, 11}},
	{{8, 0, 3}, {4, 5, 9}, {10, 2, 1}, {11, 6, 7}},
	{{7, 11, 6}, {10, 4, 5}, {2, 4, 10}, {0, 4, 2}},
	{{8, 2, 3}, {10, 2, 8}, {4, 10, 8}, {5, 10, 4}, {11, 6, 7}},
	{{2, 6, 3}, {6, 7, 3}, {9, 4, 5}},
	{{5, 9, 4}, {8, 6, 7}, {0, 6, 8}, {2, 6, 0}},
	{{7, 3, 6}, {6, 3, 2}, {4, 5, 0}, {0, 5, 1}},
	{{8, 1, 2}, {8, 5, 1}, {8, 4, 5}, {8, 2, 6}, {8, 6, 7}},
	{{9, 4, 5}, {6, 1, 10}, {7, 1, 6}, {3, 1, 7}},
	{{7, 8, 6}, {6, 8, 0}, {6, 0, 10}, {10, 0, 1}, {5, 9, 4}},
	{{3, 0, 10}, {0, 4, 10}, {4, 5, 10}, {7, 3, 10}, {6, 7, 10}},
	{{8, 6, 7}, {10, 6, 8}, {5, 10, 8}, {4, 5, 8}},
	{{5, 9, 6}, {6, 9, 11}, {11, 9, 8}},
	{{11, 6, 3}, {3, 6, 0}, {0, 6, 5}, {0, 5, 9}},
	{{8, 11, 0}, {0, 11, 5}, {5, 11, 6}, {0, 5, 1}},
	{{6, 3, 11}, {5, 3, 6}, {1, 3, 5}},
	{{10, 2, 1}, {5, 9, 6}, {6, 9, 11}, {11, 9, 8}},
	{{3, 11, 0}, {0, 11, 6}, {0, 6, 9}, {9, 6, 5}, {1, 10, 2}},
	{{0, 8, 5}, {8, 11, 5}, {11, 6, 5}, {2, 0, 5}, {10, 2, 5}},
	{{11, 6, 3}, {3, 6, 5}, {3, 5, 10}, {3, 10, 2}},
	{{3, 9, 8}, {6, 9, 3}, {5, 9, 6}, {2, 6, 3}},
	{{9, 6, 5}, {0, 6, 9}, {2, 6, 0}},
	{{6, 5, 8}, {5, 1, 8}, {1, 0, 8}, {2, 6, 8}, {3, 2, 8}},
	{{2, 6, 1}, {6, 5, 1}},
	{{6, 8, 3}, {6, 9, 8}, {6, 5, 9}, {6, 3, 1}, {6, 1, 10}},
	{{1, 10, 0}, {0, 10, 6}, {0, 6, 5}, {0, 5, 9}},
	{{3, 0, 8}, {6, 5, 10}},
	{{10, 6, 5}},
	{{5, 11, 10}, {5, 7, 11}},
	{{5, 11, 10}, {5, 7, 11}, {3, 8, 0}},
	{{11, 10, 7}, {10, 5, 7}, {0, 9, 1}},
	{{5, 7, 10}, {10, 7, 11}, {9, 1, 8}, {8, 1, 3}},
	{{2, 1, 11}, {11, 1, 7}, {7, 1, 5}},
	{{3, 8, 0}, {2, 1, 11}, {11, 1, 7}, {7, 1, 5}},
	{{2, 0, 11}, {11, 0, 5}, {5, 0, 9}, {11, 5, 7}},
	{{2, 9, 5}, {2, 8, 9}, {2, 3, 8}, {2, 5, 7}, {2, 7, 11}},
	{{10, 3, 2}, {5, 3, 10}, {7, 3, 5}},
	{{10, 0, 2}, {7, 0, 10}, {8, 0, 7}, {5, 7, 10}},
	{{0, 9, 1}, {10, 3, 2}, {5, 3, 10}, {7, 3, 5}},
	{{7, 8, 2}, {8, 9, 2}, {9, 1, 2}, {5, 7, 2}, {10, 5, 2}},
	{{3, 1, 7}, {7, 1, 5}},
	{{0, 7, 8}, {1, 7, 0}, {5, 7, 1}},
	{{9, 5, 0}, {0, 5, 3}, {3, 5, 7}},
	{{5, 7, 9}, {7, 8, 9}},
	{{4, 10, 5}, {8, 10, 4}, {11, 10, 8}},
	{{3, 4, 0}, {10, 4, 3}, {10, 5, 4}, {11, 10, 3}},
	{{1, 0, 9}, {4, 10, 5}, {8, 10, 4}, {11, 10, 8}},
	{{4, 3, 11}, {4, 1, 3}, {4, 9, 1}, {4, 11, 10}, {4, 10, 5}},
	{{1, 5, 2}, {2, 5, 8}, {5, 4, 8}, {2, 8, 11}},
	{{5, 4, 11}, {4, 0, 11}, {0, 3, 11}, {1, 5, 11}, {2, 1, 11}},
	{{5, 11, 2}, {5, 8, 11}, {5, 4, 8}, {5, 2, 0}, {5, 0, 9}},
	{{5, 4, 9}, {2, 3, 11}},
	{{3, 4, 8}, {2, 4, 3}, {5, 4, 2}, {10, 5, 2}},
	{{5, 4, 10}, {10, 4, 2}, {2, 4, 0}},
	{{2, 8, 3}, {4, 8, 2}, {10, 4, 2}, {5, 4, 10}, {0, 9, 1}},
	{{4, 10, 5}, {2, 10, 4}, {1, 2, 4}, {9, 1, 4}},
	{{8, 3, 4}, {4, 3, 5}, {5, 3, 1}},
	{{1, 5, 0}, {5, 4, 0}},
	{{5, 0, 9}, {3, 0, 5}, {8, 3, 5}, {4, 8, 5}},
	{{5, 4, 9}},
	{{7, 11, 4}, {4, 11, 9}, {9, 11, 10}},
	{{8, 0, 3}, {7, 11, 4}, {4, 11, 9}, {9, 11, 10}},
	{{0, 4, 1}, {1, 4, 11}, {4, 7, 11}, {1, 11, 10}},
	{{10, 1, 4}, {1, 3, 4}, {3, 8, 4}, {11, 10, 4}, {7, 11, 4}},
	{{9, 4, 1}, {1, 4, 2}, {2, 4, 7}, {2, 7, 11}},
	{{1, 9, 2}, {2, 9, 4}, {2, 4, 11}, {11, 4, 7}, {3, 8, 0}},
	{{11, 4, 7}, {2, 4, 11}, {0, 4, 2}},
	{{7, 11, 4}, {4, 11, 2}, {4, 2, 3}, {4, 3, 8}},
	{{10, 9, 2}, {2, 9, 7}, {7, 9, 4}, {2, 7, 3}},
	{{2, 10, 7}, {10, 9, 7}, {9, 4, 7}, {0, 2, 7}, {8, 0, 7}},
	{{10, 4, 7}, {10, 0, 4}, {10, 1, 0}, {10, 7, 3}, {10, 3, 2}},
	{{8, 4, 7}, {10, 1, 2}},
	{{4, 1, 9}, {7, 1, 4}, {3, 1, 7}},
	{{8, 0, 7}, {7, 0, 1}, {7, 1, 9}, {7, 9, 4}},
	{{0, 7, 3}, {0, 4, 7}},
	{{8, 4, 7}},
	{{9, 8, 10}, {10, 8, 11}},
	{{3, 11, 0}, {0, 11, 9}, {9, 11, 10}},
	{{0, 10, 1}, {8, 10, 0}, {11, 10, 8}},
	{{11, 10, 3}, {10, 1, 3}},
	{{1, 9, 2}, {2, 9, 11}, {11, 9, 8}},
	{{9, 2, 1}, {11, 2, 9}, {3, 11, 9}, {0, 3, 9}},
	{{8, 2, 0}, {8, 11, 2}},
	{{11, 2, 3}},
	{{2, 8, 3}, {10, 8, 2}, {9, 8, 10}},
	{{0, 2, 9}, {2, 10, 9}},
	{{3, 2, 8}, {8, 2, 10}, {8, 10, 1}, {8, 1, 0}},
	{{1, 2, 10}},
	{{3, 1, 8}, {1, 9, 8}},
	{{9, 0, 1}},
	{{3, 0, 8}},
	{}
};

vector<vector<int>> EDGES = {
	{ 0, 1 },
	{ 1, 2 },
	{ 2, 3 },
	{ 3, 0 },
	{ 4, 5 },
	{ 5, 6 },
	{ 6, 7 },
	{ 7, 4 },
	{ 0, 4 },
	{ 1, 5 },
	{ 2, 6 },
	{ 3, 7 },
};

vector<vector<int>> VERTICES = {
	{ 0, 0, 0 },
	{ 1, 0, 0 },
	{ 1, 1, 0 },
	{ 0, 1, 0 },
	{ 0, 0, 1 },
	{ 1, 0, 1 },
	{ 1, 1, 1 },
	{ 0, 1, 1 },
};

void simplified_mesh::draw(const glm::mat4& view, const glm::mat4& proj) {

	glUseProgram(shader); // load shader and variables
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(view));
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(vec3(1,1,1)));

	mesh.draw(); // draw
}

inline float adapt(float v0, float v1) {
	return 0.5;
}

vec3 edge_to_boundary_vertex(int edge, vec3 point, const float* f_eval, float voxelEdgeLength) {

	const int v0 = EDGES[edge][0];
	const int v1 = EDGES[edge][1];
	
	const float t0 = 1.0 - adapt(f_eval[v0], f_eval[v1]);
	const float t1 = 1.0 - t0;

	vec3 vert_pos0 = vec3(VERTICES[v0][0], VERTICES[v0][1], VERTICES[v0][2]);
	vec3 vert_pos1 = vec3(VERTICES[v1][0], VERTICES[v1][1], VERTICES[v1][2]);

	return vec3(
		point.x + (vert_pos0[0] * t0 + vert_pos1[0] * t1) * voxelEdgeLength,
		point.y + (vert_pos0[1] * t0 + vert_pos1[1] * t1) * voxelEdgeLength,
		point.z + (vert_pos0[2] * t0 + vert_pos1[2] * t1) * voxelEdgeLength
	);
}

void simplified_mesh::set_model(mesh_builder builder) {
	this->builder = builder;

	// Calculate bounding box
	topRight = builder.vertices[0].pos;
	bottomLeft = builder.vertices[0].pos;

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

	// Expand it slightly to allow for edges to be built
	bottomLeft = vec3(bottomLeft.x - voxelEdgeLength * 6.0f, bottomLeft.y - voxelEdgeLength * 6.0f, bottomLeft.z - voxelEdgeLength * 6.0f);
	topRight = vec3(topRight.x + voxelEdgeLength * 6.0f, topRight.y + voxelEdgeLength * 6.0f, topRight.z + voxelEdgeLength * 6.0f);
}

/// <summary>
/// Builds the unsigned distance field from the model, then runs build
/// </summary>
void simplified_mesh::build_from_model() {

	cout << "Input mesh with V: " << builder.vertices.size() << " I: " << builder.indices.size() << "\n";

	if (debugging == 0) {
		mesh = builder.build();
		return;
	}

	const vec3 gridSize = vec3(
		std::ceil((topRight.x - bottomLeft.x) / voxelEdgeLength),
		std::ceil((topRight.y - bottomLeft.y) / voxelEdgeLength),
		std::ceil((topRight.z - bottomLeft.z) / voxelEdgeLength)
	);

	printf("Size: %f %f %f\n", gridSize.x, gridSize.y, gridSize.z);

	const auto start = std::chrono::high_resolution_clock::now();

	G = vector(gridSize.x, vector(gridSize.y, vector(gridSize.z, 9999999999.9f)));

	// Grid discretization (Mi,d)

	// Calculate unsigned distance field

	vector<vector<vector<vec3>>> cachedWorldPositions = vector(gridSize.x, vector(gridSize.y, vector(gridSize.z, vec3(0))));

	// After
	for (mesh_vertex vertex : builder.vertices) {
		vec3 pos = world_to_grid_position(topRight, bottomLeft, vertex.pos, gridSize);

		// Get surrounding 8 vertices
		vec3 vertices[] = {
			vec3(floorf(pos.x), floorf(pos.y), floorf(pos.z)),
			vec3(ceilf(pos.x), floorf(pos.y), floorf(pos.z)),
			vec3(floorf(pos.x), floorf(pos.y), ceilf(pos.z)),
			vec3(ceilf(pos.x), floorf(pos.y), ceilf(pos.z)),
			vec3(floorf(pos.x), ceilf(pos.y), floorf(pos.z)),
			vec3(ceilf(pos.x), ceilf(pos.y), floorf(pos.z)),
			vec3(floorf(pos.x), ceilf(pos.y), ceilf(pos.z)),
			vec3(ceilf(pos.x), ceilf(pos.y), ceilf(pos.z)),
		};

		for (int i = 0; i < 8; i++) {
			vec3* gridPoint = &vertices[i];

			if (cachedWorldPositions[gridPoint->x][gridPoint->y][gridPoint->z] == vec3(0)) {
				cachedWorldPositions[gridPoint->x][gridPoint->y][gridPoint->z] = grid_to_world_position(topRight, bottomLeft, *gridPoint, gridSize);
			}

			G[gridPoint->x][gridPoint->y][gridPoint->z] =
				std::min(
					G[gridPoint->x][gridPoint->y][gridPoint->z],
					glm::distance(cachedWorldPositions[gridPoint->x][gridPoint->y][gridPoint->z], vertex.pos)
				);
		}
	}

	// Get the duration in microseconds
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start);

	// Print the duration
	cout << "Time taken to generate G: "
		<< duration.count() << " microseconds" << endl;

	// Visualize unsigned distance field
	if (debugging == 2) {
		mesh_builder debugging;

		// Loop over and create squares the size of the value
		for (int x = 0; x < gridSize.x; x++) {
			for (int y = 0; y < gridSize.y; y++) {
				for (int z = 0; z < gridSize.z; z++) {
					const vec3 gridPoint = grid_to_world_position(topRight, bottomLeft, vec3(x, y, z), gridSize);

					const float sizeFloat = G[x][y][z] < isolevel ? voxelEdgeLength / 2.f : 0.f;

					const vec3 size(sizeFloat);

					debugging.append(debug_box(gridPoint - (size / 2.0f), size));
				}
			}
		}

		debugging.append(builder);
		mesh = debugging.build();
		return;
	}

	build();

	// Print the duration
	cout << "Time taken to finish whole process G: "
		<< duration.count() << " microseconds" << endl;
}

/// <summary>
/// Builds the mesh from the unsigned distance field
/// </summary>
void simplified_mesh::build() {

	const vec3 gridSize = vec3(
		std::ceil((topRight.x - bottomLeft.x) / voxelEdgeLength),
		std::ceil((topRight.y - bottomLeft.y) / voxelEdgeLength),
		std::ceil((topRight.z - bottomLeft.z) / voxelEdgeLength)
	);

	// Bounding cube debugging
	if (debugging == 1) {
		mesh_builder t = debug_box(bottomLeft, topRight - bottomLeft);
		t.append(builder);
		mesh = t.build();
		return;
	}

	mesh_builder output;

	// Stores positions with their vertices indices
	std::unordered_map<vec3, int> positions;
	
	// Convert into mesh
	for (int x = 0; x < gridSize.x - 1; x++) {
		for (int y = 0; y < gridSize.y - 1; y++) {
			for (int z = 0; z < gridSize.z - 1; z++) {

				const float values[8] = {
					G[x][y][z],
					G[x + 1][y][z],
					G[x + 1][y + 1][z],
					G[x][y + 1][z],
					G[x][y][z + 1],
					G[x + 1][y][z + 1],
					G[x + 1][y + 1][z + 1],
					G[x][y + 1][z + 1],
				};

				// Calculate cube index
				int cubeIndex = 0;
				if (values[0] < isolevel) cubeIndex |= 1;
				if (values[1] < isolevel) cubeIndex |= 2;
				if (values[2] < isolevel) cubeIndex |= 4;
				if (values[3] < isolevel) cubeIndex |= 8;
				if (values[4] < isolevel) cubeIndex |= 16;
				if (values[5] < isolevel) cubeIndex |= 32;
				if (values[6] < isolevel) cubeIndex |= 64;
				if (values[7] < isolevel) cubeIndex |= 128;

				vector<vector<int>>* faces = &TRI_TABLE[cubeIndex];

				const vec3 gridPoint = grid_to_world_position(topRight, bottomLeft, vec3(x, y, z), gridSize);

				for (int face = 0; face < faces->size(); face++) {

					vector<int>* faceEdges = &(*faces)[face];

					int size = output.vertices.size();

					// Calculate vertices
					const vec3 vertices[3] = {
						edge_to_boundary_vertex((*faceEdges)[0], gridPoint, values, voxelEdgeLength),
						edge_to_boundary_vertex((*faceEdges)[1], gridPoint, values, voxelEdgeLength),
						edge_to_boundary_vertex((*faceEdges)[2], gridPoint, values, voxelEdgeLength)
					};

					const vec3 v1 = vec3(vertices[1].x - vertices[0].x, vertices[1].y - vertices[0].y, vertices[1].z - vertices[0].z);
					const vec3 v2 = vec3(vertices[2].x - vertices[0].x, vertices[2].y - vertices[0].y, vertices[2].z - vertices[0].z);

					// Set normals of face
					const vec3 surfNormal = normalize(cross(v1, v2));

					for (int vertex = 0; vertex < 3; vertex++) {
						if (!smoothNormals || positions.find(vertices[vertex]) == positions.end()) {
							// New vertex!

							// Push data to builder
							// Use first field of uv as total normals stored in vertex to keep a running total for smoothing
							output.push_vertex(mesh_vertex{ vertices[vertex], surfNormal, vec2(1, 0)});
							output.push_index(size);

							// Store vertex
							positions[vertices[vertex]] = size;

							size++;
						}
						else {
							// Existing vertex, combine
							int index = positions[vertices[vertex]];
							output.push_index(index);

							// Add normal data
							output.vertices[index].norm += surfNormal;
							// Use first field of uv as total normals stored in vertex to keep a running total for smoothing
							output.vertices[index].uv.x++;
						}
					}
				}
			}
		}
	}

	// Calculate final running totals for normal map
	for (mesh_vertex vertex : output.vertices) {
		vertex.norm = vertex.norm / vertex.uv.x;
		vertex.uv.x = 0.f;
	}

	cout << "Mesh generated with V: " << output.vertices.size() << " I: " << output.indices.size() << "\n";

	// Buffers cannot be empty, so create perceptually empty mesh
	if (output.vertices.size() > 0) {
		mesh = output.build();
	}
	else {
		// Build an invisible mesh
		mesh = debug_box(vec3(0), vec3(0)).build();
	}
}

vec3 world_to_grid_position(vec3 topRight, vec3 bottomLeft, vec3 position, vec3 gridSize) {
	const vec3 size = topRight - bottomLeft;
	return ((position - bottomLeft) / size) * gridSize;
}

vec3 grid_to_world_position(vec3 topRight, vec3 bottomLeft, vec3 position, vec3 gridSize) {

	if (position.x > gridSize.x || position.x < 0 || position.y > gridSize.y || position.y < 0 || position.z > gridSize.z || position.z < 0) {
		throw std::invalid_argument("Position outside of gridsize");
	}

	const vec3 size = topRight - bottomLeft;

	return bottomLeft + vec3(position.x * (size.x / gridSize.x), position.y * (size.y / gridSize.y), position.z * (size.z / gridSize.z));
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