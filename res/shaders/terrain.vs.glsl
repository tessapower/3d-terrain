#version 330 core

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;
uniform sampler2D uHeightTexture1;
uniform sampler2D uHeightTexture2;
uniform sampler2D uHeightTexture3;
uniform float uHeightChange1;
uniform float uHeightChange2;
uniform float uHeightScale;

uniform sampler2D uLeavesHeight;
uniform int uType;

// mesh data
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

// model data (this must match the input of the vertex shader)
out VertexData {
    vec3 position;
    vec3 normal;
    vec2 textureCoord;
    vec3 vertexPosition;
    vec3 tangent;
    vec3 bitangent;
} v_out;

void main() {
    vec3 new_position = aPosition;

    // if terrain mesh
    if (uType != -1) { 
        // Sample the heightmap texture to get the height value
        float y = aPosition[1];
        float height = 0.0f;
        if (uType == 1) {
            height = texture(uLeavesHeight, aTexCoord).r;
        } else if (y > uHeightChange1) {
            height = texture(uHeightTexture1, aTexCoord).r;
        } else if (y > uHeightChange2) {
            height = texture(uHeightTexture2, aTexCoord).r;
        } else {
            height = texture(uHeightTexture3, aTexCoord).r;
        }

        height = (height * 2.0f) - 1.0f; // range -1 to 1
        if (uType == 1) height /= 5.0f;
        if (uType == 2) height = 0.0f;
        // Adjust the position based on the height and terrain scale, using the normal direction
        new_position = aPosition + (aNormal * height * uHeightScale);
    }

    // transform vertex data to viewspace
    v_out.position = (uModelViewMatrix * vec4(new_position, 1.0f)).xyz;
    v_out.normal = normalize((uModelViewMatrix * vec4(aNormal, 0.0f)).xyz);
    v_out.textureCoord = aTexCoord;
    v_out.vertexPosition = new_position;
    v_out.tangent = normalize((uModelViewMatrix * vec4(aTangent, 0.0f)).xyz);
    v_out.bitangent = normalize((uModelViewMatrix * vec4(aBitangent, 0.0f)).xyz);

    // set the screenspace position (needed for converting to fragment data)
    gl_Position = uProjectionMatrix * uModelViewMatrix * vec4(new_position, 1);
}