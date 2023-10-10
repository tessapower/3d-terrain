#pragma once

// Include necessary headers
#include <vector>
#include <glm/glm.hpp>

// glm
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

// project
#include "opengl.hpp"
#include "cgra/cgra_mesh.hpp"

struct branch {
    int _id;
    glm::vec3 _start;
    glm::vec3 _end;
    glm::vec3 _direction;
    glm::vec3 _directionCopy;
    int _parent; // Use a pointer to represent the parent branch
    std::vector<int> _children;
    std::vector<int> vertices;
    float _count = 0.0;

    // Constructor
    branch(const glm::vec3& start, const glm::vec3& end, const glm::vec3& direction, int parent = 0, const int id = 0)
        : _start(start), _end(end), _direction(direction), _parent(parent), _id(id), _directionCopy(direction){}

    void reset() {
        _direction = _directionCopy;
        _count = 0.0;
    }
};

struct leaf {
    glm::vec3 _leafColour;
    glm::vec3 _leafSize;
    glm::vec3 _leafPos;

    leaf(const glm::vec3& leafColour, const glm::vec3& leafSize, const glm::vec3& leafPos)
        : _leafColour(leafColour), _leafSize(leafSize), _leafPos(leafPos) {}
};

class Tree {
public:
    Tree() {}
    GLuint m_shader = 0;
    glm::mat4 modelTranslate{ 1.0 };
    glm::vec3 modelScale{1.0f};
    bool spookyMode = false;

    // Function to draw the tree
    void draw(const glm::mat4& view, const glm::mat4& projection);
    void generateLeaves(float radius, int leaves);
    void generateTree();
    void printTree();
    void drawBranchesNodes(int currentBranch, const glm::mat4& view, const glm::mat4& proj);
    float process(float x);
    void drawTree(const glm::mat4& view, const glm::mat4 proj);
    void generateMesh();
    cgra::gl_mesh mesh;
    std::vector<leaf> leaves;
    std::vector<glm::vec3> m_leafPositions;
    std::vector<glm::vec3> reachedLeaves;
    std::vector<branch> Branches;
    // Add other methods and member variables as needed

private:
    // Member variables for the tree
	void generateBranches();
    bool grow();
    glm::vec3 generateRandomLeafPosition(float radius);
    void drawLeaves(const glm::mat4& view, const glm::mat4& proj);
    void drawFoliage(const glm::mat4& view, const glm::mat4 proj);
    void calculateFoliage();
    int randomInt(int a, int b);
    float randomFloat(int a, int b);
    glm::vec3 startPosition{0.0};
    float branchLength = 0.5;
    float m_attractionRange = 1.5;
    float m_repulsionRange = 1;
    float killRange = 0.5;
    float m_randomnessFactor = 0.1;
    float minimumLength = 5;
    float maximumLength = 15;
    int maxDepth = 10;

    // Add other member variables like mesh, texture, etc. as needed
};