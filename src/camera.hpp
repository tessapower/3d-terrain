#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/**
 * \brief Defines the different directions the camera can move in.
 */
enum camera_movement { forward, backward, left, right };

constexpr float default_pitch = 0.0f;
constexpr float default_yaw = -89.0f;

/**
 * \brief Represents a camera that moves around the scene in response to user input.
 */
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

  /**
   * \brief Creates an instance of the camera class with the given parameters.
   * \param position The camera's starting position.
   * \param up A normalized vector which points in the up direction.
   * \param front A normalized vector which points in the forward facing direction.
   * \param yaw The starting yaw of the camera.
   * \param pitch The starting pitch of the camera.
   */
  explicit camera(const glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
                  const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
                  const glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f),
                  const float yaw = default_yaw, const float pitch = default_pitch)
      : m_position_(position), m_front_(front), m_up_(up), m_world_up_(up),
        m_yaw(yaw), m_pitch(pitch) {
    m_right_ = glm::normalize(glm::cross(m_front_, m_world_up_));
    update_vectors();
  }

  /**
   * \brief Returns the view matrix based on the camera's position and orientation.
   */
  [[nodiscard]] glm::mat4 view_matrix() const noexcept {
    return glm::lookAt(m_position_, m_position_ + m_front_, m_up_);
  }

  /**
   * \brief Moves the camera in the given direction based on its internal settings and the given
   * time since the last frame.
   * \param direction The direction in which the camera should move.
   * \param delta_time The time since the last frame.
   */
  void move(const camera_movement direction, const float delta_time) noexcept {
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

  /**
   * \brief Changes the pitch and yaw of the camera based on the given offset.
   * \param x_offset The offset of the new camera angle in the x direction.
   * \param y_offset The offset of the new camera angle in the y direction.
   */
  void update_angle(float x_offset, float y_offset) noexcept {
    x_offset *= m_sensitivity_;
    y_offset *= m_sensitivity_;
    
    m_yaw   += x_offset;
    m_pitch += y_offset;

    if (m_pitch > m_max_pitch_) m_pitch = m_max_pitch_;
    if (m_pitch < -m_max_pitch_) m_pitch = -m_max_pitch_;

    update_vectors();
  }

private:
  /**
   * \brief Updates the camera's front, right and up vectors based on its current pitch and yaw.
   */
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