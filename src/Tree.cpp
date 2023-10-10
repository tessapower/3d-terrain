// std
#define _USE_MATH_DEFINES
#include <iostream>
#include <string>
#include <chrono>
#include <cmath>

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// project
#include "Tree.hpp"
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stack>
#include <queue>
#include <random>

using namespace std;
using namespace cgra;
using namespace glm;

void Tree::draw(const glm::mat4& view, const glm::mat4& proj) {
    glUseProgram(m_shader);
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
    if(m_leafPositions.size() > 0) {
       // drawLeaves(view, proj);
    }
    //drawBranchesNodes(0, view, proj);
    if (!spookyMode) {
        drawFoliage(view, proj);
    }
    if (mesh.index_count > 0) {
        drawTree(view, proj);
    }
}

glm::vec3 Tree::generateRandomLeafPosition(float radius) {
    float theta = randomFloat(0.0f, 2.0f * glm::pi<float>()); // Random angle
    float phi = randomFloat(0.0f, glm::pi<float>()); // Random inclination angle
    float x = radius * sin(phi) * cos(theta);
    float y = radius * sin(phi) * sin(theta);
    float z = radius * cos(phi);
    return glm::vec3(x, y, z);
}

void Tree::generateLeaves(float radius, int leaves) {
    if (m_leafPositions.size() > 0) {
        m_leafPositions.clear();
    }
    for (int i = 0; i < leaves; i++) {
        vec3 leaf;
        leaf = process(randomFloat(0, 1)) * generateRandomLeafPosition(radius);
        m_leafPositions.push_back(leaf + vec3(0, 7 , 0));
    }
}

float Tree::process(float x) {
    return pow(sin(x * M_PI / 2.0), 0.8);
}

int Tree::randomInt(int a, int b)
{
    if (a > b)
        return randomInt(b, a);
    if (a == b)
        return a;
    return a + (rand() % (b - a));
}

float Tree::randomFloat(int a, int b)
{
    if (a > b)
        return randomFloat(b, a);
    if (a == b)
        return a;
    return (float)randomInt(a, b) + (float)(rand()) / (float)(RAND_MAX);
}

void Tree::drawLeaves(const glm::mat4& view, const glm::mat4& proj) {
    for (int i = 0; i < m_leafPositions.size(); i++) {
        mat4 positionMatrix = glm::translate(view, m_leafPositions[i]);
        mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.1));

        mat4 sphereModelMatrix = positionMatrix * scalingMatrix;

        glUseProgram(m_shader);
        GLuint sphereModelViewMatrixLocation = glGetUniformLocation(m_shader, "uModelViewMatrix");
        if (sphereModelViewMatrixLocation != -1) {
            glUniformMatrix4fv(sphereModelViewMatrixLocation, 1, false, glm::value_ptr(sphereModelMatrix));
            glUniform3fv(glGetUniformLocation(m_shader, "uColor"), 1, value_ptr(vec3(1, 0, 0)));
        }
        draw_sphere();
        glUseProgram(0);
    }
}

void Tree::generateTree() {
    if (killRange < branchLength || killRange > m_attractionRange) {
        return;
    }
    if (Branches.size() > 0) {
        Branches.clear();
    }
    generateBranches();
    bool growing = true;
    while (growing) {
        growing = grow();
    }
    calculateFoliage();
    generateMesh();
}

void Tree::generateBranches() {
    branch trunk(startPosition, startPosition + glm::vec3(0, branchLength, 0), glm::vec3(0, 1, 0), NULL, 0);
    Branches.push_back(trunk);
    int current = 0;

    branch b = Branches[current];
    bool foundLeaf = false;
    while (!foundLeaf) {
        for (int i = 0; i < m_leafPositions.size(); i++) {
            float distance = glm::distance(b._end, m_leafPositions[i]);
            if (distance < m_attractionRange) {
                foundLeaf = true;
            }
        }
        if (!foundLeaf) {
            glm::vec3 randomVector = glm::normalize(glm::vec3(randomFloat(-1.0f, 1.0f), randomFloat(-1.0f, 1.0f), randomFloat(-1.0f, 1.0f)));
            randomVector *= m_randomnessFactor;
            glm::vec3 newDirection = glm::normalize(b._direction + randomVector);
            vec3 nextDir = newDirection * branchLength;
            glm::vec3 newEnd = b._end + nextDir;
            branch newBranch(b._end, newEnd, newDirection, current, Branches.size());
            b._children.push_back(Branches.size());
            Branches.push_back(newBranch);
            Branches[current] = b;
            b = newBranch;
            current = newBranch._id;
        }
    }
}

bool Tree::grow() {
    bool returnVal = false;
    for (auto i = 0; i < m_leafPositions.size(); i++) {
        auto currectPos = m_leafPositions[i];
        float record = FLT_MAX;
        int closestBranch = NULL;

        for (auto j = 0; j < Branches.size(); j++) {
            branch b = Branches[j];
            float distance = glm::distance(b._end, m_leafPositions[i]);
            if (distance < killRange) {
                reachedLeaves.push_back(currectPos);
                closestBranch = NULL;
                break;
            }
            else if (distance > m_attractionRange) {
                
            }
            else if (closestBranch == NULL || distance < record) {
                closestBranch = j;
                record = distance;
            }
        }
        if (closestBranch != NULL) {
            returnVal = true;
            branch b = Branches[closestBranch];
            glm::vec3 toAttractor = glm::normalize(currectPos - b._end);
            glm::vec3 newDirection = glm::normalize(b._direction + toAttractor);
            b._direction = newDirection;
            b._count = b._count + 1.0f;
            Branches[closestBranch] = b;
        }  
    }
    for (int i = m_leafPositions.size() - 1; i >= 0; i--) {
        if (std::find(reachedLeaves.begin(), reachedLeaves.end(), m_leafPositions[i]) != reachedLeaves.end()) {
            m_leafPositions.erase(m_leafPositions.begin() + i);
        }
    }
    for (int i = Branches.size() - 1; i >= 0; i--) {
		auto currentBranch = Branches[i];
        if (currentBranch._count > 0) {
            currentBranch._direction = normalize(currentBranch._direction);
			currentBranch._direction = currentBranch._direction / (currentBranch._count + 1.0f);
            vec3 nextDir = currentBranch._direction * branchLength;
            currentBranch._end = Branches[currentBranch._parent]._end + currentBranch._direction;
            glm::vec3 newEnd = currentBranch._end + nextDir;
            branch newBranch(currentBranch._end, newEnd, currentBranch._direction, i, Branches.size());
            currentBranch._children.push_back(Branches.size());

            branch branchCheck = Branches[currentBranch._parent];
            bool reachedEnd = false;
            while (!reachedEnd) {
                branchCheck._children.push_back(Branches.size());
                Branches[branchCheck._id] = branchCheck;
                if (branchCheck._id == 0) {
                    reachedEnd = true;
                }
                else {
                    branchCheck = Branches[branchCheck._parent];
                }
            }

            Branches.push_back(newBranch);
        }
        currentBranch.reset();
        Branches[i] = currentBranch;
    }
    return returnVal;
}

void Tree::printTree() {
    if (Branches.size() <= 0) {
        cout << "No Trees" << endl;
        return;
    }
    for(branch b : Branches){
        cout << "Branch = " << b._id << endl;
        cout << "Children: (Count = " << b._children.size() << ")(";
        for (int i = 0; i < b._children.size(); i++) {
            cout << b._children[i] << ",";
        }
	    cout << ")" << endl;
        cout << "Parent: " << b._parent << endl;
        cout << "branch start: (" << b._start.x << "," << b._start.y << "," << b._start.z << ")" << endl;
        cout << "Current end: (" << b._end.x << "," << b._end.y << "," << b._end.z << ")" << endl;
        cout << endl;
    }
}

void Tree::drawBranchesNodes(int currentBranch, const glm::mat4& view, const glm::mat4& proj) {
    for (branch b : Branches) {
        glm::vec3 start = b._start;
        glm::vec3 end = b._end;

        mat4 positionMatrix = glm::translate(view, end);
        mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.3));

        mat4 sphereModelMatrix = positionMatrix * scalingMatrix;

        glUseProgram(m_shader);

        GLuint sphereModelViewMatrixLocation = glGetUniformLocation(m_shader, "uModelViewMatrix");
        if (sphereModelViewMatrixLocation != -1) {
            glUniformMatrix4fv(sphereModelViewMatrixLocation, 1, false, glm::value_ptr(sphereModelMatrix));
            glUniform3fv(glGetUniformLocation(m_shader, "uColor"), 1, value_ptr(vec3(0, 0, 1)));
        }

        draw_sphere();

        glUseProgram(0);
    }
    if (Branches.size() == 0) {
        return;
    }
}

void Tree::generateMesh() {
    cgra::mesh_builder builder;
	builder.vertices.clear();
    float radius = 0.2;
    int numVertices = 20;
 
    vector<int> initialCircle;

    float angleIncrement = 2 * glm::pi<float>() / numVertices;

    for (int j = 0; j < numVertices; j++) {
        float branchRadius = Branches.size() * 0.0002;
        branchRadius = 0.9 * (pow(branchRadius - 1, 3)) + 1;

        float angle = j * angleIncrement;
        float x = 0.55 * cos(angle);
        float z = 0.55 * sin(angle);

        mesh_vertex vtex;
        vtex.pos = vec3(x, 0, z);
        vtex.norm = glm::normalize(vec3(x, 0, z));
        vtex.uv = vec2(0, 0);

        initialCircle.push_back(builder.vertices.size());
        builder.push_vertex(vtex);
    }

    for (int i = 0; i < Branches.size(); i++) {

        float branchRadius = radius;

        if (Branches[i]._children.size() < 1) {
            branchRadius = 0;
        }
        else {
            branchRadius = Branches[i]._children.size() * 0.0002;
            branchRadius = 0.95 * (pow(branchRadius - 1, 3)) + 1;
        }

        Branches[i].vertices.clear();
        for (int j = 0; j < numVertices; j++) {
            float angle = j * angleIncrement;
            float x = branchRadius * cos(angle);
            float z = branchRadius * sin(angle);

            mesh_vertex vtex;

            float angleBetweenNormals = glm::acos(glm::dot(vec3(0,1,0), normalize(Branches[i]._direction)));
            mat4 rotationMatrix = rotate(mat4(1), angleBetweenNormals, cross(vec3(0, 1, 0), normalize(Branches[i]._direction)));
            vec4 rotatedVertex = rotationMatrix * glm::vec4(x, 0, z, 1.0f);
            vtex.pos = Branches[i]._end + glm::vec3(rotatedVertex); 
            vtex.norm = Branches[i]._end + glm::vec3(rotatedVertex); 
            vtex.uv = vec2(0, 0); 

            Branches[i].vertices.push_back(builder.vertices.size());
            builder.push_vertex(vtex);
        }
    }
    for (int r = 1; r < Branches.size(); r++) {
        branch b = Branches[r];
		branch parent = Branches[b._parent];
        for (int v = 0; v < numVertices - 1; v++) {
            unsigned int k1 = b.vertices[v];
            unsigned int k1_1 = b.vertices[v + 1];
            unsigned int k2 = parent.vertices[v];
            unsigned int k2_1 = parent.vertices[v + 1];
            builder.push_indices({k1,k2,k1_1});
            builder.push_indices({ k1_1,k2,k2_1});
        }
        unsigned int k1 = b.vertices[0];
        unsigned int k1_1 = b.vertices[b.vertices.size() - 1];
        unsigned int k2 = parent.vertices[0];
        unsigned int k2_1 = parent.vertices[parent.vertices.size() - 1];
        builder.push_indices({ k1,k2,k1_1 });
        builder.push_indices({ k1_1,k2,k2_1 });
    }

    branch b = Branches[1];
    for (int v = 0; v < numVertices - 1; v++) {
        unsigned int k1 = b.vertices[v];
        unsigned int k1_1 = b.vertices[v + 1];
        unsigned int k2 = initialCircle[v];
        unsigned int k2_1 = initialCircle[v + 1];
        builder.push_indices({ k1,k2,k1_1 });
        builder.push_indices({ k1_1,k2,k2_1 });
    }
    unsigned int k1 = b.vertices[0];
    unsigned int k1_1 = b.vertices[b.vertices.size() - 1];
    unsigned int k2 = initialCircle[0];
    unsigned int k2_1 = initialCircle[initialCircle.size() - 1];
    builder.push_indices({ k1,k2,k1_1 });
    builder.push_indices({ k1_1,k2,k2_1 });

    mesh = builder.build();
}

void Tree::drawTree(const glm::mat4& view, const glm::mat4 proj) {
    mat4 modelview = view * (glm::scale(modelTranslate,modelScale));
    glUseProgram(m_shader); 
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "uModelViewMatrix"), 1, false, value_ptr(modelview));
    glUniform3fv(glGetUniformLocation(m_shader, "uColor"), 1, value_ptr(vec3(0.28, 0.23, 0.15)));
    mesh.draw();
}

void Tree::calculateFoliage() {
    leaves.clear();

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<float> colDis(0.4, 0.8);
    std::uniform_real_distribution<float> sizeDis(0.5, 1.0);

    for (branch b : Branches) {
        if (b._children.size() < 1) {
            leaf newLeaf(vec3( 0, colDis(gen), 0), vec3(sizeDis(gen)), b._end);
			leaves.push_back(newLeaf);
        }
    }
}

void Tree::drawFoliage(const glm::mat4& view, const glm::mat4 proj) {
    glUseProgram(m_shader);
    for (leaf le : leaves) {
        mat4 positionMatrix = glm::translate(view, le._leafPos * modelScale);
        positionMatrix = positionMatrix * modelTranslate;
        mat4 scalingMatrix = glm::scale(positionMatrix, le._leafSize);
        mat4 sphereModelMatrix = glm::scale(scalingMatrix, modelScale);

        glUseProgram(m_shader);

        GLuint sphereModelViewMatrixLocation = glGetUniformLocation(m_shader, "uModelViewMatrix");
        if (sphereModelViewMatrixLocation != -1) {
            glUniformMatrix4fv(sphereModelViewMatrixLocation, 1, false, glm::value_ptr(sphereModelMatrix));
            glUniform3fv(glGetUniformLocation(m_shader, "uColor"), 1, value_ptr(le._leafColour));
        }

        draw_sphere();
    }
    glUseProgram(0);
}