SET(SHADERS
	"shaders/color_fs.glsl"
	"shaders/color_vs.glsl"
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
