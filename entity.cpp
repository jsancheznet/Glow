#pragma once

#include "entity.h"
#include "collision.h"

b32 E_EntitiesCollide(entity *A, entity *B, collision_result *CollisionResult)
{
    return C_CheckCollision(A->Rect, B->Rect, CollisionResult);
}

entity *E_CreateEntity(texture *Texture,
                       glm::vec3 Position,
                       glm::vec3 Size,
                       f32 RotationAngle,
                       f32 Speed,
                       f32 Drag)
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
    Result->Rect.Center = glm::vec2(Position.x, Position.y);
    Result->Rect.HalfWidth = Size.x * 0.5f;
    Result->Rect.HalfHeight = Size.y * 0.5f;
    Result->Rect.Angle = RotationAngle;

    return (Result);
}

void E_Update(entity *Entity, f32 TimeStep)
{
    Entity->Position = 0.5f * Entity->Acceleration * (TimeStep * TimeStep) + Entity->Velocity + Entity->Position;
    Entity->Velocity = Entity->Acceleration * TimeStep + Entity->Velocity;
    Entity->Acceleration = {};
    Entity->Velocity *= Entity->Drag;

    // Collision Data
    Entity->Rect.Center = glm::vec2(Entity->Position.x, Entity->Position.y);
    Entity->Rect.Angle = Entity->RotationAngle;
    Entity->Rect.HalfWidth = Entity->Size.x * 0.5f;
    Entity->Rect.HalfHeight = Entity->Size.y * 0.5f;
}
