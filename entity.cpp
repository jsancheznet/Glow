#pragma once

#include "entity.h"

// Globals
global f32 EntityHalfWidth = 0.5f;
global f32 EntityHalfHeight = 0.5f;

b32 E_Overlapping1D(f32 MinA, f32 MaxA, f32 MinB, f32 MaxB)
{
    return MinB <= MaxA && MinA <= MaxB;
}

b32 E_RectanglesCollide(rectangle A, rectangle B)
{
    // NOTE: AABB Collision

    // Compute MinA, MaxA, MinB, MaxB for horizontal plane
    f32 ALeft = A.Center.x - A.HalfWidth;
    f32 ARight = A.Center.x + A.HalfWidth;
    f32 BLeft = B.Center.x - B.HalfWidth;
    f32 BRight = B.Center.x + B.HalfWidth;

    // Compute MinA, MaxA, MinB, MaxB for vertical plane
    f32 ABottom = A.Center.y - A.HalfHeight;
    f32 ATop = A.Center.y + A.HalfHeight;
    f32 BBottom = B.Center.y - B.HalfHeight;
    f32 BTop = B.Center.y + B.HalfHeight;

    return E_Overlapping1D(ALeft, ARight, BLeft, BRight) && E_Overlapping1D(ABottom, ATop, BBottom, BTop);
}

b32 E_EntitiesCollide(entity *A, entity *B)
{
    return E_RectanglesCollide(A->Rect, B->Rect);
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
    Result->Size = Size;
    Result->Acceleration = glm::vec3(0.0f);
    Result->Direction = Direction;
    Result->RotationAngle = RotationAngle;
    Result->Speed = Speed;
    Result->DragCoefficient = DragCoefficient;

    // Collision Data
    Result->Rect.Center = Position;
    Result->Rect.HalfWidth = EntityHalfWidth;
    Result->Rect.HalfHeight = EntityHalfHeight;

    return (Result);
}

void E_CalculateMotion(entity *Entity, f32 TimeStep)
{
    Entity->Position = 0.5f * Entity->Acceleration * (TimeStep * TimeStep) + Entity->Velocity + Entity->Position;
    Entity->Velocity = Entity->Acceleration * TimeStep + Entity->Velocity;
    Entity->Acceleration = {};
    f32 DragCoefficient = 0.8f;
    Entity->Velocity *= DragCoefficient; // Drag

    // Update Collision Data
    Entity->Rect.Center = Entity->Position;
}
