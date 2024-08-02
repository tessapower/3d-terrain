set(UTIL_SOURCES
	"perlin_noise.cpp"
	"texture_loader.cpp"
	"skybox.cpp"
	"CMakeLists.txt"
)

set(UTIL_HEADERS
	"${PROJECT_SOURCE_DIR}/include/utils/texture_loader.hpp"
	"${PROJECT_SOURCE_DIR}/include/utils/skybox.hpp"
	"${PROJECT_SOURCE_DIR}/include/utils/perlin_noise.hpp"
	"${PROJECT_SOURCE_DIR}/include/utils/camera.hpp"
	"${PROJECT_SOURCE_DIR}/include/utils/intersections.hpp"
	"${PROJECT_SOURCE_DIR}/include/utils/opengl.hpp"
)

add_library(utils_lib STATIC
    ${UTIL_SOURCES}
    ${UTIL_HEADERS}
)

target_link_libraries(utils_lib PRIVATE glew glfw ${GLFW_LIBRARIES})
target_link_libraries(utils_lib PRIVATE stb imgui)

# Support <my_lib/my_lib.h> imports in public headers
target_include_directories(utils_lib PUBLIC ../include)
# Support "my_lib.h" imports in private headers and source files
target_include_directories(utils_lib PRIVATE ../include/utils)

source_group("src" FILES ${UTIL_SOURCES})
