#pragma once

#include "entity.h"


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

    return (Result);
}

void E_CalculateMotion(entity *Entity, f32 TimeStep)
{
    Entity->Position = 0.5f * Entity->Acceleration * (TimeStep * TimeStep) + Entity->Velocity + Entity->Position;
    Entity->Velocity = Entity->Acceleration * TimeStep + Entity->Velocity;
    Entity->Acceleration = {};
    f32 DragCoefficient = 0.8f;
    Entity->Velocity *= DragCoefficient; // Drag
}
