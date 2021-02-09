#pragma once

#include "entity.h"
#include "collision.h"

b32 E_EntitiesCollide(entity *A, entity *B, collision_result *CollisionResult)
{
    return C_Collision(A->Collider, B->Collider, CollisionResult);
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
    Result->Collider = {};

    // Initialize Collider with 0, Collider_Null

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
            Result->Collider.Circle.Center = glm::vec2(Position.x, Position.y);

            // To avoid changing the code to properly support the
            // radius of a circle we check if both Size.x and Size.y
            // are the same and set the radius accordingly. If they
            // are not the same it's not a valid size for a circle.
            if(EqFloats(Size.x, Size.y))
            {
                // Size.x and Size.y are equal, it's a valid size for a circle!
                Result->Collider.Circle.Radius = Size.x * 0.5f;
            }
            else
            {
                // Size.x and Size.y are not equal, it's not a valid size for a circle! Error!
                InvalidCodePath;
            }

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
            // Entity->Collider.Circle.Center.x = Entity->Position.x;
            // Entity->Collider.Circle.Center.y = Entity->Position.y;

            // NOTE(Jorge): Entity->Position is a vec3, Circle.Center
            // is a vec2. If something does not work, looking here
            // might be a good idea.
            Entity->Collider.Circle.Center = Entity->Position;
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
