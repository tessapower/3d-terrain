#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Defines several possible options for camera movement. Used as abstraction to
// stay away from window-system specific input methods
enum camera_movement { forward, backward, left, right };

constexpr float default_pitch = 0.0f;
constexpr float default_yaw = -89.0f;

// An abstract camera class that processes input and calculates the
// corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class camera {
  // Constants
  const float m_max_pitch_ = 89.0f;
  const float m_speed_ = 100.0f;
  const float m_sensitivity_ = 0.5f;

  // Vectors
  glm::vec3 m_position_{};
  glm::vec3 m_front_{};
  glm::vec3 m_up_{};
  glm::vec3 m_right_{};
  glm::vec3 m_world_up_{};

 public:
  float m_yaw;
  float m_pitch;

  explicit camera(const glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
                  const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
                  const glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f),
                  const float yaw = default_yaw, const float pitch = default_pitch)
      : m_position_(position), m_front_(front), m_up_(up), m_world_up_(up),
        m_yaw(yaw), m_pitch(pitch) {
    m_right_ = glm::normalize(glm::cross(m_front_, m_world_up_));
    update_vectors();
  }

  // returns the view matrix calculated using Euler Angles and the LookAt Matrix
  [[nodiscard]] glm::mat4 view_matrix() const noexcept {
    return glm::lookAt(m_position_, m_position_ + m_front_, m_up_);
  }

  // Processes input received from any keyboard-like input system. Accepts input
  // parameter in the form of camera defined ENUM (to abstract it from windowing
  // systems)
  void process_keyboard(const camera_movement direction, const float delta_time) noexcept {
    const float speed = m_speed_ * delta_time;

    switch(direction) {
      case forward: {
        m_position_ += m_front_ * speed;
        break;
      }
      case backward: {
        m_position_ -= m_front_ * speed;
        break;
      }
      case left: {
        m_position_ -= m_right_ * speed;
        break;
      }
      case right: {
        m_position_ += m_right_ * speed;
        break;
      }
    }
  }

  // Processes input received from a mouse input system. Expects the offset
  // value in both the x and y direction.
  void process_mouse_movement(float x_offset, float y_offset) noexcept {
    x_offset *= m_sensitivity_;
    y_offset *= m_sensitivity_;
    
    m_yaw   += x_offset;
    m_pitch += y_offset;

    if (m_pitch > m_max_pitch_) m_pitch = m_max_pitch_;
    if (m_pitch < -m_max_pitch_) m_pitch = -m_max_pitch_;

    update_vectors();
  }

private:
  // Calculate the front vector from the camera's Euler Angles
  void update_vectors() {
    glm::vec3 front;

    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front_ = glm::normalize(front);

    m_right_ = glm::normalize(glm::cross(m_front_, m_world_up_));

    // Normalize the vectors, because their length gets closer to 0 the more you look up or down
    // which results in slower movement.
    m_up_ = glm::normalize(glm::cross(m_right_, m_front_));
  }
};