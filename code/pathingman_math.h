#pragma once

struct aabb
{
    v3 Min;
    v3 Max;
};

v3 Power(v3 V, f32 Exp)
{
    v3 Result;
    Result.X = powf(V.X, Exp);
    Result.Y = powf(V.Y, Exp);
    Result.Z = powf(V.Z, Exp);
    return Result;
}

v3 Exp(v3 V)
{
    v3 Result;
    Result.X = expf(V.X);
    Result.Y = expf(V.Y);
    Result.Z = expf(V.Z);
    return Result;
}

aabb
InitBound()
{
    aabb Result = {};
    Result.Min = V3(F32Max);
    Result.Max = V3(-F32Max);
    return Result;
}
