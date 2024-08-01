#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <terrain/terrain_model.hpp>

#include "intersections.hpp"

/// Code Author: Tessa Power
///
/// Created with the help of the Camera tutorial from:
/// https://learnopengl.com/Getting-started/Camera

/**
 * \brief Defines the different directions the camera can move in.
 */
enum camera_movement { forward, backward, left, right, rest };

constexpr float default_pitch = 0.0f;
constexpr float default_yaw = -89.0f;

/**
 * \brief Represents a camera that moves around the scene in response to user
 * input.
 */
class camera {
  float m_max_pitch_ = 89.0f;
  float m_sensitivity_ = 0.5f;
  float m_default_speed_ = 30.0f;
  float m_max_speed_ = 400.0f;
  float m_acceleration_ = 5.0f;

  // Vectors
  glm::vec3 m_position_{};
  glm::vec3 m_front_{};
  glm::vec3 m_up_{};
  glm::vec3 m_right_{};
  glm::vec3 m_world_up_{};

  camera_movement m_current_direction_ = rest;
  float m_delta_time_ = 0.0f;
  float m_speed_ = m_default_speed_;

 public:
  float m_yaw;
  float m_pitch;

  /**
   * \brief Creates an instance of the camera class with the given parameters.
   * \param position The camera's starting position.
   * \param up A normalized vector which points in the up direction.
   * \param front A normalized vector which points in the forward facing
   * direction. \param yaw The starting yaw of the camera. \param pitch The
   * starting pitch of the camera.
   */
  explicit camera(const glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
                  const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
                  const glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f),
                  const float yaw = default_yaw,
                  const float pitch = default_pitch)
      : m_position_(position),
        m_front_(front),
        m_up_(up),
        m_world_up_(up),
        m_yaw(yaw),
        m_pitch(pitch) {
    m_right_ = glm::normalize(glm::cross(m_front_, m_world_up_));
    update_vectors();
  }

  /**
   * \brief Returns the view matrix based on the camera's position and
   * orientation.
   */
  [[nodiscard]] auto view_matrix() const noexcept -> glm::mat4 {
    return glm::lookAt(m_position_, m_position_ + m_front_, m_up_);
  }

  /**
   * \brief Updates the camera given the given time since the last frame.
   * \param delta_time The time since the last frame.
   * \param model The terrain model used to check for collisions.
   */
  auto update(const float delta_time, const terrain_model& model) noexcept
      -> void {
    m_delta_time_ = delta_time;
    update_position(model);
  }

  /**
   * \brief Sets the direction in which the camera will move in the next frame.
   */
  auto set_direction(camera_movement const& direction) noexcept -> void {
    m_current_direction_ = direction;
  }

  /**
   * \brief Changes the pitch and yaw of the camera based on the given offset.
   * \param x_offset The offset of the new camera angle in the x direction.
   * \param y_offset The offset of the new camera angle in the y direction.
   */
  auto update_angle(float x_offset, float y_offset) noexcept -> void {
    x_offset *= m_sensitivity_;
    y_offset *= m_sensitivity_;

    m_yaw += x_offset;
    m_pitch += y_offset;

    if (m_pitch > m_max_pitch_) m_pitch = m_max_pitch_;
    if (m_pitch < -m_max_pitch_) m_pitch = -m_max_pitch_;

    update_vectors();
  }

 private:
  /**
   * \brief Updates the camera's front, right and up vectors based on its
   * current pitch and yaw.
   */
  auto update_vectors() -> void {
    glm::vec3 front;

    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front_ = glm::normalize(front);

    m_right_ = glm::normalize(glm::cross(m_front_, m_world_up_));

    // Normalize the vectors, because their length gets closer to 0 the more you
    // look up or down which results in slower movement.
    m_up_ = glm::normalize(glm::cross(m_right_, m_front_));
  }

  /**
   * \brief Updates the position of the camera, taking into account the terrain
   * model to avoid collisions. \param model The terrain model being used in the
   * application.
   */
  auto update_position(const terrain_model& model) -> void {
    const float speed = m_speed_ * m_delta_time_;
    glm::vec3 delta{};

    switch (m_current_direction_) {
      case forward: {
        delta = m_front_ * speed;
        break;
      }
      case backward: {
        delta = -m_front_ * speed;
        break;
      }
      case left: {
        delta = -m_right_ * speed;
        break;
      }
      case right: {
        delta = m_right_ * speed;
        break;
      }
      case rest: {
        m_speed_ = m_default_speed_;
        break;
      }
    }

    // Check if the camera will collide with the terrain
    // If it is colliding, reduce the speed to the default speed, otherwise move
    // like normal
    if (m_current_direction_ != rest) {
      if (!does_collide(m_position_ + delta, model, delta)) {
        m_position_ += delta;
        m_speed_ = glm::min(m_max_speed_, m_speed_ + m_acceleration_);
      } else {  // Reset the speed to default speed if the camera has collided
                // with the terrain
        m_speed_ = m_default_speed_;
      }
    }
  }

  /**
   * \brief Returns whether the camera will collide with the terrain if it moves
   * to the given new position. \param new_pos The new position of the camera.
   * \param model The terrain model being used in the application.
   * \param direction The direction vector of the camera's movement.
   */
  [[nodiscard]] auto does_collide(const glm::vec3& new_pos,
                                  const terrain_model& model,
                                  const glm::vec3& direction) const noexcept
      -> bool {
    // Iterate through the model's mesh vertices and test for intersection
    const auto size = model.m_builder.m_vertices.size();
    for (size_t i = 0; i < size; ++i) {
      const cgra::mesh_vertex v = model.m_builder.m_vertices[i];

      // Check if this vertex is within a reasonable distance of the new
      // position to avoid unnecessary calculations
      const auto x = new_pos.x - v.pos.x;
      const auto y = new_pos.y - v.pos.y;
      const auto z = new_pos.z - v.pos.z;
      const auto distance_sq = x * x + y * y + z * z;
      if (distance_sq > m_speed_) continue;

      // Check if the ray intersects with the vertex
      if (ray_intersects_vertex(m_position_, direction, static_cast<int>(i),
                                model) && new_pos.y <= v.pos.y)
        return true;
    }

    return false;
  }
};

#endif // CAMERA_HPP
