# CGRA Framework Source files
set(MESH_SOURCES
	"mesh_deformation.cpp"
	"simplified_mesh.cpp"
	"CMakeLists.txt"
)

set(MESH_HEADERS
    "${PROJECT_SOURCE_DIR}/include/mesh/simplified_mesh_debugging.hpp"
    "${PROJECT_SOURCE_DIR}/include/mesh/simplified_mesh.hpp"
    "${PROJECT_SOURCE_DIR}/include/mesh/mesh_deformation.hpp"
)

add_library(mesh_lib STATIC
    ${MESH_SOURCES}
    ${MESH_HEADERS}
)

target_link_libraries(mesh_lib PRIVATE glew glfw ${GLFW_LIBRARIES})
target_link_libraries(mesh_lib PRIVATE stb imgui)

# Support <my_lib/my_lib.h> imports in public headers
target_include_directories(mesh_lib PUBLIC ../include)
# Support "my_lib.h" imports in private headers and source files
target_include_directories(mesh_lib PRIVATE ../include/mesh)

# Add these sources to the project target
source_group("src" FILES ${MESH_SOURCES})
