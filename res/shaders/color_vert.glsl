#version 330 core

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;

// Mesh data
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

out VertexData {
    vec3 position;
    vec3 normal;
    vec2 textureCoord;
    vec3 vertexPosition;
    vec3 tangent;
    vec3 bitangent;
} v_out;

void main() {
    // Transform vertex data to viewspace
    v_out.position = (uModelViewMatrix * vec4(aPosition, 1.0)).xyz;
    v_out.normal = normalize((uModelViewMatrix * vec4(aNormal, 0.0)).xyz);
    v_out.textureCoord = aTexCoord;
    v_out.vertexPosition = aPosition;
    v_out.tangent = normalize((uModelViewMatrix * vec4(aTangent, 0.0)).xyz);
    v_out.bitangent = normalize((uModelViewMatrix * vec4(aBitangent, 0.0)).xyz);

    // set the screenspace position (needed for converting to fragment data)
    gl_Position = uProjectionMatrix * uModelViewMatrix * vec4(aPosition, 1.0);
}