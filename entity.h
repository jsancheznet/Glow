#pragma once

#include "shared.h"
#include "renderer.h"
#include "collision.h"

enum entity_type
{
    Type_None, // This is used for types that do not yet have a specific entity_type
    Type_Player,
    Type_Seeker,
    Type_Wanderer,
    Type_Bouncer,
    Type_Bullet,
    Type_Pickup,
    Type_Wall
};

struct entity
{
    texture *Texture;

    glm::vec3 Position;
    glm::vec3 Velocity;
    glm::vec3 Acceleration;
    glm::vec3 Size;
    // glm::vec3 Direction;
    f32 Speed;
    f32 Angle;
    f32 Drag;

    entity_type Type;
    collider Collider;
};


struct entity_node
{
    entity *Entity;
    entity_node *Next;
    entity_node *Previous;
};

struct entity_list
{
    u32 Count;
    u32 MaxCount;
    entity_node *Head;
    entity_node *Tail;
};
