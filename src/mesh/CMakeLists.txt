
# Source files
set(sources
	"mesh_deformation.cpp"
	"mesh_deformation.hpp"
	"simplified_mesh.cpp"
	"simplified_mesh.hpp"
	"simplified_mesh_debugging.hpp"
	"CMakeLists.txt"
)

# Add these sources to the project target
target_relative_sources(${CGRA_PROJECT} ${sources})
