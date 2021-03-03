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

void C_GetRectangleVertices(rectangle RectangleInput, glm::vec2 *OutputArray)
{
    // 1- Generate the obb vertices around the origin.
    // 2- Rotate the vertices, and later translate them to the world position.

    /*
      Vertex0              Vertex1
         |-----------------|
         |                 |
         |        .        | Center   Rotation Angle
         |                 |
         |-----------------|
      Vertex2              Vertex3
     */

    OutputArray[0] = {-RectangleInput.HalfWidth, RectangleInput.HalfHeight};
    OutputArray[1] = {+RectangleInput.HalfWidth, RectangleInput.HalfHeight};
    OutputArray[2] = {-RectangleInput.HalfWidth, -RectangleInput.HalfHeight};
    OutputArray[3] = {+RectangleInput.HalfWidth, -RectangleInput.HalfHeight};

    // Rotate the vertices and add the RectangleInput.Center, translating the vertices to the correct world position.
    OutputArray[0] = RectangleInput.Center + glm::rotate(OutputArray[0], glm::radians(RectangleInput.Angle));
    OutputArray[1] = RectangleInput.Center + glm::rotate(OutputArray[1], glm::radians(RectangleInput.Angle));
    OutputArray[2] = RectangleInput.Center + glm::rotate(OutputArray[2], glm::radians(RectangleInput.Angle));
    OutputArray[3] = RectangleInput.Center + glm::rotate(OutputArray[3], glm::radians(RectangleInput.Angle));
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

                // This if checks the direction in which the obb has
                // to be displaced Got it from:
                // https://gamedev.stackexchange.com/questions/27596/implementing-separating-axis-theorem-sat-and-minimum-translation-vector-mtv/27633#27633
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

glm::vec2 C_FindClosestVertex(glm::vec2 Vertices[4], glm::vec2 CircleCenter)
{
    glm::vec2 Result = {};

    f32 MinDistance = FLT_MAX;
    for(u32 i  = 0; i < 4; i++)
    {
        f32 CurrentDistance = Distance(Vertices[i], CircleCenter);
        if(CurrentDistance < MinDistance)
        {
            MinDistance = CurrentDistance;
            Result.x = Vertices[i].x;
            Result.y = Vertices[i].y;
        }
    }

    return (Result);
}

b32 C_CollisionRectangleCircle(rectangle InputRectangle, circle InputCircle, collision_result *CollisionResult)
{
    /*

       OBB vs Circle

       1) Testear en los ejes del obb
       2) Calcular cual es el vertice mas cercano al centro del circulo
       3) Testear en el eje paralelo a vertice_obb-centro_circulo

       extra: usar voronoi regiones para encontrar el vertice mas
       cercano. Esto lo hace mas rapido y supuestamente lo unico que
       preciso son las pruebas hechas en los ejes del obb
    */

    f32 SmallestOverlap = FLT_MAX;
    glm::vec2 SmallestAxis = {};

    { // SECTION: Test for collision on the OBB separation axes
        rectangle_collision_data RectangleCollisionData = C_GenerateRectangleCollisionData(InputRectangle);

        // Get the points of the circle on x,y axes
        // Generate the vertices on model space
        glm::vec2 CircleVertex1X = {-InputCircle.Radius, 0.0f};
        glm::vec2 CircleVertex2X = {InputCircle.Radius, 0.0f};
        glm::vec2 CircleVertex1Y = {0.0f, InputCircle.Radius};
        glm::vec2 CircleVertex2Y = {0.0f, -InputCircle.Radius};

        // Rotate the vertices and add the Circle.Center, translating the vertices to the correct world position.
        CircleVertex1X = InputCircle.Center + glm::rotate(CircleVertex1X, glm::radians(InputRectangle.Angle));
        CircleVertex2X = InputCircle.Center + glm::rotate(CircleVertex2X, glm::radians(InputRectangle.Angle));
        CircleVertex1Y = InputCircle.Center + glm::rotate(CircleVertex1Y, glm::radians(InputRectangle.Angle));
        CircleVertex2Y = InputCircle.Center + glm::rotate(CircleVertex2Y, glm::radians(InputRectangle.Angle));

        { // SECTION: Test collision on the rectangle axes
            // Now we have the rectangle vertices, the rectangle SAT axes and the Circle Vertices in world space, let's do this.
            for(i32 i = 0; i < 2; i++)
            {
                // Project all vertices of obb onto one axis
                f32 MinA = 0.0f;
                f32 MaxA = 0.0f;
                C_ProjectOBBVertices(RectangleCollisionData, RectangleCollisionData.SeparationAxes[i], &MinA, &MaxA);

                // Project circle vertices onto the same axis
                f32 MinB;
                f32 MaxB;

                f32 Projection1 = glm::dot(CircleVertex1X, RectangleCollisionData.SeparationAxes[i]);
                f32 Projection2 = glm::dot(CircleVertex2X, RectangleCollisionData.SeparationAxes[i]);
                f32 Projection3 = glm::dot(CircleVertex1Y, RectangleCollisionData.SeparationAxes[i]);
                f32 Projection4 = glm::dot(CircleVertex2Y, RectangleCollisionData.SeparationAxes[i]);

                // First iteration
                MinB = Projection1;
                MaxB = Projection1;

                // Get MinB, MaxB
                if(Projection1 > MaxB)
                {
                    MaxB = Projection1;
                }
                else if(Projection1 < MinB)
                {
                    MinB = Projection1;
                }

                if(Projection2 > MaxB)
                {
                    MaxB = Projection2;
                }
                else if(Projection2 < MinB)
                {
                    MinB = Projection2;
                }

                if(Projection3 > MaxB)
                {
                    MaxB = Projection3;
                }
                else if(Projection3 < MinB)
                {
                    MinB = Projection3;
                }

                if(Projection4 > MaxB)
                {
                    MaxB = Projection4;
                }
                else if(Projection4 < MinB)
                {
                    MinB = Projection4;
                }

                // Get the min and max, than do overlapping
                if(!C_Overlapping1D(MinA, MaxA, MinB, MaxB))
                {
                    // There is no overlap, according to SAT, the shapes are not colliding, return false.
                    *CollisionResult = {};
                    return false;
                }
                else
                {
                    // if its the SmallestOverlap, set te variable accordingly
                    f32 Overlap = C_GetOverlap(MinA, MaxA, MinB, MaxB);
                    if(Overlap < SmallestOverlap)
                    {
                        SmallestOverlap = Overlap;
                        SmallestAxis = glm::normalize(RectangleCollisionData.SeparationAxes[i]);

                        // This if checks the direction in which the obb
                        // has to be displaced Got it from:
                        // https://gamedev.stackexchange.com/questions/27596/implementing-separating-axis-theorem-sat-and-minimum-translation-vector-mtv/27633#27633
                        if(MinA < MinB)
                        {
                            SmallestAxis *= -1;
                        }
                    }
                }
            }
        }

    CollisionResult->Overlap = SmallestOverlap;
    CollisionResult->Direction = SmallestAxis;

    } // ENDSECTION

    { // SECTION: Test for collision using the circle's data
        // If we got to here, we have found collision on the 2 rect axes.

        glm::vec2 Vertices[4];
        C_GetRectangleVertices(InputRectangle, Vertices);

        glm::vec2 ClosestVertex = {};
        ClosestVertex = C_FindClosestVertex(Vertices, InputCircle.Center);

        // Get the ClosestVertex -> CircleCenter Axis
        glm::vec2 Axis = Normalize(ClosestVertex - InputCircle.Center);

        // Project the obb vertices, get MaxA, MinB

        // Get the Circle Vertices along the axis, How?

    }

    return true;
}

b32 C_Collision(collider A, collider B, collision_result *CollisionResult)
{
    Assert(CollisionResult);

    // TODO(Jorge): Make sure A or B are independent. Ex: CircleVsRectangle RectangleVsCircle
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
