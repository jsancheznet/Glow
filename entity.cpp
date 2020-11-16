#pragma once

#include "entity.h"
#include "collision.h"

b32 E_EntitiesCollide(entity *A, entity *B, collision_result *CollisionResult)
{
    return C_CheckCollision(A->Collider.Rectangle, B->Collider.Rectangle, CollisionResult);
}

entity *E_CreateEntity(texture *Texture,
                       glm::vec3 Position,
                       glm::vec3 Size,
                       f32 RotationAngle,
                       f32 Speed,
                       f32 Drag,
                       collider_type ColliderType)
{
    entity *Result = (entity*)Malloc(sizeof(entity)); Assert(Result);

    Result->Texture = Texture;
    Result->Position = Position;
    Result->Size = Size;
    Result->Acceleration = glm::vec3(0.0f);
    Result->RotationAngle = RotationAngle;
    Result->Speed = Speed;
    Result->Drag = Drag;

    // Collision Data
    switch(ColliderType)
    {
        case Collider_Rectangle:
        {
            Result->Collider.Type = Collider_Rectangle;
            Result->Collider.Rectangle.Center = glm::vec2(Position.x, Position.y);
            Result->Collider.Rectangle.HalfWidth = Size.x * 0.5f;
            Result->Collider.Rectangle.HalfHeight = Size.y * 0.5f;
            Result->Collider.Rectangle.Angle = RotationAngle;
            break;
        }
        case Collider_Circle:
        {
            Result->Collider.Type = Collider_Circle;
            break;
        }
        default:
        {
            // Invalid code path
            Assert(0);
            break;
        }
    }

    return (Result);
}

void E_Update(entity *Entity, f32 TimeStep)
{
    Entity->Position = 0.5f * Entity->Acceleration * (TimeStep * TimeStep) + Entity->Velocity + Entity->Position;
    Entity->Velocity = Entity->Acceleration * TimeStep + Entity->Velocity;
    Entity->Acceleration = {};
    Entity->Velocity *= Entity->Drag;


    // Collision Data
    switch(Entity->Collider.Type)
    {
        case Collider_Rectangle:
        {
            Entity->Collider.Rectangle.Center = glm::vec2(Entity->Position.x, Entity->Position.y);
            Entity->Collider.Rectangle.Angle = Entity->RotationAngle;
            Entity->Collider.Rectangle.HalfWidth = Entity->Size.x * 0.5f;
            Entity->Collider.Rectangle.HalfHeight = Entity->Size.y * 0.5f;
            break;
        }
        case Collider_Circle:
        {
            break;
        }
        default:
        {
            // Invalid code path
            Assert(0);
            break;
        }
    }
}
