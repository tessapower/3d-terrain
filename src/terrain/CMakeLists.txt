# Source files
set(sources
	"terrain_model.cpp"
	"terrain_model.hpp"
	"CMakeLists.txt"
)

# Add these sources to the project target
target_relative_sources(${CGRA_PROJECT} ${sources})
