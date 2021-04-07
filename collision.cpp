#pragma once

#include "collision.h"

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
    f32 Min = FLT_MAX;
    f32 Max = -FLT_MAX;
    f32 Values[4] = { MinA, MaxA, MinB, MaxB};
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

void C_RectangleVertices(rectangle RectangleInput, glm::vec2 *OutputArray)
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
    C_RectangleVertices(Input, Result.Vertices);
    // Get the separation Axes and Normalize them so we can project vectors onto them
    Result.SeparationAxes[0] = glm::normalize(C_RighthandNormal(Result.Vertices[2] - Result.Vertices[0]));
    Result.SeparationAxes[1] = glm::normalize(C_RighthandNormal(Result.Vertices[0] - Result.Vertices[1]));

    return (Result);
}

void C_ProjectRectangleVertices(rectangle_collision_data Rectangle, glm::vec2 Axis, f32 *Min, f32 *Max)
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

glm::vec2 C_ClosestVertexToPoint(glm::vec2 *Vertices, glm::vec2 Point)
{
    glm::vec2 Result = {};

    f32 Dist = FLT_MAX;
    f32 CurrentDistance;

    for(i32 i = 0; i < 4; i++)
    {
        CurrentDistance = Distance(Vertices[i], Point);
        if(CurrentDistance < Dist)
        {
            Dist = CurrentDistance;
            Result = Vertices[i];
        }
    }

    return Result;
}

b32 C_CollisionCircleCircle(circle A, circle B, glm::vec2 *ResolutionDirection, f32 *ResolutionOverlap)
{
    Assert(ResolutionDirection);
    Assert(ResolutionOverlap);

    *ResolutionDirection = {};
    *ResolutionOverlap = 0.0f;

    f32 RadiiSum = A.Radius + B.Radius;
    f32 DistanceBetween = Distance(A.Center, B.Center);

    if(DistanceBetween < RadiiSum)
    {
        // Collision
        *ResolutionOverlap = Abs(RadiiSum - DistanceBetween);
        *ResolutionDirection  = Normalize(A.Center - B.Center);

        return true;
    }
    else
    {
        // No Collision
        return false;
    }
}

<<<<<<< HEAD
b32 C_CollisionRectangleRectangle(rectangle A, rectangle B, glm::vec2 *ResolutionDirection, f32 *ResolutionOverlap)
=======
b32 C_CollisionRectangleRectangle(rectangle A, rectangle B, collision_result *Result)
>>>>>>> 6b1935ac85eedd7a2ce9ddeeb2fcd96e0fd4c5e5
{
    Assert(ResolutionDirection);
    Assert(ResolutionOverlap);

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
        C_ProjectRectangleVertices(DataA, Axes[i], &MinA, &MaxA);

        f32 MinB = 0.0f;
        f32 MaxB = 0.0f;
        C_ProjectRectangleVertices(DataB, Axes[i], &MinB, &MaxB);

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

    *ResolutionOverlap = SmallestOverlap;
    *ResolutionDirection = SmallestAxis;

    return true;
}

b32 C_CollisionRectangleCircle(rectangle InputRectangle, circle InputCircle, glm::vec2 *ResolutionDirection, f32 *ResolutionOverlap)
{
    Assert(ResolutionDirection);
    Assert(ResolutionOverlap);

    f32 SmallestOverlap = FLT_MAX;
    glm::vec2 SmallestAxis = {};

    rectangle_collision_data RectangleCollisionData = C_GenerateRectangleCollisionData(InputRectangle);

    // Test collision on the rectangle axes
    // Now we have the rectangle vertices, the rectangle SAT axes and the Circle Vertices in world space, let's do this.
    for(i32 i = 0; i < 2; i++)
    {
        // Project all vertices of obb onto one axis
        f32 MinA = 0.0f;
        f32 MaxA = 0.0f;
        C_ProjectRectangleVertices(RectangleCollisionData, RectangleCollisionData.SeparationAxes[i], &MinA, &MaxA);

        // Project the circle center on the obb separation axes, add and substract radius to get min,max
        f32 CircleCenter = dot(InputCircle.Center, RectangleCollisionData.SeparationAxes[i]);
        f32 MinB = CircleCenter - InputCircle.Radius;
        f32 MaxB = CircleCenter + InputCircle.Radius;

        // Get the min and max, than do overlapping
        if(!C_Overlapping1D(MinA, MaxA, MinB, MaxB))
        {
            // There is no overlap, according to SAT, the shapes are not colliding, return false.
            *ResolutionDirection = {};
            *ResolutionOverlap = 0.0f;
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

    *ResolutionOverlap = SmallestOverlap;
    *ResolutionDirection = SmallestAxis;

    // Find the closest vertex of the rectangle to the center of the circle
    glm::vec2 Vertices[4];
    C_RectangleVertices(InputRectangle, Vertices);
    glm::vec2 ClosestVertex = C_ClosestVertexToPoint(Vertices, InputCircle.Center);

    // Get the axis to test
    glm::vec2 Axis = glm::normalize(ClosestVertex - InputCircle.Center);

    // Project Circle to Axis
    f32 CircleMin = glm::dot(InputCircle.Center, Axis) - InputCircle.Radius;
    f32 CircleMax = glm::dot(InputCircle.Center, Axis) + InputCircle.Radius;

    // Project Rectangle
    f32 RectMin;
    f32 RectMax;
    C_ProjectRectangleVertices(RectangleCollisionData, Axis, &RectMin, &RectMax);

    // Get the min and max, than do overlapping
    if(!C_Overlapping1D(RectMin, RectMax, CircleMin, CircleMax))
    {
        // There is no overlap, according to SAT, the shapes are not colliding, return false.
<<<<<<< HEAD
        *ResolutionDirection = {};
        *ResolutionOverlap = 0.0f;

=======
        *CollisionResult = {};
>>>>>>> 6b1935ac85eedd7a2ce9ddeeb2fcd96e0fd4c5e5
        return false;
    }
    else
    {
        // if its the SmallestOverlap, set te variable accordingly
        f32 Overlap = C_GetOverlap(RectMin, RectMax, CircleMin, CircleMax);
        if(Overlap < SmallestOverlap)
        {
            SmallestOverlap = Overlap;
            SmallestAxis = Axis;

            // This if checks the direction in which the obb
            // has to be displaced Got it from:
            // https://gamedev.stackexchange.com/questions/27596/implementing-separating-axis-theorem-sat-and-minimum-translation-vector-mtv/27633#27633
            if(RectMin < CircleMin)
            {
                SmallestAxis *= -1;
            }
        }
    }

<<<<<<< HEAD
    *ResolutionDirection = SmallestAxis;
    *ResolutionOverlap = SmallestOverlap;
=======
    CollisionResult->Overlap = SmallestOverlap;
    CollisionResult->Direction = SmallestAxis;
>>>>>>> 6b1935ac85eedd7a2ce9ddeeb2fcd96e0fd4c5e5

    return true;
}

b32 C_Collision(collider A, collider B, glm::vec2 *ResolutionDirection, f32 *ResolutionOverlap)
{
    Assert(ResolutionDirection);
    Assert(ResolutionOverlap);

    if(A.Type == Collider_Rectangle && B.Type == Collider_Rectangle)
    {
        return C_CollisionRectangleRectangle(A.Rectangle, B.Rectangle, ResolutionDirection, ResolutionOverlap);
    }
    else if(A.Type == Collider_Rectangle && B.Type == Collider_Circle)
    {
        return C_CollisionRectangleCircle(A.Rectangle, B.Circle, ResolutionDirection, ResolutionOverlap);
    }
    else if(A.Type == Collider_Circle && B.Type == Collider_Rectangle)
    {
        return C_CollisionRectangleCircle(B.Rectangle, A.Circle, ResolutionDirection, ResolutionOverlap);
    }
    else if(A.Type == Collider_Circle && B.Type == Collider_Circle)
    {
        return C_CollisionCircleCircle(A.Circle, B.Circle, ResolutionDirection, ResolutionOverlap);
    }
    else
    {
        InvalidCodePath;
        return false;
    }
}
