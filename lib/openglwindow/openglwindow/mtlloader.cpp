#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>

//#include <glm/glm.hpp>

#include "mtlloader.hpp"

// Very, VERY simple mlt loader.
// Here is a short list of features a real function would provide :
// - Binary files. Reading a model should be just a few memcpy's away, not parsing a file at runtime. In short : OBJ is not very great.
// - Animations & bones (includes bones weights)
// - Multiple UVs
// - All attributes should be optional, not "forced"
// - More stable. Change a line in the OBJ file and it crashes.
// - More secure. Change another line and you can inject code.
// - Loading from memory, stream, etc

bool loadMTL(
    const char *path,
    std::vector<MATERIAL> &out_materials
)
{
    MATERIAL tmp_material;
    int isFirstTime = 1;
    printf("Loading mtl file %s...\n", path);


    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
        getchar();
        return false;
    }

    while (1)
    {
        char lineHeader[128];
        char *obj;
        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.

        // else : parse lineHeader
        if (strcmp(lineHeader, "newmtl") == 0)
        {
PUSH_PACK:
            if (!isFirstTime)
            {
                out_materials.push_back(tmp_material);
            }
            else
                isFirstTime = 0;
            //read out the materal name
            fscanf(file, "%s", tmp_material.name);

REPEAT:
            // read the materal params out
            int res = fscanf(file, "%s", lineHeader);
            if (res == EOF)
                break; // EOF = End Of File. Quit the loop.

            // alpha
            if (strcmp(lineHeader, "Tf") == 0)
                fscanf(file, "%f %f %f", &tmp_material.Alpha.r, &tmp_material.Alpha.g, &tmp_material.Alpha.b);
            // color
            else if (strcmp(lineHeader, "Ka") == 0)
                fscanf(file, "%f %f %f", &tmp_material.ColorKa.r, &tmp_material.ColorKa.g, &tmp_material.ColorKa.b);
            else if (strcmp(lineHeader, "Kd") == 0)
                fscanf(file, "%f %f %f", &tmp_material.ColorKd.r, &tmp_material.ColorKd.g, &tmp_material.ColorKd.b);
            else if (strcmp(lineHeader, "Ks") == 0)
                fscanf(file, "%f %f %f", &tmp_material.ColorKs.r, &tmp_material.ColorKs.g, &tmp_material.ColorKs.b);
            else if (strcmp(lineHeader, "illum") == 0)
                fscanf(file, "%d", &tmp_material.Illum);
            else if (strcmp(lineHeader, "newmtl") == 0)
            {
                goto PUSH_PACK;
            }
            else
            {
                // for other params, eat up the rest of the line
                char stupidBuffer[1000];
                fgets(stupidBuffer, 1000, file);
            }
            goto REPEAT;

        }
        else
        {
            // for other params, eat up the rest of the line
            char stupidBuffer[1000];
            fgets(stupidBuffer, 1000, file);
        }
    }

    // push the last MATERAL in the vector
    out_materials.push_back(tmp_material);

#if 0
    printf("out_materals size %d \n", out_materials.size());
    for (int i = 0; i < out_materials.size(); i++)
    {
        printf("out_materals[%d].name %s \n", i, out_materials[i].name);
        printf("out_materals[%d].alpha %f %f %f \n", i, out_materials[i].Alpha.r,
               out_materials[i].Alpha.g, out_materials[i].Alpha.b);
        printf("out_materals[%d].colorKa %f %f %f \n", i, out_materials[i].ColorKa.r,
               out_materials[i].ColorKa.g, out_materials[i].ColorKa.b);
        printf("out_materals[%d].colorKd %f %f %f \n", i, out_materials[i].ColorKd.r,
               out_materials[i].ColorKd.g, out_materials[i].ColorKd.b);
        printf("out_materals[%d].colorKs %f %f %f \n", i, out_materials[i].ColorKs.r,
               out_materials[i].ColorKs.g, out_materials[i].ColorKs.b);
        printf("out_materals[%d].Illum %d \n", i, out_materials[i].Illum);
    }
#endif
    return true;
}

void EditMtl(std::vector<MATERIAL> &out_materials, const char *KeyWord, color3 ReplaceColor)
{
    int i;
    for (i = 0; i < out_materials.size(); i++)
    {
        if(strcmp(out_materials[i].name, KeyWord) == 0)
        {
            out_materials[i].ColorKa.r = ReplaceColor.r;
            out_materials[i].ColorKa.g = ReplaceColor.g;
            out_materials[i].ColorKa.b = ReplaceColor.b;

            out_materials[i].ColorKd.r = ReplaceColor.r;
            out_materials[i].ColorKd.g = ReplaceColor.g;
            out_materials[i].ColorKd.b = ReplaceColor.b;
        }
    }
}