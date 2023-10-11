// std
#include <iostream>
#include <numeric>
#include <random>

// project
#include <vector>

#include "perlin_noise.hpp"

// initialize with set or seeded permutation vector
Perlin::Perlin(const unsigned int seed, const unsigned int octaves,
               const float lacunarity, const float persistence,
               const unsigned int repeat)
  : repeat(repeat), octaves(octaves), lacunarity(lacunarity), persistence(persistence)
{
  if (seed == 0) {
    // initialise permutation vector with reference values
    p = {151, 160, 137, 91,  90,  15,  131, 13,  201, 95,  96,  53,  194, 233,
         7,   225, 140, 36,  103, 30,  69,  142, 8,   99,  37,  240, 21,  10,
         23,  190, 6,   148, 247, 120, 234, 75,  0,   26,  197, 62,  94,  252,
         219, 203, 117, 35,  11,  32,  57,  177, 33,  88,  237, 149, 56,  87,
         174, 20,  125, 136, 171, 168, 68,  175, 74,  165, 71,  134, 139, 48,
         27,  166, 77,  146, 158, 231, 83,  111, 229, 122, 60,  211, 133, 230,
         220, 105, 92,  41,  55,  46,  245, 40,  244, 102, 143, 54,  65,  25,
         63,  161, 1,   216, 80,  73,  209, 76,  132, 187, 208, 89,  18,  169,
         200, 196, 135, 130, 116, 188, 159, 86,  164, 100, 109, 198, 173, 186,
         3,   64,  52,  217, 226, 250, 124, 123, 5,   202, 38,  147, 118, 126,
         255, 82,  85,  212, 207, 206, 59,  227, 47,  16,  58,  17,  182, 189,
         28,  42,  223, 183, 170, 213, 119, 248, 152, 2,   44,  154, 163, 70,
         221, 153, 101, 155, 167, 43,  172, 9,   129, 22,  39,  253, 19,  98,
         108, 110, 79,  113, 224, 232, 178, 185, 112, 104, 218, 246, 97,  228,
         251, 34,  242, 193, 238, 210, 144, 12,  191, 179, 162, 241, 81,  51,
         145, 235, 249, 14,  239, 107, 49,  192, 214, 31,  181, 199, 106, 157,
         184, 84,  204, 176, 115, 121, 50,  45,  127, 4,   150, 254, 138, 236,
         205, 93,  222, 114, 67,  29,  24,  72,  243, 141, 128, 195, 78,  66,
         215, 61,  156, 180};
  } else {
    p.resize(256);

    // fill p with int values from 0 to 255
    iota(p.begin(), p.end(), 0);

    // shuffle with seeded random engine
    std::ranges::shuffle(p.begin(), p.end(), std::default_random_engine(seed));
  }

  // duplicate permutation vector
  p.insert(p.end(), p.begin(), p.end());
}

float Perlin::perlin(const float x, const float y, const float z) const {
  float total = 0.0f;
  float frequency = 1.0f;
  float amplitude = 1.0f;
  float max = 0.0f;

  for (unsigned i = 0; i < octaves; i++) {
    total += noise(x * frequency / 200, y * frequency, z * frequency / 200) * amplitude;

    max += amplitude;

    frequency *= lacunarity;
    amplitude *= persistence;
  }

  return total / max;
}

float Perlin::noise(float x, float y, float z) const {
  if (repeat > 0) {
    x = static_cast<float>(fmod(x, repeat));
    y = static_cast<float>(fmod(y, repeat));
    z = static_cast<float>(fmod(z, repeat));
  }

  // define unit cube containing point
  const int xi = static_cast<int>(floor(x)) & 255;
  const int yi = static_cast<int>(floor(y)) & 255;
  const int zi = static_cast<int>(floor(z)) & 255;

  // define relative coordinates of point in unit cube
  const float xf = x - floor(x);
  const float yf = y - floor(y);
  const float zf = z - floor(z);

  // define fade curves for relative coordinates
  const float u = fade(xf);
  const float v = fade(yf);
  const float w = fade(zf);

  // find coordinates of 8 unit cube corners
  const int a  = p[xi]     + yi;
  const int aa = p[a]      + zi;
  const int ab = p[a + 1]  + zi;
  const int b  = p[xi + 1] + yi;
  const int ba = p[b]      + zi;
  const int bb = p[b + 1]  + zi;

  float x1 = lerp(grad(p[aa], xf, yf, zf),      // a
                grad(p[ba], xf - 1.0f, yf, zf), // b
                u);                             // w

  float x2 = lerp(grad(p[ab], xf, yf - 1.0f, zf),        // a
                  grad(p[bb], xf - 1.0f, yf - 1.0f, zf), // b
                u);                                      // w

  const float y1 = lerp(x1, x2, v);

  x1 = lerp(grad(p[aa + 1], xf, yf, zf - 1.0f),        // a
            grad(p[ba + 1], xf - 1.0f, yf, zf - 1.0f), // b
          u);                                          // w

  x2 = lerp(grad(p[ab + 1], xf, yf - 1.0f, zf - 1.0f),        // a
            grad(p[bb + 1], xf - 1.0f, yf - 1.0f, zf - 1.0f), // b
          u);                                                 // w
  const float y2 = lerp(x1, x2, v);

  // Normalize the result
  const float result = (lerp(y1, y2, w) + 1.0f) / 2.0f;

  return result;
}

// smooth transition between gradients with ease curve
float Perlin::fade(const float t) { return t * t * t * (t * (t * 6 - 15) + 10); }

// linearly interpolate between a and b
// weight w should be between 0 and 1
float Perlin::lerp(const float a, const float b, const float w) {
  return (b - a) * w + a;
}

// convert hash code to gradient directions
float Perlin::grad(const int hash, const float x, const float y, const float z) {
  switch (hash & 0xF) {
    case 0x0: return  x + y;
    case 0x1: return -x + y;
    case 0x2: return  x - y;
    case 0x3: return -x - y;
    case 0x4: return  x + z;
    case 0x5: return -x + z;
    case 0x6: return  x - z;
    case 0x7: return -x - z;
    case 0x8: return  y + z;
    case 0x9: return -y + z;
    case 0xA: return  y - z;
    case 0xB: return -y - z;
    case 0xC: return  y + x;
    case 0xD: return -y + z;
    case 0xE: return  y - x;
    case 0xF: return -y - z;
    default:  return 0;
  }
}

// increment number to ensure repetition
int Perlin::inc(int num) const {
  num++;

  if (repeat > 0) num %= repeat;

  return num;
}

// debug
void Perlin::debug(std::string const& str, auto val) {
  std::cout << str << ": " << val << std::endl;
}