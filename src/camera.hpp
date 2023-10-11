#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <terrain_model.hpp>

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
 * \brief Represents a camera that moves around the scene in response to user input.
 */
class camera {
    // Constants
    const float m_max_pitch_ = 89.0f;
    const float m_sensitivity_ = 0.5f;
    const float m_default_speed_ = 30.0f;
    const float m_max_speed_ = 400.0f;
    const float m_speed_accel_ = 5.0f;

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
     * \brief Updates the camera given the given time since the last frame.
     * \param delta_time The time since the last frame.
     */
    void update(const float delta_time, terrain_model& m) noexcept {
        m_delta_time_ = delta_time;
        update_position(m);
    }

    /**
     * \brief Sets the direction in which the camera will move in the next frame.
     */
    void set_direction(camera_movement const& direction) noexcept {
        m_current_direction_ = direction;
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

    void update_position(terrain_model& m) {
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
            m_speed_ = m_default_speed_; // return speed back to default speed
            break;
        }
        }

        if (m_current_direction_ != rest) { // in movement
            glm::vec3 newPos = m_position_ + delta;
            if (!collide(newPos, m, delta)) { // no collision
                m_position_ += delta;
                m_speed_ = glm::min(m_max_speed_, m_speed_ + m_speed_accel_); // accelerate speed
            }
            else {
                m_speed_ = m_default_speed_; // return speed back to default speed if collision
            }
        }
    }

    bool collide(glm::vec3& newPos, terrain_model& model, glm::vec3& delta) {
        // Iterate through your mesh vertices and test for intersection
        for (int i = 0; i < model.builder.vertices.size(); ++i) {
            cgra::mesh_vertex vertex = model.builder.vertices[i];

            // first check if it is within a reasonable distance
            float distance_2 = (newPos[0] - vertex.pos[0]) * (newPos[0] - vertex.pos[0]) +
                (newPos[1] - vertex.pos[1]) * (newPos[1] - vertex.pos[1]); +
                (newPos[2] - vertex.pos[2]) * (newPos[2] - vertex.pos[2]);
            if (distance_2 > m_speed_) continue;

            // Check if the ray intersects with the vertex
            if (rayIntersectsVertex(m_position_, delta, i, model)) {
                if (newPos.y <= vertex.pos.y) return true;
            }
        }
        return false;
    }


    bool rayIntersectsVertex(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, int vertex, terrain_model& model) {
        // Iterate through the adjacent faces of the vertex
        for (int i = 0; i < model.adjacent_faces[vertex].size(); i += 3) {
            // Get the vertices of the triangle formed by the current adjacent faces
            const glm::vec3& vertex0 = model.builder.vertices[model.adjacent_faces[vertex][i]].pos;
            const glm::vec3& vertex1 = model.builder.vertices[model.adjacent_faces[vertex][i + 1]].pos;
            const glm::vec3& vertex2 = model.builder.vertices[model.adjacent_faces[vertex][i + 2]].pos;

            // Check if the ray intersects with the triangle
            if (rayIntersectsTriangle(rayOrigin, rayDirection, vertex0, vertex1, vertex2)) {
                return true; // Intersection found
            }
        }

        return false; // No intersection
    }

    // Möller–Trumbore intersection algorithm
    bool rayIntersectsTriangle(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
        const glm::vec3& vertex0, const glm::vec3& vertex1, const glm::vec3& vertex2) {
        // Calculate edge vectors of the triangle
        glm::vec3 edge1 = vertex1 - vertex0;
        glm::vec3 edge2 = vertex2 - vertex0;

        // Calculate the determinant of the matrix formed by the direction vector and edge vectors
        glm::vec3 h = cross(rayDirection, edge2);
        float a = dot(edge1, h);

        // Check if the ray and triangle are parallel
        if (a > -0.00001f && a < 0.00001f)
            return false;

        float f = 1.0f / a;
        glm::vec3 s = rayOrigin - vertex0;
        float u = f * dot(s, h);

        if (u < 0.0f || u > 1.0f)
            return false;

        glm::vec3 q = cross(s, edge1);
        float v = f * dot(rayDirection, q);

        if (v < 0.0f || u + v > 1.0f)
            return false;

        // Calculate the intersection distance along the ray
        float t = f * dot(edge2, q);

        // Check if the intersection point is in front of the ray's origin
        if (t > 0.00001f) {
            // The ray intersects with the triangle, and t is the intersection distance
            return true;
        }

        return false; // No intersection
    }
};
