#pragma once

#include "collision.h"


/* TODO

   OBB vs Circle

   1 Testear en los ejes del obb
   2 Calcular cual es el vertice mas cercano al centro del circulo
   3 Testear en el eje paralelo a vertice_obb-centro_circulo

   extra: usar voronoi regiones para encontrar el vertice mas
   cercano. Esto lo hace mas rapido y supuestamente lo unico que
   preciso son las pruebas hechas en los ejes del obb
 */

b32 C_Overlapping1D(f32 MinA, f32 MaxA, f32 MinB, f32 MaxB)
{
    return MinB <= MaxA && MinA <= MaxB;
}

f32 C_GetOverlap(f32 MinA, f32 MaxA, f32 MinB, f32 MaxB)
{
    f32 Result = 0.0f;

    // Get the length of both segments
    f32 LengthA = Abs(MaxA - MinA);
    f32 LengthB = Abs(MaxB - MinB);

    // Add the lengths
    f32 TotalLength = LengthA + LengthB;

    // Get the Min and Max from MinA,MaxA,MinB,MaxB
    f32 HugeNegativeNumber = -99999999999999.f;
    f32 HugePositiveNumber = 99999999999999.f;
    f32 Min = HugePositiveNumber;
    f32 Max = HugeNegativeNumber;
    f32 Values[4] =
    {
        MinA, MaxA, MinB, MaxB
    };
    for(u32 i = 0; i < 4; i++)
    {
        if(Values[i] < Min)
        {
            Min = Values[i];
        }

        if(Values[i] > Max)
        {
            Max = Values[i];
        }
    }

    // Get the Length of the Min Max
    f32 Length = Abs(Max - Min);

    Result = Abs(Length - TotalLength);

    return Result;
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
    // 2- Rotate the vertices, and later translate them to the world position.
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

void C_ProjectOBBVertices(rectangle_collision_data OBB, glm::vec2 Axis, f32 *Min, f32 *Max)
{
    *Min = 0.0f;
    *Max = 0.0f;
    b32 FirstIteration = true;

    // Project One OBB onto an axis
    // Loop through the vertices, since we only support OBB we only need 4 loops.
    for(int i = 0; i < 4; i++)
    {
        f32 P = glm::dot(OBB.Vertices[i], Axis);

        if(FirstIteration)
        {
            *Min = P;
            *Max = P;
            FirstIteration = false;
        }

        if(P < *Min)
        {
            *Min = P;
        }
        else if(P > *Max)
        {
            *Max = P;
        }
    }
}

b32 C_CheckCollision(oriented_rectangle A, oriented_rectangle B, collision_result *CollisionResult)
{
    Assert(CollisionResult);

    *CollisionResult = {};

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

    f32 HugeNumber = 999999999999.9f;
    f32 SmallestOverlap = HugeNumber;
    glm::vec2 SmallestAxis = {};

    // Loop over the Axes
    for(int i = 0; i < 4; i++)
    {
        // Project all vertices to one of the axes, keep only the min and max of each obb then do overlapping function
        f32 MinA = 0.0f;
        f32 MaxA = 0.0f;
        C_ProjectOBBVertices(OBBA, Axes[i], &MinA, &MaxA);

        f32 MinB = 0.0f;
        f32 MaxB = 0.0f;
        C_ProjectOBBVertices(OBBB, Axes[i], &MinB, &MaxB);

        if(!C_Overlapping1D(MinA, MaxA, MinB, MaxB))
        {
            return false;
        }
        else
        {
            f32 Overlap = C_GetOverlap(MinA, MaxA, MinB, MaxB);
            if(Overlap < SmallestOverlap)
            {
                SmallestOverlap = Overlap;
                SmallestAxis = glm::normalize(Axes[i]);

                // This if checks the direction in which the obb has to be displaced
                // Got it from: https://gamedev.stackexchange.com/questions/27596/implementing-separating-axis-theorem-sat-and-minimum-translation-vector-mtv/27633#27633
                if(MinA < MinB)
                {
                    SmallestAxis *= -1;
                }
            }
        }
    }

    CollisionResult->Overlap = SmallestOverlap;
    CollisionResult->Direction = SmallestAxis;

    return true;
}

b32 C_CheckCollision(oriented_rectangle OBB, circle Circle, collision_result *CollisionResult)
{

    /* TODO

       OBB vs Circle

       1 Testear en los ejes del obb
       2 Calcular cual es el vertice mas cercano al centro del circulo
       3 Testear en el eje paralelo a vertice_obb-centro_circulo

       extra: usar voronoi regiones para encontrar el vertice mas
       cercano. Esto lo hace mas rapido y supuestamente lo unico que
       preciso son las pruebas hechas en los ejes del obb
    */

    // First do OBB vs Circle using only the OBB Axes
    rectangle_collision_data Rect = C_GenerateCollisionData(OBB);

    f32 HugeNumber = 999999999999.9f;
    f32 SmallestOverlap = HugeNumber;
    glm::vec2 SmallestAxis = {};

    return true;
}
