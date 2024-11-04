# Add all source files in dir
file(GLOB sources *.c *.cpp)

list(APPEND
    sources
    "CMakeLists.txt"
)

set(
    headers
    "${PROJECT_SOURCE_DIR}/include/application.hpp"
)

add_subdirectory(cgra)
add_subdirectory(clouds)
add_subdirectory(mesh)
add_subdirectory(terrain)
add_subdirectory(trees)
add_subdirectory(utils)

# Add executable target and link libraries
add_executable(${CGRA_PROJECT} ${sources} ${headers})

set_property(TARGET ${CGRA_PROJECT} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$(SolutionDir)../work")

#########################################################
# Link and Build Executable
#########################################################

# Set source groups (helper method)
target_source_group_tree(${CGRA_PROJECT})

# Set working folder
target_compile_definitions(${CGRA_PROJECT} PRIVATE "-DCGRA_SRCDIR=\"${PROJECT_SOURCE_DIR}\"")

# Link external libraries
target_link_libraries(${CGRA_PROJECT} PRIVATE GLEW::GLEW)
target_link_libraries(${CGRA_PROJECT} PRIVATE glfw ${GLFW_LIBRARIES})
target_link_libraries(${CGRA_PROJECT} PRIVATE glm::glm)
target_link_libraries(${CGRA_PROJECT} PRIVATE imgui::imgui)

target_include_directories(${CGRA_PROJECT} PRIVATE ${Stb_INCLUDE_DIR})

# Link local libraries
target_link_libraries(${CGRA_PROJECT} PRIVATE cgra_lib)
target_link_libraries(${CGRA_PROJECT} PRIVATE cloud_lib)
target_link_libraries(${CGRA_PROJECT} PRIVATE mesh_lib)
target_link_libraries(${CGRA_PROJECT} PRIVATE terrain_lib)
target_link_libraries(${CGRA_PROJECT} PRIVATE tree_lib)
target_link_libraries(${CGRA_PROJECT} PRIVATE utils_lib)

# For experimental <filesystem>
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	target_link_libraries(${CGRA_PROJECT} PRIVATE -lstdc++fs)
endif()
