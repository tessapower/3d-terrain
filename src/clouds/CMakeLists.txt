set(CLOUD_SOURCES
	"cloud_model.cpp"
	"CMakeLists.txt"
)

set(CLOUD_HEADERS
	"${PROJECT_SOURCE_DIR}/include/clouds/cloud_model.hpp"
)

add_library(cloud_lib STATIC
    ${CLOUD_SOURCES}
    ${CLOUD_HEADERS}
)

target_link_libraries(cloud_lib PRIVATE glew glfw ${GLFW_LIBRARIES})
target_link_libraries(cloud_lib PRIVATE stb imgui)

# Support <my_lib/my_lib.h> imports in public headers
target_include_directories(cloud_lib PUBLIC ../include)
# Support "my_lib.h" imports in private headers and source files
target_include_directories(cloud_lib PRIVATE ../include/clouds)

source_group("src" FILES ${CLOUD_SOURCES})
