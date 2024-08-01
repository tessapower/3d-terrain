#ifndef PERLIN_NOISE_HPP
#define PERLIN_NOISE_HPP

#include <vector>

class perlin {
 public:
  // initialize with set or seeded permutation vector
  perlin(unsigned int seed, unsigned int octaves, float lacunarity,
         float persistence, unsigned int repeat);

  [[nodiscard]] auto generate_perlin(float x, float y, float z) const -> float;

 private:
  std::vector<int> m_p_;

  unsigned int m_repeat_;

  // octave params
  unsigned int m_octaves_;
  float m_lacunarity_;
  float m_persistence_;

  [[nodiscard]] auto noise(float x, float y, float z) const -> float;

  // smooth transition between gradients with ease curve
  static auto fade(float t) -> float;

  // linearly interpolate between a and b
  // weight w should be a float between 0 and 1
  static auto lerp(float a, float b, float w) -> float;

  static auto grad(int hash, float x, float y, float z) -> float;

  // increment number to ensure repetition
  [[nodiscard]] auto inc(int num) const -> int;

  // debug
  static auto debug(std::string const& str, auto val) -> void;
};

#endif  // PERLIN_NOISE_HPP
