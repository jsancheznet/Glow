#pragma once

#include "entity.h"
#include "collision.h"

b32 E_EntitiesCollide(entity *A, entity *B, glm::vec2 *ResolutionDirection, f32 *ResolutionOverlap)
{
    return C_Collision(A->Collider, B->Collider, ResolutionDirection, ResolutionOverlap);
}

entity *E_CreateEntity(texture *Texture,
                       glm::vec3 Position,
                       glm::vec3 Direction,
                       glm::vec3 Size,
                       f32 RotationAngle,
                       f32 Speed,
                       f32 Drag,
                       entity_type EntityType,
                       collider_type ColliderType)
{
    entity *Result = (entity*)Malloc(sizeof(entity)); Assert(Result);

    Result->Texture = Texture;
    Result->Position = Position;
    Result->Direction = glm::normalize(Direction);
    Result->Size = Size;
    Result->Acceleration = glm::vec3(0.0f);
    Result->Angle = RotationAngle;
    Result->Speed = Speed;
    Result->Drag = Drag;
    Result->Type = EntityType;
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
            if(Equals(Size.x, Size.y))
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
            InvalidCodePath;
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

    // Do not allow negative size
    if(Entity->Size.x < 0.0f) Entity->Size.x = 0.0f;
    if(Entity->Size.y < 0.0f) Entity->Size.y = 0.0f;
    if(Entity->Size.z < 0.0f) Entity->Size.z = 0.0f;

    // Update collision Data
    switch(Entity->Collider.Type)
    {
        case Collider_Rectangle:
        {
            Entity->Collider.Rectangle.Center = glm::vec2(Entity->Position.x, Entity->Position.y);
            Entity->Collider.Rectangle.Angle = Entity->Angle;
            Entity->Collider.Rectangle.HalfWidth = Entity->Size.x * 0.5f;
            Entity->Collider.Rectangle.HalfHeight = Entity->Size.y * 0.5f;
            break;
        }
        case Collider_Circle:
        {
            Entity->Collider.Circle.Center.x = Entity->Position.x;
            Entity->Collider.Circle.Center.y = Entity->Position.y;
            Entity->Collider.Circle.Radius = Entity->Size.x * 0.5f;
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

entity_list *E_CreateEntityList(u32 MaxEntityCount)
{
    Assert(MaxEntityCount > 0);

    entity_list *Result = NULL;
    Result = (entity_list*)Malloc(sizeof(entity_list)); Assert(Result);

    Result->Count = 0;
    Result->MaxCount = MaxEntityCount;
    Result->Head = NULL;
    Result->Tail = NULL;

    return Result;
}

// Always pushes an entity to the back of the list
void E_PushEntity(entity_list *List, entity *Entity)
{
    Assert(Entity);
    Assert(List);

    // Create the Node
    entity_node *Node = (entity_node*)Malloc(sizeof(entity_node)); Assert(Node);
    Node->Entity = Entity;
    Node->Next = NULL;
    Node->Previous = NULL;

    if(List->Count < List->MaxCount)
    {
        // It's the first node on the list
        if(List->Count == 0)
        {
            List->Head = Node;
            List->Tail = Node;
            List->Count++;
        }
        else
        {
            Node->Previous = List->Tail;
            List->Tail->Next = Node;
            List->Tail = Node;
            List->Count++;
        }
    }
    else
    {
        printf("List is full, List->Count >= List->MaxCount\n");
        Free(Node);
        Free(Entity);
    }
}

void E_FreeNode(entity_list *List, entity_node *Node)
{
    Assert(List);
    Assert(Node);

    // TODO(Joreg): Rearrange items in order of most likely to improve performance

    if(List->Count == 0)
    {
        // List is empty, no item to free
        return;
    }
    else if(List->Count == 1)
    {
        // Its the only node
        List->Head = NULL;
        List->Tail = NULL;
        List->Count--;
        Free(Node->Entity);
        Free(Node);
    }
    else if(Node->Next == NULL)
    {
        // Its the last node
        List->Tail = Node->Previous;
        List->Tail->Next = NULL;
        List->Count--;

        Free(Node->Entity);
        Free(Node);
    }
    else if(Node->Previous == NULL)
    {
        // Its the first node
        List->Head = Node->Next;
        List->Head->Previous = NULL;
        List->Count--;

        Free(Node->Entity);
        Free(Node);
    }
    else
    {
        // Its somewhere in the middle
        Node->Previous->Next = Node->Next;
        Node->Next->Previous = Node->Previous;
        List->Count--;

        Free(Node->Entity);
        Free(Node);
    }
}
