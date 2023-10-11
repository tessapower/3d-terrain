
#########################################################
# Source Files
#########################################################
SET(sources
	"application.hpp"
	"application.cpp"
	"main.cpp"
	"../ext/PerlinNoise.hpp"
	"CMakeLists.txt"
)

# Add executable target and link libraries
add_executable(${CGRA_PROJECT} ${sources})

add_subdirectory(cgra)
add_subdirectory(clouds)
add_subdirectory(mesh)
add_subdirectory(terrain)
add_subdirectory(trees)
add_subdirectory(utils)

#########################################################
# Link and Build Executable
#########################################################

# Set source groups (helper method)
target_source_group_tree(${CGRA_PROJECT})

# Set working folder
target_compile_definitions(${CGRA_PROJECT} PRIVATE "-DCGRA_SRCDIR=\"${PROJECT_SOURCE_DIR}\"")

# Link usage requirements
target_link_libraries(${CGRA_PROJECT} PRIVATE glew glfw ${GLFW_LIBRARIES})
target_link_libraries(${CGRA_PROJECT} PRIVATE stb imgui)

# For experimental <filesystem>
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	target_link_libraries(${CGRA_PROJECT} PRIVATE -lstdc++fs)
endif()
