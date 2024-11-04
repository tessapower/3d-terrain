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

set_property(TARGET terrain_lib PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$(SolutionDir)../")

target_link_libraries(terrain_lib PRIVATE GLEW::GLEW)
target_link_libraries(terrain_lib PRIVATE glfw ${GLFW_LIBRARIES})
target_link_libraries(terrain_lib PRIVATE imgui::imgui)

target_include_directories(terrain_lib PRIVATE ${Stb_INCLUDE_DIR})

# Support <my_lib/my_lib.h> imports in public headers
target_include_directories(terrain_lib PUBLIC ../include)
# Support "my_lib.h" imports in private headers and source files
target_include_directories(terrain_lib PRIVATE ../include/terrain)

source_group("src" FILES ${TERRAIN_SOURCES})
