set(TREE_SOURCES
	"trees.cpp"
	"CMakeLists.txt"
)

set(TREE_HEADERS
	"${PROJECT_SOURCE_DIR}/include/trees/trees.hpp"
)

add_library(tree_lib STATIC
    ${TREE_SOURCES}
    ${TREE_HEADERS}
)

set_property(TARGET tree_lib PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$(SolutionDir)../work")

target_link_libraries(tree_lib PRIVATE glew)
target_link_libraries(tree_lib PRIVATE glfw ${GLFW_LIBRARIES})
target_link_libraries(tree_lib PRIVATE imgui)
target_link_libraries(tree_lib PRIVATE stb)

# Support <my_lib/my_lib.h> imports in public headers
target_include_directories(tree_lib PUBLIC ../include)
# Support "my_lib.h" imports in private headers and source files
target_include_directories(tree_lib PRIVATE ../include/trees)

source_group("src" FILES ${TREE_SOURCES})
