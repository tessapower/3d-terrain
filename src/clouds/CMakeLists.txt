
# Source files
set(sources
	"cloud_model.cpp"
	"cloud_model.hpp"
	"CMakeLists.txt"
)

# Add these sources to the project target
target_relative_sources(${CGRA_PROJECT} ${sources})
