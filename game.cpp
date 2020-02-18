#pragma once

#include "game.h"

global glm::vec3 InitialCameraPosition = glm::vec3(0.0f, 0.0f, 23.0f);

camera *G_CreateCamera(i32 WindowWidth, i32 WindowHeight)
{
    camera *Result;
    Result = (camera*)Malloc(sizeof(camera));
    Assert(Result);

    Result->Position = InitialCameraPosition;
    Result->Front = glm::vec3(0.0f, 0.0f, -1.0f);
    Result->Up = glm::vec3(0.0f, 1.0f, 0.0f);
    Result->Speed = 1.5f;
    Result->FoV = 90.0f;
    Result->Near = 0.1f;
    Result->Far = 1500.0f;
    Result->View = glm::lookAt(Result->Position, Result->Position + Result->Front, Result->Up);
    Result->Projection = glm::perspective(glm::radians(Result->FoV), (f32)WindowWidth / (f32)WindowHeight, Result->Near, Result->Far);
    Result->Ortho = glm::ortho(0.0f, (f32)WindowWidth, 0.0f, (f32)WindowHeight);

    return (Result);
}

void G_ResetCamera(camera *Camera, i32 WindowWidth, i32 WindowHeight)
{
    Camera->Position = glm::vec3(0.0f, 0.0f, 6.0f);
    Camera->Front = glm::vec3(0.0f, 0.0f, -1.0f);
    Camera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
    Camera->Speed = 1.5f;
    Camera->FoV = 90.0f;
    Camera->Near = 0.1f;
    Camera->Far = 1500.0f;
    Camera->View = glm::lookAt(Camera->Position, Camera->Position + Camera->Front, Camera->Up);
    Camera->Projection = glm::perspective(glm::radians(Camera->FoV), (f32)WindowWidth / (f32)WindowHeight, Camera->Near, Camera->Far);
    Camera->Ortho = glm::ortho(0.0f, (f32)WindowWidth, 0.0f, (f32)WindowHeight);
}

void ComputeNewtonMotion(physics_data *Data, f32 TimeStep)
{
    Data->Position = (0.5f * Data->Acceleration) * (TimeStep * TimeStep) + Data->Velocity * TimeStep + Data->Position;
    Data->Velocity = Data->Acceleration * TimeStep + Data->Velocity;
    Data->Acceleration = {};
    f32 DragCoefficient = 0.8f;
    Data->Velocity *= DragCoefficient; // Drag
}
