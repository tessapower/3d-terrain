# Source files
set(sources
	"camera.hpp"
	"intersections.hpp"
	"opengl.hpp"
	"perlin_noise.cpp"
	"perlin_noise.hpp"
	"texture_loader.cpp"
	"texture_loader.hpp"
	"skybox.hpp"
	"skybox.cpp"
	"CMakeLists.txt"
)

# Add these sources to the project target
target_relative_sources(${CGRA_PROJECT} ${sources})
