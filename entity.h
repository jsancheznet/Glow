#pragma once

#include "shared.h"
#include "renderer.h"
#include "collision.h"

enum entity_type
{
    EntityType_None, // This is used for types that do not yet have a specific entity_type
    EntityType_Player,
    EntityType_Seeker,
    EntityType_Wanderer,
    EntityType_Kamikaze,
    EntityType_Bullet,
    EntityType_Pickup,
    EntityType_Wall
};

struct entity
{
    texture *Texture;

    glm::vec3 Position;
    glm::vec3 Velocity;
    glm::vec3 Acceleration;
    glm::vec3 Direction;
    glm::vec3 Size;

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
