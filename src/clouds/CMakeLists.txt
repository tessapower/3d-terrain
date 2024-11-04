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

set_property(TARGET cloud_lib PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$(SolutionDir)../work")

target_link_libraries(cloud_lib PRIVATE GLEW::GLEW)
target_link_libraries(cloud_lib PRIVATE glfw ${GLFW_LIBRARIES})
target_link_libraries(cloud_lib PRIVATE imgui::imgui)

target_include_directories(cloud_lib PRIVATE ${Stb_INCLUDE_DIR})

# Support <my_lib/my_lib.h> imports in public headers
target_include_directories(cloud_lib PUBLIC ../include)
# Support "my_lib.h" imports in private headers and source files
target_include_directories(cloud_lib PRIVATE ../include/clouds)

source_group("src" FILES ${CLOUD_SOURCES})
