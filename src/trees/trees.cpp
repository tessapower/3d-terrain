#define _USE_MATH_DEFINES
#include "trees.hpp"

#include <chrono>
#include <cmath>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <queue>
#include <random>

#include "application.hpp"
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_image.hpp"

constexpr float pi = glm::pi<float>();

// Draws the tree by calling the appropriate draw functions.
auto tree::draw(const glm::mat4& view, const glm::mat4& projection) -> void {
  glUseProgram(m_shader);
  glUniformMatrix4fv(glGetUniformLocation(m_shader, "uProjectionMatrix"), 1,
                     false, value_ptr(projection));

  // TODO: replace this with #ifdef directive for debugging
  // uncomment this for debugging
  /*
  if(m_leafPositions.size() > 0) {
     drawLeaves(view, proj);
  }
  drawBranchesNodes(0, view, proj);
  */

  if (!m_spooky_mode) {
    draw_foliage(view);
  }
  if (m_mesh.index_count > 0) {
    draw_tree(view, projection);
  }
}

// finds a random positions within a sphere and returns a point within it
auto tree::generate_random_leaf_position(const float radius) -> glm::vec3 {
  const float theta = random_float(0.0f, 2.0f * pi);  // Random angle
  const float phi = random_float(0.0f, pi);  // Random inclination angle
  const float x = radius * sin(phi) * cos(theta);
  const float y = radius * sin(phi) * sin(theta);
  const float z = radius * cos(phi);

  return {x, y, z};
}

// creates a set of positions to be used as attractors or "leaves"
auto tree::generate_leaves(const float radius, const int leaves) -> void {
  if (!m_leaf_positions.empty()) {
    m_leaf_positions.clear();
  }

  for (int i = 0; i < leaves; i++) {
    glm::vec3 leaf;
    leaf = process(random_float(0.0f, 1.0f)) *
           generate_random_leaf_position(radius);
    m_leaf_positions.push_back(leaf + glm::vec3(0.0f, 7.0f, 0.0f));
  }
}

// Helper function for processing leaf positions
// Stops leaves from swarming the center position
auto tree::process(const float x) -> float {
  return glm::pow(glm::sin(x * pi / 2.0f), 0.8f);
}

// creates a random int within a and b
auto tree::random_int(const int a, const int b) -> int {
  if (a > b) return random_int(b, a);
  if (a == b) return a;

  return a + (rand() % (b - a));
}

// creates a random float within a and b
auto tree::random_float(const int a, const int b) -> float {
  if (a > b) return random_float(b, a);
  if (a == b) return a;
  return static_cast<float>(random_int(a, b)) +
         static_cast<float>(rand()) / static_cast<float>((RAND_MAX));
}

// goes through all leaf positions and draws a sphere at the location of the
// leaf.
auto tree::draw_leaves(const glm::mat4& view, const glm::mat4& projection) const
    -> void {
  for (auto m_leaf_position : m_leaf_positions) {
    glm::mat4 position_matrix = glm::translate(view, m_leaf_position);
    glm::mat4 scaling_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

    glm::mat4 sphere_model_matrix = position_matrix * scaling_matrix;

    glUseProgram(m_shader);
    if (const GLuint sphere_model_view_matrix_location =
            glGetUniformLocation(m_shader, "uModelViewMatrix");
        sphere_model_view_matrix_location != -1) {
      glUniformMatrix4fv(sphere_model_view_matrix_location, 1, false,
                         glm::value_ptr(sphere_model_matrix));
      glUniform3fv(glGetUniformLocation(m_shader, "uColor"), 1,
                   glm::value_ptr(glm::vec3(1, 0, 0)));
    }

    cgra::draw_sphere();
    glUseProgram(0);
  }
}

// set up function for creating the tree.
auto tree::generate_tree() -> void {
  // returns if tree shouldn't be made
  if (m_kill_range_ < m_branch_length_ || m_kill_range_ > m_attraction_range_) {
    return;
  }
  // clears out branches in case tree already exists
  if (!m_branches.empty()) {
    m_branches.clear();
  }
  // generates and then grows branches
  generate_branches();
  bool growing = true;
  while (growing) {
    growing = grow();
  }
  // calculates the leaf positions and then generates the mesh
  calculate_foliage();
  generate_mesh();
}

// Generates Branches from the base position up to when attractors are found.
auto tree::generate_branches() -> void {
  branch trunk(start_position_,
               start_position_ + glm::vec3(0, m_branch_length_, 0),
               glm::vec3(0, 1, 0), NULL, 0);
  m_branches.push_back(trunk);
  int current = 0;

  // generates new branches moving upwards until leaves enter branches
  // attraction range
  branch b = m_branches[current];
  bool found_leaf = false;
  while (!found_leaf) {
    for (auto m_leaf_position : m_leaf_positions) {
      if (float distance = glm::distance(b.m_end, m_leaf_position);
          distance < m_attraction_range_) {
        found_leaf = true;
      }
    }
    if (!found_leaf) {
      glm::vec3 random_vector = glm::normalize(
          glm::vec3(random_float(-1.0f, 1.0f), random_float(-1.0f, 1.0f),
                    random_float(-1.0f, 1.0f)));
      random_vector *= m_randomness_factor_;

      glm::vec3 new_direction = glm::normalize(b.m_direction + random_vector);
      glm::vec3 next_dir = new_direction * m_branch_length_;

      glm::vec3 new_end = b.m_end + next_dir;
      branch new_branch(b.m_end, new_end, new_direction, current,
                        static_cast<int>(m_branches.size()));

      b.children.push_back(static_cast<int>(m_branches.size()));
      m_branches.push_back(new_branch);
      m_branches[current] = b;
      b = new_branch;
      current = new_branch.m_id;
    }
  }
}

// Depth first creates branches towards the attractors.
auto tree::grow() -> bool {
  bool should_grow = false;
  // iterates through all leaf positions
  for (auto m_leaf_position : m_leaf_positions) {
    auto current_pos = m_leaf_position;
    float record = FLT_MAX;
    int closest_branch = NULL;

    // find the closest branch
    for (size_t j = 0; j < m_branches.size(); j++) {
      branch b = m_branches[j];
      if (float distance = glm::distance(b.m_end, m_leaf_position);
          distance < m_kill_range_) {
        m_reached_leaves.push_back(current_pos);
        closest_branch = NULL;
        break;
      } else if (distance > m_attraction_range_) {
      } else if (closest_branch == NULL || distance < record) {
        closest_branch = j;
        record = distance;
      }
    }
    // gives branch new direction based on its attractors
    if (closest_branch != NULL) {
      should_grow = true;
      branch b = m_branches[closest_branch];

      glm::vec3 to_attractor = glm::normalize(current_pos - b.m_end);
      glm::vec3 new_direction = glm::normalize(b.m_direction + to_attractor);

      b.m_direction = new_direction;
      b.count = b.count + 1.0f;
      m_branches[closest_branch] = b;
    }
  }

  // removes attractors that have entered a branches kill range
  for (size_t i = m_leaf_positions.size() - 1; i > 0; i--) {
    if (std::ranges::find(m_reached_leaves, m_leaf_positions[i]) !=
        m_reached_leaves.end()) {
      m_leaf_positions.erase(m_leaf_positions.begin() + i);
    }
  }
  // generates a new branch if a branch has detected an attractor.
  for (int i = m_branches.size() - 1; i >= 0; i--) {
    auto current_branch = m_branches[i];
    if (current_branch.count > 0) {
      current_branch.m_direction = normalize(current_branch.m_direction);
      current_branch.m_direction =
          current_branch.m_direction / (current_branch.count + 1.0f);

      glm::vec3 next_dir = current_branch.m_direction * m_branch_length_;
      current_branch.m_end = m_branches[current_branch.m_parent].m_end +
                             current_branch.m_direction;

      glm::vec3 new_end = current_branch.m_end + next_dir;
      branch new_branch(current_branch.m_end, new_end,
                        current_branch.m_direction, i, m_branches.size());
      current_branch.children.push_back(m_branches.size());

      branch branch_check = m_branches[current_branch.m_parent];
      bool reached_end = false;

      // backwards iterates to add child to parent branches
      while (!reached_end) {
        branch_check.children.push_back(m_branches.size());
        m_branches[branch_check.m_id] = branch_check;
        if (branch_check.m_id == 0) {
          reached_end = true;
        } else {
          branch_check = m_branches[branch_check.m_parent];
        }
      }

      m_branches.push_back(new_branch);
    }
    current_branch.reset();
    m_branches[i] = current_branch;
  }
  return should_grow;
}

// USED FOR DEBUGGING
// Print Tree gives info on the tree by printing the information from every
// branch
auto tree::print_tree() const -> void {
  if (m_branches.empty()) {
    std::cout << "No Trees" << '\n';
    return;
  }
  for (const branch& b : m_branches) {
    std::cout << "Branch = " << b.m_id << '\n'
      << "Children: (Count = " << b.children.size() << ")(";

    for (const int i : b.children) {
      std::cout << i << ",";
    }

    std::cout << ")" << '\n'
      << "Parent: " << b.m_parent << '\n'
      << "branch start: (" << b.m_start.x << "," << b.m_start.y << ","
              << b.m_start.z << ")" << '\n'
      << "Current end: (" << b.m_end.x << "," << b.m_end.y << ","
              << b.m_end.z << ")" << '\n'
      << '\n';
  }
}

// Draws a node at the end position of ever branch to show tree.
auto tree::draw_branches_nodes(int current_branch, const glm::mat4& view,
                               const glm::mat4& projection) const -> void {
  for (const branch& b : m_branches) {
    glm::vec3 start = b.m_start;
    glm::vec3 end = b.m_end;

    glm::mat4 position_matrix = glm::translate(view, end);
    glm::mat4 scaling_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.3));

    glm::mat4 sphere_model_matrix = position_matrix * scaling_matrix;

    glUseProgram(m_shader);

    if (const GLuint sphere_model_view_matrix_location =
            glGetUniformLocation(m_shader, "uModelViewMatrix");
        sphere_model_view_matrix_location != -1) {
      glUniformMatrix4fv(sphere_model_view_matrix_location, 1, false,
                         glm::value_ptr(sphere_model_matrix));
      glUniform3fv(glGetUniformLocation(m_shader, "uColor"), 1,
                   glm::value_ptr(glm::vec3(0.0f, 0.0f, 1.0f)));
    }

    cgra::draw_sphere();

    glUseProgram(0);
  }
}

// Generates Mesh for the tree.
auto tree::generate_mesh() -> void {
  cgra::mesh_builder builder;
  builder.m_vertices.clear();
  int num_vertices = 20;

  std::pmr::vector<int> initial_circle;

  float angle_increment = 2.0f * pi / num_vertices;

  // generates initial circle. This is the base of the tree
  for (int j = 0; j < num_vertices; j++) {
    float branch_radius = static_cast<float>(m_branches.size()) * 0.0002f;
    branch_radius = 0.9f * (glm::pow(branch_radius - 1.0f, 3.0f)) + 1.0f;

    float angle = static_cast<float>(j) * angle_increment;
    float x = 0.55f * cos(angle);
    float z = 0.55f * sin(angle);

    cgra::mesh_vertex vertex;
    vertex.pos = glm::vec3(x, 0.0f, z);
    vertex.norm = normalize(glm::vec3(x, 0.0f, z));
    vertex.uv = glm::vec2(0.0f, 0.0f);

    initial_circle.push_back(builder.m_vertices.size());
    builder.push_vertex(vertex);
  }

  // iterates through every branch finding the correct radius and then
  // generating a rotated circle around it.
  for (auto& branch : m_branches) {
    float radius = 0.2f;
    float branch_radius = radius;

    if (branch.children.empty()) {
      branch_radius = 0.0f;
    } else {
      branch_radius = branch.children.size() * 0.0002f;
      branch_radius = 0.95f * (pow(branch_radius - 1.0f, 3.0f)) + 1.0f;
    }

    branch.vertices.clear();
    for (int j = 0; j < num_vertices; j++) {
      float angle = j * angle_increment;
      float x = branch_radius * cos(angle);
      float z = branch_radius * sin(angle);

      cgra::mesh_vertex vertex;

      float angle_between_normals = glm::acos(
          glm::dot(glm::vec3(0, 1, 0), normalize(branch.m_direction)));

      glm::mat4 rotation_matrix =
          glm::rotate(glm::mat4(1.0f), angle_between_normals,
                      cross(glm::vec3(0, 1, 0), normalize(branch.m_direction)));
      glm::vec4 rotated_vertex = rotation_matrix * glm::vec4(x, 0.0f, z, 1.0f);

      vertex.pos = branch.m_end + glm::vec3(rotated_vertex);
      vertex.norm = branch.m_end + glm::vec3(rotated_vertex);
      vertex.uv = glm::vec2(0, 0);

      branch.vertices.push_back(builder.m_vertices.size());
      builder.push_vertex(vertex);
    }
  }

  // Assigns indices for each of the branches and their parent branch.
  for (int r = 1; r < m_branches.size(); r++) {
    branch b = m_branches[r];
    branch parent = m_branches[b.m_parent];
    for (int v = 0; v < num_vertices - 1; v++) {
      unsigned int k1 = b.vertices[v];
      unsigned int k1_1 = b.vertices[v + 1];
      unsigned int k2 = parent.vertices[v];
      unsigned int k2_1 = parent.vertices[v + 1];

      builder.push_indices({k1, k2, k1_1});
      builder.push_indices({k1_1, k2, k2_1});
    }

    unsigned int k1 = b.vertices[0];
    unsigned int k1_1 = b.vertices[b.vertices.size() - 1];
    unsigned int k2 = parent.vertices[0];
    unsigned int k2_1 = parent.vertices[parent.vertices.size() - 1];

    builder.push_indices({k1, k2, k1_1});
    builder.push_indices({k1_1, k2, k2_1});
  }

  // assigns indices for the initial branches.
  branch b = m_branches[1];
  for (int v = 0; v < num_vertices - 1; v++) {
    unsigned int k1 = b.vertices[v];
    unsigned int k1_1 = b.vertices[v + 1];
    unsigned int k2 = initial_circle[v];
    unsigned int k2_1 = initial_circle[v + 1];

    builder.push_indices({k1, k2, k1_1});
    builder.push_indices({k1_1, k2, k2_1});
  }

  unsigned int k1 = b.vertices[0];
  unsigned int k1_1 = b.vertices[b.vertices.size() - 1];
  unsigned int k2 = initial_circle[0];
  unsigned int k2_1 = initial_circle[initial_circle.size() - 1];

  builder.push_indices({k1, k2, k1_1});
  builder.push_indices({k1_1, k2, k2_1});

  m_mesh = builder.build();
}

// Draws the mesh
auto tree::draw_tree(const glm::mat4& view, const glm::mat4& projection) const
    -> void {
  glm::mat4 model_view = view * (glm::scale(m_model_translate, m_model_scale));

  glUseProgram(m_shader);
  glUniformMatrix4fv(glGetUniformLocation(m_shader, "uProjectionMatrix"), 1,
                     false, value_ptr(projection));
  glUniformMatrix4fv(glGetUniformLocation(m_shader, "uModelViewMatrix"), 1,
                     false, value_ptr(model_view));
  glUniform3fv(glGetUniformLocation(m_shader, "uColor"), 1,
               glm::value_ptr(glm::vec3(0.28f, 0.23f, 0.15f)));

  m_mesh.draw();
}

// calculates the positions of the foliage by getting a random size and color
// and assigning its position to that of the very end of branches
auto tree::calculate_foliage() -> void {
  m_leaves.clear();

  std::random_device rd;
  std::mt19937 gen(rd());

  std::uniform_real_distribution<float> col_dis(0.4f, 0.8f);
  std::uniform_real_distribution<float> size_dis(0.5f, 1.0f);

  for (branch b : m_branches) {
    if (b.children.empty()) {
      leaf new_leaf(glm::vec3(0.0f, col_dis(gen), 0.0f),
                    glm::vec3(size_dis(gen)), b.m_end);
      m_leaves.push_back(new_leaf);
    }
  }
}

// Draws the Foliage for each of the leaves
void tree::draw_foliage(const glm::mat4& view) const {
  glUseProgram(m_shader);

  for (leaf le : m_leaves) {
    glm::mat4 position_matrix =
        glm::translate(view, le.m_leaf_pos * m_model_scale);
    position_matrix = position_matrix * m_model_translate;
    glm::mat4 scaling_matrix = glm::scale(position_matrix, le.m_leaf_size);
    glm::mat4 sphere_model_matrix = glm::scale(scaling_matrix, m_model_scale);

    glUseProgram(m_shader);

    if (const GLuint sphere_model_view_matrix_location =
            glGetUniformLocation(m_shader, "uModelViewMatrix");
        sphere_model_view_matrix_location != -1) {
      glUniformMatrix4fv(sphere_model_view_matrix_location, 1, false,
                         glm::value_ptr(sphere_model_matrix));
      glUniform3fv(glGetUniformLocation(m_shader, "uColor"), 1,
                   value_ptr(le.m_leaf_color));
    }

    cgra::draw_sphere();
  }

  glUseProgram(0);
}
