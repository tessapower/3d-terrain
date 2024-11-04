# CGRA Framework Source files
set(CGRA_SOURCES
    "cgra_geometry.cpp"
    "cgra_gui.cpp"
    "cgra_image.cpp"
    "cgra_mesh.cpp"
    "cgra_shader.cpp"
)

set(CGRA_HEADERS
    "${PROJECT_SOURCE_DIR}/include/cgra/cgra_basic_model.hpp"
    "${PROJECT_SOURCE_DIR}/include/cgra/cgra_geometry.hpp"
    "${PROJECT_SOURCE_DIR}/include/cgra/cgra_gui.hpp"
    "${PROJECT_SOURCE_DIR}/include/cgra/cgra_image.hpp"
    "${PROJECT_SOURCE_DIR}/include/cgra/cgra_mesh.hpp"
    "${PROJECT_SOURCE_DIR}/include/cgra/cgra_shader.hpp"
    "${PROJECT_SOURCE_DIR}/include/cgra/cgra_wavefront.hpp"
)

add_library(cgra_lib STATIC
    ${CGRA_SOURCES}
    ${CGRA_HEADERS}
)

set_property(TARGET cgra_lib PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$(SolutionDir)../")

target_link_libraries(cgra_lib PRIVATE GLEW::GLEW)
target_link_libraries(cgra_lib PRIVATE glfw ${GLFW_LIBRARIES})
target_link_libraries(cgra_lib PRIVATE imgui::imgui)

target_include_directories(cgra_lib PRIVATE ${Stb_INCLUDE_DIR})

# Support <my_lib/my_lib.h> imports in public headers
target_include_directories(cgra_lib PUBLIC ../include)
# Support "my_lib.h" imports in private headers and source files
target_include_directories(cgra_lib PRIVATE ../include/cgra)

# Add these sources to the project target
source_group("src" FILES ${CGRA_SOURCES})
