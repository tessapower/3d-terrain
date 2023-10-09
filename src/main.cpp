#include <iostream>
#include <stdexcept>
#include <string>

#include "application.hpp"
#include "cgra/cgra_gui.hpp"
#include "opengl.hpp"

namespace {
void cursor_pos_cb(GLFWwindow *, double x_pos, double y_pos);
void mouse_button_cb(GLFWwindow *win, int button, int action, int mods);
void scroll_cb(GLFWwindow *win, double x_offset, double y_offset);
void key_cb(GLFWwindow *win, int key, int scan_code, int action, int mods);
void char_cb(GLFWwindow *win, unsigned int c);
void APIENTRY debug_cb(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar *,
                       GLvoid *);

// global static pointer to application once we create it
// necessary for interfacing with the GLFW callbacks
application *application_ptr = nullptr;
}  // namespace

int main() {

  // initialize the GLFW library
  if (!glfwInit()) {
    std::cerr << "Error: Could not initialize GLFW" << std::endl;
    abort();  // unrecoverable error
  }

  // force OpenGL to create a 3.3 core context
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // disallow legacy functionality (helps OS X work)
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  // get the version for GLFW for later
  int glfw_major, glfw_minor, glfw_revision;
  glfwGetVersion(&glfw_major, &glfw_minor, &glfw_revision);

  // request a debug context so we get debug callbacks
  // remove this for possible GL performance increases
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

  // create a windowed mode window and its OpenGL context
  GLFWwindow *window =
      glfwCreateWindow(800, 600, "Hello World!", nullptr, nullptr);
  if (!window) {
    std::cerr << "Error: Could not create GLFW window" << std::endl;
    abort();  // unrecoverable error
  }

  // make the window's context current.
  // if we have multiple windows we will need to switch contexts
  glfwMakeContextCurrent(window);

  // initialize GLEW
  // must be done after making a GL context current (glfwMakeContextCurrent in
  // this case)
  glewExperimental =
      GL_TRUE;  // required for full GLEW functionality for OpenGL 3.0+
  if (const GLenum err = glewInit();
      GLEW_OK !=
      err) {  // problem: glewInit failed, something is seriously wrong.
    std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
    abort();  // unrecoverable error
  }

  // print out our OpenGL versions
  std::cout << "Using OpenGL " << glGetString(GL_VERSION) << std::endl;
  std::cout << "Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
  std::cout << "Using GLFW " << glfw_major << "." << glfw_minor << "."
            << glfw_revision << std::endl;

  // enable GL_ARB_debug_output if available (not necessary, just helpful)
  if (glfwExtensionSupported("GL_ARB_debug_output")) {
    // this allows the error location to be determined from a stacktrace
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
    // setup up the callback
    glDebugMessageCallbackARB(debug_cb, nullptr);
    glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0,
                             nullptr, true);
    std::cout << "GL_ARB_debug_output callback installed" << std::endl;
  } else {
    std::cout << "GL_ARB_debug_output not available. No worries." << std::endl;
  }

  // initialize ImGui
  if (!cgra::gui::init(window)) {
    std::cerr << "Error: Could not initialize ImGui" << std::endl;
    abort();  // unrecoverable error
  }

  // attach input callbacks to window
  glfwSetCursorPosCallback(window, cursor_pos_cb);
  glfwSetMouseButtonCallback(window, mouse_button_cb);
  glfwSetScrollCallback(window, scroll_cb);
  glfwSetKeyCallback(window, key_cb);
  glfwSetCharCallback(window, char_cb);

  // create the application object (and a global pointer to it)
  application application(window);
  application_ptr = &application;

  // loop until the user closes the window
  while (!glfwWindowShouldClose(window)) {
    // main Render
    // glEnable(GL_FRAMEBUFFER_SRGB); // use if you know about gamma correction
    application.render();

    // GUI Render on top
    // glDisable(GL_FRAMEBUFFER_SRGB); // use if you know about gamma correction
    cgra::gui::newFrame();
    application.render_gui();
    cgra::gui::render();

    // swap front and back buffers
    glfwSwapBuffers(window);

    // poll for and process events
    glfwPollEvents();
  }

  // clean up ImGui
  cgra::gui::shutdown();
  glfwTerminate();
}

namespace {
void cursor_pos_cb(GLFWwindow *, const double x_pos, const double y_pos) {
  // if not captured then forward to application
  if (const ImGuiIO &io = ImGui::GetIO(); io.WantCaptureMouse) return;
  application_ptr->cursor_pos_cb(x_pos, y_pos);
}

void mouse_button_cb(GLFWwindow *win, const int button, const int action,
                     const int mods) {
  // forward callback to ImGui
  cgra::gui::mouseButtonCallback(win, button, action, mods);

  // if not captured then forward to application
  if (const ImGuiIO &io = ImGui::GetIO(); io.WantCaptureMouse) return;
  application_ptr->mouse_button_cb(button, action, mods);
}

void scroll_cb(GLFWwindow *win, const double x_offset, const double y_offset) {
  // forward callback to ImGui
  cgra::gui::scrollCallback(win, x_offset, y_offset);

  // if not captured then forward to application
  if (const ImGuiIO &io = ImGui::GetIO(); io.WantCaptureMouse) return;
  application_ptr->scroll_cb(x_offset, y_offset);
}

void key_cb(GLFWwindow *win, const int key, const int scan_code,
            const int action, const int mods) {
  // forward callback to ImGui
  cgra::gui::keyCallback(win, key, scan_code, action, mods);

  // if not captured then forward to application
  if (const ImGuiIO &io = ImGui::GetIO(); io.WantCaptureKeyboard) return;
  application_ptr->key_cb(key, scan_code, action, mods);
}

void char_cb(GLFWwindow *win, const unsigned int c) {
  // forward callback to ImGui
  cgra::gui::charCallback(win, c);

  // if not captured then forward to application
  if (const ImGuiIO &io = ImGui::GetIO(); io.WantTextInput) return;
  application_ptr->char_cb(c);
}

// function to translate source to string
const char *get_string_for_source(const GLenum source) {
  switch (source) {
    case GL_DEBUG_SOURCE_API:
      return "API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
      return "Window System";
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      return "Shader Compiler";
    case GL_DEBUG_SOURCE_THIRD_PARTY:
      return "Third Party";
    case GL_DEBUG_SOURCE_APPLICATION:
      return "Application";
    case GL_DEBUG_SOURCE_OTHER:
      return "Other";
    default:
      return "n/a";
  }
}

// function to translate severity to string
const char *get_string_for_severity(const GLenum severity) {
  switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
      return "High";
    case GL_DEBUG_SEVERITY_MEDIUM:
      return "Medium";
    case GL_DEBUG_SEVERITY_LOW:
      return "Low";
    case GL_DEBUG_SEVERITY_NOTIFICATION:
      return "None";
    default:
      return "n/a";
  }
}

// function to translate type to string
const char *get_string_for_type(const GLenum type) {
  switch (type) {
    case GL_DEBUG_TYPE_ERROR:
      return "Error";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      return "Deprecated Behaviour";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      return "Undefined Behaviour";
    case GL_DEBUG_TYPE_PORTABILITY:
      return "Portability";
    case GL_DEBUG_TYPE_PERFORMANCE:
      return "Performance";
    case GL_DEBUG_TYPE_OTHER:
      return "Other";
    default:
      return "n/a";
  }
}

void APIENTRY debug_cb(const GLenum source, const GLenum type, const GLuint id,
                       const GLenum severity, GLsizei, const GLchar *message,
                       GLvoid *) {
  // Don't report notification messages
  if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;

  // nvidia: avoid debug spam about attribute offsets
  if (id == 131076) return;

  std::cerr << "GL [" << get_string_for_source(source) << "] "
            << get_string_for_type(type) << ' ' << id << " : ";
  std::cerr << message << " (Severity: " << get_string_for_severity(severity)
            << ')' << std::endl;

  if (type == GL_DEBUG_TYPE_ERROR_ARB)
    throw std::runtime_error(std::string{"GL Error: "} + message);
}
}  // namespace