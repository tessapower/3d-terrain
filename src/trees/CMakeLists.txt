# Source files
set(sources
	"trees.cpp"
	"trees.hpp"
	"CMakeLists.txt"
)

# Add these sources to the project target
target_relative_sources(${CGRA_PROJECT} ${sources})
