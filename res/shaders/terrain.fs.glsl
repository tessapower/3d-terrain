#version 330 core

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;

// viewspace data (this must match the output of the fragment shader)
in VertexData {
    vec3 position;
    vec3 normal;
    vec2 textureCoord;
    vec3 vertexPosition;
    vec3 tangent;
    vec3 bitangent;
    vec3 worldNormal;  // World-space normal for side detection
} f_in;

// framebuffer output
out vec4 fb_color;
uniform vec3 uCenter; // Center point for selection
uniform float uRadius; // Radius for selection

uniform sampler2D uTexture1;
uniform sampler2D uTexture2;
uniform sampler2D uTexture3;

uniform sampler2D uNormTex1;
uniform sampler2D uNormTex2;
uniform sampler2D uNormTex3;

uniform float uHeightChange1;
uniform float uHeightChange2;

uniform int uTex;

uniform sampler2D uLeavesTexture;
uniform sampler2D uBarkTexture;

uniform int uType;

// const data
const float PI = 3.14159;
const float mirror = 0.0;
const float air = 1.0;
const float bubble = 1.1;
const float ice = 1.31;
const float water = 1.33;
const float glass = 1.5;
const float standard = 2.0;
const float steel = 2.5;

float cook_torrance(float NdotV, float NdotL, float NdotH, float VdotH) {
    // Fresnel Term
    float n1 = air;
    float n2 = standard;
    float F0 = (n1 - n2) / (n1 + n2);
    F0 = F0 * F0;
    float F = F0 + (1 - F0) * pow(1.0 - VdotH, 5.0);

    // Normal Distribution - Beckmann
    float roughness2 = pow(0.1, 2.0f);
    float angle = acos(NdotH);
    float tangent2 = -pow(tan(angle), 2.0f);
    float e_exponent = exp(tangent2 / roughness2);
    float D = e_exponent  / (PI * roughness2 * pow(cos(angle), 4.0f));

    // Geometric Attenuation Factor
    float G = min(1.0f, min(2.0f * NdotH * NdotV / VdotH, 2.0f * NdotH * NdotL / VdotH));

    return (D * F * G) / (4.0f * NdotL * NdotV);
}

float oren_nayar(float NdotV, float NdotL){
    float roughness2 = pow(0.1, 2.0f);
    float angleVN = acos(NdotV);
    float angleLN = acos(NdotL);
    float alpha = max(angleVN, angleLN);
    float beta = min(angleVN, angleLN);
    float gamma = cos(angleVN - angleLN);
    float A = 1.0 - 0.5 * (roughness2 / (roughness2 + 0.57));
    float B = 0.45 * (roughness2 / (roughness2 + 0.09));
    float C = sin(alpha) * tan(beta);

    return NdotL * (A + (B * max(0.0, gamma) * C));
}

vec3 textureColour(sampler2D tex, sampler2D norm){
    vec3 baseCol = vec3(texture(tex, f_in.textureCoord));
    vec3 normCol = texture(norm, f_in.textureCoord).rgb;
    vec3 normal = normalize(f_in.normal);
    vec3 lightCol = vec3(1);
    vec3 color = baseCol;

    if (uTex == 1) {
        // TBN
        mat3 TBN = mat3(normalize(f_in.tangent), normalize(f_in.bitangent), normalize(f_in.normal));
        normal = normCol * 2.0 - 1.0; 
        normal = normalize(TBN * normal);
    }

    // ambient
    vec3 ambient = 0.1 * color;	
    // diffuse
    vec3 lightDir = normalize(vec3(0, 100, 0) - f_in.position);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // directions
    vec3 viewDir = normalize(-f_in.position);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    // specular
    float shininess = 32;
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specular = lightCol * spec;

    float NdotV = max(dot(normal, viewDir), 0.01);
    float NdotL = max(dot(normal, lightDir), 0.01); 
    float NdotH = max(dot(normal, halfwayDir), 0.01); 
    float VdotH = max(dot(viewDir, halfwayDir), 0.01);

    // output to the framebuffer
    float rs = cook_torrance(NdotV, NdotL, NdotH, VdotH);
    vec3 on = oren_nayar(NdotV, NdotL) * color;

    specular = vec3(rs);
    diffuse = on;

    // result
    float d = 0.7;
    float s = 0.3;
    return ambient + lightCol * NdotL * (d * diffuse + s * specular);
}

void main() {

    if (uType == 0){ // terrain
        // Check if this is a side face (normal not pointing up)
        // worldNormal comes from vertex shader to avoid expensive inverse in fragment shader
        
        // If normal is not pointing mostly upward, render as black (sides and bottom)
        if (f_in.worldNormal.y < 0.5) {
            fb_color = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        }

        vec3 selectColor = vec3(0,0.5,1); //blue

        // Sample textures
        vec4 texture1Color = vec4(textureColour(uTexture1, uNormTex1), 1);
        vec4 texture2Color = vec4(textureColour(uTexture2, uNormTex2), 1);
        vec4 texture3Color = vec4(textureColour(uTexture3, uNormTex3), 1);

        float y = f_in.vertexPosition[1];
        float blendAmount = 25;

        float blendFactor = clamp((uHeightChange1-y) / blendAmount, 0.0, 1.0);
        // Blend the two textures based on the blend factor
        vec4 textureColor = mix(texture1Color, texture2Color, blendFactor);

        blendFactor = clamp((uHeightChange2-y) / blendAmount, 0.0, 1.0);
        // Blend the two textures based on the blend factor
        textureColor = mix(textureColor, texture3Color, blendFactor);

        vec3 color = vec3(textureColor);
    
        // distance using x and z coordinates
        float distance_2 = (f_in.vertexPosition[0] - uCenter[0]) * (f_in.vertexPosition[0] - uCenter[0]) +
            (f_in.vertexPosition[2] - uCenter[2]) * (f_in.vertexPosition[2] - uCenter[2]);

        if (distance_2 <= (uRadius * uRadius)) {
            // Inside the selection radius: Set color to blue
            color = (selectColor * 0.2) + (color * 0.8);
        }
        // output to the frambuffer
        fb_color = vec4(color, 1);
    } else if (uType == 1) { // leaves
        fb_color = vec4(textureColour(uLeavesTexture, uLeavesTexture), 1); // ignore normal map as no tang/bitang
    } else if (uType == 2) { // wood
        fb_color = vec4(textureColour(uBarkTexture, uBarkTexture), 1); // ignore normal map as no tang/bitang
    } else if (uType == -1) { 		// basic lighting
        // calculate lighting (hack)
        vec3 eye = normalize(-f_in.position);
        float light = abs(dot(normalize(f_in.normal), eye));
        vec3 color = mix(uColor / 4, uColor, light);

        // output to the framebuffer
        fb_color = vec4(color, 1);
    }

}