#include "utils/aabb_tree.hpp"
#include <algorithm>

auto aabb_tree::build(const std::vector<glm::vec3>& vertices,
    const std::vector<unsigned int>& indices) -> void {
    triangles.clear();

    // Build triangle list
    for (size_t i = 0; i < indices.size(); i += 3) {
        triangle tri{};
        tri.v0 = vertices[indices[i]];
        tri.v1 = vertices[indices[i + 1]];
        tri.v2 = vertices[indices[i + 2]];
        tri.index = i / 3;
        triangles.push_back(tri);
    }

    // Build tree
    std::vector<unsigned int> all_indices(triangles.size());
    for (size_t i = 0; i < all_indices.size(); i++) {
        all_indices[i] = i;
    }

    root = build_recursive(all_indices, 0);
}

auto aabb_tree::build_recursive(
    std::vector<unsigned int>& tri_indices, int depth) -> std::unique_ptr<aabb_tree::node> {

    auto node = std::make_unique<aabb_tree::node>();
    node->is_leaf = false;

    // Compute bounds for this node
    for (unsigned int idx : tri_indices) {
        const triangle& tri = triangles[idx];
        node->bounds.expand(tri.v0);
        node->bounds.expand(tri.v1);
        node->bounds.expand(tri.v2);
    }

    // Leaf condition: few triangles or max depth
    if (tri_indices.size() <= 4 || depth > 20) {
        node->is_leaf = true;
        node->triangle_indices = tri_indices;
        return node;
    }

    // Find longest axis
    glm::vec3 extent = node->bounds.max - node->bounds.min;
    int axis = 0;
    if (extent.y > extent.x) axis = 1;
    if (extent.z > extent[axis]) axis = 2;

    // Sort triangles by centroid along axis
    std::sort(tri_indices.begin(), tri_indices.end(),
        [this, axis](unsigned int a, unsigned int b) {
            glm::vec3 ca = (triangles[a].v0 + triangles[a].v1 + triangles[a].v2) / 3.0f;
            glm::vec3 cb = (triangles[b].v0 + triangles[b].v1 + triangles[b].v2) / 3.0f;
            return ca[axis] < cb[axis];
        });

    // Split in half
    size_t mid = tri_indices.size() / 2;
    std::vector<unsigned int> left_indices(tri_indices.begin(), tri_indices.begin() + mid);
    std::vector<unsigned int> right_indices(tri_indices.begin() + mid, tri_indices.end());

    node->left = build_recursive(left_indices, depth + 1);
    node->right = build_recursive(right_indices, depth + 1);

    return node;
}

auto aabb_tree::query_ray(const glm::vec3& origin,
    const glm::vec3& direction) const -> std::vector<unsigned int> {
    std::vector<unsigned int> results;
    glm::vec3 dir_inv = 1.0f / direction;  // Precompute for efficiency
    query_recursive(root.get(), origin, dir_inv, results);
    return results;
}

auto aabb_tree::query_recursive(const node* node, const glm::vec3& origin,
    const glm::vec3& dir_inv,
    std::vector<unsigned int>& results) const -> void {
    if (!node) return;

    // Test ray against node's AABB
    if (!node->bounds.intersects_ray(origin, dir_inv)) {
        return;
    }

    // If leaf, add all triangles
    if (node->is_leaf) {
        results.insert(results.end(),
            node->triangle_indices.begin(),
            node->triangle_indices.end());
        return;
    }

    // Recurse into children
    query_recursive(node->left.get(), origin, dir_inv, results);
    query_recursive(node->right.get(), origin, dir_inv, results);
}
