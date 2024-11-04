#include "cgra/cgra_gui.hpp"

#include <iostream>

#include "imgui.h"

namespace cgra {
namespace {
// internal data
GLFWwindow* g_window = nullptr;
double g_time = 0.0f;
bool g_mouse_pressed[3] = {false, false, false};
float g_mouse_wheel = 0.0f;
GLuint g_font_texture = 0;
int g_shader_handle = 0, g_vertex_handle = 0, g_frag_handle = 0;
int g_attribute_location_tex = 0, g_attribute_location_proj_mtx = 0;
int g_attribute_location_position = 0, g_attribute_location_uv = 0,
    g_attribute_location_color = 0;
unsigned int g_vbo_handle = 0, g_vao_handle = 0, g_elements_handle = 0;

void create_fonts_texture() {
  // build texture atlas
  const ImGuiIO& io = ImGui::GetIO();
  unsigned char* pixels;
  int width, height;

  // Load as RGBA 32-bits (75% of the memory is wasted, but default font is so
  // small) because it is more likely to be compatible with user's existing
  // shaders. If your ImTextureId represent a higher-level concept than just a
  // GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU
  // memory.
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

  // upload texture to graphics system
  GLint last_texture;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  glGenTextures(1, &g_font_texture);
  glBindTexture(GL_TEXTURE_2D, g_font_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, pixels);

  // store our identifier
  io.Fonts->TexID =
      reinterpret_cast<void*>(static_cast<intptr_t>(g_font_texture));

  // restore state
  glBindTexture(GL_TEXTURE_2D, last_texture);
}

bool create_device_objects() {
  // backup GL state
  GLint last_texture, last_array_buffer, last_vertex_array;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

  const auto vertex_shader =
      "#version 330\n"
      "uniform mat4 uProjectionMatrix;\n"
      "in vec2 vPosition;\n"
      "in vec2 vUV;\n"
      "in vec4 vColor;\n"
      "out vec2 Frag_UV;\n"
      "out vec4 Frag_Color;\n"
      "void main() {\n"
      "   Frag_UV = vUV;\n"
      "   Frag_Color = vColor;\n"
      "   gl_Position = uProjectionMatrix * vec4(vPosition.xy,0,1);\n"
      "}\n";

  const auto fragment_shader =
      "#version 330\n"
      "uniform sampler2D Texture;\n"
      "in vec2 Frag_UV;\n"
      "in vec4 Frag_Color;\n"
      "out vec4 Out_Color;\n"
      "void main() {\n"
      "   Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
      "}\n";

  g_shader_handle = glCreateProgram();
  g_vertex_handle = glCreateShader(GL_VERTEX_SHADER);
  g_frag_handle = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(g_vertex_handle, 1, &vertex_shader, nullptr);
  glShaderSource(g_frag_handle, 1, &fragment_shader, nullptr);
  glCompileShader(g_vertex_handle);
  glCompileShader(g_frag_handle);
  glAttachShader(g_shader_handle, g_vertex_handle);
  glAttachShader(g_shader_handle, g_frag_handle);
  glLinkProgram(g_shader_handle);

  g_attribute_location_tex = glGetUniformLocation(g_shader_handle, "Texture");
  g_attribute_location_proj_mtx =
      glGetUniformLocation(g_shader_handle, "uProjectionMatrix");
  g_attribute_location_position =
      glGetAttribLocation(g_shader_handle, "vPosition");
  g_attribute_location_uv = glGetAttribLocation(g_shader_handle, "vUV");
  g_attribute_location_color = glGetAttribLocation(g_shader_handle, "vColor");

  glGenBuffers(1, &g_vbo_handle);
  glGenBuffers(1, &g_elements_handle);

  glGenVertexArrays(1, &g_vao_handle);
  glBindVertexArray(g_vao_handle);
  glBindBuffer(GL_ARRAY_BUFFER, g_vbo_handle);
  glEnableVertexAttribArray(g_attribute_location_position);
  glEnableVertexAttribArray(g_attribute_location_uv);
  glEnableVertexAttribArray(g_attribute_location_color);

#define OFFSETOF(TYPE, ELEMENT) ((size_t) & (((TYPE*)0)->ELEMENT))
  glVertexAttribPointer(g_attribute_location_position, 2, GL_FLOAT, GL_FALSE,
                        sizeof(ImDrawVert),
                        reinterpret_cast<GLvoid*>(OFFSETOF(ImDrawVert, pos)));
  glVertexAttribPointer(g_attribute_location_uv, 2, GL_FLOAT, GL_FALSE,
                        sizeof(ImDrawVert),
                        reinterpret_cast<GLvoid*>(OFFSETOF(ImDrawVert, uv)));
  glVertexAttribPointer(g_attribute_location_color, 4, GL_UNSIGNED_BYTE,
                        GL_TRUE, sizeof(ImDrawVert),
                        reinterpret_cast<GLvoid*>(OFFSETOF(ImDrawVert, col)));
#undef OFFSETOF

  create_fonts_texture();

  // restore modified GL state
  glBindTexture(GL_TEXTURE_2D, last_texture);
  glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
  glBindVertexArray(last_vertex_array);

  return true;
}

auto invalidate_device_objects() -> void {
  if (g_vao_handle) glDeleteVertexArrays(1, &g_vao_handle);
  if (g_vbo_handle) glDeleteBuffers(1, &g_vbo_handle);
  if (g_elements_handle) glDeleteBuffers(1, &g_elements_handle);
  g_vao_handle = g_vbo_handle = g_elements_handle = 0;

  if (g_shader_handle && g_vertex_handle)
    glDetachShader(g_shader_handle, g_vertex_handle);
  if (g_vertex_handle) glDeleteShader(g_vertex_handle);
  g_vertex_handle = 0;

  if (g_shader_handle && g_frag_handle)
    glDetachShader(g_shader_handle, g_frag_handle);
  if (g_frag_handle) glDeleteShader(g_frag_handle);
  g_frag_handle = 0;

  if (g_shader_handle) glDeleteProgram(g_shader_handle);
  g_shader_handle = 0;

  if (g_font_texture) {
    glDeleteTextures(1, &g_font_texture);
    ImGui::GetIO().Fonts->TexID = nullptr;
    g_font_texture = 0;
  }
}

auto render_draw_lists(ImDrawData* draw_data) -> void {
  // avoid rendering when minimized, scale coordinates for
  // retina displays (screen coordinates != framebuffer coordinates)
  const ImGuiIO& io = ImGui::GetIO();
  const int fb_width =
      static_cast<int>(io.DisplaySize.x * io.DisplayFramebufferScale.x);
  const int fb_height =
      static_cast<int>(io.DisplaySize.y * io.DisplayFramebufferScale.y);
  if (fb_width == 0 || fb_height == 0) return;
  draw_data->ScaleClipRects(io.DisplayFramebufferScale);

  // backup GL state
  GLenum last_active_texture;
  glGetIntegerv(GL_ACTIVE_TEXTURE,
                reinterpret_cast<GLint*>(&last_active_texture));
  glActiveTexture(GL_TEXTURE0);
  GLint last_program;
  glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
  GLint last_texture;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  GLint last_array_buffer;
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
  GLint last_element_array_buffer;
  glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
  GLint last_vertex_array;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
  GLint last_polygon_mode[2];
  glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
  GLint last_viewport[4];
  glGetIntegerv(GL_VIEWPORT, last_viewport);
  GLint last_scissor_box[4];
  glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
  GLenum last_blend_src_rgb;
  glGetIntegerv(GL_BLEND_SRC_RGB,
                reinterpret_cast<GLint*>(&last_blend_src_rgb));
  GLenum last_blend_dst_rgb;
  glGetIntegerv(GL_BLEND_DST_RGB,
                reinterpret_cast<GLint*>(&last_blend_dst_rgb));
  GLenum last_blend_src_alpha;
  glGetIntegerv(GL_BLEND_SRC_ALPHA,
                reinterpret_cast<GLint*>(&last_blend_src_alpha));
  GLenum last_blend_dst_alpha;
  glGetIntegerv(GL_BLEND_DST_ALPHA,
                reinterpret_cast<GLint*>(&last_blend_dst_alpha));
  GLenum last_blend_equation_rgb;
  glGetIntegerv(GL_BLEND_EQUATION_RGB,
                reinterpret_cast<GLint*>(&last_blend_equation_rgb));
  GLenum last_blend_equation_alpha;
  glGetIntegerv(GL_BLEND_EQUATION_ALPHA,
                reinterpret_cast<GLint*>(&last_blend_equation_alpha));
  const GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
  const GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
  const GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
  const GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

  // setup render state: alpha-blending enabled, no face culling, no depth
  // testing, scissor enabled, polygon fill
  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // setup viewport, orthographic projection matrix
  glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
  const float ortho_projection[4][4] = {
      {2.0f / io.DisplaySize.x, 0.0f, 0.0f, 0.0f},
      {0.0f, 2.0f / -io.DisplaySize.y, 0.0f, 0.0f},
      {0.0f, 0.0f, -1.0f, 0.0f},
      {-1.0f, 1.0f, 0.0f, 1.0f},
  };
  glUseProgram(g_shader_handle);
  glUniform1i(g_attribute_location_tex, 0);
  glUniformMatrix4fv(g_attribute_location_proj_mtx, 1, GL_FALSE,
                     &ortho_projection[0][0]);
  glBindVertexArray(g_vao_handle);

  for (int n = 0; n < draw_data->CmdListsCount; n++) {
    const ImDrawList* cmd_list = draw_data->CmdLists[n];
    const ImDrawIdx* idx_buffer_offset = nullptr;

    glBindBuffer(GL_ARRAY_BUFFER, g_vbo_handle);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(cmd_list->VtxBuffer.Size) * sizeof(ImDrawVert),
        static_cast<const GLvoid*>(cmd_list->VtxBuffer.Data), GL_STREAM_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_elements_handle);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(cmd_list->IdxBuffer.Size) * sizeof(ImDrawIdx),
        static_cast<const GLvoid*>(cmd_list->IdxBuffer.Data), GL_STREAM_DRAW);

    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
      const ImDrawCmd* p_cmd = &cmd_list->CmdBuffer[cmd_i];
      if (p_cmd->UserCallback) {
        p_cmd->UserCallback(cmd_list, p_cmd);
      } else {
        glBindTexture(
            GL_TEXTURE_2D,
            static_cast<GLuint>(reinterpret_cast<intptr_t>(p_cmd->TextureId)));
        glScissor(static_cast<int>(p_cmd->ClipRect.x),
                  static_cast<int>(fb_height - p_cmd->ClipRect.w),
                  static_cast<int>(p_cmd->ClipRect.z - p_cmd->ClipRect.x),
                  static_cast<int>(p_cmd->ClipRect.w - p_cmd->ClipRect.y));
        glDrawElements(
            GL_TRIANGLES, static_cast<GLsizei>(p_cmd->ElemCount),
            sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
            idx_buffer_offset);
      }
      idx_buffer_offset += p_cmd->ElemCount;
    }
  }

  // restore modified GL state
  glUseProgram(last_program);
  glBindTexture(GL_TEXTURE_2D, last_texture);
  glActiveTexture(last_active_texture);
  glBindVertexArray(last_vertex_array);
  glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
  glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
  glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb,
                      last_blend_src_alpha, last_blend_dst_alpha);
  if (last_enable_blend)
    glEnable(GL_BLEND);
  else
    glDisable(GL_BLEND);
  if (last_enable_cull_face)
    glEnable(GL_CULL_FACE);
  else
    glDisable(GL_CULL_FACE);
  if (last_enable_depth_test)
    glEnable(GL_DEPTH_TEST);
  else
    glDisable(GL_DEPTH_TEST);
  if (last_enable_scissor_test)
    glEnable(GL_SCISSOR_TEST);
  else
    glDisable(GL_SCISSOR_TEST);
  glPolygonMode(GL_FRONT_AND_BACK, last_polygon_mode[0]);
  glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2],
             (GLsizei)last_viewport[3]);
  glScissor(last_scissor_box[0], last_scissor_box[1],
            (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

auto get_clipboard_text(void* user_data) -> const char* {
  return glfwGetClipboardString(static_cast<GLFWwindow*>(user_data));
}

auto set_clipboard_text(void* user_data, const char* text) -> void {
  glfwSetClipboardString(static_cast<GLFWwindow*>(user_data), text);
}
}  // namespace

namespace gui {
auto mouse_button_callback(GLFWwindow*, const int button, const int action,
                           int mods) -> void {
  if (action == GLFW_PRESS && button >= 0 && button < 3)
    g_mouse_pressed[button] = true;
}

auto scroll_callback(GLFWwindow*, double x_offset, const double y_offset)
    -> void {
  g_mouse_wheel += static_cast<float>(
      y_offset);  // use fractional mouse wheel, 1.0 unit 5 lines.
}

auto key_callback(GLFWwindow*, const int key, int scan_code,
                  const int action, const int mods) -> void {
  ImGuiIO& io = ImGui::GetIO();
  if (action == GLFW_PRESS) io.KeysDown[key] = true;
  if (action == GLFW_RELEASE) io.KeysDown[key] = false;

  (void)mods;  // modifiers are not reliable across systems
  io.KeyCtrl =
      io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
  io.KeyShift =
      io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
  io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
  io.KeySuper =
      io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

auto char_callback(GLFWwindow*, const unsigned int c) -> void {
  ImGuiIO& io = ImGui::GetIO();
  if (c > 0 && c < 0x10000)
    io.AddInputCharacter(static_cast<unsigned short>(c));
}

auto init(GLFWwindow* window, const bool install_callbacks) -> bool {
  g_window = window;

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  if (install_callbacks) {
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, char_callback);
  }

  return true;
}

auto new_frame() -> void {
  if (!g_font_texture) create_device_objects();

  ImGuiIO& io = ImGui::GetIO();

  // setup display size (every frame to accommodate for window resizing)
  int w, h;
  int display_w, display_h;
  glfwGetWindowSize(g_window, &w, &h);
  glfwGetFramebufferSize(g_window, &display_w, &display_h);
  io.DisplaySize = ImVec2(static_cast<float>(w), static_cast<float>(h));
  io.DisplayFramebufferScale =
      ImVec2(w > 0 ? (static_cast<float>(display_w) / w) : 0,
             h > 0 ? (static_cast<float>(display_h) / h) : 0);

  // setup time step
  const double current_time = glfwGetTime();
  io.DeltaTime = g_time > 0.0 ? static_cast<float>(current_time - g_time)
                              : (float)(1.0f / 60.0f);
  g_time = current_time;

  // setup inputs
  // we already got mouse wheel, keyboard keys & characters
  // from glfwCallbacksPolled in glfwPollEvents()
  if (glfwGetWindowAttrib(g_window, GLFW_FOCUSED)) {
    double mouse_x, mouse_y;
    glfwGetCursorPos(g_window, &mouse_x, &mouse_y);
    // Mouse position in screen coordinates
    // (set to -1,-1 if no mouse / on another screen, etc.)
    io.MousePos =
        ImVec2(static_cast<float>(mouse_x), static_cast<float>(mouse_y));
  } else {
    io.MousePos = ImVec2(-1, -1);
  }

  for (int i = 0; i < 3; i++) {
    // If a mouse press event came, always pass it as "mouse held this frame",
    // so we don't miss click-release events that are shorter than 1 frame.
    io.MouseDown[i] =
        g_mouse_pressed[i] || glfwGetMouseButton(g_window, i) != 0;
    g_mouse_pressed[i] = false;
  }

  io.MouseWheel = g_mouse_wheel;
  g_mouse_wheel = 0.0f;

  // hide OS mouse cursor if ImGui is drawing it
  glfwSetInputMode(
      g_window, GLFW_CURSOR,
      io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

  // Start the frame
  ImGui::NewFrame();
}

auto render() -> void { ImGui::Render(); }

auto shutdown() -> void {
  invalidate_device_objects();
  ImGui::DestroyContext();
}
}  // namespace gui
}  // namespace cgra
