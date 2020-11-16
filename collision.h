#pragma once

#include "shared.h"

enum collider_type
{
    Collider_Rectangle = 1,
    Collider_Circle = 2,
};

struct rectangle
{
    glm::vec2 Center;
    f32 HalfWidth;
    f32 HalfHeight;
    f32 Angle; // TODO: Change to Rotation
};

struct circle
{
    glm::vec2 Center;
    f32 Radius;
};

struct collider
{
    collider_type Type;
    union
    {
        rectangle Rectangle;
        circle Circle;
    };
};

// TODO: Refactor these remaining structs, they dont make much sense
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
