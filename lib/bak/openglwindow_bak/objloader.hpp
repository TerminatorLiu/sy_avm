#ifndef OBJLOADER_H
#define OBJLOADER_H

#include "mtlloader.hpp"

typedef struct _vec2
{
    float x;
    float y;
} vec2;

typedef struct _vec3
{
    float x;
    float y;
    float z;
} vec3;

typedef struct _vec4
{
    float r;
    float g;
    float b;
    float a;
} vec4;

typedef struct _objectName
{
    char name[STRING_NAME_LENGTH]; // materal name defined follow "usemtl"
} OBJECTNAME;

bool loadOBJ(
    const char *path,
    std::vector<MATERIAL> &materialList,
    std::vector<OBJECTNAME> &objNameList,
    std::vector< std::vector<vec3> > &out_vertices,
    std::vector< std::vector<vec3> > &out_normals,
    std::vector< std::vector<vec2> > &out_textures,
    std::vector<unsigned int> &materialIdx,
    float *maxX, float *maxY, float *maxZ
);

bool loadOBJ6(
    const char *path,
    std::vector<MATERIAL> &materialList,
    std::vector<OBJECTNAME> &objNameList,
    std::vector< std::vector<vec3> > &outVertices,
    std::vector< std::vector<vec3> > &outNormals,
    std::vector< std::vector<vec2> > &outTextures,
    std::vector< std::vector<vec3> > &outTangents,
    std::vector< std::vector<vec3> > &outBitTangents,
    std::vector<unsigned int> &materialIdx,
    float *maxX, float *maxY, float *maxZ
);
#endif