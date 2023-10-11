#version 330 core

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;

layout(location = 0) in vec3 aPosition;
out vec3 texture_coords;

void main() {
    texture_coords = aPosition;
    gl_Position = (uProjectionMatrix * uModelViewMatrix * vec4(aPosition, 1.0f)).xyww;
}
