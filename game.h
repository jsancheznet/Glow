#pragma once

#include "shared.h"

struct physics_data
{
    glm::vec3 Acceleration;
    glm::vec3 Velocity;
    glm::vec3 Position;
};

struct entity
{
    physics_data Physics;
};

struct camera // TODO: This might need to be in something like entities.cpp
{
    // Camera implemented using: https://learnopengl.com/Getting-started/Camera
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;

    f32 Speed;
    f32 Yaw;
    f32 Pitch;

    f32 FoV;
    f32 Near;
    f32 Far;

    glm::mat4 View;
    glm::mat4 Projection;
    glm::mat4 Ortho;
};
