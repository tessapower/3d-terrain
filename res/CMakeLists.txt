# add resources as targets so they show up in IDEs

# list your shader source files here
SET(SHADERS
	"shaders/color_frag.glsl"
	"shaders/color_vert.glsl"
	"shaders/skybox.fs.glsl"
	"shaders/skybox.vs.glsl"
	"shaders/terrain.fs.glsl"
	"shaders/terrain.vs.glsl"
)

add_custom_target(
	res
	SOURCES
	${SHADERS}
)

source_group("shader" FILES ${SHADERS})
