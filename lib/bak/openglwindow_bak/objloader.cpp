#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>
#include <cmath>

//#include <glm/glm.hpp>

#include "objloader.hpp"

// Very, VERY simple OBJ loader.
// Here is a short list of features a real function would provide :
// - Binary files. Reading a model should be just a few memcpy's away, not parsing a file at runtime. In short : OBJ is not very great.
// - Animations & bones (includes bones weights)
// - Multiple UVs
// - All attributes should be optional, not "forced"
// - More stable. Change a line in the OBJ file and it crashes.
// - More secure. Change another line and you can inject code.
// - Loading from memory, stream, etc

static vec3 getCrossProduct( vec3 p1,vec3 p2)
{
	vec3 res;
	res.x = p1.y * p2.z - p2.y * p1.z; 
	res.y = p1.z * p2.x - p2.z * p1.x; 
	res.z = p1.x * p2.y - p2.x * p1.y;
	return res; 
}

static vec3 vectorNormal(vec3 vector) 
{
	vec3 res;
	float module = (float) sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);

	res.x = vector.x / module;
	res.y = vector.y / module;
	res.z = vector.z / module;

	return res;
}

bool loadOBJ1(
    const char *path,
    std::vector<MATERIAL> &materialList,
    std::vector<OBJECTNAME> &objNameList,
    std::vector< std::vector<vec3> > &out_vertices,
    std::vector< std::vector<vec3> > &out_normals,
    std::vector< std::vector<vec2> > &out_textures,
    std::vector<unsigned int> &materialIdx,
    float *maxX, float *maxY, float *maxZ
)
{
    int i, j;
	static int count = 0;
	OBJECTNAME tmpObjName;
    printf("Loading OBJ file %s...\n", path);
    std::vector< std::vector<unsigned int> > verTexidx;
    std::vector< std::vector<unsigned int> > Normalidx;
	std::vector< std::vector<unsigned int> > textureIdx;
    std::vector<unsigned int> tmp_verticeIdx;
    std::vector<unsigned int> tmp_normalIdx;
	std::vector<unsigned int> tmp_textureIdx;
    std::vector< std::vector<vec3> > temp_verticesGroup;
    std::vector< std::vector<vec3> > TempNormalsGroup;
	std::vector< std::vector<vec2> > tempTextureGroup;
    std::vector<vec3> temp_vertices;
    std::vector<vec3> temp_normals;
    std::vector<vec2> temp_uvs;

    *maxX = 0.0001f;
    *maxY = 0.0001f;
    *maxZ = 0.0001f;

	float minX = 1000000;
	float minY = 1000000;
	float minZ = 1000000;

    FILE *file = fopen(path, "r");
    if( file == NULL )
    {
        printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
        getchar();
        return false;
    }

    while( 1 )
    {
        char lineHeader[128];
        char *obj;
        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.

        // else : parse lineHeader
        if ( strcmp( lineHeader, "v" ) == 0 )
        {
            vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
            temp_vertices.push_back(vertex);
        }
		else if ( strcmp( lineHeader, "vt" ) == 0 )
		{
			vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			//printf("%f %f %f\n",uv.x, uv.y, uv.z);
			uv.y = 1.0-uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}
        else if ( strcmp( lineHeader, "vn" ) == 0 )
        {
            vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
            temp_normals.push_back(normal);
        }
        else if ( strcmp( lineHeader, "f" ) == 0 )
        {
            std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
            //int matches = fscanf(file, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
            //printf("matches = %d \n",matches);
            if (matches == 9)
            {
                //printf("virtex only faces\n");
#if 0
                vertexIndices.push_back(vertexIndex[0]);
                vertexIndices.push_back(vertexIndex[1]);
                vertexIndices.push_back(vertexIndex[2]);

                tmp_verTexidx.push_back(vertexIndex[0]);
                tmp_verTexidx.push_back(vertexIndex[1]);
                tmp_verTexidx.push_back(vertexIndex[2]);
#else
                tmp_verticeIdx.push_back(vertexIndex[0]);
                tmp_verticeIdx.push_back(vertexIndex[1]);
                tmp_verticeIdx.push_back(vertexIndex[2]);
				tmp_textureIdx.push_back(uvIndex[0]);
                tmp_textureIdx.push_back(uvIndex[1]);
                tmp_textureIdx.push_back(uvIndex[2]);
                tmp_normalIdx.push_back(normalIndex[0]);
                tmp_normalIdx.push_back(normalIndex[1]);
                tmp_normalIdx.push_back(normalIndex[2]);
#endif
            }
        }
        else if (obj = strstr( lineHeader, "usemtl"))
        {
            char mtl_name[128];
            fscanf(file, "%s", mtl_name);
            //printf("find one object:  ");
            //printf("material name: %s\n",mtl_name);
            //found the material idx
            for (i = 0; i < materialList.size(); i++)
            {
                if (strcmp(materialList[i].name, mtl_name) == 0)
                {
                    break;
                }
            }
            materialIdx.push_back(i);
            //printf("material index : %d\n",i);
            if (tmp_verticeIdx.size() != 0)
            {
                verTexidx.push_back(tmp_verticeIdx);
                temp_verticesGroup.push_back(temp_vertices);
                tmp_verticeIdx.clear();

                Normalidx.push_back(tmp_normalIdx);
                TempNormalsGroup.push_back(temp_normals);
                tmp_normalIdx.clear();

				textureIdx.push_back(tmp_textureIdx);
                tempTextureGroup.push_back(temp_uvs);
                tmp_textureIdx.clear();
            }
        }
		else if (obj = strstr( lineHeader, "g"))
		{
			//char vertice_name[128];
            fscanf(file, "%s", tmpObjName.name);
			printf("%d %s\n",count, tmpObjName.name);
			objNameList.push_back(tmpObjName);
			count++;
		}
        else
        {
            // Probably a comment, eat up the rest of the line
            char stupidBuffer[1000];
            fgets(stupidBuffer, 1000, file);
        }

    }

	//printf("car 3d MIN size:[x y z] = [%f %f %f]\n", minX, minY, minZ);

    verTexidx.push_back(tmp_verticeIdx);
    temp_verticesGroup.push_back(temp_vertices);

    Normalidx.push_back(tmp_normalIdx);
    TempNormalsGroup.push_back(temp_normals);

	textureIdx.push_back(tmp_textureIdx);
	tempTextureGroup.push_back(temp_uvs);

    std::vector<vec3> tmp_vertex;
	std::vector<vec3> tmp_normal;
	std::vector<vec2> tmp_texture;

#if 1
	unsigned int vertexIndex;
	unsigned int NormalIndex;
	unsigned int textureIndex;
	
	vec3 vertex;
	vec3 normal;
	vec2 texture;

	for (i = 0; i < verTexidx.size(); i++)
	{
		tmp_vertex.clear();
		temp_vertices = temp_verticesGroup[i];
		tmp_normal.clear();
		temp_normals = TempNormalsGroup[i];
		tmp_texture.clear();
		temp_uvs = tempTextureGroup[i];

		for (j = 0; j < verTexidx[i].size(); j++)
		{
			// Get the indices of its attributes
			vertexIndex = verTexidx[i][j];			
			NormalIndex = Normalidx[i][j];
			textureIndex = textureIdx[i][j];
			
			// Get the attributes according to the index
			vertex = temp_vertices[vertexIndex - 1];

			if(vertex.x > *maxX)
			{
				*maxX = vertex.x;
			}
			else if(vertex.x < minX)
			{
				minX = vertex.x;
			}
			
			if(vertex.y > *maxY)
			{
				*maxY = vertex.y;
			}
			else if(vertex.y < minY)
			{
				minY = vertex.y;
			}

			if(strcmp(objNameList[i].name,"WHEEL_F_R") == 0)
			{
				if (vertex.z > *maxZ)
				{
					*maxZ = vertex.z;
					//printf("!!! %f\n",*maxZ);
				}
			}

			if(strcmp(objNameList[i].name,"WHEEL_F_L") == 0)
			{
				if (vertex.z < minZ)
				{
					minZ = vertex.z;
					//printf("### %f\n",minZ);
				}
			}
				  
			normal = temp_normals[NormalIndex - 1];
			texture = temp_uvs[textureIndex  - 1];

			// Put the attributes in buffers
			tmp_vertex.push_back(vertex);
			tmp_normal.push_back(normal);
			tmp_texture.push_back(texture);
		}
		out_vertices.push_back(tmp_vertex);
		out_normals.push_back(tmp_normal);
		out_textures.push_back(tmp_texture);
	}
#endif

    printf("out_vertices size %d\n", out_vertices.size());

	printf("car 3d MIN size:[x y z] = [%f %f %f]\n", minX, minY, minZ);
	printf("car 3d MAX size:[x y z] = [%f %f %f]\n", *maxX, *maxY, *maxZ);

    return true;
}



bool loadOBJ00(
    const char *path,
    std::vector<MATERIAL> &materialList,
    std::vector<OBJECTNAME> &objNameList,
    std::vector< std::vector<vec3> > &out_vertices,
    std::vector< std::vector<vec3> > &out_normals,
    std::vector< std::vector<vec2> > &out_textures,
    std::vector<unsigned int> &materialIdx,
    float *maxX, float *maxY, float *maxZ
)
{
    int i, j;
	static int count = 0;
	OBJECTNAME tmpObjName;
    printf("Loading OBJ file %s...\n", path);
    std::vector< std::vector<unsigned int> > verTexidx;
    std::vector< std::vector<unsigned int> > Normalidx;
	std::vector< std::vector<unsigned int> > textureIdx;
    std::vector<unsigned int> tmp_verticeIdx;
    std::vector<unsigned int> tmp_normalIdx;
	std::vector<unsigned int> tmp_textureIdx;
    std::vector< std::vector<vec3> > temp_verticesGroup;
    std::vector< std::vector<vec3> > TempNormalsGroup;
	std::vector< std::vector<vec2> > tempTextureGroup;
    std::vector<vec3> temp_vertices;
    std::vector<vec3> temp_normals;
    std::vector<vec2> temp_uvs;

    *maxX = 0.0001f;
    *maxY = 0.0001f;
    *maxZ = 0.0001f;

	float minX = 1000000;
	float minY = 1000000;
	float minZ = 1000000;

    FILE *file = fopen(path, "r");
    if( file == NULL )
    {
        printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
        getchar();
        return false;
    }

    while( 1 )
    {
        char lineHeader[128];
        char *obj;
        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.

        // else : parse lineHeader
        if ( strcmp( lineHeader, "v" ) == 0 )
        {
            vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
            temp_vertices.push_back(vertex);
        }
		else if ( strcmp( lineHeader, "vt" ) == 0 )
		{
			vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			//printf("%f %f %f\n",uv.x, uv.y, uv.z);
			uv.y = 1.0-uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}
        else if ( strcmp( lineHeader, "vn" ) == 0 )
        {
            vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.z, &normal.y );
            temp_normals.push_back(normal);
        }
        else if ( strcmp( lineHeader, "f" ) == 0 )
        {
            std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
            //int matches = fscanf(file, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
            //printf("matches = %d \n",matches);
            if (matches == 9)
            {
                //printf("virtex only faces\n");
#if 0
                vertexIndices.push_back(vertexIndex[0]);
                vertexIndices.push_back(vertexIndex[1]);
                vertexIndices.push_back(vertexIndex[2]);

                tmp_verTexidx.push_back(vertexIndex[0]);
                tmp_verTexidx.push_back(vertexIndex[1]);
                tmp_verTexidx.push_back(vertexIndex[2]);
#else
                tmp_verticeIdx.push_back(vertexIndex[0]);
                tmp_verticeIdx.push_back(vertexIndex[1]);
                tmp_verticeIdx.push_back(vertexIndex[2]);
				tmp_textureIdx.push_back(uvIndex[0]);
                tmp_textureIdx.push_back(uvIndex[1]);
                tmp_textureIdx.push_back(uvIndex[2]);
                tmp_normalIdx.push_back(normalIndex[0]);
                tmp_normalIdx.push_back(normalIndex[1]);
                tmp_normalIdx.push_back(normalIndex[2]);
#endif
            }
        }
        else if (obj = strstr( lineHeader, "usemtl"))
        {
            char mtl_name[128];
            fscanf(file, "%s", mtl_name);
            //printf("find one object:  ");
            //printf("material name: %s\n",mtl_name);
            //found the material idx
            for (i = 0; i < materialList.size(); i++)
            {
                if (strcmp(materialList[i].name, mtl_name) == 0)
                {
                    break;
                }
            }
            materialIdx.push_back(i);
            //printf("material index : %d\n",i);
            if (tmp_verticeIdx.size() != 0)
            {
                verTexidx.push_back(tmp_verticeIdx);
                temp_verticesGroup.push_back(temp_vertices);
                tmp_verticeIdx.clear();

                Normalidx.push_back(tmp_normalIdx);
                TempNormalsGroup.push_back(temp_normals);
                tmp_normalIdx.clear();

				textureIdx.push_back(tmp_textureIdx);
                tempTextureGroup.push_back(temp_uvs);
                tmp_textureIdx.clear();
            }
        }
		else if (obj = strstr( lineHeader, "g"))
		{
			//char vertice_name[128];
            fscanf(file, "%s", tmpObjName.name);
			printf("%d %s\n",count, tmpObjName.name);
			objNameList.push_back(tmpObjName);
			count++;
		}
        else
        {
            // Probably a comment, eat up the rest of the line
            char stupidBuffer[1000];
            fgets(stupidBuffer, 1000, file);
        }

    }

	//printf("car 3d MIN size:[x y z] = [%f %f %f]\n", minX, minY, minZ);

    verTexidx.push_back(tmp_verticeIdx);
    temp_verticesGroup.push_back(temp_vertices);

    Normalidx.push_back(tmp_normalIdx);
    TempNormalsGroup.push_back(temp_normals);

	textureIdx.push_back(tmp_textureIdx);
	tempTextureGroup.push_back(temp_uvs);

    std::vector<vec3> tmp_vertex;
	std::vector<vec3> tmp_normal;
	std::vector<vec2> tmp_texture;

#if 1
	unsigned int vertexIndex;
	unsigned int NormalIndex;
	unsigned int textureIndex;
	
	vec3 vertex;
	vec3 normal;
	vec2 texture;

	unsigned int vertexIndex1, vertexIndex2;
	vec3 vertex1, vertex2;

	vec3 vertexA, vertexB;
	vec3 c_normal;

	for (i = 0; i < verTexidx.size(); i++)
	{
		tmp_vertex.clear();
		temp_vertices = temp_verticesGroup[i];
		tmp_normal.clear();
		temp_normals = TempNormalsGroup[i];
		tmp_texture.clear();
		temp_uvs = tempTextureGroup[i];

		for (j = 0; j < verTexidx[i].size(); j++)
		{
			// Get the indices of its attributes
			vertexIndex = verTexidx[i][j];			
			NormalIndex = Normalidx[i][j];
			textureIndex = textureIdx[i][j];

			if(j % 3 == 0)
			{
				vertexIndex1 = verTexidx[i][j + 1];
				vertexIndex2 = verTexidx[i][j + 2];
			}
			
			// Get the attributes according to the index
			vertex = temp_vertices[vertexIndex - 1];

			if(vertex.x > *maxX)
			{
				*maxX = vertex.x;
			}
			else if(vertex.x < minX)
			{
				minX = vertex.x;
			}
			
			if(vertex.y > *maxY)
			{
				*maxY = vertex.y;
			}
			else if(vertex.y < minY)
			{
				minY = vertex.y;
			}

			if (vertex.z > *maxZ)
				{
					*maxZ = vertex.z;
					//printf("!!! %f\n",*maxZ);
				}

			if (vertex.z < minZ)
							{
								minZ = vertex.z;
								//printf("### %f\n",minZ);
							}

			/*if(strcmp(objNameList[i].name,"WHEEL_F_R") == 0)
			{
				if (vertex.z > *maxZ)
				{
					*maxZ = vertex.z;
					//printf("!!! %f\n",*maxZ);
				}
			}

			if(strcmp(objNameList[i].name,"WHEEL_F_L") == 0)
			{
				if (vertex.z < minZ)
				{
					minZ = vertex.z;
					//printf("### %f\n",minZ);
				}
			}*/

			if(j % 3 == 0)
			{
				vertex1 = temp_vertices[vertexIndex1 - 1];
				vertex2 = temp_vertices[vertexIndex2 - 1];

				vertexA.x = vertex1.x - vertex.x;
				vertexA.y = vertex1.y - vertex.y;
				vertexA.z = vertex1.z - vertex.z;

				vertexB.x = vertex2.x - vertex.x;
				vertexB.y = vertex2.y - vertex.y;
				vertexB.z = vertex2.z - vertex.z;

				c_normal = vectorNormal(getCrossProduct(vertexA, vertexB));
			}
				  
			normal = temp_normals[NormalIndex - 1];
			texture = temp_uvs[textureIndex  - 1];

			//printf("obj %f %f %f\n", normal.x, normal.y, normal.z);
			//printf("cal %f %f %f\n\n", c_normal.x, c_normal.y, c_normal.z);

			// Put the attributes in buffers
			tmp_vertex.push_back(vertex);
			tmp_normal.push_back(c_normal);
			tmp_texture.push_back(texture);
		}
		out_vertices.push_back(tmp_vertex);
		out_normals.push_back(tmp_normal);
		out_textures.push_back(tmp_texture);
	}
#endif

    printf("out_vertices size %d\n", out_vertices.size());

	printf("car 3d MIN size:[x y z] = [%f %f %f]\n", minX, minY, minZ);
	printf("car 3d MAX size:[x y z] = [%f %f %f]\n", *maxX, *maxY, *maxZ);

    return true;
}




bool loadOBJ(
    const char *path,
    std::vector<MATERIAL> &materialList,
    std::vector<OBJECTNAME> &objNameList,
    std::vector< std::vector<vec3> > &outVertices,
    std::vector< std::vector<vec3> > &outNormals,
    std::vector< std::vector<vec2> > &outTextures,
    std::vector<unsigned int> &materialIdx,
    float *maxX, float *maxY, float *maxZ
)
{
    int i, j;
	static int count = 0;
	OBJECTNAME tmpObjName;
    printf("Loading OBJ file %s...\n", path);
    /*$)A<GB<6%5c7(OrA?NF@m5DKwR}V5*/
    std::vector< std::vector<unsigned int> > verticeIdx;
    std::vector< std::vector<unsigned int> > normalIdx;
	std::vector< std::vector<unsigned int> > textureIdx;
    std::vector<unsigned int> tmpVerticeIdx;
    std::vector<unsigned int> tmpNormalIdx;
	std::vector<unsigned int> tmpTextureIdx;
    std::vector< std::vector<vec3> > tempVerticesGroup;
    std::vector< std::vector<vec3> > tempNormalsGroup;
	std::vector< std::vector<vec2> > tempTextureGroup;
    std::vector<vec3> tempVertices;
    std::vector<vec3> tempNormals;
    std::vector<vec2> tempUvs;
    
    std::vector<vec3> tmpVertex;
	std::vector<vec3> tmpNormal;
	std::vector<vec2> tmpTexture;

	unsigned int tmpVertexIndex;
	unsigned int tmpNormalIndex;
	unsigned int tmpTextureIndex;
	
	vec3 vertex;
	vec3 normal, aNormal;
	vec2 texture;

    *maxX = 0.0001f;
    *maxY = 0.0001f;
    *maxZ = 0.0001f;

	float minX = 1000000;
	float minY = 1000000;
	float minZ = 1000000;

    FILE *file = fopen(path, "r");
    char lineHeader[128], mtl_name[128], stupidBuffer[1000];
    char *obj;
    int res; 
    vec2 uv;
	unsigned int matches, vertexIndex[3], uvIndex[3], normalIndex[3];
	unsigned int verNum, norNum, texNum;

	vec3 vertex0, vertex1, vertex2;
			
	vec3 vertexA, vertexB;
	vec3 cNormal;


    if( file == NULL )
    {
        printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
        getchar();
        return false;
    }

    while( 1 )
    {
        res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.

        // else : parse lineHeader
        if ( strcmp( lineHeader, "v" ) == 0 )
        {
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
            tempVertices.push_back(vertex);
        }
		else if ( strcmp( lineHeader, "vt" ) == 0 )
		{
			fscanf(file, "%f %f\n", &uv.x, &uv.y);//$)A;qH!NF@mWx1jV5
			//PRINTF("%f %f %f\n",uv.x, uv.y, uv.z);
			uv.y = 1.0-uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			tempUvs.push_back(uv);
		}
        else if ( strcmp( lineHeader, "vn" ) == 0 )//$)A;qH!7(OrA?Wx1jV5
        {
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );	//$)AD?G035D#VP7(OrA?C;SP8zWER;Fp1d6/Wx1jO5#,PhR*JV6/=+y:Mz5DV56T5wOB
            tempNormals.push_back(normal);
        }
        else if ( strcmp( lineHeader, "f" ) == 0 )//$)A;qH!CfVPH}8v5c5D6%5c!"7(OrA?!"NF@m5DKwR}V5
        {
            matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
            
            if (matches == 9)
            {
				/*$)AQ9Hk*/
                tmpVerticeIdx.push_back(vertexIndex[0]);
                tmpVerticeIdx.push_back(vertexIndex[1]);
                tmpVerticeIdx.push_back(vertexIndex[2]);
				tmpTextureIdx.push_back(uvIndex[0]);
                tmpTextureIdx.push_back(uvIndex[1]);
                tmpTextureIdx.push_back(uvIndex[2]);
                tmpNormalIdx.push_back(normalIndex[0]);
                tmpNormalIdx.push_back(normalIndex[1]);
                tmpNormalIdx.push_back(normalIndex[2]);
            }
        }
        else if ( strcmp( lineHeader, "usemtl" ) == 0 )//(obj = strstr( lineHeader, "usemtl"))//$)A;qH!2DVJC{3F
        {
            fscanf(file, "%s", mtl_name);
            //PRINTF("find one obj:  %s\n", lineHeader);
            //PRINTF("material name: %s\n",mtl_name);
            //found the material idx
            /*$)A1i@zKySP5D2DVJC{#,UR5=Ub8vAc<~J9SC5DJGDD8v2DVJ#,<GB<2DVJ5DKwR}:E*/
            for (i = 0; i < materialList.size(); i++)
            {
                if (strcmp(materialList[i].name, mtl_name) == 0)
                {
                    break;
                }
            }
            materialIdx.push_back(i);
        }
		else if ( strcmp( lineHeader, "g" ) == 0 )//(obj = strstr( lineHeader, "g"))//$)A<GB<Ac<~C{3F
		{
			//char vertice_name[128];
            fscanf(file, "%s", tmpObjName.name);
			printf("%d %s\n",count, tmpObjName.name);
			objNameList.push_back(tmpObjName);
			count++;
		}
		else if (obj = strstr( lineHeader, "faces"))//$)A<GB<Ac<~C{3F
		{
			printf("push faces index\n");
			
            verticeIdx.push_back(tmpVerticeIdx);
            tempVerticesGroup.push_back(tempVertices);
            tmpVerticeIdx.clear();
			tempVertices.clear();

            normalIdx.push_back(tmpNormalIdx);
            tempNormalsGroup.push_back(tempNormals);
            tmpNormalIdx.clear();
            tempNormals.clear();

			textureIdx.push_back(tmpTextureIdx);
            tempTextureGroup.push_back(tempUvs);
            tmpTextureIdx.clear();
            tempUvs.clear();
		}
        else
        {
            // Probably a comment, eat up the rest of the line
            //fgets(stupidBuffer, 1000, file);
            //PRINTF("%s\n",stupidBuffer);
        }

    }

	printf("\n\n start rearrange idx \n\n\n");

#if 1
	verNum = 0;
	norNum = 0;
	texNum = 0;

	/*$)A1#4f8w8vAc<~5D6%5cWx1j#,7(OrA?Wx1j:MNF@mWx1j*/
	for (i = 0; i < verticeIdx.size(); i++)
	{
		tmpVertex.clear();
		tempVertices = tempVerticesGroup[i];
		tmpNormal.clear();
		tempNormals = tempNormalsGroup[i];
		tmpTexture.clear();
		tempUvs = tempTextureGroup[i];

		printf("%d %d %d\n", verNum, norNum, texNum );
		for (j = 0; j < verticeIdx[i].size(); j++)
		{
			// Get the indices of its attributes
			tmpVertexIndex = verticeIdx[i][j] - 1 - verNum;			
			tmpNormalIndex = normalIdx[i][j] - 1 - norNum;
			tmpTextureIndex = textureIdx[i][j]  - 1 - texNum;
			
			// Get the attributes according to the index
			vertex = tempVertices[tmpVertexIndex];
			normal = tempNormals[tmpNormalIndex];
			texture = tempUvs[tmpTextureIndex];

			if(vertex.x > *maxX)
			{
				*maxX = vertex.x;
			}
			else if(vertex.x < minX)
			{
				minX = vertex.x;
			}
			
			if(vertex.y > *maxY)
			{
				*maxY = vertex.y;
			}
			else if(vertex.y < minY)
			{
				minY = vertex.y;
			}

			if(strcmp(objNameList[i].name,"WHEEL_F_R") == 0)
			{
				if (vertex.z > *maxZ)
				{
					*maxZ = vertex.z;
					//PRINTF("!!! %f\n",*maxZ);
				}
			}

			if(strcmp(objNameList[i].name,"WHEEL_F_L") == 0)
			{
				if (vertex.z < minZ)
				{
					minZ = vertex.z;
					//PRINTF("### %f\n",minZ);
				}
			}
			
			// Put the attributes in buffers
			tmpVertex.push_back(vertex);
			tmpNormal.push_back(normal);
			tmpTexture.push_back(texture);
		}
		outVertices.push_back(tmpVertex);
		outNormals.push_back(tmpNormal);
		outTextures.push_back(tmpTexture);

		verNum += tempVerticesGroup[i].size();
        norNum += tempNormalsGroup[i].size();
        texNum += tempTextureGroup[i].size();
	}
#endif

    //PRINTF("out_vertices size %d\n", out_vertices.size());

	printf("car 3d MIN size:[x y z] = [%f %f %f]\n", minX, minY, minZ);
	printf("car 3d MAX size:[x y z] = [%f %f %f]\n", *maxX, *maxY, *maxZ);

#if 0
	vertex0.x = 1212.9004 ;
	vertex0.y = -448.8619;
	vertex0.z = -142.6291;

	vertex1.x = 1215.1285 ;
	vertex1.y = -426.9590;
	vertex1.z = -143.2021;

	vertex2.x = 1223.2183;
	vertex2.y = -427.5952;
	vertex2.z =  -143.0923;

	vertexA.x = vertex1.x - vertex0.x;
	vertexA.y = vertex1.y - vertex0.y;
	vertexA.z = vertex1.z - vertex0.z;

	vertexB.x = vertex2.x - vertex0.x;
	vertexB.y = vertex2.y - vertex0.y;
	vertexB.z = vertex2.z - vertex0.z;
#endif

#if 0
	vertex0.x = -894.4929;
	vertex0.y = 230.4886;
	vertex0.z = -1882.0220;

	vertex1.x = -925.5280;
	vertex1.y = 341.2486;
	vertex1.z = -1912.8213;

	vertex2.x = -871.2502;
	vertex2.y = 341.4839;
	vertex2.z =  -1974.6982;

	vertexA.x = vertex1.x - vertex0.x;
	vertexA.y = vertex1.y - vertex0.y;
	vertexA.z = vertex1.z - vertex0.z;

	vertexB.x = vertex2.x - vertex0.x;
	vertexB.y = vertex2.y - vertex0.y;
	vertexB.z = vertex2.z - vertex0.z;
#endif

#if 0
	vertex0.x = 1212.9003;
	vertex0.y = 950.6550;
	vertex0.z = 362.1469;

	vertex1.x = 1215.1284;
	vertex1.y = 951.2280;
	vertex1.z = 384.0497;

	vertex2.x = 1223.2180;
	vertex2.y = 951.1182;
	vertex2.z = 383.4135;

	vertexA.x = vertex1.x - vertex0.x;
	vertexA.y = vertex1.y - vertex0.y;
	vertexA.z = vertex1.z - vertex0.z;

	vertexB.x = vertex2.x - vertex0.x;
	vertexB.y = vertex2.y - vertex0.y;
	vertexB.z = vertex2.z - vertex0.z;

	cNormal = vectorNormal(getCrossProduct(vertexA, vertexB));

	printf("cNormal = %f %f %f\n", cNormal.x, cNormal.y, cNormal.z);
#endif

#if 0
	vertex0.x = 1212.9003;
	vertex0.z = -950.6550;
	vertex0.y = 362.1469;

	vertex1.x = 1215.1284;
	vertex1.z = -951.2280;
	vertex1.y = 384.0497;

	vertex2.x = 1223.2180;
	vertex2.z = -951.1182;
	vertex2.y = 383.4135;

	vertexA.x = vertex1.x - vertex0.x;
	vertexA.y = vertex1.y - vertex0.y;
	vertexA.z = vertex1.z - vertex0.z;

	vertexB.x = vertex2.x - vertex0.x;
	vertexB.y = vertex2.y - vertex0.y;
	vertexB.z = vertex2.z - vertex0.z;

	cNormal = vectorNormal(getCrossProduct(vertexA, vertexB));

	printf("cNormal = %f %f %f\n", cNormal.x, cNormal.y, cNormal.z);
#endif

#if 1
	vertex0.x = 1607.4031;
	vertex0.z = 499.7527;
	vertex0.y = -858.1920;

	vertex1.x = 1607.4032;
	vertex1.z = 499.7526;
	vertex1.y = -640.0670;

	vertex2.x = 1610.3053;
	vertex2.z = 483.2939;
	vertex2.y = -640.0670;

	vertexA.x = vertex1.x - vertex0.x;
	vertexA.y = vertex1.y - vertex0.y;
	vertexA.z = vertex1.z - vertex0.z;

	vertexB.x = vertex2.x - vertex0.x;
	vertexB.y = vertex2.y - vertex0.y;
	vertexB.z = vertex2.z - vertex0.z;

	cNormal = vectorNormal(getCrossProduct(vertexA, vertexB));

	printf("cNormal = %f %f %f\n", cNormal.x, cNormal.y, cNormal.z);
#endif

    return true;
}




vec3 vectorScalar(vec3 input, float scale)
{
	vec3 output;

	output.x = input.x * scale;
	output.y = input.y * scale;
	output.z = input.z * scale;

	return output;
}


vec3 vectorSub(vec3 a, vec3 b)
{
	vec3 output;
	
	output.x = a.x - b.x;
	output.y = a.y - b.y;
	output.z = a.z - b.z;

	return output;
}

vec3 normalizeVector(vec3 in)
{
	vec3 out;
	float f;

	f = sqrt(in.x * in.x + in.y * in.y + in.z * in.z);
	
	out.x = in.x / f;
	out.y = in.y / f;
	out.z = in.z / f;

	return out;

}

//void convertTBN(V3N3UV2* vertices,V3N3UV2TB6* nmVerts)
void convertTBN8(int size, vec3 *vertices, vec3 *normals, vec2 *uvs, vec3 *tangents, vec3 *bitTangents)
{
	int i;
	vec3 vertice[3], tangent, bitTangent;
	vec2 uv[3];
	
    for (i = 0; i <size; i += 3) // $)AR;4N2YWwR;8vH}=GPN5DH}8v5c
    {
        // copy xyz normal uv
        vertice[0]  = vertices[i + 0];
        //normal[0] = normals[i + 0];
        uv[0]  = uvs[i + 0];
 

        vertice[1]  = vertices[i + 1];
        //normal[1] = normals[i + 1];
        uv[1]  = uvs[i + 1];

        vertice[2]  = vertices[i + 2];
        //normal[2] = normals[i + 2];
        uv[2]  = uvs[i + 2];

        // Shortcuts for vertices
        vec3 v0  = {vertices[i + 0].x, vertices[i + 0].y, vertices[i + 0].z};
        vec3 v1  = {vertices[i + 1].x,vertices[i + 1].y,vertices[i + 1].z};
       	vec3 v2  = {vertices[i + 2].x,vertices[i + 2].y,vertices[i + 2].z};
        vec2 uv0 = {uvs[i + 0].x, uvs[i + 0].y};
        vec2 uv1 = {uvs[i + 1].x, uvs[i + 1].y};
        vec2 uv2 = {uvs[i + 2].x, uvs[i + 2].y};
        // $)A99=(triangleF=Cf5D7=OrOrA? (position delta, &D)
        vec3 deltaPos1 = {v1.x - v0.x, v1.y - v0.y, v1.z - v0.z};
        vec3 deltaPos2 = {v2.x - v0.x, v2.y - v0.y, v2.z - v0.z};
        // $)A99=(UVF=Cf5DA=8v7=Or5DOrA? (uv delta, &D)
        vec2 deltaUV1   = {uv1.x - uv0.x, uv1.y - uv0.y};
        vec2 deltaUV2   = {uv2.x - uv0.x, uv2.y - uv0.y};

        float   f  = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);  // uv$)A2f;}Ww5W
        //CELL::float3 tangent    = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r; // $)A5C3vGPO_
        //CELL::float3 binormal   = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r; // $)A5C3v81GPO_

		//tangent = vectorScalar(vectorSub(vectorScalar(deltaPos1, deltaUV2.y), vectorScalar(deltaPos2, deltaUV1.y)), r);
		//bitTangent = vectorScalar(vectorSub(vectorScalar(deltaPos2, deltaUV2.x), vectorScalar(deltaPos1, deltaUV2.x)), r);

		tangent.x = f * (deltaUV2.y * deltaPos1.x - deltaUV1.y * deltaPos2.x);
		tangent.y = f * (deltaUV2.y * deltaPos1.y - deltaUV1.y * deltaPos2.y);
		tangent.z = f * (deltaUV2.y * deltaPos1.z - deltaUV1.y * deltaPos2.z);

		bitTangent.x = f * (-deltaUV2.x * deltaPos1.x + deltaUV1.x * deltaPos2.x);
		bitTangent.y = f * (-deltaUV2.x * deltaPos1.y + deltaUV1.x * deltaPos2.y);
		bitTangent.z = f * (-deltaUV2.x * deltaPos1.z + deltaUV1.x * deltaPos2.z);

		tangent = normalizeVector(tangent);
		bitTangent = normalizeVector(bitTangent);

		printf("%f %f %f\n", tangent.x, tangent.y, tangent.z);
		printf("%f %f %f\n\n", bitTangent.x, bitTangent.y, bitTangent.z);
		printf("%f\n",tangent.x*bitTangent.x+tangent.y*bitTangent.y+tangent.z*bitTangent.z);
        tangents[i + 0] = tangent; bitTangents[i + 0] = bitTangent;
        tangents[i + 1] = tangent; bitTangents[i + 1] = bitTangent;
        tangents[i + 2] = tangent; bitTangents[i + 2] = bitTangent;
    }
}



void convertTBN6(int size, vec3 *vertices, vec3 *normals, vec2 *uvs, vec3 *tangents, vec3 *bitTangents)
{
	int i;
	vec3 vertice[3], normal[3], tangent, bitTangent;
	vec2 uv[3];
	
    for (i = 0; i <size; i += 3) // $)AR;4N2YWwR;8vH}=GPN5DH}8v5c
    {
        // copy xyz normal uv
        vertice[0]  = vertices[i + 0];
        normal[0] = normals[i + 0];
        uv[0]  = uvs[i + 0];
 

        vertice[1]  = vertices[i + 1];
        normal[1] = normals[i + 1];
        uv[1]  = uvs[i + 1];

        vertice[2]  = vertices[i + 2];
        normal[2] = normals[i + 2];
        uv[2]  = uvs[i + 2];

        // Shortcuts for vertices
        vec3 v0  = vertice[0];
        vec3 v1  = vertice[1];
       	vec3 v2  = vertice[2];
        vec2 uv0 = uv[0];
        vec2 uv1 = uv[1];
        vec2 uv2 = uv[2];
		
        // $)A99=(triangleF=Cf5D7=OrOrA? (position delta, &D)
        vec3 deltaPos1 = {v1.x - v0.x, v1.y - v0.y, v1.z - v0.z};
        vec3 deltaPos2 = {v2.x - v0.x, v2.y - v0.y, v2.z - v0.z};
        // $)A99=(UVF=Cf5DA=8v7=Or5DOrA? (uv delta, &D)
        vec2 deltaUV1   = {uv1.x - uv0.x, uv1.y - uv0.y};
        vec2 deltaUV2   = {uv2.x - uv0.x, uv2.y - uv0.y};

        float   f  = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);  // uv$)A2f;}Ww5W

		//tangent = vectorScalar(vectorSub(vectorScalar(deltaPos1, deltaUV2.y), vectorScalar(deltaPos2, deltaUV1.y)), r);
		//bitTangent = vectorScalar(vectorSub(vectorScalar(deltaPos2, deltaUV2.x), vectorScalar(deltaPos1, deltaUV2.x)), r);

		tangent.x = f * (deltaUV2.y * deltaPos1.x - deltaUV1.y * deltaPos2.x);
		tangent.y = f * (deltaUV2.y * deltaPos1.y - deltaUV1.y * deltaPos2.y);
		tangent.z = f * (deltaUV2.y * deltaPos1.z - deltaUV1.y * deltaPos2.z);

		bitTangent.x = f * (-deltaUV2.x * deltaPos1.x + deltaUV1.x * deltaPos2.x);
		bitTangent.y = f * (-deltaUV2.x * deltaPos1.y + deltaUV1.x * deltaPos2.y);
		bitTangent.z = f * (-deltaUV2.x * deltaPos1.z + deltaUV1.x * deltaPos2.z);

		tangent = vectorNormal(tangent);

		
		//bitTangent = getCrossProduct( normal[0],tangent);

		bitTangent = vectorNormal(bitTangent);

		//printf("%f %f %f\n", tangent.x, tangent.y, tangent.z);
		//printf("%f %f %f\n", bitTangent.x, bitTangent.y, bitTangent.z);
		//printf("%f\n\n",tangent.x*bitTangent.x+tangent.y*bitTangent.y+tangent.z*bitTangent.z);
        tangents[i + 0] = tangent; bitTangents[i + 0] = bitTangent;
        tangents[i + 1] = tangent; bitTangents[i + 1] = bitTangent;
        tangents[i + 2] = tangent; bitTangents[i + 2] = bitTangent;
    }
}






void convertTBN(int size, vec3 *vertices, vec3 *normals, vec2 *uvs, vec3 *tangents, vec3 *bitTangents)
{
	int i;
	vec3 vertice[3], normal[3], tangent, bitTangent;
	vec2 uv[3];
	vec3 vec11, vec22;
	float s1, t1, s2, t2, f;
	
    for (i = 0; i <size; i += 3) // $)AR;4N2YWwR;8vH}=GPN5DH}8v5c
    {
        // copy xyz normal uv
        vertice[0]  = vertices[i + 0];
        normal[0] = normals[i + 0];
        uv[0]  = uvs[i + 0];
 

        vertice[1]  = vertices[i + 1];
        normal[1] = normals[i + 1];
        uv[1]  = uvs[i + 1];

        vertice[2]  = vertices[i + 2];
        normal[2] = normals[i + 2];
        uv[2]  = uvs[i + 2];

		vec11.x = vertice[1].x - vertice[0].x;vec11.y = vertice[1].y - vertice[0].y;vec11.z = vertice[1].z - vertice[0].x;
		vec22.x = vertice[2].x - vertice[0].x;vec22.y = vertice[2].y - vertice[0].y;vec22.z = vertice[2].z - vertice[0].x;

		s1 = uv[1].x - uv[0].x;t1 = uv[1].y - uv[0].y;s2 = uv[2].x - uv[0].x;t2 = uv[2].y - uv[0].y;

		f = s1 * t2 - s2 * t1;

		tangent.x = (vec11.x * t2 - vec22.x * t1) / f;
		bitTangent.x = (vec11.x * s1 - vec22.x * s2) / f;

		tangent.y = (vec11.y * t2 - vec22.y * t1) / f;
		bitTangent.y = (vec11.y * s1 - vec22.y * s2) / f;

		tangent.z = (vec11.z * t2 - vec22.z * t1) / f;
		bitTangent.z = (vec11.z * s1 - vec22.z * s2) / f;


		tangent = vectorNormal(tangent);
		bitTangent = vectorNormal(bitTangent);

		//printf("%f %f %f\n", tangent.x, tangent.y, tangent.z);
		//printf("%f %f %f\n", bitTangent.x, bitTangent.y, bitTangent.z);
		//printf("%f\n\n",tangent.x*bitTangent.x+tangent.y*bitTangent.y+tangent.z*bitTangent.z);
        tangents[i + 0] = tangent; bitTangents[i + 0] = bitTangent;
        tangents[i + 1] = tangent; bitTangents[i + 1] = bitTangent;
        tangents[i + 2] = tangent; bitTangents[i + 2] = bitTangent;
    }
}

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
)
{
    int i, j;
	static int count = 0;
	OBJECTNAME tmpObjName;
    printf("Loading OBJ file %s...\n", path);
    /*$)A<GB<6%5c7(OrA?NF@m5DKwR}V5*/
    std::vector< std::vector<unsigned int> > verticeIdx;
    std::vector< std::vector<unsigned int> > normalIdx;
	std::vector< std::vector<unsigned int> > textureIdx;
    std::vector<unsigned int> tmpVerticeIdx;
    std::vector<unsigned int> tmpNormalIdx;
	std::vector<unsigned int> tmpTextureIdx;
    std::vector< std::vector<vec3> > tempVerticesGroup;
    std::vector< std::vector<vec3> > tempNormalsGroup;
	std::vector< std::vector<vec2> > tempTextureGroup;
    std::vector<vec3> tempVertices;
    std::vector<vec3> tempNormals;
    std::vector<vec2> tempUvs;
    
    std::vector<vec3> tmpVertex;
	std::vector<vec3> tmpNormal;
	std::vector<vec2> tmpTexture;
	std::vector<vec3> tmpTangents;
	std::vector<vec3> tmpBitTangent;

	unsigned int tmpVertexIndex;
	unsigned int tmpNormalIndex;
	unsigned int tmpTextureIndex;
	
	vec3 vertex;
	vec3 normal, aNormal;
	vec2 texture;

    *maxX = 0.0001f;
    *maxY = 0.0001f;
    *maxZ = 0.0001f;

	float minX = 1000000;
	float minY = 1000000;
	float minZ = 1000000;

    FILE *file = fopen(path, "r");
    char lineHeader[128], mtl_name[128], stupidBuffer[1000];
    char *obj;
    int res; 
    vec2 uv;
	unsigned int matches, vertexIndex[3], uvIndex[3], normalIndex[3];
	unsigned int verNum, norNum, texNum;

	vec3 vertex0, vertex1, vertex2;
			
	vec3 vertexA, vertexB;
	vec3 cNormal;


    if( file == NULL )
    {
        printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
        getchar();
        return false;
    }

    while( 1 )
    {
        res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.

        // else : parse lineHeader
        if ( strcmp( lineHeader, "v" ) == 0 )
        {
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
            tempVertices.push_back(vertex);
        }
		else if ( strcmp( lineHeader, "vt" ) == 0 )
		{
			fscanf(file, "%f %f\n", &uv.x, &uv.y);//$)A;qH!NF@mWx1jV5
			//PRINTF("%f %f %f\n",uv.x, uv.y, uv.z);
			uv.y = 1.0-uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			tempUvs.push_back(uv);
		}
        else if ( strcmp( lineHeader, "vn" ) == 0 )//$)A;qH!7(OrA?Wx1jV5
        {
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );	//$)AD?G035D#VP7(OrA?C;SP8zWER;Fp1d6/Wx1jO5#,PhR*JV6/=+y:Mz5DV56T5wOB
            tempNormals.push_back(normal);
        }
        else if ( strcmp( lineHeader, "f" ) == 0 )//$)A;qH!CfVPH}8v5c5D6%5c!"7(OrA?!"NF@m5DKwR}V5
        {
            matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
            
            if (matches == 9)
            {
				/*$)AQ9Hk*/
                tmpVerticeIdx.push_back(vertexIndex[0]);
                tmpVerticeIdx.push_back(vertexIndex[1]);
                tmpVerticeIdx.push_back(vertexIndex[2]);
				tmpTextureIdx.push_back(uvIndex[0]);
                tmpTextureIdx.push_back(uvIndex[1]);
                tmpTextureIdx.push_back(uvIndex[2]);
                tmpNormalIdx.push_back(normalIndex[0]);
                tmpNormalIdx.push_back(normalIndex[1]);
                tmpNormalIdx.push_back(normalIndex[2]);
            }
        }
        else if ( strcmp( lineHeader, "usemtl" ) == 0 )//(obj = strstr( lineHeader, "usemtl"))//$)A;qH!2DVJC{3F
        {
            fscanf(file, "%s", mtl_name);
            //PRINTF("find one obj:  %s\n", lineHeader);
            //PRINTF("material name: %s\n",mtl_name);
            //found the material idx
            /*$)A1i@zKySP5D2DVJC{#,UR5=Ub8vAc<~J9SC5DJGDD8v2DVJ#,<GB<2DVJ5DKwR}:E*/
            for (i = 0; i < materialList.size(); i++)
            {
                if (strcmp(materialList[i].name, mtl_name) == 0)
                {
                    break;
                }
            }
            materialIdx.push_back(i);
        }
		else if ( strcmp( lineHeader, "g" ) == 0 )//(obj = strstr( lineHeader, "g"))//$)A<GB<Ac<~C{3F
		{
			//char vertice_name[128];
            fscanf(file, "%s", tmpObjName.name);
			printf("%d %s\n",count, tmpObjName.name);
			objNameList.push_back(tmpObjName);
			count++;
		}
		else if (obj = strstr( lineHeader, "faces"))//$)A<GB<Ac<~C{3F
		{
			printf("push faces index\n");
			
            verticeIdx.push_back(tmpVerticeIdx);
            tempVerticesGroup.push_back(tempVertices);
            tmpVerticeIdx.clear();
			tempVertices.clear();

            normalIdx.push_back(tmpNormalIdx);
            tempNormalsGroup.push_back(tempNormals);
            tmpNormalIdx.clear();
            tempNormals.clear();

			textureIdx.push_back(tmpTextureIdx);
            tempTextureGroup.push_back(tempUvs);
            tmpTextureIdx.clear();
            tempUvs.clear();
		}
        else
        {
            // Probably a comment, eat up the rest of the line
            //fgets(stupidBuffer, 1000, file);
            //PRINTF("%s\n",stupidBuffer);
        }

    }

	printf("\n\n start rearrange idx \n\n\n");

#if 1
	verNum = 0;
	norNum = 0;
	texNum = 0;

	/*$)A1#4f8w8vAc<~5D6%5cWx1j#,7(OrA?Wx1j:MNF@mWx1j*/
	for (i = 0; i < verticeIdx.size(); i++)
	{
		tmpVertex.clear();
		tempVertices = tempVerticesGroup[i];
		tmpNormal.clear();
		tempNormals = tempNormalsGroup[i];
		tmpTexture.clear();
		tempUvs = tempTextureGroup[i];

		tmpTangents.clear();
		tmpBitTangent.clear();

		//printf("%d %d %d\n", verNum, norNum, texNum );
		for (j = 0; j < verticeIdx[i].size(); j++)
		{
			// Get the indices of its attributes
			tmpVertexIndex = verticeIdx[i][j] - 1 - verNum;			
			tmpNormalIndex = normalIdx[i][j] - 1 - norNum;
			tmpTextureIndex = textureIdx[i][j]  - 1 - texNum;
			
			// Get the attributes according to the index
			vertex = tempVertices[tmpVertexIndex];
			normal = tempNormals[tmpNormalIndex];
			texture = tempUvs[tmpTextureIndex];

			if(vertex.x > *maxX)
			{
				*maxX = vertex.x;
			}
			else if(vertex.x < minX)
			{
				minX = vertex.x;
			}
			
			if(vertex.y > *maxY)
			{
				*maxY = vertex.y;
			}
			else if(vertex.y < minY)
			{
				minY = vertex.y;
			}

			//if(strcmp(objNameList[i].name,"WHEEL_F_R") == 0)
			{
				if (vertex.z > *maxZ)
				{
					*maxZ = vertex.z;
					//PRINTF("!!! %f\n",*maxZ);
				}
			}

			//if(strcmp(objNameList[i].name,"WHEEL_F_L") == 0)
			{
				if (vertex.z < minZ)
				{
					minZ = vertex.z;
					//PRINTF("### %f\n",minZ);
				}
			}
			
			// Put the attributes in buffers
			tmpVertex.push_back(vertex);
			tmpNormal.push_back(normal);
			tmpTexture.push_back(texture);
		}

		tmpTangents.resize(tmpVertex.size());
		tmpBitTangent.resize(tmpVertex.size());

		convertTBN6(tmpVertex.size(), &tmpVertex[0], &tmpNormal[0], &tmpTexture[0], &tmpTangents[0], &tmpBitTangent[0]);
		
		outVertices.push_back(tmpVertex);
		outNormals.push_back(tmpNormal);
		outTextures.push_back(tmpTexture);

		outTangents.push_back(tmpTangents);
		outBitTangents.push_back(tmpBitTangent);

		verNum += tempVerticesGroup[i].size();
        norNum += tempNormalsGroup[i].size();
        texNum += tempTextureGroup[i].size();
	}
#endif

    //PRINTF("out_vertices size %d\n", out_vertices.size());

	printf("car 3d MIN size:[x y z] = [%f %f %f]\n", minX, minY, minZ);
	printf("car 3d MAX size:[x y z] = [%f %f %f]\n", *maxX, *maxY, *maxZ);

#if 0
	vertex0.x = 1212.9004 ;
	vertex0.y = -448.8619;
	vertex0.z = -142.6291;

	vertex1.x = 1215.1285 ;
	vertex1.y = -426.9590;
	vertex1.z = -143.2021;

	vertex2.x = 1223.2183;
	vertex2.y = -427.5952;
	vertex2.z =  -143.0923;

	vertexA.x = vertex1.x - vertex0.x;
	vertexA.y = vertex1.y - vertex0.y;
	vertexA.z = vertex1.z - vertex0.z;

	vertexB.x = vertex2.x - vertex0.x;
	vertexB.y = vertex2.y - vertex0.y;
	vertexB.z = vertex2.z - vertex0.z;
#endif

#if 0
	vertex0.x = -894.4929;
	vertex0.y = 230.4886;
	vertex0.z = -1882.0220;

	vertex1.x = -925.5280;
	vertex1.y = 341.2486;
	vertex1.z = -1912.8213;

	vertex2.x = -871.2502;
	vertex2.y = 341.4839;
	vertex2.z =  -1974.6982;

	vertexA.x = vertex1.x - vertex0.x;
	vertexA.y = vertex1.y - vertex0.y;
	vertexA.z = vertex1.z - vertex0.z;

	vertexB.x = vertex2.x - vertex0.x;
	vertexB.y = vertex2.y - vertex0.y;
	vertexB.z = vertex2.z - vertex0.z;
#endif

#if 0
	vertex0.x = 1212.9003;
	vertex0.y = 950.6550;
	vertex0.z = 362.1469;

	vertex1.x = 1215.1284;
	vertex1.y = 951.2280;
	vertex1.z = 384.0497;

	vertex2.x = 1223.2180;
	vertex2.y = 951.1182;
	vertex2.z = 383.4135;

	vertexA.x = vertex1.x - vertex0.x;
	vertexA.y = vertex1.y - vertex0.y;
	vertexA.z = vertex1.z - vertex0.z;

	vertexB.x = vertex2.x - vertex0.x;
	vertexB.y = vertex2.y - vertex0.y;
	vertexB.z = vertex2.z - vertex0.z;

	cNormal = vectorNormal(getCrossProduct(vertexA, vertexB));

	printf("cNormal = %f %f %f\n", cNormal.x, cNormal.y, cNormal.z);
#endif

#if 0
	vertex0.x = 1212.9003;
	vertex0.z = -950.6550;
	vertex0.y = 362.1469;

	vertex1.x = 1215.1284;
	vertex1.z = -951.2280;
	vertex1.y = 384.0497;

	vertex2.x = 1223.2180;
	vertex2.z = -951.1182;
	vertex2.y = 383.4135;

	vertexA.x = vertex1.x - vertex0.x;
	vertexA.y = vertex1.y - vertex0.y;
	vertexA.z = vertex1.z - vertex0.z;

	vertexB.x = vertex2.x - vertex0.x;
	vertexB.y = vertex2.y - vertex0.y;
	vertexB.z = vertex2.z - vertex0.z;

	cNormal = vectorNormal(getCrossProduct(vertexA, vertexB));

	printf("cNormal = %f %f %f\n", cNormal.x, cNormal.y, cNormal.z);
#endif

#if 1
	vertex0.x = 1607.4031;
	vertex0.z = 499.7527;
	vertex0.y = -858.1920;

	vertex1.x = 1607.4032;
	vertex1.z = 499.7526;
	vertex1.y = -640.0670;

	vertex2.x = 1610.3053;
	vertex2.z = 483.2939;
	vertex2.y = -640.0670;

	vertexA.x = vertex1.x - vertex0.x;
	vertexA.y = vertex1.y - vertex0.y;
	vertexA.z = vertex1.z - vertex0.z;

	vertexB.x = vertex2.x - vertex0.x;
	vertexB.y = vertex2.y - vertex0.y;
	vertexB.z = vertex2.z - vertex0.z;

	cNormal = vectorNormal(getCrossProduct(vertexA, vertexB));

	printf("cNormal = %f %f %f\n", cNormal.x, cNormal.y, cNormal.z);
#endif

    return true;
}

