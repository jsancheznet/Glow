#pragma once

#include "entity.h"


// void E_CreateEntity(texture *Texture, glm::vec3 Position, glm::vec3 Velocity, glm::vec3 Acceleration)

void E_NewtonMotion(entity *Entity, f32 TimeStep)
{
    Entity->Position = 0.5f * Entity->Acceleration * (TimeStep * TimeStep) + Entity->Velocity + Entity->Position;
    Entity->Velocity = Entity->Acceleration * TimeStep + Entity->Velocity;
    Entity->Acceleration = {};
    f32 DragCoefficient = 0.8f;
    Entity->Velocity *= DragCoefficient; // Drag
}
