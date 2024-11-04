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

set_property(TARGET mesh_lib PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$(SolutionDir)../")

target_link_libraries(mesh_lib PRIVATE GLEW::GLEW)
target_link_libraries(mesh_lib PRIVATE glfw ${GLFW_LIBRARIES})
target_link_libraries(mesh_lib PRIVATE imgui::imgui)

target_include_directories(mesh_lib PRIVATE ${Stb_INCLUDE_DIR})

# Support <my_lib/my_lib.h> imports in public headers
target_include_directories(mesh_lib PUBLIC ../include)
# Support "my_lib.h" imports in private headers and source files
target_include_directories(mesh_lib PRIVATE ../include/mesh)

# Add these sources to the project target
source_group("src" FILES ${MESH_SOURCES})
