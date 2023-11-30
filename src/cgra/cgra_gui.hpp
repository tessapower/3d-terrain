#pragma once

#include <utils/opengl.hpp>

namespace cgra {
namespace gui {
// callback functions required to forward input to ImGui
auto mouse_button_callback(GLFWwindow*, int button, int action, int mods)
    -> void;
auto scroll_callback(GLFWwindow*, double x_offset, double y_offset) -> void;
auto key_callback(GLFWwindow*, int key, int scan_code, int action, int mods)
    -> void;
auto char_callback(GLFWwindow*, unsigned int c) -> void;

// helper functions to setup, run and shutdown ImGui
auto init(GLFWwindow* window, bool install_callbacks = false) -> bool;
auto new_frame() -> void;
auto render() -> void;
auto shutdown() -> void;
}  // namespace gui
}  // namespace cgra
