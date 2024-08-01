#ifndef CGRA_WAVEFRONT_HPP
#define CGRA_WAVEFRONT_HPP

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "cgra_mesh.hpp"

namespace cgra {
inline mesh_builder load_wavefront_data(const std::string& file_name) {
  using namespace std;
  using namespace glm;

  // struct for storing wavefront index data
  struct wavefront_vertex {
    unsigned int p, n, t;
  };

  // create reading buffers
  vector<vec3> positions;
  vector<vec3> normals;
  vector<vec2> uvs;
  vector<wavefront_vertex> wv_vertices;

  // open file
  ifstream obj_file(file_name);
  if (!obj_file.is_open()) {
    cerr << "Error: could not open " << file_name << '\n';
    throw runtime_error("Error: could not open file " + file_name);
  }

  // good() means that failbit, badbit and eofbit are all not set
  while (obj_file.good()) {
    // Pull out line from file
    string line;
    getline(obj_file, line);
    istringstream obj_line(line);

    // Pull out mode from line
    string mode;
    obj_line >> mode;

    // Reading like this means whitespace at the start of the line is fine
    // attempting to read from an empty string/line will set the failbit
    if (obj_line.good()) {
      if (mode == "v") {
        vec3 v;
        obj_line >> v.x >> v.y >> v.z;
        positions.push_back(v);
      } else if (mode == "vn") {
        vec3 vn;
        obj_line >> vn.x >> vn.y >> vn.z;
        normals.push_back(vn);

      } else if (mode == "vt") {
        vec2 vt;
        obj_line >> vt.x >> vt.y;
        uvs.push_back(vt);

      } else if (mode == "f") {
        std::vector<wavefront_vertex> face;
        while (obj_line.good()) {
          wavefront_vertex v;

          // scan in position index
          obj_line >> v.p;
          if (obj_line.fail()) break;

          // look ahead for a match
          if (obj_line.peek() == '/') {
            // ignore the '/' character
            obj_line.ignore(1);

            // scan in uv (texture coordinate) index (if it's there)
            if (obj_line.peek() != '/') {
              obj_line >> v.t;
            }

            // scan in normal index (if it's there)
            if (obj_line.peek() == '/') {
              obj_line.ignore(1);
              obj_line >> v.n;
            }
          }

          // subtract 1 because of wavefront indexing
          v.p -= 1;
          v.n -= 1;
          v.t -= 1;

          face.push_back(v);
        }

        // IFF we have 3 vertices, construct a triangle
        if (face.size() == 3) {
          for (int i = 0; i < 3; ++i) {
            wv_vertices.push_back(face[i]);
          }
        }
      }
    }
  }

  // if we don't have any normals, create them naively
  if (normals.empty()) {
    // Create the normals as 3d vectors of 0
    normals.resize(positions.size(), vec3(0));

    // add the normal for every face to each vertex-normal
    for (size_t i = 0; i < wv_vertices.size() / 3; i++) {
      auto& [a_p, a_n, a_t] = wv_vertices[i * 3];
      auto& [b_p, b_n, b_t] = wv_vertices[i * 3 + 1];
      auto& [c_p, c_n, c_t] = wv_vertices[i * 3 + 2];

      // set the normal index to be the same as position index
      a_n = a_p;
      b_n = b_p;
      c_n = c_p;

      // calculate the face normal
      vec3 a_b = positions[b_p] - positions[a_p];
      vec3 a_c = positions[c_p] - positions[a_p];
      vec3 face_norm = cross(a_b, a_c);

      // contribute the face norm to each vertex
      if (float l = length(face_norm); l > 0) {
        normals[a_n] += face_norm;
        normals[b_n] += face_norm;
        normals[c_n] += face_norm;
      }
    }

    // normalize the normals
    for (auto& normal : normals) {
      normal = normalize(normal);
    }
  }

  // create mesh data
  mesh_builder mb;

  for (unsigned int i = 0; i < wv_vertices.size(); ++i) {
    mb.push_index(i);
    mb.push_vertex(mesh_vertex{
        positions[wv_vertices[i].p], normals[wv_vertices[i].n],
        // Marshall removed texture mapping here
    });
  }

  return mb;
}
}  // namespace cgra

#endif  // CGRA_WAVEFRONT_HPP
