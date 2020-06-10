#pragma once

#include "collision.h"

b32 C_Overlapping1D(f32 MinA, f32 MaxA, f32 MinB, f32 MaxB)
{
    return MinB <= MaxA && MinA <= MaxB;
}

b32 C_AABBCollision(rectangle A, rectangle B)
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

    return C_Overlapping1D(ALeft, ARight, BLeft, BRight) && C_Overlapping1D(ABottom, ATop, BBottom, BTop);
}

glm::vec2 C_GetSeparationAxisFromSegment(glm::vec2 A, glm::vec2 B)
{
    glm::vec2 Result = {};

    glm::vec2 Segment = A - B;

    // Get the normal for the segment
    Result.x = -Segment.y;
    Result.y = Segment.x;

    Result = glm::normalize(Result);

    return Result;
}

rectangle_collision_data C_GenerateCollisionData(oriented_rectangle Input)
{
    rectangle_collision_data Result = {};

    // 1- Generate the obb vertices around the origin.
    // 2- Rotate the vertices, and later translate them to they're correct position.
    // 3- Generate the separations axes and normalize them.
    /*
      Vertex0              Vertex1
         |-----------------|
         |                 |
         |        .        | Center   Rotation Angle
         |                 |
         |-----------------|
      Vertex2              Vertex3
     */
    Result.Vertices[0] = {-Input.HalfWidth, Input.HalfHeight};
    Result.Vertices[1] = {+Input.HalfWidth, Input.HalfHeight};
    Result.Vertices[2] = {-Input.HalfWidth, -Input.HalfHeight};
    Result.Vertices[3] = {+Input.HalfWidth, -Input.HalfHeight};

    // Rotate the vertices and add the Input.Center, translating the vertices to the correct world position.
    Result.Vertices[0] = Input.Center + glm::rotate(Result.Vertices[0], glm::radians(Input.Angle));
    Result.Vertices[1] = Input.Center + glm::rotate(Result.Vertices[1], glm::radians(Input.Angle));
    Result.Vertices[2] = Input.Center + glm::rotate(Result.Vertices[2], glm::radians(Input.Angle));
    Result.Vertices[3] = Input.Center + glm::rotate(Result.Vertices[3], glm::radians(Input.Angle));

    // Get the separation Axes and Normalize them so we can project vectors onto them
    Result.SeparationAxes[0] = C_GetSeparationAxisFromSegment(Result.Vertices[2], Result.Vertices[0]);
    Result.SeparationAxes[1] = C_GetSeparationAxisFromSegment(Result.Vertices[0], Result.Vertices[1]);
    Result.SeparationAxes[0] = glm::normalize(Result.SeparationAxes[0]);
    Result.SeparationAxes[1] = glm::normalize(Result.SeparationAxes[1]);

    return (Result);
}

// TODO: Comment this function!
b32 C_OBBCollision(oriented_rectangle A, oriented_rectangle B)
{
    // TODO: Maybe generate a single collision_data struct. I should
    // maybe have another function to get the axes, or pass them by
    // pointer. Creating collision data and then again creating an
    // array of 4 axes is not pretty nor efficient(maybe).
    rectangle_collision_data OBBA = C_GenerateCollisionData(A);
    rectangle_collision_data OBBB = C_GenerateCollisionData(B);

    glm::vec2 Axes[4] =
    {
        OBBA.SeparationAxes[0],
        OBBA.SeparationAxes[1],
        OBBB.SeparationAxes[0],
        OBBB.SeparationAxes[1],
    };

    // Loop over the Axes
    // TODO: Comment this whole for loop
    for(int i = 0; i < 4; i++)
    {
        // Do this scope 4 times, one for each axis
        // Project all vertices to one SA, keep only the min and max of each obb then do overlapping function
        glm::vec2 CurrentAxis = Axes[i];
        float MinA = 0.0f;
        float MaxA = 0.0f;
        bool FirstIteration = true;

        for(int j = 0; j < 4; j++)
        {
            float P = glm::dot(OBBA.Vertices[j], CurrentAxis);

            if(FirstIteration)
            {
                MinA = P;
                MaxA = P;
                FirstIteration = false;
            }

            if(P < MinA)
            {
                MinA = P;
            }
            else if(P > MaxA)
            {
                MaxA = P;
            }
        }

        float MinB = 0.0f;
        float MaxB = 0.0f;
        FirstIteration = true;
        for(int j = 0; j < 4; j++)
        {
            float P = glm::dot(OBBB.Vertices[j], CurrentAxis);

            if(FirstIteration)
            {
                MinB = P;
                MaxB = P;
                FirstIteration = false;
            }

            if(P < MinB)
            {
                MinB = P;
            }
            else if(P > MaxB)
            {
                MaxB = P;
            }
        }

        if(!C_Overlapping1D(MinA, MaxA, MinB, MaxB))
        {
            return false;
        }
    }

    return true;
}
