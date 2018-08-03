char *ReadEntireFile(char *Path)
{
    char *Result = 0;
    
    FILE *File = fopen(Path, "rb");
    if (File)
    {
        fseek(File, 0, SEEK_END);
        int FileSize = ftell(File);
        rewind(File);
        
        Result = (char *)calloc(FileSize+1, sizeof(char));
        fread(Result, sizeof(char), FileSize, File);
        
        fclose(File);
    }
    
    return Result;
}

inline char *
Advance(char *Input)
{
    if (*Input)
    {
        Input += 1;
    }
    return Input;
}

inline char*
AdvanceToNextLine(char *Input)
{
    while (*Input && *Input != '\n')
    {
        Input = Advance(Input);
    }
    Input = Advance(Input);
    return Input;
}

inline bool
IsEqual(char *Str1, char *Str2)
{
    return strcmp(Str1, Str2) == 0;
}

mesh *asset_map::operator[](char *Name)
{
    mesh *Result = 0;
    for (int MeshIndex = 0; MeshIndex < BufLen(Meshes); ++MeshIndex)
    {
        if (IsEqual(Meshes[MeshIndex].Name, Name))
        {
            Result = Meshes + MeshIndex;
            break;
        }
    }
    return Result;
}

void LoadAsset(char *Filename)
{
    char FilePath[255] = {};
    strcpy(FilePath, Assets.Dir);
    strcat(FilePath, Filename);
    
    char *ObjContent = ReadEntireFile(FilePath);
    ASSERT(ObjContent);
    
    v3 *Vertices = 0;
    v3 *Normals = 0;
    triangle *Triangles = 0;
    
    char *ObjContentWalker = ObjContent;
    while (*ObjContentWalker)
    {
        char LeadWord[255];
        sscanf(ObjContentWalker, "%s", LeadWord);
        
        if (IsEqual(LeadWord, "v"))
        {
            v3 NewVertex = {};
            sscanf(ObjContentWalker, "v %f %f %f", &NewVertex.X, &NewVertex.Y, &NewVertex.Z);
            BufPush(Vertices, NewVertex);
        }
        else if (IsEqual(LeadWord, "vn"))
        {
            v3 NewNormal = {};
            sscanf(ObjContentWalker, "vn %f %f %f", &NewNormal.X, &NewNormal.Y, &NewNormal.Z);
            BufPush(Normals, NewNormal);
        }
        else if (IsEqual(LeadWord, "f"))
        {
            triangle NewTriangle = {};
            
            int AIndex, BIndex, CIndex, ANIndex, BNIndex, CNIndex;
            sscanf(ObjContentWalker, "f %d//%d %d//%d %d//%d", 
                   &AIndex, &ANIndex, &BIndex, &BNIndex, &CIndex, &CNIndex);
            
            //NOTE(chen): OBJ's indexing is base 1
            NewTriangle.E[0] = Vertices[AIndex-1];
            NewTriangle.E[1] = Vertices[BIndex-1];
            NewTriangle.E[2] = Vertices[CIndex-1];
            NewTriangle.N = Normals[ANIndex-1];
            NewTriangle.MatIndex = 4;
            
            BufPush(Triangles, NewTriangle);
        }
        
        ObjContentWalker = AdvanceToNextLine(ObjContentWalker);
    }
    
    BufFree(Vertices);
    BufFree(Normals);
    free(ObjContent);
    
    char *FileExtension = strrchr(Filename, '.');
    int FileNameLen = (int)(FileExtension - Filename);
    
    mesh NewMesh = {};
    NewMesh.E = Triangles;
    strncpy(NewMesh.Name, Filename, FileNameLen);
    BufPush(Assets.Meshes, NewMesh);
}