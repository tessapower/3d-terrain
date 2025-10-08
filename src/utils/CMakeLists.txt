set(UTIL_SOURCES
    "perlin_noise.cpp"
    "texture_loader.cpp"
    "skybox.cpp"
    "CMakeLists.txt"
)

set(UTIL_HEADERS
    "${PROJECT_SOURCE_DIR}/include/utils/camera.hpp"
    "${PROJECT_SOURCE_DIR}/include/utils/intersections.hpp"
    "${PROJECT_SOURCE_DIR}/include/utils/opengl.hpp"
    "${PROJECT_SOURCE_DIR}/include/utils/perlin_noise.hpp"
    "${PROJECT_SOURCE_DIR}/include/utils/skybox.hpp"
    "${PROJECT_SOURCE_DIR}/include/utils/texture_loader.hpp"
)

add_library(utils_lib STATIC
    ${UTIL_SOURCES}
    ${UTIL_HEADERS}
)

set_property(TARGET utils_lib PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$(SolutionDir)../")

target_link_libraries(utils_lib PRIVATE GLEW::GLEW)
target_link_libraries(utils_lib PRIVATE glfw ${GLFW_LIBRARIES})
target_link_libraries(utils_lib PRIVATE imgui::imgui)

target_include_directories(utils_lib PRIVATE ${Stb_INCLUDE_DIR})

# Support <my_lib/my_lib.h> imports in public headers
target_include_directories(utils_lib PUBLIC ../include)
# Support "my_lib.h" imports in private headers and source files
target_include_directories(utils_lib PRIVATE ../include/utils)
