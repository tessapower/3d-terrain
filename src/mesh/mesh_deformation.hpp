#pragma once

#include <cgra/cgra_mesh.hpp>
#include <terrain/terrain_model.hpp>

/*

Based on SIGGRAPH 2004 Technical Paper:
Mesh editing with poisson-based gradient field manipulation (Yizhou Yu, Kun Zhou, Dong Xu, Xiaohan Shi, Hujun Bao, Baining Guo, Heung-Yeung Shum)
https://dl.acm.org/doi/10.1145/1015706.1015774

Created with the help of the Normal Mapping tutorial from:
https://learnopengl.com/Advanced-Lighting/Normal-Mapping

Code Author: Shekinah Pratap

*/

class mesh_deformation {
public:
    glm::mat4 m_view, m_proj;

    mesh_deformation() { }
    void setModel(terrain_model& m);
    void deformMesh(const cgra::mesh_vertex& center, bool isBump, double deformationRadius, double maxDeformationStrength);
    glm::vec3 calculateFaceNormal(const glm::vec3& vertex1, const glm::vec3& vertex2, const glm::vec3& vertex3);
    void computeVertexNormals();
    void mouseIntersectMesh(double xpos, double ypos, double windowsize_x, double windowsize_y);
    void computeTBN();
    void calculateTBN(cgra::mesh_builder& mb, bool topLeft, int k1, int k2, int k3, int k4);
    terrain_model getModel();

private:
    terrain_model model;
};