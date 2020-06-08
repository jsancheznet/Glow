#pragma once

#include "entity.h"
#include "collision.cpp"

// Globals
global f32 EntityHalfWidth = 0.5f;
global f32 EntityHalfHeight = 0.5f;

b32 E_EntitiesCollide(entity *A, entity *B)
{
    return C_OBBCollision(A->Rect, B->Rect);
}

entity E_CreateEntityA(texture *Texture,
                       glm::vec3 Position,
                       glm::vec3 Size,
                       glm::vec3 Direction,
                       f32 RotationAngle,
                       f32 Speed, f32 DragCoefficient)
{
    entity Result = {};

    Result.Texture = Texture;
    Result.Position = Position;
    Result.InitialPosition = Position;
    Result.Size = Size;
    Result.Acceleration = glm::vec3(0.0f);
    Result.Direction = Direction;
    Result.RotationAngle = RotationAngle;
    Result.Speed = Speed;
    Result.DragCoefficient = DragCoefficient;

    Result.Rect.Center = Position;
    Result.Rect.HalfWidth = EntityHalfWidth;
    Result.Rect.HalfHeight = EntityHalfHeight;
    Result.Rect.Angle = RotationAngle;

    return (Result);
}

entity *E_CreateEntity(texture *Texture,
                       glm::vec3 Position,
                       glm::vec3 Size,
                       glm::vec3 Direction,
                       f32 RotationAngle,
                       f32 Speed, f32 DragCoefficient)
{
    entity *Result = (entity*)Malloc(sizeof(entity)); Assert(Result);

    Result->Texture = Texture;
    Result->Position = Position;
    Result->InitialPosition = Position;
    Result->Size = Size;
    Result->Acceleration = glm::vec3(0.0f);
    Result->Direction = Direction;
    Result->RotationAngle = RotationAngle;
    Result->Speed = Speed;
    Result->DragCoefficient = DragCoefficient;

    Result->Rect.Center = Position;
    Result->Rect.HalfWidth = EntityHalfWidth;
    Result->Rect.HalfHeight = EntityHalfHeight;
    Result->Rect.Angle = RotationAngle;

    return (Result);
}

void E_CalculateMotion(entity *Entity, f32 TimeStep)
{
    Entity->Position = 0.5f * Entity->Acceleration * (TimeStep * TimeStep) + Entity->Velocity + Entity->Position;
    Entity->Velocity = Entity->Acceleration * TimeStep + Entity->Velocity;
    Entity->Acceleration = {};
    // f32 DragCoefficient = 0.8f;
    Entity->Velocity *= Entity->DragCoefficient; // Drag

    // Update Collision Data
    Entity->Rect.Center = Entity->Position;
}
