#include "clouds/cloud_model.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "PerlinNoise.hpp"
#include "cgra/cgra_wavefront.hpp"
#include "mesh/simplified_mesh.hpp"

auto cloud_falloff(const float x) -> float {
  return 1.0f - ((x - 1.0f) * (x - 1.0f));
}

auto cloud_model::simulate() -> void {
  // Generate volumetric data
  cloud_data =
      std::vector(size.x, std::vector(size.y, std::vector(size.z, 1.0f)));

  const siv::PerlinNoise perlin{0};

  for (int x = 0; x < size.x; x++) {
    for (int y = 0; y < size.y; y++) {
      for (int z = 0; z < size.z; z++) {
        double noise = perlin.octave3D_01(
            (x / noise_scale.x), y / noise_scale.y, z / noise_scale.z, 3);

        // Fade out at top and bottom
        if (y > size.y - fade_out_range) {
          noise *= cloud_falloff(1.0f - (y - (size.y - fade_out_range)) /
                                           fade_out_range);
        } else if (y < fade_out_range) {
          noise *= cloud_falloff(y / fade_out_range);
        }

        // 1 is not solid, 0 is solid
        cloud_data[x][y][z] = noise > cloud_threshold ? 0.0f : 1.0f;
      }
    }
  }

  std::cout << "Finished building noise map\n";

  mesh.m_grid = cloud_data;
  mesh.m_bb_top_right = size;
  mesh.m_bb_bottom_left = glm::vec3(0.0f);
  mesh.m_voxel_edge_length = 1.0f;
  mesh.build();
}

auto cloud_model::draw(const glm::mat4& view, const glm::mat4& projection)
    -> void {
  glm::mat4 viewmodel = glm::scale(view, glm::vec3(5.0f));
  viewmodel = glm::translate(viewmodel,
                             glm::vec3(size.x / -2.0f, 40.0f, size.z / -2.0f));
  mesh.m_shader = m_shader;
  mesh.draw(viewmodel, projection);
}
