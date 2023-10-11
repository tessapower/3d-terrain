#pragma once

#include <vector>

class Perlin {
public:
  // initialize with set or seeded permutation vector
  Perlin(unsigned int seed, unsigned int octaves, float lacunarity, float persistence, unsigned int repeat);

  [[nodiscard]] float perlin(float x, float y, float z) const;

private:
  std::vector<int> p;

  unsigned int repeat;

  // octave params
  unsigned int octaves;
  float lacunarity;
  float persistence;

  [[nodiscard]] float noise(float x, float y, float z) const;

  // smooth transition between gradients with ease curve
  static float fade(float t);

  // linearly interpolate between a and b
  // weight w should be a float between 0 and 1
  static float lerp(float a, float b, float w);

  static float grad(int hash, float x, float y, float z);

  // increment number to ensure repetition
  [[nodiscard]] int inc(int num) const;

  // debug
  static void debug(std::string const& str, auto val);
};
