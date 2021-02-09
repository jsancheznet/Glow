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

glm::vec2 C_RighthandNormal(glm::vec2 A)
{
    glm::vec2 Result;

    Result.x = -A.y;
    Result.y = A.x;

    return Result;
}

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

rectangle_collision_data C_GenerateRectangleCollisionData(rectangle Input)
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
    Result.SeparationAxes[0] = glm::normalize(C_RighthandNormal(Result.Vertices[2] - Result.Vertices[0]));
    Result.SeparationAxes[1] = glm::normalize(C_RighthandNormal(Result.Vertices[0] - Result.Vertices[1]));

    return (Result);
}

void C_ProjectOBBVertices(rectangle_collision_data Rectangle, glm::vec2 Axis, f32 *Min, f32 *Max)
{
    *Min = 0.0f;
    *Max = 0.0f;
    b32 FirstIteration = true;

    // Project One OBB onto an axis
    // Loop through the vertices, since we only support OBB we only need 4 loops.
    for(int i = 0; i < 4; i++)
    {
        f32 P = glm::dot(Rectangle.Vertices[i], Axis);

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

b32 C_CollisionCircleCircle(circle A, circle B, collision_result *Result)
{
    Assert(Result);

    *Result = {};
    f32 RadiiSum = A.Radius + B.Radius;
    f32 DistanceBetween = Distance(A.Center, B.Center);

    if(DistanceBetween < RadiiSum)
    {
        // Collision
        Result->Overlap = Abs(RadiiSum - DistanceBetween);
        Result->Direction  = Normalize(A.Center - B.Center);

        return true;
    }
    else
    {
        // No Collision
        return false;
    }
}


b32 C_CollisionRectangleRectangle(rectangle A, rectangle B, collision_result *Result)
{
    Assert(Result);

    rectangle_collision_data DataA = C_GenerateRectangleCollisionData(A);
    rectangle_collision_data DataB = C_GenerateRectangleCollisionData(B);

    glm::vec2 Axes[4] =
    {
        DataA.SeparationAxes[0],
        DataA.SeparationAxes[1],
        DataB.SeparationAxes[0],
        DataB.SeparationAxes[1],
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
        C_ProjectOBBVertices(DataA, Axes[i], &MinA, &MaxA);

        f32 MinB = 0.0f;
        f32 MaxB = 0.0f;
        C_ProjectOBBVertices(DataB, Axes[i], &MinB, &MaxB);

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

    Result->Overlap = SmallestOverlap;
    Result->Direction = SmallestAxis;

    return true;
}

b32 C_CollisionRectangleCircle(rectangle Rectangle, circle Circle, collision_result *CollisionResult)
{
    /* TODO

       OBB vs Circle

       1) Testear en los ejes del obb
       2) Calcular cual es el vertice mas cercano al centro del circulo
       3) Testear en el eje paralelo a vertice_obb-centro_circulo

       extra: usar voronoi regiones para encontrar el vertice mas
       cercano. Esto lo hace mas rapido y supuestamente lo unico que
       preciso son las pruebas hechas en los ejes del obb
    */

    f32 HugeNumber = 999999999999.9f; // To know which overlap is the lowest, we need to compare it to a huge number
    f32 SmallestOverlap = HugeNumber;
    glm::vec2 SmallestAxis = {};

    { // SECTION: Test for collision on the OBB separation axes

        /*
          struct rectangle_collision_data
          {
              glm::vec2 Vertices[4];
              glm::vec2 SeparationAxes[2];
          };
         */
        rectangle_collision_data RectData = C_GenerateRectangleCollisionData(Rectangle);

        // Get the points of the circle on x,y axes
        // Generate the vertices on model space
        glm::vec2 CircleVertex1X = {-Circle.Radius, 0.0f};
        glm::vec2 CircleVertex2X = {Circle.Radius, 0.0f};
        glm::vec2 CircleVertex1Y = {0.0f, Circle.Radius};
        glm::vec2 CircleVertex2Y = {0.0f, -Circle.Radius};

        // Rotate the vertices and add the Circle.Center, translating the vertices to the correct world position.
        CircleVertex1X = Circle.Center + glm::rotate(CircleVertex1X, glm::radians(Rectangle.Angle));
        CircleVertex2X = Circle.Center + glm::rotate(CircleVertex2X, glm::radians(Rectangle.Angle));
        CircleVertex1Y = Circle.Center + glm::rotate(CircleVertex1Y, glm::radians(Rectangle.Angle));
        CircleVertex2Y = Circle.Center + glm::rotate(CircleVertex2Y, glm::radians(Rectangle.Angle));

        // Now we have the rectangle vertices, the rectangle SAT axes and the Circle Vertices in world space, let's do this.
        // Testear en los ejes del obb
        for(i32 i = 0; i < 2; i++)
        {
            // void C_ProjectOBBVertices(rectangle_collision_data Rectangle, glm::vec2 Axis, f32 *Min, f32 *Max)
            f32 MinA = 0.0f;
            f32 MaxA = 0.0f;
            // Project all vertices of obb onto one axis
            C_ProjectOBBVertices(RectData, RectData.SeparationAxes[i], &MinA, &MaxA);

            f32 MinB = 0.0f;
            f32 MaxB = 0.0f;
            // Project circle vertices onto the same axis
            // Project Vertex1
            f32 Projection1 = glm::dot(CircleVertex1X, RectData.SeparationAxes[i]);
            // Project Vertex2
            f32 Projection2 = glm::dot(CircleVertex2X, RectData.SeparationAxes[i]);
            // Project Vertex3
            f32 Projection3 = glm::dot(CircleVertex1Y, RectData.SeparationAxes[i]);
            // Project Vertex4
            f32 Projection4 = glm::dot(CircleVertex2Y, RectData.SeparationAxes[i]);
            // Get the min and max, than do overlapping

            if(!C_Overlapping1D(MinA, MaxA, MinB, MaxB))
            {
                return false;
            }
            else
            {
                // Calculate overlap
            }
        }
    }

    // First do OBB vs Circle using only the OBB Axes


    return true;
}

b32 C_Collision(collider A, collider B, collision_result *CollisionResult)
{
    Assert(CollisionResult);

    if(A.Type == Collider_Rectangle && B.Type == Collider_Rectangle)
    {
        return C_CollisionRectangleRectangle(A.Rectangle, B.Rectangle, CollisionResult);
    }
    else if(A.Type == Collider_Rectangle && B.Type == Collider_Circle)
    {
        return C_CollisionRectangleCircle(A.Rectangle, B.Circle, CollisionResult);
    }
    else if(A.Type == Collider_Circle && B.Type == Collider_Circle)
    {
        return C_CollisionCircleCircle(A.Circle, B.Circle, CollisionResult);
    }
    else
    {
        InvalidCodePath;
        return false;
    }
}


#if 0
void C_GetRectangleVertices(rectangle Rectangle, glm::vec2 RectangleVertices[4])
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
    RectangleVertices[0] = {-Rectangle.HalfWidth, Rectangle.HalfHeight};
    RectangleVertices[1] = {+Rectangle.HalfWidth, Rectangle.HalfHeight};
    RectangleVertices[2] = {-Rectangle.HalfWidth, -Rectangle.HalfHeight};
    RectangleVertices[3] = {+Rectangle.HalfWidth, -Rectangle.HalfHeight};

    // Rotate the vertices and add the Rectangle.Center, translating the vertices to the correct world position.
    RectangleVertices[0] = Rectangle.Center + glm::rotate(RectangleVertices[0], glm::radians(Rectangle.Angle));
    RectangleVertices[1] = Rectangle.Center + glm::rotate(RectangleVertices[1], glm::radians(Rectangle.Angle));
    RectangleVertices[2] = Rectangle.Center + glm::rotate(RectangleVertices[2], glm::radians(Rectangle.Angle));
    RectangleVertices[3] = Rectangle.Center + glm::rotate(RectangleVertices[3], glm::radians(Rectangle.Angle));
}
#endif
