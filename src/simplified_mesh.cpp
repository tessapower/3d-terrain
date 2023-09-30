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

float adapt(float v0, float v1) {
	return (0 - v0) / (v1 - v0);
}

vec3 edge_to_boundary_vertex(int edge, vec3 point, float* f_eval) {

	int v0 = EDGES[edge][0];
	int v1 = EDGES[edge][1];

	float f0 = f_eval[v0];
	float f1 = f_eval[v1];
	
	float t0 = 1.0 - adapt(f0, f1);
	float t1 = 1.0 - t0;

	printf("%f %f\n", t0, t1);

	vec3 vert_pos0 = vec3(VERTICES[v0][0], VERTICES[v0][1], VERTICES[v0][2]);
	vec3 vert_pos1 = vec3(VERTICES[v1][0], VERTICES[v1][1], VERTICES[v1][2]);

	return vec3(
		point.x + vert_pos0[0] * t0 + vert_pos1[0] * t1,
		point.y + vert_pos0[1] * t0 + vert_pos1[1] * t1,
		point.z + vert_pos0[2] * t0 + vert_pos1[2] * t1
	);
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
	
	vec2 screenSpacePos(300, 100);

	float lp = screenSpacePos.x; // Pixel length of screenspace bounding box
	float l = sqrt(pow(screenSpacePos.x, 2) + pow(screenSpacePos.y, 2)); // Diagonal length of screenspace bounding box

	float np = (screenSpacePos.x * screenSpacePos.y) / 100; //l / lp; // the maximum number of pixels that the high-poly mesh’s diagonal could occupy across all potential rendering view
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
					G[x][y][z] = std::min(G[x][y][z], glm::distance(t.pos, gridPoint));
				}
			}
		}
	}

	// Visualize unsigned distance field
	if (debugging == 2) {
		mesh_builder debugging;
		//mesh_builder builder;
		for (int x = 0; x < gridSize.x; x++) {
			for (int y = 0; y < gridSize.y; y++) {
				for (int z = 0; z < gridSize.z; z++) {
					vec3 gridPoint = gridToWorldPosition(topRight, bottomLeft, vec3(x, y, z), gridSize);

					float sizeFloat = 0.5 - (G[x][y][z] / 5.0);

					sizeFloat = G[x][y][z] < 0.5f ? 0.5f : 0.f;

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

	mesh_builder output;
	
	for (int x = 0; x < gridSize.x - 1; x++) {
		for (int y = 0; y < gridSize.y - 1; y++) {
			for (int z = 0; z < gridSize.z - 1; z++) {

				float circle_function(float x, float y, float z) {
					return 2.5 - std::sqrt(x * x + y * y + z * z);
				}

				float values[8] = {
					G[x][y][z],
					G[x + 1][y][z],
					G[x + 1][y + 1][z],
					G[x][y + 1][z],
					G[x][y][z + 1],
					G[x + 1][y][z + 1],
					G[x + 1][y + 1][z + 1],
					G[x][y + 1][z + 1],
				};

				float isolevel = 0.5;

				int cubeIndex = 0;
				if (values[0] < isolevel) cubeIndex |= 1;
				if (values[1] < isolevel) cubeIndex |= 2;
				if (values[2] < isolevel) cubeIndex |= 4;
				if (values[3] < isolevel) cubeIndex |= 8;
				if (values[4] < isolevel) cubeIndex |= 16;
				if (values[5] < isolevel) cubeIndex |= 32;
				if (values[6] < isolevel) cubeIndex |= 64;
				if (values[7] < isolevel) cubeIndex |= 128;

				vector<vector<int>> faces = TRI_TABLE[cubeIndex];

				vec3 gridPoint = gridToWorldPosition(topRight, bottomLeft, vec3(x, y, z), gridSize);

				mesh_builder b;

				for (int face = 0; face < faces.size(); face++) {

					vector<int> faceEdges = faces[face];

					int size = b.vertices.size() + 1;

					for (int i = 0; i < faceEdges.size(); i++) {
						b.push_vertex(mesh_vertex {
							edge_to_boundary_vertex(faceEdges[i], gridPoint, values),
							vec3(0, 1, 0)
						});
					}

					b.push_index(size);
					b.push_index(size + 1);
					b.push_index(size + 2);
				}

				output.append(b);

				if (b.vertices.size() > 0) {
					mesh = output.build();
					return;
				}
			}
		}
	}

	// TODO: Run edge flip on output

	// TODO: Run feature filter on output

	// TODO: Run remove interior on output

	mesh = output.build();

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