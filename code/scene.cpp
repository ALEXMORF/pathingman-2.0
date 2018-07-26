inline plane
Plane(v3 P, v3 N, int MatIndex)
{
    plane Result = {};
    Result.P = P;
    Result.N = N;
    Result.MatIndex = MatIndex;
    return Result;
}

inline sphere
Sphere(v3 P, f32 R, int MatIndex)
{
    sphere Result = {};
    Result.P = P;
    Result.R = R;
    Result.MatIndex = MatIndex;
    return Result;
}

inline material
Mat(v3 RefColor, v3 Emission = {})
{
    material Result = {};
    Result.RefColor = RefColor;
    Result.Emission = Emission;
    return Result;
}

inline v3
CalcNormal(triangle Tri)
{
    return Normalize(Cross(Tri.E[1] - Tri.E[0], Tri.E[2] - Tri.E[0]));
}

void InitScene()
{
    SampleCount = 64;
    
    BufPush(Mats, Mat(V3(0), V3(0.001f)));
    BufPush(Mats, Mat(V3(0.3f)));
    BufPush(Mats, Mat(V3(0.92f)));
    BufPush(Mats, Mat(V3(0.9f), V3(4)));
    BufPush(Mats, Mat(V3(0.9f, 0.6f, 0.2f)));
    BufPush(Mats, Mat(V3(0.9f, 0.3f, 0.2f)));
    
    BufPush(Planes, Plane(V3(0), YAxis(), 1));
    
    BufPush(Spheres, Sphere(V3(2.5f, 3.0f, 0.0f), 0.5f, 3));
    BufPush(Spheres, Sphere(V3(0.0f, 1.0f, 2.0f), 1.0f, 2));
    BufPush(Spheres, Sphere(V3(1.5f, 0.7f, 1.0f), 0.7f, 4));
    
    BufPush(Triangles, {});
    BufLast(Triangles)->E[0] = {-0.5f, 0.2f, -0.0f};
    BufLast(Triangles)->E[1] = {0.5f, 0.2f, 0.7f};
    BufLast(Triangles)->E[2] = {0.0f, 1.2f, 0.7f};
    BufLast(Triangles)->N = CalcNormal(Triangles[0]);
    BufLast(Triangles)->MatIndex = 5;
}


