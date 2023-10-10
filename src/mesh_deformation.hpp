#pragma once

#include <cgra/cgra_mesh.hpp>
#include <terrain_model.hpp>

class mesh_deformation {
public:
    glm::mat4 m_view, m_proj;

    mesh_deformation() { }
    void setModel(terrain_model& m);
    void deformMesh(const cgra::mesh_vertex& center, bool isBump, double deformationRadius, double maxDeformationStrength);
    glm::vec3 calculateFaceNormal(const glm::vec3& vertex1, const glm::vec3& vertex2, const glm::vec3& vertex3);
    void computeVertexNormals();
    void mouseIntersectMesh(double xpos, double ypos, double windowsize_x, double windowsize_y);
    bool rayIntersectsVertex(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, int vertex);
    bool rayIntersectsTriangle(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& vertex0, const glm::vec3& vertex1, const glm::vec3& vertex2);
    void computeTBN();
    void calculateTBN(cgra::mesh_builder& mb, bool topLeft, int k1, int k2, int k3, int k4);
    terrain_model getModel();

private:
    terrain_model model;
};