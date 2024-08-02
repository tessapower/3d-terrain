#define GLM_ENABLE_EXPERIMENTAL 1

#include "mesh/simplified_mesh.hpp"

#include <chrono>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>
#include <unordered_map>

#include "cgra/cgra_geometry.hpp"

auto grid_to_world_position(glm::vec3 top_right, glm::vec3 bottom_left,
                            glm::vec3 position, glm::vec3 grid_size)
    -> glm::vec3;

auto world_to_grid_position(glm::vec3 top_right, glm::vec3 bottom_left,
                            glm::vec3 position, glm::vec3 grid_size)
    -> glm::vec3;

auto debug_box(glm::vec3 bottom_left, glm::vec3 size) -> cgra::mesh_builder;

/*
 * Implementing the paper Robust Low-Poly Meshing for General 3D Models by Zhen
 * Chen, Zherong Pan, Kui Wu, Etienne Vouga, Xifeng Gao
 * https://dl.acm.org/doi/10.1145/3592396
 *
 * Code Author: Marshall Scott
 */

// Pre-computed tables taken from
// https://github.com/BorisTheBrave/mc-dc/blob/a165b326849d8814fb03c963ad33a9faf6cc6dea/marching_cubes_3d.py

std::vector<std::vector<std::vector<int>>> tri_table = {
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
    {}};

std::vector<std::vector<int>> cube_edges = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6},
    {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7},
};

std::vector<std::vector<int>> cube_vertices = {
    {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},
    {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1},
};

auto simplified_mesh::draw(const glm::mat4& view, const glm::mat4& proj) const
    -> void {
  glUseProgram(m_shader);  // load shader and variables
  glUniformMatrix4fv(glGetUniformLocation(m_shader, "uProjectionMatrix"), 1,
                     false, value_ptr(proj));
  glUniformMatrix4fv(glGetUniformLocation(m_shader, "uModelViewMatrix"), 1,
                     false, value_ptr(view));
  glUniform3fv(glGetUniformLocation(m_shader, "uColor"), 1,
               glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));

  m_mesh.draw();
}

auto edge_to_boundary_vertex(const int edge, const glm::vec3 point,
                             const float* f_eval, const float voxel_edge_length)
    -> glm::vec3 {
  const int v0 = cube_edges[edge][0];
  const int v1 = cube_edges[edge][1];

  constexpr float t0 = 1.0f - 0.5f;
  constexpr float t1 = 1.0f - t0;

  auto v_pos0 = glm::vec3(cube_vertices[v0][0], cube_vertices[v0][1],
                          cube_vertices[v0][2]);
  auto v_pos1 = glm::vec3(cube_vertices[v1][0], cube_vertices[v1][1],
                          cube_vertices[v1][2]);

  return {point.x + (v_pos0[0] * t0 + v_pos1[0] * t1) * voxel_edge_length,
          point.y + (v_pos0[1] * t0 + v_pos1[1] * t1) * voxel_edge_length,
          point.z + (v_pos0[2] * t0 + v_pos1[2] * t1) * voxel_edge_length};
}

auto simplified_mesh::set_model(const cgra::mesh_builder& builder) -> void {
  this->m_builder = builder;

  // Calculate bounding box
  m_bb_top_right = builder.m_vertices[0].pos;
  m_bb_bottom_left = builder.m_vertices[0].pos;

  for (const cgra::mesh_vertex vertex : builder.m_vertices) {
    if (vertex.pos.y > m_bb_top_right.y) {
      m_bb_top_right.y = vertex.pos.y;
    }
    if (vertex.pos.y < m_bb_bottom_left.y) {
      m_bb_bottom_left.y = vertex.pos.y;
    }
    if (vertex.pos.x > m_bb_top_right.x) {
      m_bb_top_right.x = vertex.pos.x;
    }
    if (vertex.pos.x < m_bb_bottom_left.x) {
      m_bb_bottom_left.x = vertex.pos.x;
    }
    if (vertex.pos.z > m_bb_top_right.z) {
      m_bb_top_right.z = vertex.pos.z;
    }
    if (vertex.pos.z < m_bb_bottom_left.z) {
      m_bb_bottom_left.z = vertex.pos.z;
    }
  }

  // Expand it slightly to allow for edges to be built
  m_bb_bottom_left = glm::vec3(m_bb_bottom_left.x - m_voxel_edge_length * 6.0f,
                               m_bb_bottom_left.y - m_voxel_edge_length * 6.0f,
                               m_bb_bottom_left.z - m_voxel_edge_length * 6.0f);

  m_bb_top_right = glm::vec3(m_bb_top_right.x + m_voxel_edge_length * 6.0f,
                             m_bb_top_right.y + m_voxel_edge_length * 6.0f,
                             m_bb_top_right.z + m_voxel_edge_length * 6.0f);
}

/// <summary>
/// Builds the unsigned distance field from the model, then runs build
/// </summary>
auto simplified_mesh::build_from_model() -> void {
  std::cout << "Input mesh with V: " << m_builder.m_vertices.size()
            << " I: " << m_builder.m_indices.size() << "\n";

  if (m_debugging == 0) {
    m_mesh = m_builder.build();
    return;
  }

  const auto grid_size = glm::vec3(
      std::ceil((m_bb_top_right.x - m_bb_bottom_left.x) / m_voxel_edge_length),
      std::ceil((m_bb_top_right.y - m_bb_bottom_left.y) / m_voxel_edge_length),
      std::ceil((m_bb_top_right.z - m_bb_bottom_left.z) / m_voxel_edge_length));

  printf("Size: %f %f %f\n", grid_size.x, grid_size.y, grid_size.z);

  const auto start = std::chrono::high_resolution_clock::now();

  m_grid = std::vector(
      grid_size.x,
      std::vector(grid_size.y, std::vector(grid_size.z, 9999999999.9f)));

  // Grid discretization (Mi,d)
  // Calculate unsigned distance field

  std::vector<std::vector<std::vector<glm::vec3>>> cached_world_positions =
      std::vector(
          grid_size.x,
          std::vector(grid_size.y, std::vector(grid_size.z, glm::vec3(0))));

  // After
  for (cgra::mesh_vertex vertex : m_builder.m_vertices) {
    glm::vec3 pos = world_to_grid_position(m_bb_top_right, m_bb_bottom_left,
                                           vertex.pos, grid_size);

    // Get surrounding 8 vertices
    for (glm::vec3 vertices[] =
             {
                 glm::vec3(floorf(pos.x), floorf(pos.y), floorf(pos.z)),
                 glm::vec3(ceilf(pos.x), floorf(pos.y), floorf(pos.z)),
                 glm::vec3(floorf(pos.x), floorf(pos.y), ceilf(pos.z)),
                 glm::vec3(ceilf(pos.x), floorf(pos.y), ceilf(pos.z)),
                 glm::vec3(floorf(pos.x), ceilf(pos.y), floorf(pos.z)),
                 glm::vec3(ceilf(pos.x), ceilf(pos.y), floorf(pos.z)),
                 glm::vec3(floorf(pos.x), ceilf(pos.y), ceilf(pos.z)),
                 glm::vec3(ceilf(pos.x), ceilf(pos.y), ceilf(pos.z)),
             };
         auto& v : vertices) {
      glm::vec3* grid_point = &v;

      if (cached_world_positions[grid_point->x][grid_point->y][grid_point->z] ==
          glm::vec3(0)) {
        cached_world_positions[grid_point->x][grid_point->y][grid_point->z] =
            grid_to_world_position(m_bb_top_right, m_bb_bottom_left,
                                   *grid_point, grid_size);
      }

      m_grid[grid_point->x][grid_point->y][grid_point->z] = std::min(
          m_grid[grid_point->x][grid_point->y][grid_point->z],
          glm::distance(cached_world_positions[grid_point->x][grid_point->y]
                                              [grid_point->z],
                        vertex.pos));
    }
  }

  // Get the duration in microseconds
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::high_resolution_clock::now() - start);

  // Print the duration
  std::cout << "Time taken to generate G: " << duration.count()
            << " microseconds" << '\n';

  // Visualize unsigned distance field
  if (m_debugging == 2) {
    cgra::mesh_builder debugging;

    // Loop over and create squares the size of the value
    for (int x = 0; x < grid_size.x; x++) {
      for (int y = 0; y < grid_size.y; y++) {
        for (int z = 0; z < grid_size.z; z++) {
          const glm::vec3 grid_point = grid_to_world_position(
              m_bb_top_right, m_bb_bottom_left, glm::vec3(x, y, z), grid_size);

          const float sz_float =
              m_grid[x][y][z] < m_iso_level ? m_voxel_edge_length / 2.f : 0.f;

          const glm::vec3 size(sz_float);

          debugging.append(debug_box(grid_point - (size / 2.0f), size));
        }
      }
    }

    debugging.append(m_builder);
    m_mesh = debugging.build();
    return;
  }

  build();

  // Print the duration
  std::cout << "Time taken to finish whole process G: " << duration.count()
            << " microseconds" << '\n';
}

/// <summary>
/// Builds the mesh from the unsigned distance field
/// </summary>
auto simplified_mesh::build() -> void {
  const auto grid_size = glm::vec3(
      std::ceil((m_bb_top_right.x - m_bb_bottom_left.x) / m_voxel_edge_length),
      std::ceil((m_bb_top_right.y - m_bb_bottom_left.y) / m_voxel_edge_length),
      std::ceil((m_bb_top_right.z - m_bb_bottom_left.z) / m_voxel_edge_length));

  // Bounding cube debugging
  if (m_debugging == 1) {
    cgra::mesh_builder t =
        debug_box(m_bb_bottom_left, m_bb_top_right - m_bb_bottom_left);
    t.append(m_builder);
    m_mesh = t.build();
  }

  cgra::mesh_builder output;

  // Stores positions with their vertices indices
  std::unordered_map<glm::vec3, int> positions;

  // Convert into mesh
  for (int x = 0; x < grid_size.x - 1; x++) {
    for (int y = 0; y < grid_size.y - 1; y++) {
      for (int z = 0; z < grid_size.z - 1; z++) {
        const float values[8] = {
            m_grid[x][y][z],
            m_grid[x + 1][y][z],
            m_grid[x + 1][y + 1][z],
            m_grid[x][y + 1][z],
            m_grid[x][y][z + 1],
            m_grid[x + 1][y][z + 1],
            m_grid[x + 1][y + 1][z + 1],
            m_grid[x][y + 1][z + 1],
        };

        // Calculate cube index
        int cube_index = 0;
        if (values[0] < m_iso_level) cube_index |= 1;
        if (values[1] < m_iso_level) cube_index |= 2;
        if (values[2] < m_iso_level) cube_index |= 4;
        if (values[3] < m_iso_level) cube_index |= 8;
        if (values[4] < m_iso_level) cube_index |= 16;
        if (values[5] < m_iso_level) cube_index |= 32;
        if (values[6] < m_iso_level) cube_index |= 64;
        if (values[7] < m_iso_level) cube_index |= 128;

        std::vector<std::vector<int>>* faces = &tri_table[cube_index];

        const glm::vec3 grid_point = grid_to_world_position(
            m_bb_top_right, m_bb_bottom_left, glm::vec3(x, y, z), grid_size);

        for (auto& face : *faces) {
          std::vector<int>* face_edges = &face;

          auto size = output.m_vertices.size();

          // Calculate vertices
          const glm::vec3 vertices[3] = {
              edge_to_boundary_vertex((*face_edges)[0], grid_point, values,
                                      m_voxel_edge_length),
              edge_to_boundary_vertex((*face_edges)[1], grid_point, values,
                                      m_voxel_edge_length),
              edge_to_boundary_vertex((*face_edges)[2], grid_point, values,
                                      m_voxel_edge_length)};

          const auto v1 = glm::vec3(vertices[1].x - vertices[0].x,
                                    vertices[1].y - vertices[0].y,
                                    vertices[1].z - vertices[0].z);
          const auto v2 = glm::vec3(vertices[2].x - vertices[0].x,
                                    vertices[2].y - vertices[0].y,
                                    vertices[2].z - vertices[0].z);

          // Set normals of face
          const glm::vec3 surface_normal = normalize(cross(v1, v2));

          for (auto v : vertices) {
            if (!m_smooth_normals || !positions.contains(v)) {
              // New vertex!

              // Push data to builder
              // Use first field of uv as total normals stored in vertex to keep
              // a running total for smoothing
              output.push_vertex(
                  cgra::mesh_vertex{v, surface_normal, glm::vec2(1.0f, 0.0f)});
              output.push_index(size);

              // Store vertex
              positions[v] = size;

              size++;
            } else {
              // Existing vertex, combine
              int index = positions[v];
              output.push_index(index);

              // Add normal data
              output.m_vertices[index].norm += surface_normal;
              // Use first field of uv as total normals stored in vertex to keep
              // a running total for smoothing
              output.m_vertices[index].uv.x++;
            }
          }
        }
      }
    }
  }

  // Calculate final running totals for normal map
  for (cgra::mesh_vertex v : output.m_vertices) {
    v.norm = v.norm / v.uv.x;
    v.uv.x = 0.f;
  }

  std::cout << "Mesh generated with V: " << output.m_vertices.size()
            << " I: " << output.m_indices.size() << "\n";

  // Buffers cannot be empty, so create perceptually empty mesh
  if (!output.m_vertices.empty()) {
    m_mesh = output.build();
  } else {
    // Build an invisible mesh
    m_mesh = debug_box(glm::vec3(0.0f), glm::vec3(0.0f)).build();
  }
}

auto world_to_grid_position(const glm::vec3 top_right,
                            const glm::vec3 bottom_left,
                            const glm::vec3 position, const glm::vec3 grid_size)
    -> glm::vec3 {
  const glm::vec3 size = top_right - bottom_left;

  return ((position - bottom_left) / size) * grid_size;
}

auto grid_to_world_position(const glm::vec3 top_right,
                            const glm::vec3 bottom_left,
                            const glm::vec3 position, const glm::vec3 grid_size)
    -> glm::vec3 {
  if (position.x > grid_size.x || position.x < 0 || position.y > grid_size.y ||
      position.y < 0 || position.z > grid_size.z || position.z < 0) {
    throw std::invalid_argument("Position outside of grid size");
  }

  const glm::vec3 size = top_right - bottom_left;

  return bottom_left + glm::vec3(position.x * (size.x / grid_size.x),
                                 position.y * (size.y / grid_size.y),
                                 position.z * (size.z / grid_size.z));
}

auto debug_box(const glm::vec3 bottom_left, const glm::vec3 size)
    -> cgra::mesh_builder {
  auto bounding_box = cgra::mesh_builder();
  bounding_box.push_vertex(
      cgra::mesh_vertex{bottom_left, glm::vec3(1.0f, 0.0f, 0.0f)});
  bounding_box.push_vertex(
      cgra::mesh_vertex{bottom_left + glm::vec3(size.x, 0.0f, 0.0f),
                        glm::vec3(1.0f, 0.0f, 0.0f)});
  bounding_box.push_vertex(
      cgra::mesh_vertex{bottom_left + glm::vec3(0.0f, 0.0f, size.z),
                        glm::vec3(1.0f, 0.0f, 0.0f)});
  bounding_box.push_vertex(
      cgra::mesh_vertex{bottom_left + glm::vec3(size.x, 0.0f, size.z),
                        glm::vec3(1.0f, 0.0f, 0.0f)});

  bounding_box.push_vertex(
      cgra::mesh_vertex{bottom_left + glm::vec3(0.0f, size.y, 0.0f),
                        glm::vec3(1.0f, 0.0f, 0.0f)});
  bounding_box.push_vertex(
      cgra::mesh_vertex{bottom_left + glm::vec3(size.x, size.y, 0.0f),
                        glm::vec3(1.0f, 0.0f, 0.0f)});
  bounding_box.push_vertex(
      cgra::mesh_vertex{bottom_left + glm::vec3(0.0f, size.y, size.z),
                        glm::vec3(1.0f, 0.0f, 0.0f)});
  bounding_box.push_vertex(
      cgra::mesh_vertex{bottom_left + glm::vec3(size.x, size.y, size.z),
                        glm::vec3(1.0f, 0.0f, 0.0f)});

  bounding_box.push_indices({0, 1, 2,           // front face
                             1, 3, 2, 4, 5, 6,  // back face
                             5, 7, 6, 0, 1, 4,  // bottom face
                             1, 5, 4, 2, 3, 6,  // top face
                             3, 7, 6, 0, 2, 4,  // left face
                             2, 6, 4, 1, 3, 5,  // right face
                             3, 7, 5});

  return bounding_box;
}
