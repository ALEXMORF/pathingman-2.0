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

inline void
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

inline bvh_node *
AllocateBvhNode()
{
    return (bvh_node *)calloc(1, sizeof(bvh_node));
}

inline b32
IsLeaf(bvh_node *Node)
{
    return Node->Left == 0 && Node->Right == 0;
}

inline aabb 
Union(aabb Bound, triangle Triangle)
{
    Bound.Min = Min(Bound.Min, Triangle.E[0]);
    Bound.Min = Min(Bound.Min, Triangle.E[1]);
    Bound.Min = Min(Bound.Min, Triangle.E[2]);
    Bound.Max = Max(Bound.Max, Triangle.E[0]);
    Bound.Max = Max(Bound.Max, Triangle.E[1]);
    Bound.Max = Max(Bound.Max, Triangle.E[2]);
    
    return Bound;
}

inline f32
SurfaceArea(aabb Bound)
{
    if (Bound.Min.X == F32Max  &&
        Bound.Min.Y == F32Max  &&
        Bound.Min.Z == F32Max  &&
        Bound.Max.X == -F32Max &&
        Bound.Max.Y == -F32Max &&
        Bound.Max.Z == -F32Max)
    {
        return 0.0f;
    }
    
    v3 BoundDiff = Bound.Max - Bound.Min;
    f32 FaceArea1 = BoundDiff.X * BoundDiff.Y;
    f32 FaceArea2 = BoundDiff.X * BoundDiff.Z;
    f32 FaceArea3 = BoundDiff.Y * BoundDiff.Z;
    
    return 2.0f * (FaceArea1 + FaceArea2 + FaceArea3);
}

inline int 
SAHSplit(bvh_node *Node, triangle *Triangles, int TriangleCount, int DominantAxis)
{
    bucket_info CostBuckets[20] = {};
    f32 AxisMin = Centroid(Triangles[0]).E[DominantAxis];
    f32 AxisMax = Centroid(Triangles[TriangleCount-1]).E[DominantAxis];
    f32 CentroidMaxDist = AxisMax - AxisMin;
    f32 Interval = CentroidMaxDist / 19.0f;
    for (int BucketIndex = 0; BucketIndex < ARRAY_COUNT(CostBuckets); ++BucketIndex)
    {
        f32 MiddlePoint = AxisMin + (f32)BucketIndex * Interval; 
        
        int MiddleIndex = 0;
        while (Centroid(Triangles[MiddleIndex]).E[DominantAxis] <= MiddlePoint &&
               MiddleIndex < TriangleCount)
        {
            MiddleIndex += 1;
        }
        
        aabb LeftUnion = InitBound();
        for (int TriIndex = 0; TriIndex < MiddleIndex; ++TriIndex)
        {
            LeftUnion = Union(LeftUnion, Triangles[TriIndex]);
        }
        
        aabb RightUnion = InitBound();
        for (int TriIndex = MiddleIndex; TriIndex < TriangleCount; ++TriIndex)
        {
            RightUnion = Union(RightUnion, Triangles[TriIndex]);
        }
        
        int LeftTriCount = MiddleIndex;
        int RightTriCount = TriangleCount-MiddleIndex;
        CostBuckets[BucketIndex].Cost = 0.1f + (SurfaceArea(LeftUnion) * (f32)LeftTriCount + 
                                                SurfaceArea(RightUnion) * (f32)RightTriCount) / SurfaceArea(Node->Bound);
        CostBuckets[BucketIndex].MiddleIndex = MiddleIndex;
    }
    
    int MinBucket = 0;
    for (int BucketIndex = 1; BucketIndex < ARRAY_COUNT(CostBuckets); ++BucketIndex)
    {
        if (CostBuckets[MinBucket].Cost > CostBuckets[BucketIndex].Cost)
        {
            MinBucket = BucketIndex;
        }
    }
    
    return CostBuckets[MinBucket].MiddleIndex;
}

bvh_node *
PartitionTriangles(triangle *Triangles, int TriangleCount)
{
    bvh_node *Node = AllocateBvhNode();
    
    Node->Bound = InitBound();
    for (int TriangleIndex = 0; TriangleIndex < TriangleCount; ++TriangleIndex)
    {
        Node->Bound = Union(Node->Bound, Triangles[TriangleIndex]);
    }
    
    if (TriangleCount > 4) //keep partitioning
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
        
        int Middle = 0;
        if (TriangleCount > 12)
        {
            Middle = SAHSplit(Node, Triangles, TriangleCount, DominantAxis);
        }
        else
        {
            Middle = TriangleCount / 2;
        }
        int Rest = TriangleCount - Middle;
        
        if (Middle != 0 && Rest != 0) //if there is still something to partition
        {
            Node->Left = PartitionTriangles(Triangles, Middle);
            Node->Right = PartitionTriangles(Triangles + Middle, Rest);
        }
        else
        {
            Node->PrimitiveOffset = int(Triangles - Scene.Triangles);
            Node->PrimitiveCount = TriangleCount;
        }
    }
    else //stop and make it leaf node
    {
        Node->PrimitiveOffset = int(Triangles - Scene.Triangles);
        Node->PrimitiveCount = TriangleCount;
    }
    
    return Node;
}

void
FlattenBvh(bvh_node *Node)
{
    if (Node)
    {
        bvh_linear_node LinearNode = {};
        LinearNode.Bound = Node->Bound;
        
        if (IsLeaf(Node))
        {
            LinearNode.IsLeafNode = true;
            
            ASSERT(Node->PrimitiveCount < 1000); // make sure it fits in u16
            LinearNode.PrimitiveOffset = Node->PrimitiveOffset;
            LinearNode.PrimitiveCount = (u16)Node->PrimitiveCount;
            
            BufPush(Scene.LinearNodes, LinearNode);
        }
        else
        {
            LinearNode.IsLeafNode = false;
            
            //push it first
            int CurrNodeIndex = BufLen(Scene.LinearNodes);
            BufPush(Scene.LinearNodes, LinearNode);
            
            //push on left children (left recurring)
            FlattenBvh(Node->Left);
            
            //after stack gets back, now add info about second child offset
            Scene.LinearNodes[CurrNodeIndex].SecondChildOffset = BufLen(Scene.LinearNodes) - CurrNodeIndex;
            
            FlattenBvh(Node->Right);
        }
    }
}

void InitScene()
{
    Scene.SampleCount = 1;
    
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
    BufPush(Scene.Spheres, Sphere(V3(0.0f, 3.0f, -0.4f), 0.5f, 3));
    
    printf("   loading assets ...\n");
    Assets.Dir = "../data/";
    LoadAsset("box.obj");
    LoadAsset("tiger.obj");
    LoadAsset("moose.obj");
    LoadAsset("bigmouth.obj");
    LoadAsset("icosphere.obj");
    LoadAsset("sphinx.obj");
    //LoadAsset("gaul.obj");
    printf("   loading assets done\n");
    
    printf("   instantiating meshes ...\n");
    //InstantiateMesh("tiger", V3(0.0f, 0.65f, 0.0f), 2.0f, Quaternion(YAxis(), 1.2f*Pi32));
    
    //InstantiateMesh("sphinx", V3(-1.0f, 0.0f, 0.0f), 0.8f, Quaternion(YAxis(), 0.8f*Pi32));
    //InstantiateMesh("sphinx", V3(0.8f, 0.0f, 0.0f), 0.8f, Quaternion(YAxis(), 0.8f*Pi32));
    InstantiateMesh("sphinx", V3(0.0f, 0.0f, 0.0f), 0.8f, Quaternion(YAxis(), 0.8f*Pi32));
    //InstantiateMesh("gaul", V3(0.0f, 0.0f, 0.0f), 2.0f, Quaternion(YAxis(), 0.8f*Pi32));
    
    printf("   instantiating meshes done\n");
    
    printf("   partitioning polygons ...\n");
    TriangleCompares[0] = TriangleCompareX;
    TriangleCompares[1] = TriangleCompareY;
    TriangleCompares[2] = TriangleCompareZ;
    Scene.Root = PartitionTriangles(Scene.Triangles, BufLen(Scene.Triangles));
    FlattenBvh(Scene.Root);
    printf("   partitioning polygons done\n");
}
