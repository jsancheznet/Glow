#pragma once

#include "shared.h"

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

struct rectangle_collision_data
{
    glm::vec2 Vertices[4];
    glm::vec2 SeparationAxes[2];
};

struct obb_projection_result
{
    f32 Min;
    f32 Max;
    glm::vec2 Axis;
};

struct collision_result
{
    f32 Overlap;
    glm::vec2 Direction;
};
