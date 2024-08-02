set(TERRAIN_SOURCES
	"terrain_model.cpp"
	"CMakeLists.txt"
)

set(TERRAIN_HEADERS
	"${PROJECT_SOURCE_DIR}/include/terrain/terrain_model.hpp"
)

add_library(terrain_lib STATIC
    ${TERRAIN_SOURCES}
    ${TERRAIN_HEADERS}
)

target_link_libraries(terrain_lib PRIVATE glew glfw ${GLFW_LIBRARIES})
target_link_libraries(terrain_lib PRIVATE stb imgui)

# Support <my_lib/my_lib.h> imports in public headers
target_include_directories(terrain_lib PUBLIC ../include)
# Support "my_lib.h" imports in private headers and source files
target_include_directories(terrain_lib PRIVATE ../include/terrain)

source_group("src" FILES ${TERRAIN_SOURCES})
