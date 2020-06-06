#pragma once

#include "shared.h"
#include "renderer.h"

struct rectangle
{
    glm::vec2 Center;
    f32 HalfWidth;
    f32 HalfHeight;
};

struct oriented_rectangle
{
    glm::vec2 Center;
    f32 HalfWidth;
    f32 HalfHeight;
    f32 Angle;
};

struct entity
{
    texture *Texture;

    glm::vec3 Acceleration;
    glm::vec3 Velocity;
    glm::vec3 Position;
    glm::vec3 InitialPosition;
    glm::vec3 Size;
    glm::vec3 Direction;

    rectangle Rect;

    f32 Speed;
    f32 RotationAngle; // NOTE: Degrees!
    f32 DragCoefficient;
};
