### Description
Refactor AABB tree rebuilds to use GPU for spatial indexing, enhancing performance during terrain deformation.

### Affected Files
- [`include/terrain/terrain_model.hpp`](https://github.com/tessapower/3d-terrain/blob/main/include/terrain/terrain_model.hpp)
- [`src/application.cpp`](https://github.com/tessapower/3d-terrain/blob/main/src/application.cpp)

### Resources
- [Spatial Partitioning Techniques](https://developer.nvidia.com/sites/default/files/akamai/gamedev/docs/GDC2015/MaxwellTilingRasterization.pdf)
- [AABB GPU Acceleration](https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_chapter32.html)

### Tasks
- Rewrite AABB tree building logic using compute shaders.
- Ensure thread safety and integrate with existing scene updates.
- Benchmark performance before and after the optimization.