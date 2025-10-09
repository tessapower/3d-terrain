#ifndef AABB_TREE_HPP
#define AABB_TREE_HPP

#include <glm/glm.hpp>
#include <vector>
#include <memory>

/// Code Author: Tessa Power
///
/// An Axis-Aligned Bounding Box (AABB) Tree implementation for
/// efficient ray-triangle intersection queries.

struct aabb {
    glm::vec3 min;
    glm::vec3 max;

    aabb() : min(FLT_MAX), max(-FLT_MAX) {}

    auto expand(const glm::vec3& point) -> void {
        min = glm::min(min, point);
        max = glm::max(max, point);
    }

    auto expand(const aabb& other) -> void {
        min = glm::min(min, other.min);
        max = glm::max(max, other.max);
    }

    auto intersects_ray(const glm::vec3& origin, const glm::vec3& dir_inv) const -> bool {
        float t1 = (min.x - origin.x) * dir_inv.x;
        float t2 = (max.x - origin.x) * dir_inv.x;
        float t3 = (min.y - origin.y) * dir_inv.y;
        float t4 = (max.y - origin.y) * dir_inv.y;
        float t5 = (min.z - origin.z) * dir_inv.z;
        float t6 = (max.z - origin.z) * dir_inv.z;

        float tmin = glm::max(glm::max(glm::min(t1, t2), glm::min(t3, t4)), glm::min(t5, t6));
        float tmax = glm::min(glm::min(glm::max(t1, t2), glm::max(t3, t4)), glm::max(t5, t6));

        return tmax >= tmin && tmax >= 0;
    }
};

struct triangle {
    glm::vec3 v0, v1, v2;
    unsigned int index;  // Original triangle index
};

class aabb_tree {
public:
  auto build(const std::vector<glm::vec3>& vertices,
             const std::vector<unsigned int>& indices) -> void;

  // Returns list of triangle indices that might intersect the ray
  auto query_ray(const glm::vec3& origin, const glm::vec3& direction) const
      -> std::vector<unsigned int>;

  auto get_triangles() const -> const std::vector<triangle>& { return triangles; }

private:
  struct node {
        aabb bounds;
        std::unique_ptr<node> left;
        std::unique_ptr<node> right;
        std::vector<unsigned int> triangle_indices;  // Only populated in leaves
        bool is_leaf;
  };

    std::unique_ptr<node> root;
    std::vector<triangle> triangles;

    auto build_recursive(std::vector<unsigned int>& tri_indices, int depth) -> std::unique_ptr<node>;
    auto query_recursive(const node* node, const glm::vec3& origin,
        const glm::vec3& dir_inv, std::vector<unsigned int>& results) const -> void;
};

#endif // AABB_TREE_HPP
