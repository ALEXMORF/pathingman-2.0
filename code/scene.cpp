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

inline v3
Centroid(triangle Tri)
{
    return 0.333f * Tri.E[0] + 0.333f * Tri.E[1] + 0.333f * Tri.E[2];
}

int TriangleCompareX(const void *A, const void *B)
{
    triangle *TriangleA = (triangle *)A;
    triangle *TriangleB = (triangle *)B;
    
    v3 CentroidA = Centroid(*TriangleA);
    v3 CentroidB = Centroid(*TriangleB);
    
    if (CentroidA.X < CentroidB.X)
    {
        return -1;
    }
    else if (CentroidA.X > CentroidB.X)
    {
        return 1;
    }
    
    return 0;
}

int TriangleCompareY(const void *A, const void *B)
{
    triangle *TriangleA = (triangle *)A;
    triangle *TriangleB = (triangle *)B;
    
    v3 CentroidA = Centroid(*TriangleA);
    v3 CentroidB = Centroid(*TriangleB);
    
    if (CentroidA.Y < CentroidB.Y)
    {
        return -1;
    }
    else if (CentroidA.Y > CentroidB.Y)
    {
        return 1;
    }
    
    return 0;
}

int TriangleCompareZ(const void *A, const void *B)
{
    triangle *TriangleA = (triangle *)A;
    triangle *TriangleB = (triangle *)B;
    
    v3 CentroidA = Centroid(*TriangleA);
    v3 CentroidB = Centroid(*TriangleB);
    
    if (CentroidA.Z < CentroidB.Z)
    {
        return -1;
    }
    else if (CentroidA.Z > CentroidB.Z)
    {
        return 1;
    }
    
    return 0;
}

void
InstantiateMesh(char *MeshName, v3 Offset, f32 Scale = 1.0f, quaternion Quat = Quaternion())
{
    mesh *Mesh = Assets[MeshName];
    for (int TriIndex = 0; TriIndex < BufLen(Mesh->E); ++TriIndex)
    {
        triangle NewTriangle = Mesh->E[TriIndex];
        
        NewTriangle.E[0] *= Scale;
        NewTriangle.E[1] *= Scale;
        NewTriangle.E[2] *= Scale;
        
        NewTriangle.E[0] = Rotate(NewTriangle.E[0], Quat);
        NewTriangle.E[1] = Rotate(NewTriangle.E[1], Quat);
        NewTriangle.E[2] = Rotate(NewTriangle.E[2], Quat);
        NewTriangle.N = Rotate(NewTriangle.N, Quat);
        
        NewTriangle.E[0] += Offset;
        NewTriangle.E[1] += Offset;
        NewTriangle.E[2] += Offset;
        
        BufPush(Scene.Triangles, NewTriangle);
    }
}

bvh_node *
AllocateBvhNode()
{
    return (bvh_node *)calloc(1, sizeof(bvh_node));
}

bvh_node *
PartitionTriangles(triangle *Triangles, int TriangleCount)
{
    bvh_node *Node = AllocateBvhNode();
    
    Node->Bound = InitBound();
    for (int TriangleIndex = 0; TriangleIndex < TriangleCount; ++TriangleIndex)
    {
        triangle Triangle = Triangles[TriangleIndex];
        
        Node->Bound.Min = Min(Node->Bound.Min, Triangle.E[0]);
        Node->Bound.Min = Min(Node->Bound.Min, Triangle.E[1]);
        Node->Bound.Min = Min(Node->Bound.Min, Triangle.E[2]);
        Node->Bound.Max = Max(Node->Bound.Max, Triangle.E[0]);
        Node->Bound.Max = Max(Node->Bound.Max, Triangle.E[1]);
        Node->Bound.Max = Max(Node->Bound.Max, Triangle.E[2]);
    }
    
    if (TriangleCount > 3) //keep partitioning
    {
        v3 BoundDiff = Node->Bound.Max - Node->Bound.Min;
        
        int DominantAxis = 0;
        f32 DominantLength = BoundDiff.X;
        if (BoundDiff.Y > DominantLength)
        {
            DominantAxis = 1;
            DominantLength = BoundDiff.Y;
        }
        if (BoundDiff.Z > DominantLength)
        {
            DominantAxis = 2;
            DominantLength = BoundDiff.Z;
        }
        
        qsort(Triangles, TriangleCount, sizeof(triangle), 
              TriangleCompares[DominantAxis]);
        
        int Middle = TriangleCount / 2;
        int Rest = TriangleCount - Middle;
        Node->Left = PartitionTriangles(Triangles, Middle);
        Node->Right = PartitionTriangles(Triangles + Middle, Rest);
    }
    else //stop and make it leaf node
    {
        Node->Primitives = Triangles;
        Node->PrimitiveCount = TriangleCount;
    }
    
    return Node;
}

void InitScene()
{
    Scene.SampleCount = 64;
    
    Scene.CamLookAt = {0, 1.0f, 0.0f};
    Scene.CamRo = {0, 1.8f, -3.0f};
    
    Scene.NullMatIndex = 0;
    BufPush(Scene.Mats, Mat(V3(0), V3(0.00f))); // null
    
    BufPush(Scene.Mats, Mat(V3(0.6f))); // 1
    BufPush(Scene.Mats, Mat(V3(0.92f))); // 2
    BufPush(Scene.Mats, Mat(V3(0.9f), V3(25))); // 3
    BufPush(Scene.Mats, Mat(V3(0.9f, 0.6f, 0.2f))); // 4
    BufPush(Scene.Mats, Mat(V3(0.9f, 0.3f, 0.2f))); // 5
    
    BufPush(Scene.Planes, Plane(V3(0), YAxis(), 1));
    BufPush(Scene.Spheres, Sphere(V3(0.0f, 3.0f, 0.0f), 0.5f, 3));
    
    printf("   loading assets ...\n");
    Assets.Dir = "../data/";
    LoadAsset("box.obj");
    LoadAsset("tiger.obj");
    LoadAsset("moose.obj");
    LoadAsset("bigmouth.obj");
    LoadAsset("icosphere.obj");
    LoadAsset("sphinx.obj");
    printf("   loading assets done\n");
    
    printf("   instantiating meshes ...\n");
    //InstantiateMesh("tiger", V3(0.0f, 0.65f, 0.0f), 2.0f, Quaternion(YAxis(), 1.2f*Pi32));
    InstantiateMesh("sphinx", V3(-0.1f, 0.0f, 0.0f), 0.8f, Quaternion(YAxis(), 0.8f*Pi32));
    
    printf("   instantiating meshes done\n");
    
    printf("   partitioning polygons ...\n");
    TriangleCompares[0] = TriangleCompareX;
    TriangleCompares[1] = TriangleCompareY;
    TriangleCompares[2] = TriangleCompareZ;
    Scene.Root = PartitionTriangles(Scene.Triangles, BufLen(Scene.Triangles));
    printf("   partitioning polygons done\n");
}
