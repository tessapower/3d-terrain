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
} f_in;

// Framebuffer output
out vec4 fb_color;

// Const data
const float PI             = 3.14159f;
const float blend_amount   = 25.0f;
const float roughness_2    = pow(0.1f, 2.0f);
const vec3 light_color     = vec3(1.0f);
const float shininess      = 32.0f;
const float diffuse_scale  = 0.7f;
const float specular_scale = 0.3f;
const vec3 light_src       = vec3(0.0f, 100.0f, 0.0f);

// Materials
const float mirror   = 0.0f;
const float air      = 1.0f;
const float bubble   = 1.1f;
const float ice      = 1.31f;
const float water    = 1.33f;
const float glass    = 1.5f;
const float standard = 2.0f;
const float steel    = 2.5f;

float cook_torrance(const float n_dot_v, const float n_dot_l, const float n_dot_h, const float v_dot_h);
float oren_nayar(const float n_dot_v, const float n_dot_l);

void main() {
    vec3 normal = normalize(f_in.normal);
    vec3 light_dir = normalize(light_src - f_in.position);
    vec3 view_dir = normalize(-f_in.position);

    // Ambient
    vec3 ambient = 0.1f * uColor;

    // Diffuse
    float diff = max(dot(light_dir, normal), 0.0f);
    vec3 diffuse = diff * uColor;
    
    vec3 halfway_dir = normalize(light_dir + view_dir);

    // Specular
    float spec = pow(max(dot(normal, halfway_dir), 0.0f), shininess);
    vec3 specular = light_color * spec;

    float n_dot_v = max(dot(normal, view_dir), 0.01f);
    float n_dot_l = max(dot(normal, light_dir), 0.01f); 
    float n_dot_h = max(dot(normal, halfway_dir), 0.01f); 
    float v_dot_h = max(dot(view_dir, halfway_dir), 0.01f);

    specular = vec3(cook_torrance(n_dot_v, n_dot_l, n_dot_h, v_dot_h));
    diffuse = oren_nayar(n_dot_v, n_dot_l) * uColor;

    vec3 color = ambient + light_color * n_dot_l * (diffuse_scale * diffuse + specular_scale * specular);

    // output to the framebuffer
    fb_color = vec4(color, 1.0f);
}

float cook_torrance(const float n_dot_v, const float n_dot_l, const float n_dot_h, const float v_dot_h) {
    float d = 0.0f;
    float f = 0.0f;
    float g = 0.0f;

    // Calculate the microfacet distribution
    float alpha = roughness_2;
    float alpha_2 = alpha * alpha;
    float n_dot_h_2 = n_dot_h * n_dot_h;
    float n_dot_h_4 = n_dot_h_2 * n_dot_h_2;
    float alpha_2_div_n_dot_h_2 = alpha_2 / n_dot_h_2;
    float d_denom = PI * n_dot_h_4 * alpha_2;
    d = alpha_2_div_n_dot_h_2 * exp((n_dot_h_2 - 1.0f) / d_denom) / d_denom;

    // Calculate the fresnel term
    float f_0 = 0.04f;
    f = f_0 + (1.0f - f_0) * pow(1.0f - v_dot_h, 5.0f);

    // Calculate the geometric attenuation
    float k = alpha_2 / 2.0f;
    float g_1 = n_dot_v * sqrt(k + (1.0f - k) * n_dot_v * n_dot_v);
    float g_2 = n_dot_l * sqrt(k + (1.0f - k) * n_dot_l * n_dot_l);
    g = 1.0f / (g_1 * g_2);

    return d * f * g;
}

float oren_nayar(const float n_dot_v, const float n_dot_l) {
    float angle_v_n = acos(n_dot_v);
    float angle_l_n = acos(n_dot_l);

    float alpha = max(angle_v_n, angle_l_n);
    float beta = min(angle_v_n, angle_l_n);
    float gamma = cos(angle_v_n - angle_l_n);

    float A = 1.0f - 0.5f * (roughness_2 / (roughness_2 + 0.57f));
    float B = 0.45f * (roughness_2 / (roughness_2 + 0.09f));
    float C = sin(alpha) * tan(beta);

    return n_dot_l * (A + (B * max(0.0f, gamma) * C));
}