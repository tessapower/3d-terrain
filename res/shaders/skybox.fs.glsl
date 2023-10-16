#version 330 core

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform samplerCube skybox;

in vec3 texture_coords;
out vec4 fb_color;

void main() {
    fb_color = texture(skybox, texture_coords);
}
