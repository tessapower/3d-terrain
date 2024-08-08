#ifndef CGRA_GUI_HPP
#define CGRA_GUI_HPP

#include <utils/opengl.hpp>

namespace cgra::gui {
// Callback functions required to forward input to ImGui
auto mouse_button_callback(GLFWwindow*, int button, int action, int mods)
    -> void;
auto scroll_callback(GLFWwindow*, double x_offset, double y_offset) -> void;
auto key_callback(GLFWwindow*, int key, int scan_code, int action, int mods)
    -> void;
auto char_callback(GLFWwindow*, unsigned int c) -> void;

// Helper functions to set up, run and shut down ImGui
auto init(GLFWwindow* window, bool install_callbacks = false) -> bool;
auto new_frame() -> void;
auto render() -> void;
auto shutdown() -> void;
}  // namespace cgra

#endif  // CGRA_GUI_HPP
