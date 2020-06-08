#pragma once

#include "shared.h"

// enum collision_shape
// {
//     Shape_AABB,
//     Shape_OBB,
// };

// struct collision_data
// {
//     collision_shape Shape;
//     union
//     {
//         rectangle AABB;
//         oriented_rectangle OBB;
//     };
// };

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
