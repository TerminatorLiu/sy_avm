#include <GLES3/gl3.h>
#include <EGL/egl.h>

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <vector>
#include <string>

#include "mtlloader.hpp"
#include "objloader.hpp"

#include "InitMosaic.hpp"
//#include "esUtil.h"

//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"


using std::string;

#define PI 3.141592653589793238462643383279

extern void readParamsXML(int flag);

std::vector<MATERIAL> carMaterialList;
std::vector<OBJECTNAME> carObjectList;
std::vector<GLuint> carProgramIDList;
std::vector<GLuint> carFragmentShaderIDList;
std::vector<GLuint> carVertexShaderIDList;
std::vector<GLuint> carVertexPositionIDList;
std::vector<GLuint> carNormalPositionIDList;
std::vector<GLuint> carTangentPositionIDList;  
std::vector<GLuint> carBitTangentPositionIDList; 
std::vector<GLuint> carTexturePositionIDList;
std::vector<GLuint> carMatrixModelIDList;
std::vector<GLuint> carMatrixMVPIDList;
std::vector<GLuint> carMatrixNormalIDList;
std::vector<GLuint> carFragLightPosIDList;
std::vector<GLuint> carFragViewPosIDList;
std::vector<GLuint> carFragKdColorIDList;
std::vector<GLuint> carFragAlphaIDList;
std::vector<GLuint> carTextureIDList;
std::vector<GLuint> carLocTextureIDList;
std::vector<GLuint> carLocCubeTextureIDList;
std::vector<GLuint> carFragLightStrengthIDList;

std::vector< std::vector< vec3 > > carVertices;
std::vector< std::vector< vec3 > > carNormals;
std::vector< std::vector< vec2 > > carTextures;
std::vector< std::vector< vec3 > > carTangents;		
std::vector< std::vector< vec3 > > carBitTangents;		

std::vector< unsigned int > carMaterialIdx;

TexCoords texCoords;
ObjPoints objPoints;
VertexCoords vertexCoords;
BlendAlpha blendAlpha;
LumiaAdjust lumiaAdjust;

TexCoords texCoords2D;  	
ObjPoints objPoints2D;	
VertexCoords vertexCoords2D;	
BlendAlpha blendAlpha2D;		
LumiaAdjust lumiaAdjust2D;


ObjPointsStatistics objPointsStatistics;
TexCoordsStatistics texCoordsStatistics;

ObjPointsStatistics objPointsStatistics2D;	
TexCoordsStatistics texCoordsStatistics2D;	


VBO3DMosaicImage VBO3DMosaicImageParams;
VBO3DMosaicImage VBO2DMosaicImageParams;
VBO3DCarModel VBO3DCarModelParams;

GLuint curveVerticesPoints[2];
GLuint cameraVerTexCoord[2];


SimplifyCamParams SimplifyfrontCamParams;
SimplifyCamParams SimplifyrearCamParams;
SimplifyCamParams SimplifyleftCamParams;
SimplifyCamParams SimplifyrightCamParams;

PARA_FIELD para_field;

float carScaleX, carScaleY, carScaleZ;


CvPoint3D32f verticesRearTrajLinePoint[11][LENGTH * 2];


GLfloat glVertices3DCar[] =
{
    -1.0f, 0.0f,  1.0f, // left-buttom
    1.0f, 0.0f,  1.0f, // right- buttom
    1.0f, 0.0f, -1.0f, // left-top
    -1.0f, 0.0f, -1.0f, // right-top
};

GLfloat glVertices2DCar[] =
{
    -1.0f, -1.0f, 0.0f,  // left-buttom
    1.0f, -1.0f, 0.0f,  // right- buttom
    -1.0f,  1.0f, 0.0f,   // right-top
    1.0f,  1.0f, 0.0f,   // left-top
};




GLfloat glTexCoordCar[] =
{
    0.0f,  1.0f,  // left-top
    1.0f,  1.0f,  // right-top
    0.0f,  0.0f,  // left-buttom
    1.0f,  0.0f,  // right- buttom
};

extern int bvs3DWidth;
extern int bvs3DHeight;



//$)Ah=?(!?>g$:???2e?
char *carVertexCarSourceTex =
{
    "#version 300 es						  							\n"
    "layout(location = 0) in  vec3 aPos;								\n"
    "layout(location = 1) in  vec3 aNormal;								\n"
    "layout(location = 2) in vec2 aTexcoord;              				\n"
    "uniform mat4 mvp;											\n"
    "out vec3 fragNormal;												\n"
    "out vec3 fragPos;													\n"
    "smooth centroid out vec2 fragUV;									\n"
    "void main()														\n"
    "{																	\n"
    "	 fragNormal = aNormal; 					\n"
    "	 fragUV = aTexcoord;						\n"
    "	 fragPos = aPos;						\n"
    "	 gl_Position = mvp * vec4(aPos, 1.0); 	\n"
    "}																	\n"
};


char *carVertexCarSourceADS =
{
    "#version 300 es						  							\n"
    "layout(location = 0) in  vec3 aPos;								\n"
    "layout(location = 1) in  vec3 aNormal;								\n"
    
    "uniform mat4 mvp;											\n"
    
    "out vec3 fragNormal;												\n"
    "out vec3 fragPos;													\n"
    "void main()														\n"
    "{																	\n"
    "	 fragNormal = aNormal; 					\n"
    "	 fragPos = aPos;						\n"
    "	 gl_Position = mvp * vec4(aPos, 1.0); 	\n"
    "}																	\n"
};


char *carFragmentCarSourceReflect =
{
    "#version 300 es																		\n"
	"precision highp float; 																\n"
	"uniform mat4 model;												\n"
	"uniform mat4 normalMatrix; 										\n"
	"uniform vec3 KdColor;																	\n"
    "uniform vec3 lightPos; 																\n"
    "uniform vec3 viewPos;																	\n"
    "uniform vec3 lightStrength;																	\n"
    "uniform float alpha;																	\n"
    "uniform samplerCube cubeTexture;															\n"
    "in vec3 fragNormal;																	\n"
	"in vec3 fragPos;																		\n"
	"out vec4 fragColor;																	\n"
    "void main()																			\n"
    "{																						\n"
    "	float ambientStrength = 0.5;											\n"
    "	float shininess = 2048.0;															\n"

	"	vec3 worldVertexPosition = (model * vec4(fragPos, 1.0)).xyz;								\n"
	"	vec3 transformedNormal = normalize((normalMatrix * vec4(fragNormal, 1.0)).xyz);		\n"
   	"	vec3 eyeVector = normalize(viewPos - worldVertexPosition);					\n"
	"	vec3 normalizedLightDirection = normalize(lightPos - worldVertexPosition);						\n"

	"	float diffuseStrength = dot(normalizedLightDirection, transformedNormal);			\n"
	"	diffuseStrength = clamp(diffuseStrength, 0.0, 1.0);\n"
	"	vec3 diffuse = diffuseStrength * KdColor;    \n"

	"	vec3 ambient = vec3(ambientStrength) * KdColor;     					\n"
	"	vec3 reflectVec = normalize(reflect(-eyeVector, transformedNormal)); 						\n"
    "	vec3 cubeTexColor = texture(cubeTexture, reflectVec).rgb;						\n"
   	"	ambient += 0.3 * diffuseStrength *  cubeTexColor;		\n"

	"	vec3 halfVector = normalize(normalizedLightDirection + eyeVector);\n"
	"	float specularStrength = dot(halfVector, transformedNormal);\n"
	"	specularStrength = pow(specularStrength, shininess);\n"
	"	vec3 specular = specularStrength * KdColor;\n"

	"	vec3 finalColor = diffuse + ambient + specular; \n"
	"	fragColor = vec4(finalColor, alpha); \n"
    "}																						\n"
};


char *carFragmentCarSourceTex =
{
    "#version 300 es																		\n"
	"precision highp float; 																\n"
	"uniform mat4 model;												\n"
	"uniform mat4 normalMatrix; 										\n"	
    "uniform vec3 lightPos; 																\n"
    "uniform vec3 viewPos;																	\n"
    "uniform vec3 lightStrength;																	\n"
    "uniform float alpha;																	\n"
    "uniform sampler2D s_texture;															\n"
    "in vec3 fragNormal;																	\n"
	"in vec3 fragPos;																		\n"
	"smooth centroid in vec2 fragUV;														\n"
	"out vec4 fragColor;																	\n"
	"void main()																			\n"
    "{																						\n"
    "	float ambientStrength = lightStrength.x;											\n"
    "	float shininess = 64.0;															\n"

	"	vec3 worldVertexPosition = (model * vec4(fragPos, 1.0)).xyz;								\n"
	"	vec3 transformedNormal = normalize((normalMatrix * vec4(fragNormal, 1.0)).xyz);		\n"
   	"	vec3 eyeVector = normalize(viewPos - worldVertexPosition);					\n"
	"	vec3 normalizedLightDirection = normalize(lightPos - worldVertexPosition);						\n"

	"	float diffuseStrength = dot(normalizedLightDirection, transformedNormal);			\n"
	"	diffuseStrength = clamp(diffuseStrength, 0.0, 1.0);\n"

	"	vec3 halfVector = normalize(normalizedLightDirection + eyeVector);\n"
	"	float specularStrength = dot(halfVector, transformedNormal);\n"
	"	specularStrength = pow(specularStrength, shininess);\n"

	"	 vec3 texColor = texture(s_texture, fragUV).rgb;							\n"

	"	vec3 finalColor = (diffuseStrength + ambientStrength + specularStrength) * texColor; \n"
	"	fragColor = vec4(finalColor, alpha); \n"
    "}																						\n"
};

char *carFragmentCarSourceADS =
{
    "#version 300 es																		\n"
	"precision highp float; 																\n"
	"uniform mat4 model;												\n"
	"uniform mat4 normalMatrix; 										\n"
	"uniform vec3 KdColor;																	\n"
    "uniform vec3 lightPos; 																\n"
    "uniform vec3 viewPos;																	\n"
    "uniform vec3 lightStrength;																	\n"
    "uniform float alpha;																	\n"
    "in vec3 fragNormal;																	\n"
	"in vec3 fragPos;																		\n"
	"out vec4 fragColor;																	\n"
    "void main()																			\n"
    "{																						\n"
    "	float ambientStrength = lightStrength.x;											\n"
    "	float shininess = 64.0;															\n"

	"	vec3 worldVertexPosition = (model * vec4(fragPos, 1.0)).xyz;								\n"
	"	vec3 transformedNormal = normalize((normalMatrix * vec4(fragNormal, 1.0)).xyz);		\n"
   	"	vec3 eyeVector = normalize(viewPos - worldVertexPosition);					\n"
	"	vec3 normalizedLightDirection = normalize(lightPos - worldVertexPosition);						\n"

	"	float diffuseStrength = dot(normalizedLightDirection, transformedNormal);			\n"
	"	diffuseStrength = clamp(diffuseStrength, 0.0, 1.0);\n"
	"	vec3 diffuse = diffuseStrength * KdColor;    \n"

	"	vec3 ambient = vec3(ambientStrength) * KdColor;     					\n"
	
	"	vec3 halfVector = normalize(normalizedLightDirection + eyeVector);\n"
	"	float specularStrength = dot(halfVector, transformedNormal);\n"
	"	specularStrength = pow(specularStrength, shininess);\n"
	"	vec3 specular = specularStrength * KdColor;\n"

	"	vec3 finalColor = diffuse + ambient + specular; \n"
	"	fragColor = vec4(finalColor, alpha); \n"
    "}																						\n"
};



char *carVertexCarSourceTex2 =
{
    "#version 300 es						  							\n"
    "layout(location = 0) in vec3 aPos;								\n"
    "layout(location = 1) in vec3 aNormal;								\n"
    "layout(location = 2) in vec2 aTexcoord;              				\n"
    "layout(location = 3) in vec3 aTangent;								\n"
    "layout(location = 4) in vec3 aBitTangent;              				\n"
    "uniform mat4 mvp;											\n"
    "out vec3 fragNormal;												\n"
    "out vec3 fragPos;													\n"
    "out vec3 fragTangent;												\n"
    "out vec3 fragBitTangent;													\n"
    "smooth centroid out vec2 fragUV;									\n"
    "void main()														\n"
    "{																	\n"
    "	 fragNormal = aNormal; 					\n"
    "	 fragUV = aTexcoord;						\n"
    "	 fragPos = aPos;						\n"
    "	 fragTangent = aTangent;						\n"
    "	 fragBitTangent = aBitTangent;						\n"
    "	 gl_Position = mvp * vec4(aPos, 1.0); 	\n"
    "}																	\n"
};



char *carFragmentCarSourceTex2 =
{
    "#version 300 es																		\n"
	"precision highp float; 																\n"
	"uniform mat4 model;												\n"
	"uniform mat4 normalMatrix; 										\n"	
	"uniform vec3 KdColor;																	\n"
    "uniform vec3 lightPos; 																\n"
    "uniform vec3 viewPos;																	\n"
    "uniform vec3 lightStrength;																	\n"
    "uniform float alpha;																	\n"
    "uniform sampler2D normalTexture;															\n"
    "in vec3 fragNormal;																	\n"
	"in vec3 fragPos;																		\n"
	"in vec3 fragTangent;												\n"
	"in vec3 fragBitTangent;													\n"
	"smooth centroid in vec2 fragUV;														\n"
	"out vec4 fragColor;																	\n"
	"void main()																			\n"
    "{																						\n"
    "	float ambientStrength = lightStrength.x;											\n"
    "	float shininess = 64.0;															\n"

	"	vec3 worldVertexPosition = (model * vec4(fragPos, 1.0)).xyz;								\n"

	"	vec3 transformedNormal = normalize((normalMatrix * vec4(fragNormal, 1.0)).xyz);		\n"
	"	vec3 transformedTangent = normalize((normalMatrix * vec4(fragTangent, 1.0)).xyz);		\n"
	"	vec3 transformedBitTangent = normalize((normalMatrix * vec4(fragBitTangent, 1.0)).xyz);		\n"
	"	mat3 TBN = mat3x3(transformedTangent, transformedBitTangent, transformedNormal);\n"

	"	vec3 normal = normalize(TBN * (texture(normalTexture, fragUV).xyz*2.0 - 1.0));"
	//"	vec3 normal = normalize((normalMatrix * vec4(fragNormal, 1.0)).xyz); 	\n"

   	"	vec3 eyeVector = normalize(viewPos - worldVertexPosition);					\n"
	"	vec3 normalizedLightDirection = normalize(lightPos - worldVertexPosition);						\n"

	"	float diffuseStrength = dot(normalizedLightDirection, normal);			\n"
	"	diffuseStrength = clamp(diffuseStrength, 0.0, 1.0);\n"

	"	vec3 halfVector = normalize(normalizedLightDirection + eyeVector);\n"
	"	float specularStrength = dot(halfVector, normal);\n"
	"	specularStrength = pow(specularStrength, shininess);\n"

	"	vec3 finalColor = (diffuseStrength + ambientStrength + specularStrength) * KdColor; \n"
	"	fragColor = vec4(KdColor, alpha); \n"
	//"	fragColor = texture(normalTexture, fragUV); \n"
    "}																						\n"
};



char *carVertexCarSourceTex3 =
{
    "#version 300 es						  							\n"
    "layout(location = 0) in vec3 aPos;								\n"
    "layout(location = 1) in vec3 aNormal;								\n"
    "layout(location = 2) in vec2 aTexcoord;              				\n"
    "layout(location = 3) in vec3 aTangent;								\n"
    "layout(location = 4) in vec3 aBitTangent;              				\n"
    "uniform mat4 mvp;											\n"
    "out vec3 fragNormal;												\n"
    "out vec3 fragPos;													\n"
    "out vec3 fragTangent;												\n"
    "out vec3 fragBitTangent;													\n"
    "smooth centroid out vec2 fragUV;									\n"
    "void main()														\n"
    "{																	\n"
    "	 fragNormal = normalize(aNormal); 					\n"
    "	 fragUV = aTexcoord;						\n"
    "	 fragPos = aPos;						\n"
    "	 fragTangent = normalize(aTangent);						\n"
    "	 fragBitTangent = normalize(aBitTangent);						\n"
    "	 gl_Position = mvp * vec4(aPos, 1.0); 	\n"
    "}																	\n"
};


char *carFragmentCarSourceTex3 =
{
    "#version 300 es																		\n"
	"precision highp float; 																\n"
	"uniform mat4 model;												\n"
	"uniform mat4 normalMatrix; 										\n"	
	"uniform vec3 KdColor;																	\n"
    "uniform vec3 lightPos; 																\n"
    "uniform vec3 viewPos;																	\n"
    "uniform vec3 lightStrength;																	\n"
    "uniform float alpha;																	\n"
    "uniform sampler2D normalTexture;															\n"
    "in vec3 fragNormal;																	\n"
	"in vec3 fragPos;																		\n"
	"in vec3 fragTangent;												\n"
	"in vec3 fragBitTangent;													\n"
	"smooth centroid in vec2 fragUV;														\n"
	"out vec4 fragColor;																	\n"
	"void main()																			\n"
    "{																						\n"
    "	float ambientStrength = lightStrength.x;											\n"
    "	float shininess = 64.0;															\n"

	"	vec3 worldVertexPosition = (model * vec4(fragPos, 1.0)).xyz;								\n"

	"	vec3 transformedNormal = normalize((normalMatrix * vec4(fragNormal, 1.0)).xyz);		\n"
	"	vec3 transformedTangent = normalize((normalMatrix * vec4(fragTangent, 1.0)).xyz);		\n"
	"	vec3 transformedBitTangent = normalize((normalMatrix * vec4(fragBitTangent, 1.0)).xyz);		\n"
	"	mat3 TBN = mat3x3(transformedTangent, transformedBitTangent, transformedNormal);\n"

	"	vec3 normalFromMap = (texture(normalTexture, fragUV).rgb * 2.0 - 1.0);  \n"
	"	vec3 normal = TBN * normalFromMap;\n"
	//"	vec3 normal = normalize((normalMatrix * vec4(fragNormal, 1.0)).xyz); 	\n"

   	"	vec3 eyeVector = normalize(viewPos - worldVertexPosition);					\n"
	"	vec3 normalizedLightDirection = normalize(lightPos - worldVertexPosition);						\n"

	//"	eyeVector = normalize(TBN * eyeVector);"
	//"	normalizedLightDirection = normalize(TBN * normalizedLightDirection);"

	"	float diffuseStrength = dot(normalizedLightDirection, normal);			\n"
	"	diffuseStrength = clamp(diffuseStrength, 0.0, 1.0);\n"

	"	vec3 halfVector = normalize(normalizedLightDirection + eyeVector);\n"
	"	float specularStrength = dot(halfVector, normal);\n"
	"	specularStrength = pow(specularStrength, shininess);\n"

	"	vec3 finalColor = (diffuseStrength + ambientStrength + specularStrength) * KdColor; \n"
	"	fragColor = vec4(diffuseStrength* KdColor, alpha); \n"
	//"	fragColor = texture(normalTexture, fragUV); \n"
    "}																						\n"
};



char *carVertexCarSourceTex4 =
{
    "#version 300 es						  							\n"
    "layout(location = 0) in vec3 aPos;								\n"
    "layout(location = 1) in vec3 aNormal;								\n"
    "layout(location = 2) in vec2 aTexcoord;              				\n"
    "layout(location = 3) in vec3 aTangent;								\n"
    "layout(location = 4) in vec3 aBitTangent;              				\n"
    "uniform mat4 mvp;											\n"
	"uniform mat4 model;												\n"
	"out vec3 FragPos;									\n"
	"out vec2 TexCoords;									\n"
	"out mat3 TBN;										\n"
	"out vec3 N;										\n"
    "void main()														\n"
    "{																	\n"
    "	 TexCoords = aTexcoord; 					\n"
    "	 FragPos = vec3(model * vec4(aPos, 1.0));					\n"
    "	 mat3 normalMatrix = transpose(inverse(mat3(model)));						\n"
    "	 vec3 T = normalize(normalMatrix * aTangent);					\n"
    "	 vec3 B = normalize(normalMatrix * aBitTangent);						\n"
    "	 N = normalize(normalMatrix * aNormal);						\n"
    "	 TBN = mat3 (T,B,N);								\n"
    "	 gl_Position = mvp * vec4(aPos, 1.0); 	\n"
    "}																	\n"
};


char *carFragmentCarSourceTex4 =
{
    "#version 300 es																		\n"
	"precision highp float; 																\n"
	
		
	"uniform vec3 KdColor;																	\n"
    "uniform vec3 lightPos; 																\n"
    "uniform vec3 viewPos;																	\n"
    "uniform vec3 lightStrength;																	\n"
    "uniform float alpha;																	\n"
    "uniform sampler2D normalTexture;															\n"
    "uniform mat4 normalMatrix;												\n"
	"in vec3 FragPos;									\n"
	"in	vec2 TexCoords;									\n"
	"in	mat3 TBN;										\n"
	"in vec3 N;										\n"
	"out vec4 fragColor;																	\n"
	"void main()																			\n"
    "{																						\n"
    "	float ambientStrength = lightStrength.x;											\n"
    "	float shininess = 64.0;															\n"


	"	vec3 normal = texture(normalTexture, TexCoords).rgb; \n"
	"	normal = normalize(normal * 2.0 - 1.0);						\n"
	"	normal = normalize(TBN * normal); 	\n"
	"	//normal = N;	\n"

   	
	"	vec3 lightDir = normalize(lightPos - FragPos);;						\n"

	"	float diffuseStrength = dot(lightDir, normal);			\n"
	"	diffuseStrength = clamp(diffuseStrength, 0.0, 1.0);\n"

	//"	vec3 finalColor = (diffuseStrength + ambientStrength + specularStrength) * KdColor; \n"
	"	fragColor = vec4(diffuseStrength* KdColor, alpha); \n"
	//"	fragColor = texture(normalTexture, TexCoords); \n"
    "}																						\n"
};


char *carVertexCarSourceTex7 =
{
    "#version 300 es						  							\n"
    "layout(location = 0) in  vec3 aPos;								\n"
    "layout(location = 1) in  vec3 aNormal;								\n"
    "layout(location = 2) in vec2 aTexcoord;              				\n"
    "layout(location = 3) in vec3 aTangent;								\n"
    "layout(location = 4) in vec3 aBitTangent;              				\n"
    "uniform mat4 mvp;											\n"
    "out vec3 fragNormal;												\n"
    "out vec3 fragPos;													\n"
    "out vec3 fragTangent;													\n"
    "out vec3 fragBitTangent;													\n"
    "smooth centroid out vec2 fragTexcoord;									\n"
    "void main()														\n"
    "{																	\n"
    "	 fragNormal = aNormal; 					\n"
    "	 fragTexcoord = aTexcoord;						\n"
    "	 fragPos = aPos;						\n"
    "	 fragTangent = aTangent;						\n"
    "	 fragBitTangent = aBitTangent;						\n"
    "	 gl_Position = mvp * vec4(aPos, 1.0); 	\n"
    "}																	\n"
};


char *carFragmentCarSourceTex7 =
{
    "#version 300 es																		\n"
	"precision highp float; 																\n"
	"uniform mat4 model;												\n"
	"uniform mat4 normalMatrix; 										\n"	
	"uniform vec3 KdColor;																	\n"
    "uniform vec3 lightPos; 																\n"
    "uniform vec3 viewPos;																	\n"
    "uniform vec3 lightStrength;																	\n"
    "uniform float alpha;																	\n"
    "uniform sampler2D picTexture;															\n"
	"uniform sampler2D normalTexture;															\n"
    "in vec3 fragNormal;																	\n"
	"in vec3 fragPos;																		\n"
	"in vec3 fragTangent;													\n"
	"in vec3 fragBitTangent;													\n"
	"smooth centroid in vec2 fragTexcoord;														\n"
	"out vec4 fragColor;																	\n"
	"void main()																			\n"
    "{																						\n"
    "	float ambientStrength = lightStrength.x;											\n"
    "	float shininess = 64.0;															\n"

	"	 vec3 T = normalize(mat3(normalMatrix) * fragTangent);					\n"
	"	 vec3 B = normalize(mat3(normalMatrix) * fragBitTangent);						\n"
	"	 vec3 N = normalize(mat3(normalMatrix) * fragNormal); 					\n"
	"	 mat3 TBN = mat3 (T,B,N);								\n"

	"	vec3 normal = texture(normalTexture, fragTexcoord).rgb; \n"
	"	normal = normalize(normal * 2.0 - 1.0); 					\n"
	"	vec3 transformedNormal = normalize(TBN * normal);	\n"
	"	transformedNormal = N; \n"

	"	vec3 worldVertexPosition = (model * vec4(fragPos, 1.0)).xyz;								\n"
   	"	vec3 eyeVector = normalize(viewPos - worldVertexPosition);					\n"
	"	vec3 normalizedLightDirection = normalize(lightPos - worldVertexPosition);						\n"

	"	float diffuseStrength = dot(normalizedLightDirection, transformedNormal);			\n"
	"	diffuseStrength = clamp(diffuseStrength, 0.0, 1.0);\n"

	"	vec3 halfVector = normalize(normalizedLightDirection + eyeVector);\n"
	"	float specularStrength = dot(halfVector, transformedNormal);\n"
	"	specularStrength = pow(specularStrength, shininess);\n"

	"	 vec3 texColor = texture(picTexture, fragTexcoord).rgb;							\n"

	"	vec3 finalColor = (diffuseStrength + ambientStrength + specularStrength) * KdColor; \n"
	"	fragColor = vec4(finalColor, alpha); \n"
    "}																						\n"
};


bool generateShaders(const std::vector<MATERIAL> &materialList,
                     std::vector<GLuint> &programIDList,
                     std::vector<GLuint> &vertexShaderIDList,
                     std::vector<GLuint> &fragmentShaderIDList)
{
    GLint Result = GL_FALSE;
    int InfoLogLength;
    unsigned int i; 
    char *VertexSourcePointer, *FragmentSourcePointer; 


    for (i = 0; i < materialList.size(); i++)
    {
    	//vertexShaderIDList[i] = glCreateShader(GL_VERTEX_SHADER);
		//fragmentShaderIDList[i] = glCreateShader(GL_FRAGMENT_SHADER);

#if 1
		/*$)A8y>]35D#2;M,2DVJKyPhR*4o5=dVH>P'9{7V1pJ9SC2;M,5D6%5c:MF,6NWEI+Fw*/
		if( strcmp(materialList[i].name, "BODY") == 0 ||
			strcmp(materialList[i].name, "GLASS") == 0 ||
			strcmp(materialList[i].name, "LINE") == 0)
		{
			/*$)A;7>374Id*/
			VertexSourcePointer = carVertexCarSourceADS;
			FragmentSourcePointer = carFragmentCarSourceReflect;
			printf("run reflect shader\n");
		}
		else if( strcmp(materialList[i].name, "WHEEL_HUB") == 0 ||
				 strcmp(materialList[i].name, "LINE") == 0 ||
				 strcmp(materialList[i].name, "WHEEL") == 0 ||
				 strcmp(materialList[i].name, "FENCE") == 0 ||
				 strcmp(materialList[i].name, "PLASTIC") == 0 ||
				 strcmp(materialList[i].name, "LICENSE_PLATE") == 0 ||
				 strcmp(materialList[i].name, "BLACK_PLASTIC") == 0 ||
				 strcmp(materialList[i].name, "LAMP_TAIL") == 0 ||
				 strcmp(materialList[i].name, "LAMP_HEAD_AND_TURN") == 0)
		{
			/*$)ALyM<*/
			VertexSourcePointer = carVertexCarSourceTex;
			FragmentSourcePointer = carFragmentCarSourceTex;
			printf("run texture shader\n");
		}
		else
#endif        
		{
			/*ADS$)A4r9b*/
			VertexSourcePointer = carVertexCarSourceADS;
			FragmentSourcePointer = carFragmentCarSourceADS;
		}
        
        programIDList[i] = esLoadProgram ( VertexSourcePointer, FragmentSourcePointer );
    }
    return GL_TRUE;
}

void init3DCar(void)
{
    FILE *fp;
    int i;
    int width, height;
    float car3dMaxX, car3dMaxY, car3dMaxZ;
	string resourceDirectory = CAR_PATH;
	string mtlFileName = "CAR.mtl";
	string objFileName = "CAR.obj";

    string mtlFilePath = resourceDirectory + mtlFileName;
    string objFilePath = resourceDirectory + objFileName;

    bool ret = loadMTL(mtlFilePath.c_str(), carMaterialList);


    // step2: set up the shaders
    carProgramIDList.resize(carMaterialList.size());
    carFragmentShaderIDList.resize(carMaterialList.size());    
    carVertexShaderIDList.resize(carMaterialList.size());
    carVertexPositionIDList.resize(carMaterialList.size());
	carTangentPositionIDList.resize(carMaterialList.size());
    carBitTangentPositionIDList.resize(carMaterialList.size());
    carTexturePositionIDList.resize(carMaterialList.size());
    carNormalPositionIDList.resize(carMaterialList.size());
    carMatrixModelIDList.resize(carMaterialList.size());
    carMatrixMVPIDList.resize(carMaterialList.size());
    carMatrixNormalIDList.resize(carMaterialList.size());
    carFragLightPosIDList.resize(carMaterialList.size());
    carFragViewPosIDList.resize(carMaterialList.size());
    carFragKdColorIDList.resize(carMaterialList.size());
    carFragAlphaIDList.resize(carMaterialList.size());
	carTextureIDList.resize(carMaterialList.size() + 8);
	carLocTextureIDList.resize(carMaterialList.size());	
	carLocCubeTextureIDList.resize(carMaterialList.size());
	carFragLightStrengthIDList.resize(carMaterialList.size());

    generateShaders(carMaterialList, carProgramIDList, carVertexShaderIDList,
                    carFragmentShaderIDList);

    // step3: parse the 3D obj file
    //ret = loadOBJ(objFilePath.c_str(), carMaterialList, carObjectList, carVertices,
                  //carNormals, carTextures, carMaterialIdx, &car3dMaxX, &car3dMaxY, &car3dMaxZ);

	ret = loadOBJ6(objFilePath.c_str(), carMaterialList, carObjectList, carVertices,
					carNormals, carTextures, carTangents, carBitTangents, carMaterialIdx, &car3dMaxX, &car3dMaxY, &car3dMaxZ);

    carScaleX = fabs(glVertices3DCar[0]) / car3dMaxX;
    carScaleZ = fabs(glVertices3DCar[2]) / car3dMaxZ;
    carScaleY = (carScaleX + carScaleZ) / 2;

    //printf("scale = %f %f %f\n\n\n", carScaleX, carScaleY, carScaleZ);


    printf("car vertex size: %d\n", carVertices.size());
    printf("car 3d MAX size:[x y z] = [%f %f %f]\n", car3dMaxX, car3dMaxY, car3dMaxZ);

    // upload the vertices into VBO
    //std::vector<GLuint> carVertexBuffer;
    //carVertexArrayIDs.resize(carVertices.size());
    //carVertexBufferIDs.resize(carVertices.size());
}


#if 0 //anchor second ok 03211630
float quadraticEquation(float a, float b, float c)
{
    float D;
    float z1, z2;

    D = b * b - 4 * a * c;

    if (D >= 0)
    {
        z1 = (-b + sqrt(D)) / (2 * a);
        z2 = (-b - sqrt(D)) / (2 * a);

        if (z1 >= 0)
            return z1;
        else
            return z2;
    }
    else
    {
        return -1;
    }
}

float quadraticEquation1(float a, float b, float c)
{
    float D;
    float z1, z2;

    D = b * b - 4 * a * c;

    if (D >= 0)
    {
        z1 = (-b + sqrt(D)) / (2 * a);
        z2 = (-b - sqrt(D)) / (2 * a);

        if (z1 >= 0)
            return z1;
        else
            return z2;
    }
    else
    {
        printf("error\n");
        return -1;
    }
}


#define VAL 0.001

float myCos(float radian)
{
    float res;

    res = cos(radian);
#if 1
    if(res >= 0 && res < 0.001)
    {
        res = 0.001;
    }
    else if(res < 0 && res > -0.001)
    {
        res = -0.001;
    }
#endif
    return res;
}

float mySin(float radian)
{
    float res;

    res = sin(radian);
#if 1
    if(res >= 0 && res < 0.001)
    {
        res = 0.001;
    }
    else if(res < 0 && res > -0.001)
    {
        res = -0.001;
    }
#endif
    return res;
}

float myTan(float radian)
{
    float res;

    res = tan(radian);
#if 1
    if(res >= 0 && res < 0.001)
    {
        res = 0.001;
    }
    else if(res < 0 && res > -0.001)
    {
        res = -0.001;
    }
#endif
    return res;
}

float myAtan(float radian)
{
    float res;

    res = atan(radian);
#if 1
    if(res >= 0 && res < 0.001)
    {
        res = 0.001;
    }
    else if(res < 0 && res > -0.001)
    {
        res = -0.001;
    }
#endif
    return res;
}

float myAsin(float radian)
{
    float res;

    res = asin(radian);
#if 1
    if(res > 0)
    {
        return res + 0.005;
    }
    else
    {
        return res - 0.005;
    }
#endif
    return res;
}




void init3DModel(void)
{
    float pxv, pyv, pzv;
    float pxw, pyw, pzw;
    CvPoint3D32f pt3d_w;
    CvPoint3D32f pt3d_v;
    float PIDIV2 = 1.57079632679489;
    //float PI     = 3.14159265358979;
    float inscribedR = sqrt(para_field.car_width * para_field.car_width + para_field.car_length * para_field.car_length) / 2;
    float R = 1600;
    float Rz0 = 1200;
    float Rz2 = R - Rz0;
    float cutR = 100;
    //float R = inscribedR * 4;
    //float Rz0 = inscribedR * 2;//sqrt(para_field.car_width * para_field.car_width + para_field.car_length * para_field.car_length);
    //float Rz2 = inscribedR * 2;

    float h, phi0, phi1;
    int p = 128;
    float fp = float(1) / p;
    float fp1 = float(1) / 16;
    float theta0, theta1, deltaTheta, theta;
    float sinPhi, cosPhi;
    float deltaR, deltaPhi;
    float i, j;
    float theta_end = 1.0;
    float radius_end = 1.0;
    float radius, tmpradius;
    float radius0, radius1;
    unsigned char direction_flag = 0;
    theta0 = myAtan(1.0 * para_field.car_width / para_field.car_length);
    float mosaicTheta = 35 * PI / 180;
    float tandivtheta;
    float a, b, c, z, x;
    float adjustCoeff;

    //sinPhi = (R - Rz0) / Rz2;
    //cosPhi = sqrtf(1 - sinPhi * sinPhi);
    //h = Rz2 - Rz2 * cosPhi;
    //printf("h1 ========== %f %f\n",h,Rz0);

    h = 2 * Rz2;// - sqrtf(Rz2 * Rz2 - (R - Rz0) * (R - Rz0));
    printf("h2 ========== %f %f\n", h, Rz0);

    deltaR = 1.0 * Rz0 / p;
    deltaPhi = PI / p;
    deltaTheta = 2 * theta0;


#if 1
    direction_flag = 0;

    for (i = 0; i < 1; i += fp)
    {
        phi0 = PIDIV2 - PI * i;
        radius0 = Rz0 + Rz2 * myCos(phi0);
        phi1 = PIDIV2 - PI * (i + fp);
        radius1 = Rz0 + Rz2 * myCos(phi1);
        switch(direction_flag)
        {
        case 0:
            //printf("0 radius = %f\n", radius);
            for (j = 0; j <= 1; j += fp)
            {
                tandivtheta = myTan(mosaicTheta);
                a = 1 + tandivtheta * tandivtheta;
                b = para_field.car_width + para_field.car_length * tandivtheta;
                c = para_field.car_length * para_field.car_length / 4 +
                    para_field.car_width * para_field.car_width / 4 - radius0 * radius0;

                z = quadraticEquation(a, b, c);
                x = z * tandivtheta;

                deltaTheta = 2 * myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                theta = PIDIV2 - deltaTheta / 2 + j * deltaTheta;

                pxv = radius0 * mySin(theta);
                pzv = radius0 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi0);

                //printf("%f %f %f\n",pxv,pyv,pzv);


                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = para_field.chessboard_width_corners * para_field.square_size - (pxv - para_field.car_length / 2); // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);
                //printf("000 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);

                if(pzv < (-para_field.car_width / 2))
                {
                    adjustCoeff = 0.0;
                }
                else if(pzv > (para_field.car_width / 2))
                {
                    adjustCoeff = 1.0;
                }
                else
                {
                    adjustCoeff = 1.0 * (para_field.car_width / 2 + pzv) / (para_field.car_width);
                }

                objPoints.glObjPoints_F.push_back(pt3d_w);
                vertexCoords.glVertex_F.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_F.push_back(adjustCoeff);



                tandivtheta = myTan(mosaicTheta);
                a = 1 + tandivtheta * tandivtheta;
                b = para_field.car_width + para_field.car_length * tandivtheta;
                c = para_field.car_length * para_field.car_length / 4 +
                    para_field.car_width * para_field.car_width / 4 - radius1 * radius1;

                z = quadraticEquation(a, b, c);
                x = z * tandivtheta;

                deltaTheta = 2 * myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                theta = PIDIV2 - deltaTheta / 2 + j * deltaTheta;

                pxv = radius1 * mySin(theta);
                pzv = radius1 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi1);

                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = para_field.chessboard_width_corners * para_field.square_size - (pxv - para_field.car_length / 2); // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if(pzv < (-para_field.car_width / 2))
                {
                    adjustCoeff = 0.0;
                }
                else if(pzv > (para_field.car_width / 2))
                {
                    adjustCoeff = 1.0;
                }
                else
                {
                    adjustCoeff = 1.0 * (para_field.car_width / 2 + pzv) / (para_field.car_width);
                }

                //printf("111 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_F.push_back(pt3d_w);
                vertexCoords.glVertex_F.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_F.push_back(adjustCoeff);
            }
            objPoints.glObjPoints_F.pop_back();
            vertexCoords.glVertex_F.pop_back();
            lumiaAdjust.glLumiaAdjust_F.pop_back();
            break;
        case 1:
            //printf("1 radius = %f\n", radius);
            for( j = 1; j >= 0; j -= fp )
            {
                tandivtheta = myTan(mosaicTheta);
                a = 1 + tandivtheta * tandivtheta;
                b = para_field.car_width + para_field.car_length * tandivtheta;
                c = para_field.car_length * para_field.car_length / 4 +
                    para_field.car_width * para_field.car_width / 4 - radius0 * radius0;

                z = quadraticEquation(a, b, c);
                x = z * tandivtheta;

                deltaTheta = 2 * myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                theta = PIDIV2 - deltaTheta / 2 + j * deltaTheta;

                pxv = radius0 * mySin(theta);
                pzv = radius0 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi0);


                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = para_field.chessboard_width_corners * para_field.square_size - (pxv - para_field.car_length / 2); // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if(pzv < (-para_field.car_width / 2))
                {
                    adjustCoeff = 0.0;
                }
                else if(pzv > (para_field.car_width / 2))
                {
                    adjustCoeff = 1.0;
                }
                else
                {
                    adjustCoeff = 1.0 * (para_field.car_width / 2 + pzv) / (para_field.car_width);
                }

                //printf("333 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_F.push_back(pt3d_w);
                vertexCoords.glVertex_F.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_F.push_back(adjustCoeff);



                tandivtheta = myTan(mosaicTheta);
                a = 1 + tandivtheta * tandivtheta;
                b = para_field.car_width + para_field.car_length * tandivtheta;
                c = para_field.car_length * para_field.car_length / 4 +
                    para_field.car_width * para_field.car_width / 4 - radius1 * radius1;

                z = quadraticEquation(a, b, c);
                x = z * tandivtheta;

                deltaTheta = 2 * myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                theta = PIDIV2 - deltaTheta / 2 + j * deltaTheta;


                pxv = radius1 * mySin(theta);
                pzv = radius1 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi1);


                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = para_field.chessboard_width_corners * para_field.square_size - (pxv - para_field.car_length / 2); // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if(pzv < (-para_field.car_width / 2))
                {
                    adjustCoeff = 0.0;
                }
                else if(pzv > (para_field.car_width / 2))
                {
                    adjustCoeff = 1.0;
                }
                else
                {
                    adjustCoeff = 1.0 * (para_field.car_width / 2 + pzv) / (para_field.car_width);
                }

                //printf("444 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_F.push_back(pt3d_w);
                vertexCoords.glVertex_F.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_F.push_back(adjustCoeff);
            }
            objPoints.glObjPoints_F.pop_back();
            vertexCoords.glVertex_F.pop_back();
            lumiaAdjust.glLumiaAdjust_F.pop_back();
            break;
        }
        // change direction!
        if(j - fp == 1 )
        {
            direction_flag = 1;
            //printf("direction_flag = %d\n", direction_flag);
        }
        else
        {
            direction_flag = 0;
            //printf("direction_flag = %d\n", direction_flag);
        }
    }
#endif

#if 1
    radius = Rz0 + deltaR;

    for (i = 0; i < 1; i += fp)
    {
        radius -= deltaR;
        switch(direction_flag)
        {
        case 0:
            //printf("0 radius = %f\n", radius);
            for (j = 0; j <= 1; j += fp)
            {
                if((radius - inscribedR > 0) && (radius - inscribedR < deltaR))
                {
                    deltaTheta = 2 * theta0;
                    theta = j * deltaTheta + PIDIV2 - theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if(radius >= inscribedR)
                    {
                        tandivtheta = myTan(mosaicTheta);
                        a = 1 + tandivtheta * tandivtheta;
                        b = para_field.car_width + para_field.car_length * tandivtheta;
                        c = para_field.car_length * para_field.car_length / 4 +
                            para_field.car_width * para_field.car_width / 4 - radius * radius;

                        z = quadraticEquation(a, b, c);
                        x = z * tandivtheta;

                        deltaTheta = 2 * myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                        theta = PIDIV2 - deltaTheta / 2 + j * deltaTheta;
                    }
                    else
                    {
                        deltaTheta = 2 * theta0;
                        theta = j * deltaTheta + PIDIV2 - theta0;
                    }


                    pxv = radius * mySin(theta);
                    pzv = radius * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = para_field.chessboard_width_corners * para_field.square_size - (pxv - para_field.car_length / 2); // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);
                //printf("000 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);

                if(pzv < (-para_field.car_width / 2))
                {
                    adjustCoeff = 0.0;
                }
                else if(pzv > (para_field.car_width / 2))
                {
                    adjustCoeff = 1.0;
                }
                else
                {
                    adjustCoeff = 1.0 * (para_field.car_width / 2 + pzv) / (para_field.car_width);
                }

                objPoints.glObjPoints_F.push_back(pt3d_w);
                vertexCoords.glVertex_F.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_F.push_back(adjustCoeff);

                if((radius - deltaR - inscribedR > 0) && (radius - deltaR - inscribedR < deltaR))
                {
                    deltaTheta = 2 * theta0;
                    theta = j * deltaTheta + PIDIV2 - theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if((radius - deltaR) >= inscribedR)
                    {
                        tandivtheta = myTan(mosaicTheta);
                        a = 1 + tandivtheta * tandivtheta;
                        b = para_field.car_width + para_field.car_length * tandivtheta;
                        c = para_field.car_length * para_field.car_length / 4 +
                            para_field.car_width * para_field.car_width / 4 - (radius - deltaR) * (radius - deltaR);

                        z = quadraticEquation(a, b, c);
                        x = z * tandivtheta;

                        deltaTheta = 2 * myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                        theta = PIDIV2 - deltaTheta / 2 + j * deltaTheta;
                    }
                    else
                    {
                        deltaTheta = 2 * theta0;
                        theta = j * deltaTheta + PIDIV2 - theta0;
                    }



                    pxv = (radius - deltaR) * mySin(theta);
                    pzv = (radius - deltaR) * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = para_field.chessboard_width_corners * para_field.square_size - (pxv - para_field.car_length / 2); // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if(pzv < (-para_field.car_width / 2))
                {
                    adjustCoeff = 0.0;
                }
                else if(pzv > (para_field.car_width / 2))
                {
                    adjustCoeff = 1.0;
                }
                else
                {
                    adjustCoeff = 1.0 * (para_field.car_width / 2 + pzv) / (para_field.car_width);
                }

                //printf("111 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_F.push_back(pt3d_w);
                vertexCoords.glVertex_F.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_F.push_back(adjustCoeff);
            }
            objPoints.glObjPoints_F.pop_back();
            vertexCoords.glVertex_F.pop_back();
            lumiaAdjust.glLumiaAdjust_F.pop_back();
            break;
        case 1:
            //printf("1 radius = %f\n", radius);
            for( j = 1; j >= 0; j -= fp )
            {
                if((radius - inscribedR > 0) && (radius - inscribedR < deltaR))
                {
                    deltaTheta = 2 * theta0;
                    theta = j * deltaTheta + PIDIV2 - theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if(radius >= inscribedR)
                    {
                        tandivtheta = myTan(mosaicTheta);
                        a = 1 + tandivtheta * tandivtheta;
                        b = para_field.car_width + para_field.car_length * tandivtheta;
                        c = para_field.car_length * para_field.car_length / 4 +
                            para_field.car_width * para_field.car_width / 4 - radius * radius;

                        z = quadraticEquation(a, b, c);
                        x = z * tandivtheta;

                        deltaTheta = 2 * myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                        theta = PIDIV2 - deltaTheta / 2 + j * deltaTheta;
                    }
                    else
                    {
                        deltaTheta = 2 * theta0;
                        theta = j * deltaTheta + PIDIV2 - theta0;
                    }


                    pxv = radius * mySin(theta);
                    pzv = radius * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = para_field.chessboard_width_corners * para_field.square_size - (pxv - para_field.car_length / 2); // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if(pzv < (-para_field.car_width / 2))
                {
                    adjustCoeff = 0.0;
                }
                else if(pzv > (para_field.car_width / 2))
                {
                    adjustCoeff = 1.0;
                }
                else
                {
                    adjustCoeff = 1.0 * (para_field.car_width / 2 + pzv) / (para_field.car_width);
                }

                //printf("333 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_F.push_back(pt3d_w);
                vertexCoords.glVertex_F.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_F.push_back(adjustCoeff);

                if((radius - deltaR - inscribedR > 0) && (radius - deltaR - inscribedR < deltaR))
                {
                    deltaTheta = 2 * theta0;
                    theta = j * deltaTheta + PIDIV2 - theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if((radius - deltaR) >= inscribedR)
                    {
                        tandivtheta = myTan(mosaicTheta);
                        a = 1 + tandivtheta * tandivtheta;
                        b = para_field.car_width + para_field.car_length * tandivtheta;
                        c = para_field.car_length * para_field.car_length / 4 +
                            para_field.car_width * para_field.car_width / 4 - (radius - deltaR) * (radius - deltaR);

                        z = quadraticEquation(a, b, c);
                        x = z * tandivtheta;

                        deltaTheta = 2 * myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                        theta = PIDIV2 - deltaTheta / 2 + j * deltaTheta;
                    }
                    else
                    {
                        deltaTheta = 2 * theta0;
                        theta = j * deltaTheta + PIDIV2 - theta0;
                    }


                    pxv = (radius - deltaR) * mySin(theta);
                    pzv = (radius - deltaR) * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = para_field.chessboard_width_corners * para_field.square_size - (pxv - para_field.car_length / 2); // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if(pzv < (-para_field.car_width / 2))
                {
                    adjustCoeff = 0.0;
                }
                else if(pzv > (para_field.car_width / 2))
                {
                    adjustCoeff = 1.0;
                }
                else
                {
                    adjustCoeff = 1.0 * (para_field.car_width / 2 + pzv) / (para_field.car_width);
                }

                //printf("444 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_F.push_back(pt3d_w);
                vertexCoords.glVertex_F.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_F.push_back(adjustCoeff);
            }
            objPoints.glObjPoints_F.pop_back();
            vertexCoords.glVertex_F.pop_back();
            lumiaAdjust.glLumiaAdjust_F.pop_back();
            break;
        }
        // change direction!
        if(j - fp == 1 )
        {
            direction_flag = 1;
            //printf("direction_flag = %d\n", direction_flag);
        }
        else
        {
            direction_flag = 0;
            //printf("direction_flag = %d\n", direction_flag);
        }
    }
    LOGE("VTX_NUM_F: %d\n", objPoints.glObjPoints_F.size());
    texCoords.glTexCoord_F.resize(objPoints.glObjPoints_F.size());
#endif

#if 1
    direction_flag = 0;

    for (i = 0; i < 1; i += fp)
    {
        phi0 = PIDIV2 - PI * i;
        radius0 = Rz0 + Rz2 * myCos(phi0);
        phi1 = PIDIV2 - PI * (i + fp);
        radius1 = Rz0 + Rz2 * myCos(phi1);

        switch(direction_flag)
        {
        case 0:
            //printf("radius = %f\n", radius);
            for (j = 0; j <= 1; j += fp)
            {
                tandivtheta = myTan(mosaicTheta);
                a = 1 + tandivtheta * tandivtheta;
                b = para_field.car_width + para_field.car_length * tandivtheta;
                c = para_field.car_length * para_field.car_length / 4 +
                    para_field.car_width * para_field.car_width / 4 - radius0 * radius0;

                z = quadraticEquation(a, b, c);
                x = z * tandivtheta;

                deltaTheta = 2 * myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                theta = PI + PIDIV2 - deltaTheta / 2 + j * deltaTheta;

                pxv = radius0 * mySin(theta);
                pzv = radius0 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi0);


                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = -pxv - para_field.car_length / 2; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if(pzv < (-para_field.car_width / 2))
                {
                    adjustCoeff = 0.0;
                }
                else if(pzv > (para_field.car_width / 2))
                {
                    adjustCoeff = 1.0;
                }
                else
                {
                    adjustCoeff = 1.0 * (para_field.car_width / 2 + pzv) / (para_field.car_width);
                }

                //printf("000 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_B.push_back(pt3d_w);
                vertexCoords.glVertex_B.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_B.push_back(adjustCoeff);


                tandivtheta = myTan(mosaicTheta);
                a = 1 + tandivtheta * tandivtheta;
                b = para_field.car_width + para_field.car_length * tandivtheta;
                c = para_field.car_length * para_field.car_length / 4 +
                    para_field.car_width * para_field.car_width / 4 - radius1 * radius1;

                z = quadraticEquation(a, b, c);
                x = z * tandivtheta;

                deltaTheta = 2 * myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                theta = PI + PIDIV2 - deltaTheta / 2 + j * deltaTheta;

                pxv = radius1 * mySin(theta);
                pzv = radius1 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi1);

                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = -pxv - para_field.car_length / 2; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if(pzv < (-para_field.car_width / 2))
                {
                    adjustCoeff = 0.0;
                }
                else if(pzv > (para_field.car_width / 2))
                {
                    adjustCoeff = 1.0;
                }
                else
                {
                    adjustCoeff = 1.0 * (para_field.car_width / 2 + pzv) / (para_field.car_width);
                }

                //printf("111 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_B.push_back(pt3d_w);
                vertexCoords.glVertex_B.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_B.push_back(adjustCoeff);
            }
            objPoints.glObjPoints_B.pop_back();
            vertexCoords.glVertex_B.pop_back();
            lumiaAdjust.glLumiaAdjust_B.pop_back();
            break;
        case 1:
            //printf("radius = %f\n", radius);
            for( j = 1; j >= 0; j -= fp )
            {
                tandivtheta = myTan(mosaicTheta);
                a = 1 + tandivtheta * tandivtheta;
                b = para_field.car_width + para_field.car_length * tandivtheta;
                c = para_field.car_length * para_field.car_length / 4 +
                    para_field.car_width * para_field.car_width / 4 - radius0 * radius0;

                z = quadraticEquation(a, b, c);
                x = z * tandivtheta;

                deltaTheta = 2 * myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                theta = PI + PIDIV2 - deltaTheta / 2 + j * deltaTheta;


                pxv = radius0 * mySin(theta);
                pzv = radius0 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi0);


                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = -pxv - para_field.car_length / 2; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if(pzv < (-para_field.car_width / 2))
                {
                    adjustCoeff = 0;
                }
                else if(pzv > (para_field.car_width / 2))
                {
                    adjustCoeff = 1.0;
                }
                else
                {
                    adjustCoeff = 1.0 * (para_field.car_width / 2 + pzv) / (para_field.car_width);
                }

                //printf("333 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_B.push_back(pt3d_w);
                vertexCoords.glVertex_B.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_B.push_back(adjustCoeff);



                tandivtheta = myTan(mosaicTheta);
                a = 1 + tandivtheta * tandivtheta;
                b = para_field.car_width + para_field.car_length * tandivtheta;
                c = para_field.car_length * para_field.car_length / 4 +
                    para_field.car_width * para_field.car_width / 4 - radius1 * radius1;

                z = quadraticEquation(a, b, c);
                x = z * tandivtheta;

                deltaTheta = 2 * myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                theta = PI + PIDIV2 - deltaTheta / 2 + j * deltaTheta;


                pxv = radius1 * mySin(theta);
                pzv = radius1 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi1);

                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = -pxv - para_field.car_length / 2; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if(pzv < (-para_field.car_width / 2))
                {
                    adjustCoeff = 0;
                }
                else if(pzv > (para_field.car_width / 2))
                {
                    adjustCoeff = 1.0;
                }
                else
                {
                    adjustCoeff = 1.0 * (para_field.car_width / 2 + pzv) / (para_field.car_width);
                }

                //printf("444 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_B.push_back(pt3d_w);
                vertexCoords.glVertex_B.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_B.push_back(adjustCoeff);
            }
            objPoints.glObjPoints_B.pop_back();
            vertexCoords.glVertex_B.pop_back();
            lumiaAdjust.glLumiaAdjust_B.pop_back();
            break;
        }
        // change direction!
        if(j - fp == 1 )
        {
            direction_flag = 1;
            //printf("direction_flag = %d\n", direction_flag);
        }
        else
        {
            direction_flag = 0;
            //printf("direction_flag = %d\n", direction_flag);
        }
    }

    //LOGE("VTX_NUM_B: %d\n", objPoints.glObjPoints_B.size());
    //texCoords.glTexCoord_B.resize(objPoints.glObjPoints_B.size());
#endif

#if 1
    radius = Rz0 + deltaR;

    for (i = 0; i < 1; i += fp)
    {
        radius -= deltaR;
        switch(direction_flag)
        {
        case 0:
            //printf("radius = %f\n", radius);
            for (j = 0; j <= 1; j += fp)
            {
                if((radius - inscribedR > 0) && (radius - inscribedR < deltaR))
                {
                    deltaTheta = 2 * theta0;
                    theta = j * deltaTheta + PI + PIDIV2 - theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if(radius > inscribedR)
                    {
                        tandivtheta = myTan(mosaicTheta);
                        a = 1 + tandivtheta * tandivtheta;
                        b = para_field.car_width + para_field.car_length * tandivtheta;
                        c = para_field.car_length * para_field.car_length / 4 +
                            para_field.car_width * para_field.car_width / 4 - radius * radius;

                        z = quadraticEquation(a, b, c);
                        x = z * tandivtheta;

                        deltaTheta = 2 * myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                        theta = PI + PIDIV2 - deltaTheta / 2 + j * deltaTheta;
                    }
                    else
                    {
                        deltaTheta = 2 * theta0;
                        theta = j * deltaTheta + PI + PIDIV2 - theta0;
                    }



                    pxv = radius * mySin(theta);
                    pzv = radius * myCos(theta);
                    pyv = 0.0;

                }

                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = -pxv - para_field.car_length / 2; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if(pzv < (-para_field.car_width / 2))
                {
                    adjustCoeff = 0.0;
                }
                else if(pzv > (para_field.car_width / 2))
                {
                    adjustCoeff = 1.0;
                }
                else
                {
                    adjustCoeff = 1.0 * (para_field.car_width / 2 + pzv) / (para_field.car_width);
                }

                //printf("000 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_B.push_back(pt3d_w);
                vertexCoords.glVertex_B.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_B.push_back(adjustCoeff);

                if((radius - deltaR - inscribedR > 0) && (radius - deltaR - inscribedR < deltaR))
                {
                    deltaTheta = 2 * theta0;
                    theta = j * deltaTheta + PI + PIDIV2 - theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if((radius - deltaR) > inscribedR)
                    {
                        tandivtheta = myTan(mosaicTheta);
                        a = 1 + tandivtheta * tandivtheta;
                        b = para_field.car_width + para_field.car_length * tandivtheta;
                        c = para_field.car_length * para_field.car_length / 4 +
                            para_field.car_width * para_field.car_width / 4 - (radius - deltaR) * (radius - deltaR);

                        z = quadraticEquation(a, b, c);
                        x = z * tandivtheta;

                        deltaTheta = 2 * myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                        theta = PI + PIDIV2 - deltaTheta / 2 + j * deltaTheta;
                    }
                    else
                    {
                        deltaTheta = 2 * theta0;
                        theta = j * deltaTheta + PI + PIDIV2 - theta0;
                    }


                    pxv = (radius - deltaR) * mySin(theta);
                    pzv = (radius - deltaR) * myCos(theta);
                    pyv = 0.0;

                }

                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = -pxv - para_field.car_length / 2; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if(pzv < (-para_field.car_width / 2))
                {
                    adjustCoeff = 0.0;
                }
                else if(pzv > (para_field.car_width / 2))
                {
                    adjustCoeff = 1.0;
                }
                else
                {
                    adjustCoeff = 1.0 * (para_field.car_width / 2 + pzv) / (para_field.car_width);
                }

                //printf("111 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_B.push_back(pt3d_w);
                vertexCoords.glVertex_B.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_B.push_back(adjustCoeff);
            }
            objPoints.glObjPoints_B.pop_back();
            vertexCoords.glVertex_B.pop_back();
            lumiaAdjust.glLumiaAdjust_B.pop_back();
            break;
        case 1:
            //printf("radius = %f\n", radius);
            for( j = 1; j >= 0; j -= fp )
            {
                if((radius - inscribedR > 0) && (radius - inscribedR < deltaR))
                {
                    deltaTheta = 2 * theta0;
                    theta = j * deltaTheta + PI + PIDIV2 - theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if(radius > inscribedR)
                    {
                        tandivtheta = myTan(mosaicTheta);
                        a = 1 + tandivtheta * tandivtheta;
                        b = para_field.car_width + para_field.car_length * tandivtheta;
                        c = para_field.car_length * para_field.car_length / 4 +
                            para_field.car_width * para_field.car_width / 4 - radius * radius;

                        z = quadraticEquation(a, b, c);
                        x = z * tandivtheta;

                        deltaTheta = 2 * myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                        theta = PI + PIDIV2 - deltaTheta / 2 + j * deltaTheta;
                    }
                    else
                    {
                        deltaTheta = 2 * theta0;
                        theta = j * deltaTheta + PI + PIDIV2 - theta0;
                    }


                    pxv = radius * mySin(theta);
                    pzv = radius * myCos(theta);
                    pyv = 0.0;

                }

                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = -pxv - para_field.car_length / 2; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if(pzv < (-para_field.car_width / 2))
                {
                    adjustCoeff = 0;
                }
                else if(pzv > (para_field.car_width / 2))
                {
                    adjustCoeff = 1.0;
                }
                else
                {
                    adjustCoeff = 1.0 * (para_field.car_width / 2 + pzv) / (para_field.car_width);
                }

                //printf("333 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_B.push_back(pt3d_w);
                vertexCoords.glVertex_B.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_B.push_back(adjustCoeff);

                if((radius - deltaR - inscribedR > 0) && (radius - deltaR - inscribedR < deltaR))
                {
                    deltaTheta = 2 * theta0;
                    theta = j * deltaTheta + PI + PIDIV2 - theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if((radius - deltaR) > inscribedR)
                    {
                        tandivtheta = myTan(mosaicTheta);
                        a = 1 + tandivtheta * tandivtheta;
                        b = para_field.car_width + para_field.car_length * tandivtheta;
                        c = para_field.car_length * para_field.car_length / 4 +
                            para_field.car_width * para_field.car_width / 4 - (radius - deltaR) * (radius - deltaR);

                        z = quadraticEquation(a, b, c);
                        x = z * tandivtheta;

                        deltaTheta = 2 * myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                        theta = PI + PIDIV2 - deltaTheta / 2 + j * deltaTheta;
                    }
                    else
                    {
                        deltaTheta = 2 * theta0;
                        theta = j * deltaTheta + PI + PIDIV2 - theta0;
                    }


                    pxv = (radius - deltaR) * mySin(theta);
                    pzv = (radius - deltaR) * myCos(theta);
                    pyv = 0.0;

                }

                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = -pxv - para_field.car_length / 2; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if(pzv < (-para_field.car_width / 2))
                {
                    adjustCoeff = 0;
                }
                else if(pzv > (para_field.car_width / 2))
                {
                    adjustCoeff = 1.0;
                }
                else
                {
                    adjustCoeff = 1.0 * (para_field.car_width / 2 + pzv) / (para_field.car_width);
                }

                //printf("444 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_B.push_back(pt3d_w);
                vertexCoords.glVertex_B.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_B.push_back(adjustCoeff);
            }
            objPoints.glObjPoints_B.pop_back();
            vertexCoords.glVertex_B.pop_back();
            lumiaAdjust.glLumiaAdjust_B.pop_back();
            break;
        }
        // change direction!
        if(j - fp == 1 )
        {
            direction_flag = 1;
            //printf("direction_flag = %d\n", direction_flag);
        }
        else
        {
            direction_flag = 0;
            //printf("direction_flag = %d\n", direction_flag);
        }
    }

    LOGE("VTX_NUM_B: %d\n", objPoints.glObjPoints_B.size());
    texCoords.glTexCoord_B.resize(objPoints.glObjPoints_B.size());
#endif


#if 1
    theta1 = (PIDIV2 - theta0);
    deltaTheta = 2 * theta1;

    direction_flag = 0;

    for (i = 0; i < 1; i += fp)
    {
        phi0 = PIDIV2 - PI * i;
        radius0 = Rz0 + Rz2 * myCos(phi0);
        phi1 = PIDIV2 - PI * (i + fp);
        radius1 = Rz0 + Rz2 * myCos(phi1);

        switch(direction_flag)
        {
        case 0:
            //printf("radius = %f\n", radius);
            for (j = 0; j <= 1; j += fp)
            {


                deltaTheta = 2 * myAsin(para_field.car_length / 2 / radius0);
                theta = PI - deltaTheta / 2 + j * deltaTheta;


                pxv = radius0 * mySin(theta);
                pzv = radius0 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi0);

                pxw = pzv + para_field.chessboard_width_corners * para_field.square_size + para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                adjustCoeff = 1.0 - 1.0 * (para_field.car_length / 2 + pxv) / para_field.car_length;

                //printf("000 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_L.push_back(pt3d_w);
                vertexCoords.glVertex_L.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_L.push_back(adjustCoeff);


                deltaTheta = 2 * myAsin(para_field.car_length / 2 / radius1);
                theta = PI - deltaTheta / 2 + j * deltaTheta;



                pxv = radius1 * mySin(theta);
                pzv = radius1 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi1);

                pxw = pzv + para_field.chessboard_width_corners * para_field.square_size + para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                adjustCoeff = 1.0 - 1.0 * (para_field.car_length / 2 + pxv) / para_field.car_length;

                //printf("111 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_L.push_back(pt3d_w);
                vertexCoords.glVertex_L.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_L.push_back(adjustCoeff);
            }
            objPoints.glObjPoints_L.pop_back();
            vertexCoords.glVertex_L.pop_back();
            lumiaAdjust.glLumiaAdjust_L.pop_back();
            break;
        case 1:
            //printf("radius = %f\n", radius);
            for( j = 1; j >= 0; j -= fp )
            {
                deltaTheta = 2 * myAsin(para_field.car_length / 2 / radius0);
                theta = PI - deltaTheta / 2 + j * deltaTheta;



                pxv = radius0 * mySin(theta);
                pzv = radius0 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi0);

                pxw = pzv + para_field.chessboard_width_corners * para_field.square_size + para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                adjustCoeff = 1.0 - 1.0 * (para_field.car_length / 2 + pxv) / para_field.car_length;

                //printf("333 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_L.push_back(pt3d_w);
                vertexCoords.glVertex_L.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_L.push_back(adjustCoeff);



                deltaTheta = 2 * myAsin(para_field.car_length / 2 / radius1);
                theta = PI - deltaTheta / 2 + j * deltaTheta;



                pxv = radius1 * mySin(theta);
                pzv = radius1 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi1);

                pxw = pzv + para_field.chessboard_width_corners * para_field.square_size + para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                adjustCoeff = 1.0 - 1.0 * (para_field.car_length / 2 + pxv) / para_field.car_length;

                //printf("444 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_L.push_back(pt3d_w);
                vertexCoords.glVertex_L.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_L.push_back(adjustCoeff);
            }
            objPoints.glObjPoints_L.pop_back();
            vertexCoords.glVertex_L.pop_back();
            lumiaAdjust.glLumiaAdjust_L.pop_back();
            break;
        }
        // change direction!
        if(j - fp == 1 )
        {
            direction_flag = 1;
            //printf("direction_flag = %d\n", direction_flag);
        }
        else
        {
            direction_flag = 0;
            //printf("direction_flag = %d\n", direction_flag);
        }
    }

    //LOGE("VTX_NUM_L: %d\n", objPoints.glObjPoints_L.size());
    //texCoords.glTexCoord_L.resize(objPoints.glObjPoints_L.size());
#endif

#if 1
    theta1 = (PIDIV2 - theta0);
    deltaTheta = 2 * theta1;

    radius = Rz0 + deltaR;

    for (i = 0; i < 1; i += fp)
    {
        radius -= deltaR;
        switch(direction_flag)
        {
        case 0:
            //printf("radius = %f\n", radius);
            for (j = 0; j <= 1; j += fp)
            {
                if((radius - inscribedR > 0) && (radius - inscribedR < deltaR))
                {
                    deltaTheta = 2 * theta1;
                    theta = j * deltaTheta + theta0 + PIDIV2;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;
                }
                else
                {
                    if(radius >= inscribedR)
                    {
                        deltaTheta = 2 * myAsin(para_field.car_length / 2 / radius);
                        theta = PI - deltaTheta / 2 + j * deltaTheta;
                    }
                    else
                    {
                        deltaTheta = 2 * theta1;
                        theta = j * deltaTheta + theta0 + PIDIV2;
                    }


                    pxv = radius * mySin(theta);
                    pzv = radius * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv + para_field.chessboard_width_corners * para_field.square_size + para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                adjustCoeff = 1.0 - 1.0 * (para_field.car_length / 2 + pxv) / para_field.car_length;

                //printf("000 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_L.push_back(pt3d_w);
                vertexCoords.glVertex_L.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_L.push_back(adjustCoeff);

                if((radius - deltaR - inscribedR > 0) && (radius - deltaR - inscribedR < deltaR))
                {
                    deltaTheta = 2 * theta1;
                    theta = j * deltaTheta + theta0 + PIDIV2;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;
                }
                else
                {
                    if(radius - deltaR >= inscribedR)
                    {
                        deltaTheta = 2 * myAsin(para_field.car_length / 2 / (radius - deltaR));
                        theta = PI - deltaTheta / 2 + j * deltaTheta;
                    }
                    else
                    {
                        deltaTheta = 2 * theta1;
                        theta = j * deltaTheta + theta0 + PIDIV2;
                    }



                    pxv = (radius - deltaR) * mySin(theta);
                    pzv = (radius - deltaR) * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv + para_field.chessboard_width_corners * para_field.square_size + para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                adjustCoeff = 1.0 - 1.0 * (para_field.car_length / 2 + pxv) / para_field.car_length;

                //printf("111 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_L.push_back(pt3d_w);
                vertexCoords.glVertex_L.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_L.push_back(adjustCoeff);
            }
            objPoints.glObjPoints_L.pop_back();
            vertexCoords.glVertex_L.pop_back();
            lumiaAdjust.glLumiaAdjust_L.pop_back();
            break;
        case 1:
            //printf("radius = %f\n", radius);
            for( j = 1; j >= 0; j -= fp )
            {
                if((radius - inscribedR > 0) && (radius - inscribedR < deltaR))
                {
                    deltaTheta = 2 * theta1;
                    theta = j * deltaTheta + theta0 + PIDIV2;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;
                }
                else
                {
                    if(radius >= inscribedR)
                    {
                        deltaTheta = 2 * myAsin(para_field.car_length / 2 / radius);
                        theta = PI - deltaTheta / 2 + j * deltaTheta;
                    }
                    else
                    {
                        deltaTheta = 2 * theta1;
                        theta = j * deltaTheta + theta0 + PIDIV2;
                    }



                    pxv = radius * mySin(theta);
                    pzv = radius * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv + para_field.chessboard_width_corners * para_field.square_size + para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                adjustCoeff = 1.0 - 1.0 * (para_field.car_length / 2 + pxv) / para_field.car_length;

                //printf("333 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_L.push_back(pt3d_w);
                vertexCoords.glVertex_L.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_L.push_back(adjustCoeff);

                if((radius - deltaR - inscribedR > 0) && (radius - deltaR - inscribedR < deltaR))
                {
                    deltaTheta = 2 * theta1;
                    theta = j * deltaTheta + theta0 + PIDIV2;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;
                }
                else
                {
                    if(radius - deltaR >= inscribedR)
                    {
                        deltaTheta = 2 * myAsin(para_field.car_length / 2 / (radius - deltaR));
                        theta = PI - deltaTheta / 2 + j * deltaTheta;
                    }
                    else
                    {
                        deltaTheta = 2 * theta1;
                        theta = j * deltaTheta + theta0 + PIDIV2;
                    }



                    pxv = (radius - deltaR) * mySin(theta);
                    pzv = (radius - deltaR) * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv + para_field.chessboard_width_corners * para_field.square_size + para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                adjustCoeff = 1.0 - 1.0 * (para_field.car_length / 2 + pxv) / para_field.car_length;

                //printf("444 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_L.push_back(pt3d_w);
                vertexCoords.glVertex_L.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_L.push_back(adjustCoeff);
            }
            objPoints.glObjPoints_L.pop_back();
            vertexCoords.glVertex_L.pop_back();
            lumiaAdjust.glLumiaAdjust_L.pop_back();
            break;
        }
        // change direction!
        if(j - fp == 1 )
        {
            direction_flag = 1;
            //printf("direction_flag = %d\n", direction_flag);
        }
        else
        {
            direction_flag = 0;
            //printf("direction_flag = %d\n", direction_flag);
        }
    }

    LOGE("VTX_NUM_L: %d\n", objPoints.glObjPoints_L.size());
    texCoords.glTexCoord_L.resize(objPoints.glObjPoints_L.size());
#endif

#if 1
    theta1 = (PIDIV2 - theta0);
    deltaTheta = 2 * theta1;
    direction_flag = 0;

    for (i = 0; i < 1; i += fp)
    {
        phi0 = PIDIV2 - PI * i;
        radius0 = Rz0 + Rz2 * myCos(phi0);
        phi1 = PIDIV2 - PI * (i + fp);
        radius1 = Rz0 + Rz2 * myCos(phi1);

        switch(direction_flag)
        {
        case 0:
            //printf("radius = %f\n", radius);
            for (j = 0; j <= 1; j += fp)
            {

                deltaTheta = 2 * myAsin(para_field.car_length / 2 / radius0);
                theta = -deltaTheta / 2 + j * deltaTheta;



                pxv = radius0 * mySin(theta);
                pzv = radius0 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi0);

                pxw = pzv - para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                adjustCoeff = 1.0 - 1.0 * (para_field.car_length / 2 + pxv) / para_field.car_length;

                //printf("000 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_R.push_back(pt3d_w);
                vertexCoords.glVertex_R.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_R.push_back(adjustCoeff);



                deltaTheta = 2 * myAsin(para_field.car_length / 2 / radius1);
                theta = -deltaTheta / 2 + j * deltaTheta;


                pxv = radius1 * mySin(theta);
                pzv = radius1 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi1);


                pxw = pzv - para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                adjustCoeff = 1.0 - 1.0 * (para_field.car_length / 2 + pxv) / para_field.car_length;

                //printf("111 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_R.push_back(pt3d_w);
                vertexCoords.glVertex_R.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_R.push_back(adjustCoeff);
            }
            objPoints.glObjPoints_R.pop_back();
            vertexCoords.glVertex_R.pop_back();
            lumiaAdjust.glLumiaAdjust_R.pop_back();
            break;
        case 1:
            //printf("radius = %f\n", radius);
            for( j = 1; j >= 0; j -= fp )
            {


                deltaTheta = 2 * myAsin(para_field.car_length / 2 / radius0);
                theta = -deltaTheta / 2 + j * deltaTheta;


                pxv = radius0 * mySin(theta);
                pzv = radius0 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi0);


                pxw = pzv - para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                adjustCoeff = 1.0 - 1.0 * (para_field.car_length / 2 + pxv) / para_field.car_length;

                //printf("333 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_R.push_back(pt3d_w);
                vertexCoords.glVertex_R.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_R.push_back(adjustCoeff);


                deltaTheta = 2 * myAsin(para_field.car_length / 2 / radius1);
                theta = -deltaTheta / 2 + j * deltaTheta;


                pxv = radius1 * mySin(theta);
                pzv = radius1 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi1);

                pxw = pzv - para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                adjustCoeff = 1.0 - 1.0 * (para_field.car_length / 2 + pxv) / para_field.car_length;

                //printf("444 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_R.push_back(pt3d_w);
                vertexCoords.glVertex_R.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_R.push_back(adjustCoeff);
            }
            objPoints.glObjPoints_R.pop_back();
            vertexCoords.glVertex_R.pop_back();
            lumiaAdjust.glLumiaAdjust_R.pop_back();
            break;
        }
        // change direction!
        if(j - fp == 1 )
        {
            direction_flag = 1;
            //printf("direction_flag = %d\n", direction_flag);
        }
        else
        {
            direction_flag = 0;
            //printf("direction_flag = %d\n", direction_flag);
        }
    }

    //LOGE("VTX_NUM_R: %d\n", objPoints.glObjPoints_R.size());
    //texCoords.glTexCoord_R.resize(objPoints.glObjPoints_R.size());
#endif

#if 1
    theta1 = (PIDIV2 - theta0);
    deltaTheta = 2 * theta1;

    radius = Rz0 + deltaR;

    for (i = 0; i < 1; i += fp)
    {
        radius -= deltaR;
        switch(direction_flag)
        {
        case 0:
            //printf("radius = %f\n", radius);
            for (j = 0; j <= 1; j += fp)
            {
                if((radius - inscribedR > 0) && (radius - inscribedR < deltaR))
                {
                    deltaTheta = 2 * theta1;
                    theta = j * deltaTheta + PI + PIDIV2 + theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;
                }
                else
                {
                    if(radius >= inscribedR)
                    {
                        deltaTheta = 2 * myAsin(para_field.car_length / 2 / radius);
                        theta = -deltaTheta / 2 + j * deltaTheta;
                    }
                    else
                    {
                        deltaTheta = 2 * theta1;
                        theta = j * deltaTheta + PI + PIDIV2 + theta0;
                    }



                    pxv = radius * mySin(theta);
                    pzv = radius * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv - para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                adjustCoeff = 1.0 - 1.0 * (para_field.car_length / 2 + pxv) / para_field.car_length;

                //printf("000 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_R.push_back(pt3d_w);
                vertexCoords.glVertex_R.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_R.push_back(adjustCoeff);

                if((radius - deltaR - inscribedR > 0) && (radius - deltaR - inscribedR < deltaR))
                {
                    deltaTheta = 2 * theta1;
                    theta = j * deltaTheta + PI + PIDIV2 + theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;
                }
                else
                {
                    if(radius - deltaR >= inscribedR)
                    {
                        deltaTheta = 2 * myAsin(para_field.car_length / 2 / (radius - deltaR));
                        theta = -deltaTheta / 2 + j * deltaTheta;
                    }
                    else
                    {
                        deltaTheta = 2 * theta1;
                        theta = j * deltaTheta + PI + PIDIV2 + theta0;
                    }


                    pxv = (radius - deltaR) * mySin(theta);
                    pzv = (radius - deltaR) * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv - para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                adjustCoeff = 1.0 - 1.0 * (para_field.car_length / 2 + pxv) / para_field.car_length;

                //printf("111 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_R.push_back(pt3d_w);
                vertexCoords.glVertex_R.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_R.push_back(adjustCoeff);
            }
            objPoints.glObjPoints_R.pop_back();
            vertexCoords.glVertex_R.pop_back();
            lumiaAdjust.glLumiaAdjust_R.pop_back();
            break;
        case 1:
            //printf("radius = %f\n", radius);
            for( j = 1; j >= 0; j -= fp )
            {
                if((radius - inscribedR > 0) && (radius - inscribedR < deltaR))
                {
                    deltaTheta = 2 * theta1;
                    theta = j * deltaTheta + PI + PIDIV2 + theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if(radius >= inscribedR)
                    {
                        deltaTheta = 2 * myAsin(para_field.car_length / 2 / radius);
                        theta = -deltaTheta / 2 + j * deltaTheta;
                    }
                    else
                    {
                        deltaTheta = 2 * theta1;
                        theta = j * deltaTheta + PI + PIDIV2 + theta0;
                    }


                    pxv = radius * mySin(theta);
                    pzv = radius * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv - para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                adjustCoeff = 1.0 - 1.0 * (para_field.car_length / 2 + pxv) / para_field.car_length;

                //printf("333 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_R.push_back(pt3d_w);
                vertexCoords.glVertex_R.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_R.push_back(adjustCoeff);

                if((radius - deltaR - inscribedR > 0) && (radius - deltaR - inscribedR < deltaR))
                {
                    deltaTheta = 2 * theta1;
                    theta = j * deltaTheta + PI + PIDIV2 + theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if(radius - deltaR >= inscribedR)
                    {
                        deltaTheta = 2 * myAsin(para_field.car_length / 2 / (radius - deltaR));
                        theta = -deltaTheta / 2 + j * deltaTheta;
                    }
                    else
                    {
                        deltaTheta = 2 * theta1;
                        theta = j * deltaTheta + PI + PIDIV2 + theta0;
                    }


                    pxv = (radius - deltaR) * mySin(theta);
                    pzv = (radius - deltaR) * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv - para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                adjustCoeff = 1.0 - 1.0 * (para_field.car_length / 2 + pxv) / para_field.car_length;

                //printf("444 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_R.push_back(pt3d_w);
                vertexCoords.glVertex_R.push_back(pt3d_v);
                lumiaAdjust.glLumiaAdjust_R.push_back(adjustCoeff);
            }
            objPoints.glObjPoints_R.pop_back();
            vertexCoords.glVertex_R.pop_back();
            lumiaAdjust.glLumiaAdjust_R.pop_back();
            break;
        }
        // change direction!
        if(j - fp == 1 )
        {
            direction_flag = 1;
            //printf("direction_flag = %d\n", direction_flag);
        }
        else
        {
            direction_flag = 0;
            //printf("direction_flag = %d\n", direction_flag);
        }
    }

    LOGE("VTX_NUM_R: %d\n", objPoints.glObjPoints_R.size());
    texCoords.glTexCoord_R.resize(objPoints.glObjPoints_R.size());
#endif



#if 1	//front_left

    direction_flag = 0;

    for (i = 0; i < 1; i += fp)
    {
        phi0 = PIDIV2 - PI * i;
        radius0 = Rz0 + Rz2 * myCos(phi0);
        phi1 = PIDIV2 - PI * (i + fp);
        radius1 = Rz0 + Rz2 * myCos(phi1);

        switch(direction_flag)
        {
        case 0:
            //printf("0 radius = %f\n", radius);
            for (j = 0; j <= 1; j += fp1)
            {
                tandivtheta = myTan(mosaicTheta * j);
                a = 1 + tandivtheta * tandivtheta;
                b = para_field.car_width + para_field.car_length * tandivtheta;
                c = para_field.car_length * para_field.car_length / 4 +
                    para_field.car_width * para_field.car_width / 4 - radius0 * radius0;

                z = quadraticEquation(a, b, c);
                x = z * tandivtheta;

                deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                theta = PIDIV2 + deltaTheta;


                pxv = radius0 * mySin(theta);
                pzv = radius0 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi0);

                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = para_field.chessboard_width_corners * para_field.square_size - (pxv - para_field.car_length / 2); // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FL_F.push_back(pt3d_w);
                }

                //printf("000 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_FL_F.push_back(pt3d_w);
                vertexCoords.glVertex_FL.push_back(pt3d_v);

                pxw = pzv + para_field.chessboard_width_corners * para_field.square_size + para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FL_L.push_back(pt3d_w);
                }

                objPoints.glObjPoints_FL_L.push_back(pt3d_w);
                blendAlpha.glAlpha_FL.push_back(1.0 - j);
                //blendAlpha.glAlpha_FL.push_back(0.0);



                tandivtheta = myTan(mosaicTheta * j);
                a = 1 + tandivtheta * tandivtheta;
                b = para_field.car_width + para_field.car_length * tandivtheta;
                c = para_field.car_length * para_field.car_length / 4 + \
                    para_field.car_width * para_field.car_width / 4 - radius1 * radius1;

                z = quadraticEquation(a, b, c);
                x = z * tandivtheta;

                deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                theta = PIDIV2 + deltaTheta;

                pxv = radius1 * mySin(theta);
                pzv = radius1 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi1);

                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = para_field.chessboard_width_corners * para_field.square_size - (pxv - para_field.car_length / 2); // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FL_F.push_back(pt3d_w);
                }

                //printf("111 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_FL_F.push_back(pt3d_w);
                vertexCoords.glVertex_FL.push_back(pt3d_v);

                pxw = pzv + para_field.chessboard_width_corners * para_field.square_size + para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FL_L.push_back(pt3d_w);
                }

                objPoints.glObjPoints_FL_L.push_back(pt3d_w);
                blendAlpha.glAlpha_FL.push_back(1.0 - j);
                //blendAlpha.glAlpha_FL.push_back(0.0);
            }
            //objPoints.glObjPoints_FL_F.pop_back();
            //objPoints.glObjPoints_FL_L.pop_back();
            //vertexCoords.glVertex_FL.pop_back();
            //blendAlpha.glAlpha_FL.pop_back();
            break;
        case 1:
            //printf("1 radius = %f\n", radius);
            for( j = 1; j >= 0; j -= fp1 )
            {
                tandivtheta = myTan(mosaicTheta * j);
                a = 1 + tandivtheta * tandivtheta;
                b = para_field.car_width + para_field.car_length * tandivtheta;
                c = para_field.car_length * para_field.car_length / 4 + \
                    para_field.car_width * para_field.car_width / 4 - radius0 * radius0;

                z = quadraticEquation(a, b, c);
                x = z * tandivtheta;

                deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                theta = PIDIV2 + deltaTheta;

                pxv = radius0 * mySin(theta);
                pzv = radius0 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi0);

                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = para_field.chessboard_width_corners * para_field.square_size - (pxv - para_field.car_length / 2); // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FL_F.push_back(pt3d_w);
                }

                //printf("333 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_FL_F.push_back(pt3d_w);
                vertexCoords.glVertex_FL.push_back(pt3d_v);

                pxw = pzv + para_field.chessboard_width_corners * para_field.square_size + para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FL_L.push_back(pt3d_w);
                }

                objPoints.glObjPoints_FL_L.push_back(pt3d_w);
                blendAlpha.glAlpha_FL.push_back(1.0 - j);
                //blendAlpha.glAlpha_FL.push_back(0.0);


                tandivtheta = myTan(mosaicTheta * j);
                a = 1 + tandivtheta * tandivtheta;
                b = para_field.car_width + para_field.car_length * tandivtheta;
                c = para_field.car_length * para_field.car_length / 4 + \
                    para_field.car_width * para_field.car_width / 4 - radius1 * radius1;

                z = quadraticEquation(a, b, c);
                x = z * tandivtheta;

                deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                theta = PIDIV2 + deltaTheta;

                pxv = radius1 * mySin(theta);
                pzv = radius1 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi1);

                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = para_field.chessboard_width_corners * para_field.square_size - (pxv - para_field.car_length / 2); // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FL_F.push_back(pt3d_w);
                }

                //printf("444 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_FL_F.push_back(pt3d_w);
                vertexCoords.glVertex_FL.push_back(pt3d_v);

                pxw = pzv + para_field.chessboard_width_corners * para_field.square_size + para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FL_L.push_back(pt3d_w);
                }

                objPoints.glObjPoints_FL_L.push_back(pt3d_w);
                blendAlpha.glAlpha_FL.push_back(1.0 - j);
                //blendAlpha.glAlpha_FL.push_back(0.0);
            }
            //objPoints.glObjPoints_FL_F.pop_back();
            //objPoints.glObjPoints_FL_L.pop_back();
            //vertexCoords.glVertex_FL.pop_back();
            //blendAlpha.glAlpha_FL.pop_back();
            break;
        }
        // change direction!
        if(j - fp1 == 1 )
        {
            direction_flag = 1;
            //printf("direction_flag = %d\n", direction_flag);
        }
        else
        {
            direction_flag = 0;
            //printf("direction_flag = %d\n", direction_flag);
        }
    }


    //texCoords.glTexCoord_FL_F.resize(vertexCoords.glVertex_FL.size());
    //texCoords.glTexCoord_FL_L.resize(vertexCoords.glVertex_FL.size());
    //texCoordsStatistics.glTexCoord_FL_F.resize(objPointsStatistics.
    //glObjPoints_FL_F.size());
    //texCoordsStatistics.glTexCoord_FL_L.resize(objPointsStatistics.
    //glObjPoints_FL_L.size());
    //LOGE("VTX_NUM_FL: %d\n", vertexCoords.glVertex_FL.size());
#endif


#if 1	//front_left
    deltaR = 1.0 * Rz0 / p;

    radius = Rz0 + deltaR;

    for (i = 0; i < 1; i += fp)
    {
        radius -= deltaR;
        switch(direction_flag)
        {
        case 0:
            //printf("0 radius = %f\n", radius);
            for (j = 0; j <= 1; j += fp1)
            {
                if((radius - inscribedR > 0) && (radius - inscribedR < deltaR))
                {
                    theta = PIDIV2 + theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if(radius >= inscribedR)
                    {
                        tandivtheta = myTan(mosaicTheta * j);
                        a = 1 + tandivtheta * tandivtheta;
                        b = para_field.car_width + para_field.car_length * tandivtheta;
                        c = para_field.car_length * para_field.car_length / 4 +
                            para_field.car_width * para_field.car_width / 4 - radius * radius;

                        z = quadraticEquation(a, b, c);
                        x = z * tandivtheta;

                        deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                        theta = PIDIV2 + deltaTheta;
                    }
                    else
                    {
                        theta = PIDIV2 + theta0;
                    }


                    pxv = radius * mySin(theta);
                    pzv = radius * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = para_field.chessboard_width_corners * para_field.square_size - (pxv - para_field.car_length / 2); // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FL_F.push_back(pt3d_w);
                }

                //printf("000 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_FL_F.push_back(pt3d_w);
                vertexCoords.glVertex_FL.push_back(pt3d_v);

                pxw = pzv + para_field.chessboard_width_corners * para_field.square_size + para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FL_L.push_back(pt3d_w);
                }

                objPoints.glObjPoints_FL_L.push_back(pt3d_w);
                blendAlpha.glAlpha_FL.push_back(1.0 - j);
                //blendAlpha.glAlpha_FL.push_back(0.0);

                if((radius - deltaR - inscribedR > 0) && (radius - deltaR - inscribedR < deltaR))
                {
                    theta = PIDIV2 + theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if((radius - deltaR) >= inscribedR)
                    {
                        tandivtheta = myTan(mosaicTheta * j);
                        a = 1 + tandivtheta * tandivtheta;
                        b = para_field.car_width + para_field.car_length * tandivtheta;
                        c = para_field.car_length * para_field.car_length / 4 + \
                            para_field.car_width * para_field.car_width / 4 - (radius - deltaR) * (radius - deltaR);

                        z = quadraticEquation(a, b, c);
                        x = z * tandivtheta;

                        deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                        theta = PIDIV2 + deltaTheta;
                    }
                    else
                    {
                        theta = PIDIV2 + theta0;
                    }



                    pxv = (radius - deltaR) * mySin(theta);
                    pzv = (radius - deltaR) * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = para_field.chessboard_width_corners * para_field.square_size - (pxv - para_field.car_length / 2); // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FL_F.push_back(pt3d_w);
                }

                //printf("111 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_FL_F.push_back(pt3d_w);
                vertexCoords.glVertex_FL.push_back(pt3d_v);

                pxw = pzv + para_field.chessboard_width_corners * para_field.square_size + para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FL_L.push_back(pt3d_w);
                }

                objPoints.glObjPoints_FL_L.push_back(pt3d_w);
                blendAlpha.glAlpha_FL.push_back(1.0 - j);
                //blendAlpha.glAlpha_FL.push_back(0.0);
            }
            //objPoints.glObjPoints_FL_F.pop_back();
            //objPoints.glObjPoints_FL_L.pop_back();
            //vertexCoords.glVertex_FL.pop_back();
            //blendAlpha.glAlpha_FL.pop_back();
            break;
        case 1:
            //printf("1 radius = %f\n", radius);
            for( j = 1; j >= 0; j -= fp1 )
            {
                if((radius - inscribedR > 0) && (radius - inscribedR < deltaR))
                {
                    theta = PIDIV2 + theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if(radius >= inscribedR)
                    {
                        tandivtheta = myTan(mosaicTheta * j);
                        a = 1 + tandivtheta * tandivtheta;
                        b = para_field.car_width + para_field.car_length * tandivtheta;
                        c = para_field.car_length * para_field.car_length / 4 + \
                            para_field.car_width * para_field.car_width / 4 - radius * radius;

                        z = quadraticEquation(a, b, c);
                        x = z * tandivtheta;

                        deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                        theta = PIDIV2 + deltaTheta;
                    }
                    else
                    {
                        theta = PIDIV2 + theta0;
                    }


                    pxv = radius * mySin(theta);
                    pzv = radius * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = para_field.chessboard_width_corners * para_field.square_size - (pxv - para_field.car_length / 2); // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FL_F.push_back(pt3d_w);
                }

                //printf("333 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_FL_F.push_back(pt3d_w);
                vertexCoords.glVertex_FL.push_back(pt3d_v);

                pxw = pzv + para_field.chessboard_width_corners * para_field.square_size + para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FL_L.push_back(pt3d_w);
                }

                objPoints.glObjPoints_FL_L.push_back(pt3d_w);
                blendAlpha.glAlpha_FL.push_back(1.0 - j);
                //blendAlpha.glAlpha_FL.push_back(0.0);

                if((radius - deltaR - inscribedR > 0) && (radius - deltaR - inscribedR < deltaR))
                {
                    theta = PIDIV2 + theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if((radius - deltaR) >= inscribedR)
                    {
                        tandivtheta = myTan(mosaicTheta * j);
                        a = 1 + tandivtheta * tandivtheta;
                        b = para_field.car_width + para_field.car_length * tandivtheta;
                        c = para_field.car_length * para_field.car_length / 4 + \
                            para_field.car_width * para_field.car_width / 4 - (radius - deltaR) * (radius - deltaR);

                        z = quadraticEquation(a, b, c);
                        x = z * tandivtheta;

                        deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                        theta = PIDIV2 + deltaTheta;
                    }
                    else
                    {
                        theta = PIDIV2 + theta0;
                    }


                    pxv = (radius - deltaR) * mySin(theta);
                    pzv = (radius - deltaR) * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = para_field.chessboard_width_corners * para_field.square_size - (pxv - para_field.car_length / 2); // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FL_F.push_back(pt3d_w);
                }

                //printf("444 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_FL_F.push_back(pt3d_w);
                vertexCoords.glVertex_FL.push_back(pt3d_v);

                pxw = pzv + para_field.chessboard_width_corners * para_field.square_size + para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FL_L.push_back(pt3d_w);
                }

                objPoints.glObjPoints_FL_L.push_back(pt3d_w);
                blendAlpha.glAlpha_FL.push_back(1.0 - j);
                //blendAlpha.glAlpha_FL.push_back(0.0);
            }
            //objPoints.glObjPoints_FL_F.pop_back();
            //objPoints.glObjPoints_FL_L.pop_back();
            //vertexCoords.glVertex_FL.pop_back();
            //blendAlpha.glAlpha_FL.pop_back();
            break;
        }
        // change direction!
        if(j - fp1 == 1 )
        {
            direction_flag = 1;
            //printf("direction_flag = %d\n", direction_flag);
        }
        else
        {
            direction_flag = 0;
            //printf("direction_flag = %d\n", direction_flag);
        }
    }


    texCoords.glTexCoord_FL_F.resize(vertexCoords.glVertex_FL.size());
    texCoords.glTexCoord_FL_L.resize(vertexCoords.glVertex_FL.size());
    texCoordsStatistics.glTexCoord_FL_F.resize(objPointsStatistics.
            glObjPoints_FL_F.size());
    texCoordsStatistics.glTexCoord_FL_L.resize(objPointsStatistics.
            glObjPoints_FL_L.size());
    LOGE("VTX_NUM_FL: %d\n", vertexCoords.glVertex_FL.size());
#endif

#if 1	//front_right

    direction_flag = 0;

    for (i = 0; i < 1; i += fp)
    {
        phi0 = PIDIV2 - PI * i;
        radius0 = Rz0 + Rz2 * myCos(phi0);
        phi1 = PIDIV2 - PI * (i + fp);
        radius1 = Rz0 + Rz2 * myCos(phi1);

        switch(direction_flag)
        {
        case 0:
            //printf("0 radius = %f\n", radius);
            for (j = 0; j <= 1; j += fp1)
            {


                tandivtheta = myTan(mosaicTheta * j);
                a = 1 + tandivtheta * tandivtheta;
                b = para_field.car_width + para_field.car_length * tandivtheta;
                c = para_field.car_length * para_field.car_length / 4 + \
                    para_field.car_width * para_field.car_width / 4 - radius0 * radius0;

                z = quadraticEquation(a, b, c);
                x = z * tandivtheta;

                deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                theta = PIDIV2 - deltaTheta;


                pxv = radius0 * mySin(theta);
                pzv = radius0 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi0);

                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = para_field.chessboard_width_corners * para_field.square_size - (pxv - para_field.car_length / 2); // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FR_F.push_back(pt3d_w);
                }

                //printf("000 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_FR_F.push_back(pt3d_w);
                vertexCoords.glVertex_FR.push_back(pt3d_v);

                pxw = pzv - para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FR_R.push_back(pt3d_w);
                }

                objPoints.glObjPoints_FR_R.push_back(pt3d_w);
                blendAlpha.glAlpha_FR.push_back(1 - j);



                tandivtheta = myTan(mosaicTheta * j);
                a = 1 + tandivtheta * tandivtheta;
                b = para_field.car_width + para_field.car_length * tandivtheta;
                c = para_field.car_length * para_field.car_length / 4 + \
                    para_field.car_width * para_field.car_width / 4 - radius1 * radius1;

                z = quadraticEquation(a, b, c);
                x = z * tandivtheta;

                deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                theta = PIDIV2 - deltaTheta;



                pxv = radius1 * mySin(theta);
                pzv = radius1 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi1);

                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = para_field.chessboard_width_corners * para_field.square_size - (pxv - para_field.car_length / 2); // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FR_F.push_back(pt3d_w);
                }

                //printf("111 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_FR_F.push_back(pt3d_w);
                vertexCoords.glVertex_FR.push_back(pt3d_v);

                pxw = pzv - para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FR_R.push_back(pt3d_w);
                }

                objPoints.glObjPoints_FR_R.push_back(pt3d_w);
                blendAlpha.glAlpha_FR.push_back(1 - j);
            }

            break;
        case 1:
            //printf("1 radius = %f\n", radius);
            for( j = 1; j >= 0; j -= fp1 )
            {


                tandivtheta = myTan(mosaicTheta * j);
                a = 1 + tandivtheta * tandivtheta;
                b = para_field.car_width + para_field.car_length * tandivtheta;
                c = para_field.car_length * para_field.car_length / 4 + \
                    para_field.car_width * para_field.car_width / 4 - radius0 * radius0;

                z = quadraticEquation(a, b, c);
                x = z * tandivtheta;

                deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                theta = PIDIV2 - deltaTheta;


                pxv = radius0 * mySin(theta);
                pzv = radius0 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi0);

                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = para_field.chessboard_width_corners * para_field.square_size - (pxv - para_field.car_length / 2); // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FR_F.push_back(pt3d_w);
                }

                //printf("333 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_FR_F.push_back(pt3d_w);
                vertexCoords.glVertex_FR.push_back(pt3d_v);

                pxw = pzv - para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FR_R.push_back(pt3d_w);
                }

                objPoints.glObjPoints_FR_R.push_back(pt3d_w);
                blendAlpha.glAlpha_FR.push_back(1 - j);


                tandivtheta = myTan(mosaicTheta * j);
                a = 1 + tandivtheta * tandivtheta;
                b = para_field.car_width + para_field.car_length * tandivtheta;
                c = para_field.car_length * para_field.car_length / 4 + \
                    para_field.car_width * para_field.car_width / 4 - radius1 * radius1;

                z = quadraticEquation(a, b, c);
                x = z * tandivtheta;

                deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                theta = PIDIV2 - deltaTheta;

                pxv = radius1 * mySin(theta);
                pzv = radius1 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi1);

                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = para_field.chessboard_width_corners * para_field.square_size - (pxv - para_field.car_length / 2); // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FR_F.push_back(pt3d_w);
                }

                //printf("444 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_FR_F.push_back(pt3d_w);
                vertexCoords.glVertex_FR.push_back(pt3d_v);

                pxw = pzv - para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FR_R.push_back(pt3d_w);
                }

                objPoints.glObjPoints_FR_R.push_back(pt3d_w);
                blendAlpha.glAlpha_FR.push_back(1 - j);
            }
            break;
        }
        // change direction!
        if(j - fp1 == 1 )
        {
            direction_flag = 1;
            //printf("direction_flag = %d\n", direction_flag);
        }
        else
        {
            direction_flag = 0;
            //printf("direction_flag = %d\n", direction_flag);
        }
    }


    //texCoords.glTexCoord_FR_F.resize(vertexCoords.glVertex_FR.size());
    //texCoords.glTexCoord_FR_R.resize(vertexCoords.glVertex_FR.size());
    //texCoordsStatistics.glTexCoord_FR_F.resize(objPointsStatistics.
    //glObjPoints_FR_F.size());
    //texCoordsStatistics.glTexCoord_FR_R.resize(objPointsStatistics.
    //glObjPoints_FR_R.size());
    //LOGE("VTX_NUM_FR: %d\n", vertexCoords.glVertex_FR.size());
#endif

#if 1	//front_right

    deltaR = 1.0 * Rz0 / p;

    radius = Rz0 + deltaR;

    for (i = 0; i < 1; i += fp)
    {
        radius -= deltaR;
        switch(direction_flag)
        {
        case 0:
            //printf("0 radius = %f\n", radius);
            for (j = 0; j <= 1; j += fp1)
            {
                if((radius - inscribedR > 0) && (radius - inscribedR < deltaR))
                {
                    theta = PIDIV2 - theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if(radius >= inscribedR)
                    {
                        tandivtheta = myTan(mosaicTheta * j);
                        a = 1 + tandivtheta * tandivtheta;
                        b = para_field.car_width + para_field.car_length * tandivtheta;
                        c = para_field.car_length * para_field.car_length / 4 + \
                            para_field.car_width * para_field.car_width / 4 - radius * radius;

                        z = quadraticEquation(a, b, c);
                        x = z * tandivtheta;

                        deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                        theta = PIDIV2 - deltaTheta;
                    }
                    else
                    {
                        theta = PIDIV2 - theta0;
                    }


                    pxv = radius * mySin(theta);
                    pzv = radius * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = para_field.chessboard_width_corners * para_field.square_size - (pxv - para_field.car_length / 2); // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FR_F.push_back(pt3d_w);
                }

                //printf("000 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_FR_F.push_back(pt3d_w);
                vertexCoords.glVertex_FR.push_back(pt3d_v);

                pxw = pzv - para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FR_R.push_back(pt3d_w);
                }

                objPoints.glObjPoints_FR_R.push_back(pt3d_w);
                blendAlpha.glAlpha_FR.push_back(1 - j);

                if((radius - deltaR - inscribedR > 0) && (radius - deltaR - inscribedR < deltaR))
                {
                    theta = PIDIV2 - theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if((radius - deltaR) >= inscribedR)
                    {
                        tandivtheta = myTan(mosaicTheta * j);
                        a = 1 + tandivtheta * tandivtheta;
                        b = para_field.car_width + para_field.car_length * tandivtheta;
                        c = para_field.car_length * para_field.car_length / 4 + \
                            para_field.car_width * para_field.car_width / 4 - (radius - deltaR) * (radius - deltaR);

                        z = quadraticEquation(a, b, c);
                        x = z * tandivtheta;

                        deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                        theta = PIDIV2 - deltaTheta;
                    }
                    else
                    {
                        theta = PIDIV2 - theta0;
                    }



                    pxv = (radius - deltaR) * mySin(theta);
                    pzv = (radius - deltaR) * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = para_field.chessboard_width_corners * para_field.square_size - (pxv - para_field.car_length / 2); // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FR_F.push_back(pt3d_w);
                }

                //printf("111 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_FR_F.push_back(pt3d_w);
                vertexCoords.glVertex_FR.push_back(pt3d_v);

                pxw = pzv - para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FR_R.push_back(pt3d_w);
                }

                objPoints.glObjPoints_FR_R.push_back(pt3d_w);
                blendAlpha.glAlpha_FR.push_back(1 - j);
            }

            break;
        case 1:
            //printf("1 radius = %f\n", radius);
            for( j = 1; j >= 0; j -= fp1 )
            {
                if((radius - inscribedR > 0) && (radius - inscribedR < deltaR))
                {
                    theta = PIDIV2 - theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if(radius >= inscribedR)
                    {
                        tandivtheta = myTan(mosaicTheta * j);
                        a = 1 + tandivtheta * tandivtheta;
                        b = para_field.car_width + para_field.car_length * tandivtheta;
                        c = para_field.car_length * para_field.car_length / 4 + \
                            para_field.car_width * para_field.car_width / 4 - radius * radius;

                        z = quadraticEquation(a, b, c);
                        x = z * tandivtheta;

                        deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                        theta = PIDIV2 - deltaTheta;
                    }
                    else
                    {
                        theta = PIDIV2 - theta0;
                    }


                    pxv = radius * mySin(theta);
                    pzv = radius * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = para_field.chessboard_width_corners * para_field.square_size - (pxv - para_field.car_length / 2); // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FR_F.push_back(pt3d_w);
                }

                //printf("333 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_FR_F.push_back(pt3d_w);
                vertexCoords.glVertex_FR.push_back(pt3d_v);

                pxw = pzv - para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FR_R.push_back(pt3d_w);
                }

                objPoints.glObjPoints_FR_R.push_back(pt3d_w);
                blendAlpha.glAlpha_FR.push_back(1 - j);

                if((radius - deltaR - inscribedR > 0) && (radius - deltaR - inscribedR < deltaR))
                {
                    theta = PIDIV2 - theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if((radius - deltaR) >= inscribedR)
                    {
                        tandivtheta = myTan(mosaicTheta * j);
                        a = 1 + tandivtheta * tandivtheta;
                        b = para_field.car_width + para_field.car_length * tandivtheta;
                        c = para_field.car_length * para_field.car_length / 4 + \
                            para_field.car_width * para_field.car_width / 4 - (radius - deltaR) * (radius - deltaR);

                        z = quadraticEquation(a, b, c);
                        x = z * tandivtheta;

                        deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                        theta = PIDIV2 - deltaTheta;
                    }
                    else
                    {
                        theta = PIDIV2 - theta0;
                    }


                    pxv = (radius - deltaR) * mySin(theta);
                    pzv = (radius - deltaR) * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = para_field.chessboard_width_corners * para_field.square_size - (pxv - para_field.car_length / 2); // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FR_F.push_back(pt3d_w);
                }

                //printf("444 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_FR_F.push_back(pt3d_w);
                vertexCoords.glVertex_FR.push_back(pt3d_v);

                pxw = pzv - para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_FR_R.push_back(pt3d_w);
                }

                objPoints.glObjPoints_FR_R.push_back(pt3d_w);
                blendAlpha.glAlpha_FR.push_back(1 - j);
            }
            break;
        }
        // change direction!
        if(j - fp1 == 1 )
        {
            direction_flag = 1;
            //printf("direction_flag = %d\n", direction_flag);
        }
        else
        {
            direction_flag = 0;
            //printf("direction_flag = %d\n", direction_flag);
        }
    }


    texCoords.glTexCoord_FR_F.resize(vertexCoords.glVertex_FR.size());
    texCoords.glTexCoord_FR_R.resize(vertexCoords.glVertex_FR.size());
    texCoordsStatistics.glTexCoord_FR_F.resize(objPointsStatistics.
            glObjPoints_FR_F.size());
    texCoordsStatistics.glTexCoord_FR_R.resize(objPointsStatistics.
            glObjPoints_FR_R.size());
    LOGE("VTX_NUM_FR: %d\n", vertexCoords.glVertex_FR.size());
#endif

#if 1	//rear_left

    direction_flag = 0;

    for (i = 0; i < 1; i += fp)
    {
        phi0 = PIDIV2 - PI * i;
        radius0 = Rz0 + Rz2 * myCos(phi0);
        phi1 = PIDIV2 - PI * (i + fp);
        radius1 = Rz0 + Rz2 * myCos(phi1);

        switch(direction_flag)
        {
        case 0:
            //printf("0 radius = %f\n", radius);
            for (j = 0; j <= 1; j += fp1)
            {


                tandivtheta = myTan(mosaicTheta * j);
                a = 1 + tandivtheta * tandivtheta;
                b = para_field.car_width + para_field.car_length * tandivtheta;
                c = para_field.car_length * para_field.car_length / 4 + \
                    para_field.car_width * para_field.car_width / 4 - radius0 * radius0;

                z = quadraticEquation(a, b, c);
                x = z * tandivtheta;

                deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                theta = PI + PIDIV2 - deltaTheta;

                pxv = radius0 * mySin(theta);
                pzv = radius0 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi0);

                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = -pxv - para_field.car_length / 2; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BL_B.push_back(pt3d_w);
                }

                //printf("000 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_BL_B.push_back(pt3d_w);
                vertexCoords.glVertex_BL.push_back(pt3d_v);

                pxw = pzv + para_field.chessboard_width_corners * para_field.square_size + para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BL_L.push_back(pt3d_w);
                }

                objPoints.glObjPoints_BL_L.push_back(pt3d_w);
                blendAlpha.glAlpha_BL.push_back(1 - j);



                tandivtheta = myTan(mosaicTheta * j);
                a = 1 + tandivtheta * tandivtheta;
                b = para_field.car_width + para_field.car_length * tandivtheta;
                c = para_field.car_length * para_field.car_length / 4 + \
                    para_field.car_width * para_field.car_width / 4 - radius1 * radius1;

                z = quadraticEquation(a, b, c);
                x = z * tandivtheta;

                deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                theta = PI + PIDIV2 - deltaTheta;



                pxv = radius1 * mySin(theta);
                pzv = radius1 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi1);

                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = -pxv - para_field.car_length / 2; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BL_B.push_back(pt3d_w);
                }

                //printf("111 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_BL_B.push_back(pt3d_w);
                vertexCoords.glVertex_BL.push_back(pt3d_v);

                pxw = pzv + para_field.chessboard_width_corners * para_field.square_size + para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BL_L.push_back(pt3d_w);
                }

                objPoints.glObjPoints_BL_L.push_back(pt3d_w);
                blendAlpha.glAlpha_BL.push_back(1 - j);
            }

            break;
        case 1:
            //printf("1 radius = %f\n", radius);
            for( j = 1; j >= 0; j -= fp1 )
            {


                tandivtheta = myTan(mosaicTheta * j);
                a = 1 + tandivtheta * tandivtheta;
                b = para_field.car_width + para_field.car_length * tandivtheta;
                c = para_field.car_length * para_field.car_length / 4 + \
                    para_field.car_width * para_field.car_width / 4 - radius0 * radius0;

                z = quadraticEquation(a, b, c);
                x = z * tandivtheta;

                deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                theta = PI + PIDIV2 - deltaTheta;

                pxv = radius0 * mySin(theta);
                pzv = radius0 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi0);

                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = -pxv - para_field.car_length / 2; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BL_B.push_back(pt3d_w);
                }

                //printf("333 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_BL_B.push_back(pt3d_w);
                vertexCoords.glVertex_BL.push_back(pt3d_v);

                pxw = pzv + para_field.chessboard_width_corners * para_field.square_size + para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BL_L.push_back(pt3d_w);
                }

                objPoints.glObjPoints_BL_L.push_back(pt3d_w);
                blendAlpha.glAlpha_BL.push_back(1 - j);



                tandivtheta = myTan(mosaicTheta * j);
                a = 1 + tandivtheta * tandivtheta;
                b = para_field.car_width + para_field.car_length * tandivtheta;
                c = para_field.car_length * para_field.car_length / 4 + \
                    para_field.car_width * para_field.car_width / 4 - radius1 * radius1;

                z = quadraticEquation(a, b, c);
                x = z * tandivtheta;

                deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                theta = PI + PIDIV2 - deltaTheta;


                pxv = radius1 * mySin(theta);
                pzv = radius1 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi1);

                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = -pxv - para_field.car_length / 2; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BL_B.push_back(pt3d_w);
                }

                //printf("444 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_BL_B.push_back(pt3d_w);
                vertexCoords.glVertex_BL.push_back(pt3d_v);

                pxw = pzv + para_field.chessboard_width_corners * para_field.square_size + para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BL_L.push_back(pt3d_w);
                }

                objPoints.glObjPoints_BL_L.push_back(pt3d_w);
                blendAlpha.glAlpha_BL.push_back(1 - j);
            }
            break;
        }
        // change direction!
        if(j - fp1 == 1 )
        {
            direction_flag = 1;
            //printf("direction_flag = %d\n", direction_flag);
        }
        else
        {
            direction_flag = 0;
            //printf("direction_flag = %d\n", direction_flag);
        }
    }


    //texCoords.glTexCoord_BL_B.resize(vertexCoords.glVertex_BL.size());
    //texCoords.glTexCoord_BL_L.resize(vertexCoords.glVertex_BL.size());
    //texCoordsStatistics.glTexCoord_BL_B.resize(objPointsStatistics.
    //glObjPoints_BL_B.size());
    //texCoordsStatistics.glTexCoord_BL_L.resize(objPointsStatistics.
    //glObjPoints_BL_L.size());
    //LOGE("VTX_NUM_BL: %d\n", vertexCoords.glVertex_BL.size());
#endif

#if 1	//rear_left

    deltaR = 1.0 * Rz0 / p;
    radius = Rz0 + deltaR;

    for (i = 0; i < 1; i += fp)
    {
        radius -= deltaR;
        switch(direction_flag)
        {
        case 0:
            //printf("0 radius = %f\n", radius);
            for (j = 0; j <= 1; j += fp1)
            {
                if((radius - inscribedR > 0) && (radius - inscribedR < deltaR))
                {
                    theta = PI + PIDIV2 - theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if(radius >= inscribedR)
                    {
                        tandivtheta = myTan(mosaicTheta * j);
                        a = 1 + tandivtheta * tandivtheta;
                        b = para_field.car_width + para_field.car_length * tandivtheta;
                        c = para_field.car_length * para_field.car_length / 4 + \
                            para_field.car_width * para_field.car_width / 4 - radius * radius;

                        z = quadraticEquation(a, b, c);
                        x = z * tandivtheta;

                        deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                        theta = PI + PIDIV2 - deltaTheta;
                    }
                    else
                    {
                        theta = PI + PIDIV2 - theta0;
                    }


                    pxv = radius * mySin(theta);
                    pzv = radius * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = -pxv - para_field.car_length / 2; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BL_B.push_back(pt3d_w);
                }

                //printf("000 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_BL_B.push_back(pt3d_w);
                vertexCoords.glVertex_BL.push_back(pt3d_v);

                pxw = pzv + para_field.chessboard_width_corners * para_field.square_size + para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BL_L.push_back(pt3d_w);
                }

                objPoints.glObjPoints_BL_L.push_back(pt3d_w);
                blendAlpha.glAlpha_BL.push_back(1 - j);

                if((radius - deltaR - inscribedR > 0) && (radius - deltaR - inscribedR < deltaR))
                {
                    theta = PI + PIDIV2 - theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if((radius - deltaR) >= inscribedR)
                    {
                        tandivtheta = myTan(mosaicTheta * j);
                        a = 1 + tandivtheta * tandivtheta;
                        b = para_field.car_width + para_field.car_length * tandivtheta;
                        c = para_field.car_length * para_field.car_length / 4 + \
                            para_field.car_width * para_field.car_width / 4 - (radius - deltaR) * (radius - deltaR);

                        z = quadraticEquation(a, b, c);
                        x = z * tandivtheta;

                        deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                        theta = PI + PIDIV2 - deltaTheta;
                    }
                    else
                    {
                        theta = PI + PIDIV2 - theta0;
                    }



                    pxv = (radius - deltaR) * mySin(theta);
                    pzv = (radius - deltaR) * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = -pxv - para_field.car_length / 2; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BL_B.push_back(pt3d_w);
                }

                //printf("111 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_BL_B.push_back(pt3d_w);
                vertexCoords.glVertex_BL.push_back(pt3d_v);

                pxw = pzv + para_field.chessboard_width_corners * para_field.square_size + para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BL_L.push_back(pt3d_w);
                }

                objPoints.glObjPoints_BL_L.push_back(pt3d_w);
                blendAlpha.glAlpha_BL.push_back(1 - j);
            }

            break;
        case 1:
            //printf("1 radius = %f\n", radius);
            for( j = 1; j >= 0; j -= fp1 )
            {
                if((radius - inscribedR > 0) && (radius - inscribedR < deltaR))
                {
                    theta = PI + PIDIV2 - theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if(radius >= inscribedR)
                    {
                        tandivtheta = myTan(mosaicTheta * j);
                        a = 1 + tandivtheta * tandivtheta;
                        b = para_field.car_width + para_field.car_length * tandivtheta;
                        c = para_field.car_length * para_field.car_length / 4 + \
                            para_field.car_width * para_field.car_width / 4 - radius * radius;

                        z = quadraticEquation(a, b, c);
                        x = z * tandivtheta;

                        deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                        theta = PI + PIDIV2 - deltaTheta;
                    }
                    else
                    {
                        theta = PI + PIDIV2 - theta0;
                    }


                    pxv = radius * mySin(theta);
                    pzv = radius * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = -pxv - para_field.car_length / 2; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BL_B.push_back(pt3d_w);
                }

                //printf("333 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_BL_B.push_back(pt3d_w);
                vertexCoords.glVertex_BL.push_back(pt3d_v);

                pxw = pzv + para_field.chessboard_width_corners * para_field.square_size + para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BL_L.push_back(pt3d_w);
                }

                objPoints.glObjPoints_BL_L.push_back(pt3d_w);
                blendAlpha.glAlpha_BL.push_back(1 - j);

                if((radius - deltaR - inscribedR > 0) && (radius - deltaR - inscribedR < deltaR))
                {
                    theta = PI + PIDIV2 - theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if((radius - deltaR) >= inscribedR)
                    {
                        tandivtheta = myTan(mosaicTheta * j);
                        a = 1 + tandivtheta * tandivtheta;
                        b = para_field.car_width + para_field.car_length * tandivtheta;
                        c = para_field.car_length * para_field.car_length / 4 + \
                            para_field.car_width * para_field.car_width / 4 - (radius - deltaR) * (radius - deltaR);

                        z = quadraticEquation(a, b, c);
                        x = z * tandivtheta;

                        deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                        theta = PI + PIDIV2 - deltaTheta;
                    }
                    else
                    {
                        theta = PI + PIDIV2 - theta0;
                    }


                    pxv = (radius - deltaR) * mySin(theta);
                    pzv = (radius - deltaR) * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = -pxv - para_field.car_length / 2; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BL_B.push_back(pt3d_w);
                }

                //printf("444 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_BL_B.push_back(pt3d_w);
                vertexCoords.glVertex_BL.push_back(pt3d_v);

                pxw = pzv + para_field.chessboard_width_corners * para_field.square_size + para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BL_L.push_back(pt3d_w);
                }

                objPoints.glObjPoints_BL_L.push_back(pt3d_w);
                blendAlpha.glAlpha_BL.push_back(1 - j);
            }
            break;
        }
        // change direction!
        if(j - fp1 == 1 )
        {
            direction_flag = 1;
            //printf("direction_flag = %d\n", direction_flag);
        }
        else
        {
            direction_flag = 0;
            //printf("direction_flag = %d\n", direction_flag);
        }
    }


    texCoords.glTexCoord_BL_B.resize(vertexCoords.glVertex_BL.size());
    texCoords.glTexCoord_BL_L.resize(vertexCoords.glVertex_BL.size());
    texCoordsStatistics.glTexCoord_BL_B.resize(objPointsStatistics.
            glObjPoints_BL_B.size());
    texCoordsStatistics.glTexCoord_BL_L.resize(objPointsStatistics.
            glObjPoints_BL_L.size());
    LOGE("VTX_NUM_BL: %d\n", vertexCoords.glVertex_BL.size());
#endif

#if 1	//rear_right

    direction_flag = 0;

    for (i = 0; i < 1; i += fp)
    {
        phi0 = PIDIV2 - PI * i;
        radius0 = Rz0 + Rz2 * myCos(phi0);
        phi1 = PIDIV2 - PI * (i + fp);
        radius1 = Rz0 + Rz2 * myCos(phi1);

        switch(direction_flag)
        {
        case 0:
            //printf("0 radius = %f\n", radius);
            for (j = 0; j <= 1; j += fp1)
            {


                tandivtheta = myTan(mosaicTheta * j);
                a = 1 + tandivtheta * tandivtheta;
                b = para_field.car_width + para_field.car_length * tandivtheta;
                c = para_field.car_length * para_field.car_length / 4 + \
                    para_field.car_width * para_field.car_width / 4 - radius0 * radius0;

                z = quadraticEquation(a, b, c);
                x = z * tandivtheta;

                deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                theta = PI + PIDIV2 + deltaTheta;


                pxv = radius0 * mySin(theta);
                pzv = radius0 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi0);

                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = -pxv - para_field.car_length / 2; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BR_B.push_back(pt3d_w);
                }

                //printf("000 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_BR_B.push_back(pt3d_w);
                vertexCoords.glVertex_BR.push_back(pt3d_v);

                pxw = pzv - para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BR_R.push_back(pt3d_w);
                }

                objPoints.glObjPoints_BR_R.push_back(pt3d_w);
                blendAlpha.glAlpha_BR.push_back(1.0 - j);


                tandivtheta = myTan(mosaicTheta * j);
                a = 1 + tandivtheta * tandivtheta;
                b = para_field.car_width + para_field.car_length * tandivtheta;
                c = para_field.car_length * para_field.car_length / 4 + \
                    para_field.car_width * para_field.car_width / 4 - radius1 * radius1;

                z = quadraticEquation(a, b, c);
                x = z * tandivtheta;

                deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                theta = PI + PIDIV2 + deltaTheta;



                pxv = radius1 * mySin(theta);
                pzv = radius1 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi1);

                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = -pxv - para_field.car_length / 2; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BR_B.push_back(pt3d_w);
                }

                //printf("111 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_BR_B.push_back(pt3d_w);
                vertexCoords.glVertex_BR.push_back(pt3d_v);

                pxw = pzv - para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BR_R.push_back(pt3d_w);
                }

                objPoints.glObjPoints_BR_R.push_back(pt3d_w);
                blendAlpha.glAlpha_BR.push_back(1.0 - j);
            }

            break;
        case 1:
            //printf("1 radius = %f\n", radius);
            for( j = 1; j >= 0; j -= fp1 )
            {

                tandivtheta = myTan(mosaicTheta * j);
                a = 1 + tandivtheta * tandivtheta;
                b = para_field.car_width + para_field.car_length * tandivtheta;
                c = para_field.car_length * para_field.car_length / 4 + \
                    para_field.car_width * para_field.car_width / 4 - radius0 * radius0;

                z = quadraticEquation(a, b, c);
                x = z * tandivtheta;

                deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                theta = PI + PIDIV2 + deltaTheta;


                pxv = radius0 * mySin(theta);
                pzv = radius0 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi0);

                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = -pxv - para_field.car_length / 2; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BR_B.push_back(pt3d_w);
                }

                //printf("333 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_BR_B.push_back(pt3d_w);
                vertexCoords.glVertex_BR.push_back(pt3d_v);

                pxw = pzv - para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BR_R.push_back(pt3d_w);
                }

                objPoints.glObjPoints_BR_R.push_back(pt3d_w);
                blendAlpha.glAlpha_BR.push_back(1.0 - j);


                tandivtheta = myTan(mosaicTheta * j);
                a = 1 + tandivtheta * tandivtheta;
                b = para_field.car_width + para_field.car_length * tandivtheta;
                c = para_field.car_length * para_field.car_length / 4 + \
                    para_field.car_width * para_field.car_width / 4 - radius1 * radius1;

                z = quadraticEquation(a, b, c);
                x = z * tandivtheta;

                deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                theta = PI + PIDIV2 + deltaTheta;

                pxv = radius1 * mySin(theta);
                pzv = radius1 * myCos(theta);
                pyv = Rz2 + Rz2 * mySin(phi1);

                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = -pxv - para_field.car_length / 2; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BR_B.push_back(pt3d_w);
                }

                //printf("444 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_BR_B.push_back(pt3d_w);
                vertexCoords.glVertex_BR.push_back(pt3d_v);

                pxw = pzv - para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BR_R.push_back(pt3d_w);
                }

                objPoints.glObjPoints_BR_R.push_back(pt3d_w);
                blendAlpha.glAlpha_BR.push_back(1.0 - j);
            }
            break;
        }
        // change direction!
        if(j - fp1 == 1 )
        {
            direction_flag = 1;
            //printf("direction_flag = %d\n", direction_flag);
        }
        else
        {
            direction_flag = 0;
            //printf("direction_flag = %d\n", direction_flag);
        }
    }


    //texCoords.glTexCoord_BR_B.resize(vertexCoords.glVertex_BR.size());
    //texCoords.glTexCoord_BR_R.resize(vertexCoords.glVertex_BR.size());
    //texCoordsStatistics.glTexCoord_BR_B.resize(objPointsStatistics.glObjPoints_BR_B.size());
    //texCoordsStatistics.glTexCoord_BR_R.resize(objPointsStatistics.glObjPoints_BR_R.size());
    //LOGE("VTX_NUM_BR: %d\n", vertexCoords.glVertex_BR.size());
#endif

#if 1	//rear_right

    deltaR = 1.0 * Rz0 / p;
    radius = Rz0 + deltaR;

    for (i = 0; i < 1; i += fp)
    {
        radius -= deltaR;
        switch(direction_flag)
        {
        case 0:
            //printf("0 radius = %f\n", radius);
            for (j = 0; j <= 1; j += fp1)
            {
                if((radius - inscribedR > 0) && (radius - inscribedR < deltaR))
                {
                    theta = PI + PIDIV2 + theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if(radius >= inscribedR)
                    {
                        tandivtheta = myTan(mosaicTheta * j);
                        a = 1 + tandivtheta * tandivtheta;
                        b = para_field.car_width + para_field.car_length * tandivtheta;
                        c = para_field.car_length * para_field.car_length / 4 + \
                            para_field.car_width * para_field.car_width / 4 - radius * radius;

                        z = quadraticEquation(a, b, c);
                        x = z * tandivtheta;

                        deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                        theta = PI + PIDIV2 + deltaTheta;
                    }
                    else
                    {
                        theta = PI + PIDIV2 + theta0;
                    }


                    pxv = radius * mySin(theta);
                    pzv = radius * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = -pxv - para_field.car_length / 2; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BR_B.push_back(pt3d_w);
                }

                //printf("000 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_BR_B.push_back(pt3d_w);
                vertexCoords.glVertex_BR.push_back(pt3d_v);

                pxw = pzv - para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BR_R.push_back(pt3d_w);
                }

                objPoints.glObjPoints_BR_R.push_back(pt3d_w);
                blendAlpha.glAlpha_BR.push_back(1.0 - j);

                if((radius - deltaR - inscribedR > 0) && (radius - deltaR - inscribedR < deltaR))
                {
                    theta = PI + PIDIV2 + theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if((radius - deltaR) >= inscribedR)
                    {
                        tandivtheta = myTan(mosaicTheta * j);
                        a = 1 + tandivtheta * tandivtheta;
                        b = para_field.car_width + para_field.car_length * tandivtheta;
                        c = para_field.car_length * para_field.car_length / 4 + \
                            para_field.car_width * para_field.car_width / 4 - (radius - deltaR) * (radius - deltaR);

                        z = quadraticEquation(a, b, c);
                        x = z * tandivtheta;

                        deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                        theta = PI + PIDIV2 + deltaTheta;
                    }
                    else
                    {
                        theta = PI + PIDIV2 + theta0;
                    }



                    pxv = (radius - deltaR) * mySin(theta);
                    pzv = (radius - deltaR) * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = -pxv - para_field.car_length / 2; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BR_B.push_back(pt3d_w);
                }

                //printf("111 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_BR_B.push_back(pt3d_w);
                vertexCoords.glVertex_BR.push_back(pt3d_v);

                pxw = pzv - para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BR_R.push_back(pt3d_w);
                }

                objPoints.glObjPoints_BR_R.push_back(pt3d_w);
                blendAlpha.glAlpha_BR.push_back(1.0 - j);
            }

            break;
        case 1:
            //printf("1 radius = %f\n", radius);
            for( j = 1; j >= 0; j -= fp1 )
            {
                if((radius - inscribedR > 0) && (radius - inscribedR < deltaR))
                {
                    theta = PI + PIDIV2 + theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if(radius >= inscribedR)
                    {
                        tandivtheta = myTan(mosaicTheta * j);
                        a = 1 + tandivtheta * tandivtheta;
                        b = para_field.car_width + para_field.car_length * tandivtheta;
                        c = para_field.car_length * para_field.car_length / 4 + \
                            para_field.car_width * para_field.car_width / 4 - radius * radius;

                        z = quadraticEquation(a, b, c);
                        x = z * tandivtheta;

                        deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                        theta = PI + PIDIV2 + deltaTheta;
                    }
                    else
                    {
                        theta = PI + PIDIV2 + theta0;
                    }


                    pxv = radius * mySin(theta);
                    pzv = radius * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = -pxv - para_field.car_length / 2; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BR_B.push_back(pt3d_w);
                }

                //printf("333 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_BR_B.push_back(pt3d_w);
                vertexCoords.glVertex_BR.push_back(pt3d_v);

                pxw = pzv - para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BR_R.push_back(pt3d_w);
                }

                objPoints.glObjPoints_BR_R.push_back(pt3d_w);
                blendAlpha.glAlpha_BR.push_back(1.0 - j);

                if((radius - deltaR - inscribedR > 0) && (radius - deltaR - inscribedR < deltaR))
                {
                    theta = PI + PIDIV2 + theta0;

                    pxv = inscribedR * mySin(theta);
                    pzv = inscribedR * myCos(theta);
                    pyv = 0.0;

                }
                else
                {
                    if((radius - deltaR) >= inscribedR)
                    {
                        tandivtheta = myTan(mosaicTheta * j);
                        a = 1 + tandivtheta * tandivtheta;
                        b = para_field.car_width + para_field.car_length * tandivtheta;
                        c = para_field.car_length * para_field.car_length / 4 + \
                            para_field.car_width * para_field.car_width / 4 - (radius - deltaR) * (radius - deltaR);

                        z = quadraticEquation(a, b, c);
                        x = z * tandivtheta;

                        deltaTheta = myAtan((z + para_field.car_width / 2) / (x + para_field.car_length / 2));
                        theta = PI + PIDIV2 + deltaTheta;
                    }
                    else
                    {
                        theta = PI + PIDIV2 + theta0;
                    }


                    pxv = (radius - deltaR) * mySin(theta);
                    pzv = (radius - deltaR) * myCos(theta);
                    pyv = 0.0;

                }
                pxw = pzv + para_field.chessboard_length_corners * para_field.square_size / 2;
                pyw = -pxv - para_field.car_length / 2; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;


                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
                pt3d_v = cvPoint3D32f(pxv / R, pyv / h, pzv / R);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BR_B.push_back(pt3d_w);
                }

                //printf("444 %f %f %f\n",pt3d_v.x,pt3d_v.y,pt3d_v.z);
                objPoints.glObjPoints_BR_B.push_back(pt3d_w);
                vertexCoords.glVertex_BR.push_back(pt3d_v);

                pxw = pzv - para_field.car_width / 2;
                pyw = para_field.car_length / 2 - para_field.LRchess2carFront_distance - pxv; // gbb,20140719, chessboard width 5 * 20cm
                pzw = -pyv;

                pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

                if((radius < inscribedR + cutR) && (radius > inscribedR))
                {
                    objPointsStatistics.glObjPoints_BR_R.push_back(pt3d_w);
                }

                objPoints.glObjPoints_BR_R.push_back(pt3d_w);
                blendAlpha.glAlpha_BR.push_back(1.0 - j);
            }
            break;
        }
        // change direction!
        if(j - fp1 == 1 )
        {
            direction_flag = 1;
            //printf("direction_flag = %d\n", direction_flag);
        }
        else
        {
            direction_flag = 0;
            //printf("direction_flag = %d\n", direction_flag);
        }
    }


    texCoords.glTexCoord_BR_B.resize(vertexCoords.glVertex_BR.size());
    texCoords.glTexCoord_BR_R.resize(vertexCoords.glVertex_BR.size());
    texCoordsStatistics.glTexCoord_BR_B.resize(objPointsStatistics.glObjPoints_BR_B.size());
    texCoordsStatistics.glTexCoord_BR_R.resize(objPointsStatistics.glObjPoints_BR_R.size());
    LOGE("VTX_NUM_BR: %d\n", vertexCoords.glVertex_BR.size());
#endif

    glVertices3DCar[0] = para_field.car_length / 2 / R;// p[0].x
    glVertices3DCar[2] = -para_field.car_width / 2 / R;// p[0].z

    glVertices3DCar[3] = para_field.car_length / 2 / R;// p[1].x
    glVertices3DCar[5] = para_field.car_width / 2 / R;// p[1].z

    glVertices3DCar[9] = -para_field.car_length / 2 / R;// p[2].x
    glVertices3DCar[11] = para_field.car_width / 2 / R;// p[2].z

    glVertices3DCar[6] = -para_field.car_length / 2 / R;// p[3].x
    glVertices3DCar[8] = -para_field.car_width / 2 / R;// p[3].z

}

#endif

static float getDistance(float x1, float y1, float x2, float y2, float x, float y)
{
    /*(x,y)??(x1,y1)(x2,y2)??????????*/
    float temp1, temp2;
    temp1 = (y2 - y1) * x + (x1 - x2) * y + (y1 - y2) * x1 - (x1 - x2) * y1;
    temp2 = (float)sqrt((y2 - y1) * (y2 - y1) + (x1 - x2) * (x1 - x2));	
    return (float)(temp1 / temp2);
}

static float getPixelDistance(int x1, int y1, int x2, int y2)
{
    /*(x,y)??(x1,y1)(x2,y2)??????????*/
    float temp1;
    temp1 = (float)sqrt((y2 - y1) * (y2 - y1) + (x1 - x2) * (x1 - x2));
    return temp1;
}

void init2DModel(void)
{
	float pxv, pyv, pzv;
	float pxw, pyw, pzw;
	float pxt, pyt, pzt;
	CvPoint3D32f pt3d_w, pt3d_w0, pt3d_w1;
	CvPoint3D32f pt3d_v;

	int halfWorldWidth, halfWorldHeight, worldWidth, worldHeight;
	float i, j, fp;
	float p = 64.0;
	unsigned char directionFlag = 0;
	float x, y;
	float adjustCoeff;

	CvPoint2D32f boundary[4];
	float dist1, dist2;
	float theta, theta1;
	float weight;
	float Thresh = 0;//sqrt(0);	
	float frontXfuse, frontYfuse, rearXfuse, rearYfuse;
	CvPoint2D32f f1, f2, f3;


    worldWidth = para_field.carWorldX + para_field.car_width + para_field.carWorldX2;
    worldHeight = para_field.carWorldY + para_field.car_length + para_field.carWorldY2;

    halfWorldWidth = worldWidth / 2;
    halfWorldHeight = worldHeight / 2;

	fp = 1.0 / p;
	
#if 1
	for (i = 0; i < 1; i += fp)
	{
		switch(directionFlag)
		{
		case 0:
			for (j = 0; j <= 1; j += fp)
			{
				pxt = j * para_field.car_width + para_field.carWorldX;
				pyt = i * para_field.carWorldY;
					
				pxw = pxt - worldWidth / 2 + para_field.chessboard_length_corners * para_field.square_size / 2;
				pyw = para_field.chessboard_width_corners * para_field.square_size + (pyt - para_field.carWorldY);
				pzw = 0;

				pxv = (pxt - halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - pyt) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_F.push_back(pt3d_w);
				vertexCoords2D.glVertex_F.push_back(pt3d_v);

			    adjustCoeff = 1.0 * (pxt - para_field.carWorldX) / (para_field.car_width);
				lumiaAdjust2D.glLumiaAdjust_F.push_back(adjustCoeff);

				pxt = j * para_field.car_width + para_field.carWorldX;
				pyt = (i + fp) * para_field.carWorldY;
					
				pxw = pxt - worldWidth / 2 + para_field.chessboard_length_corners * para_field.square_size / 2;
				pyw = para_field.chessboard_width_corners * para_field.square_size + (pyt - para_field.carWorldY);
				pzw = 0;

				pxv = (pxt - halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - pyt) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_F.push_back(pt3d_w);
				vertexCoords2D.glVertex_F.push_back(pt3d_v);

			    adjustCoeff = 1.0 * (pxt - para_field.carWorldX) / (para_field.car_width);
				lumiaAdjust2D.glLumiaAdjust_F.push_back(adjustCoeff);
			}
			//objPoints2D.glObjPoints_F.pop_back();
			//vertexCoords2D.glVertex_F.pop_back();
			break;
		case 1:
			for( j = 1; j >= 0; j -= fp )
			{
				pxt = j * para_field.car_width + para_field.carWorldX;
				pyt = i * para_field.carWorldY;
					
				pxw = pxt - worldWidth / 2 + para_field.chessboard_length_corners * para_field.square_size / 2;
				pyw = para_field.chessboard_width_corners * para_field.square_size + (pyt - para_field.carWorldY);
				pzw = 0;

				pxv = (pxt - halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - pyt) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_F.push_back(pt3d_w);
				vertexCoords2D.glVertex_F.push_back(pt3d_v);

				adjustCoeff = 1.0 * (pxt - para_field.carWorldX) / (para_field.car_width);
				lumiaAdjust2D.glLumiaAdjust_F.push_back(adjustCoeff);

				pxt = j * para_field.car_width + para_field.carWorldX;
				pyt = (i + fp) * para_field.carWorldY;
					
				pxw = pxt - worldWidth / 2 + para_field.chessboard_length_corners * para_field.square_size / 2;
				pyw = para_field.chessboard_width_corners * para_field.square_size + (pyt - para_field.carWorldY);
				pzw = 0;

				pxv = (pxt - halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - pyt) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_F.push_back(pt3d_w);
				vertexCoords2D.glVertex_F.push_back(pt3d_v);

				adjustCoeff = 1.0 * (pxt - para_field.carWorldX) / (para_field.car_width);
				lumiaAdjust2D.glLumiaAdjust_F.push_back(adjustCoeff);
			}
			//objPoints.glObjPoints_F.pop_back();
			//vertexCoords.glVertex_F.pop_back();
			break;
		}
		// change direction!
		if(j - fp == 1 )
		{
			directionFlag = 1;
		}
		else
		{
			directionFlag = 0;
		}
	}
	printf("VTX_NUM_F: %lu\n", objPoints2D.glObjPoints_F.size());
	texCoords2D.glTexCoord_F.resize(objPoints2D.glObjPoints_F.size());
#endif

	directionFlag = 0;

#if 1
	for (i = 0; i < 1; i += fp)
	{
		switch(directionFlag)
		{
		case 0:
			for (j = 0; j <= 1; j += fp)
			{
				pxt = j * para_field.car_width + para_field.carWorldX;
				pyt = i * para_field.carWorldY2;
					
				pxw = pxt - halfWorldWidth + para_field.chessboard_length_corners * para_field.square_size / 2;
				pyw = pyt;
				pzw = 0;

				pxv = (pxt - halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - (pyt + para_field.carWorldY + para_field.car_length)) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_B.push_back(pt3d_w);
				vertexCoords2D.glVertex_B.push_back(pt3d_v);

				adjustCoeff = 1.0 * (pxt - para_field.carWorldX) / (para_field.car_width);
				lumiaAdjust2D.glLumiaAdjust_B.push_back(adjustCoeff);

				pxt = j * para_field.car_width + para_field.carWorldX;
				pyt = (i + fp) * para_field.carWorldY2;
					
				pxw = pxt - halfWorldWidth + para_field.chessboard_length_corners * para_field.square_size / 2;
				pyw = pyt;
				pzw = 0;

				pxv = (pxt - halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - (pyt + para_field.carWorldY + para_field.car_length)) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_B.push_back(pt3d_w);
				vertexCoords2D.glVertex_B.push_back(pt3d_v);
				
				adjustCoeff = 1.0 * (pxt - para_field.carWorldX) / (para_field.car_width);
				lumiaAdjust2D.glLumiaAdjust_B.push_back(adjustCoeff);
			}
			//objPoints2D.glObjPoints_F.pop_back();
			//vertexCoords2D.glVertex_F.pop_back();
			break;
		case 1:
			for( j = 1; j >= 0; j -= fp )
			{
				pxt = j * para_field.car_width + para_field.carWorldX;
				pyt = i * para_field.carWorldY2;
					
				pxw = pxt - halfWorldWidth + para_field.chessboard_length_corners * para_field.square_size / 2;
				pyw = pyt;
				pzw = 0;

				pxv = (pxt - halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - (pyt + para_field.carWorldY + para_field.car_length)) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_B.push_back(pt3d_w);
				vertexCoords2D.glVertex_B.push_back(pt3d_v);
				
				adjustCoeff = 1.0 * (pxt - para_field.carWorldX) / (para_field.car_width);
				lumiaAdjust2D.glLumiaAdjust_B.push_back(adjustCoeff);

				pxt = j * para_field.car_width + para_field.carWorldX;
				pyt = (i + fp) * para_field.carWorldY2;
					
				pxw = pxt - halfWorldWidth + para_field.chessboard_length_corners * para_field.square_size / 2;
				pyw = pyt;
				pzw = 0;

				pxv = (pxt - halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - (pyt + para_field.carWorldY + para_field.car_length)) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_B.push_back(pt3d_w);
				vertexCoords2D.glVertex_B.push_back(pt3d_v);

				adjustCoeff = 1.0 * (pxt - para_field.carWorldX) / (para_field.car_width);
				lumiaAdjust2D.glLumiaAdjust_B.push_back(adjustCoeff);
			}
			//objPoints.glObjPoints_F.pop_back();
			//vertexCoords.glVertex_F.pop_back();
			break;
		}
		// change direction!
		if(j - fp == 1 )
		{
			directionFlag = 1;
		}
		else
		{
			directionFlag = 0;
		}
	}
	printf("VTX_NUM_B: %lu\n", objPoints2D.glObjPoints_B.size());
	texCoords2D.glTexCoord_B.resize(objPoints2D.glObjPoints_B.size());
#endif

#if 1
	for (i = 0; i < 1; i += fp)
	{
		switch(directionFlag)
		{
		case 0:
			for (j = 0; j <= 1; j += fp)
			{
				pxt = j * para_field.carWorldX;
				pyt = i * para_field.car_length;
					
				pxw = pxt - para_field.carWorldX + para_field.chessboard_width_corners * para_field.square_size;
				pyw = pyt - para_field.LRchess2carFront_distance;
				pzw = 0;

				pxv = (pxt - halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - (pyt + para_field.carWorldY)) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_L.push_back(pt3d_w);
				vertexCoords2D.glVertex_L.push_back(pt3d_v);

                adjustCoeff = 1.0 * pyt / para_field.car_length;
				lumiaAdjust2D.glLumiaAdjust_L.push_back(adjustCoeff);


				pxt = j * para_field.carWorldX;
				pyt = (i + fp) * para_field.car_length;
					
				pxw = pxt - para_field.carWorldX + para_field.chessboard_width_corners * para_field.square_size;
				pyw = pyt - para_field.LRchess2carFront_distance;
				pzw = 0;

				pxv = (pxt - halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - (pyt + para_field.carWorldY)) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_L.push_back(pt3d_w);
				vertexCoords2D.glVertex_L.push_back(pt3d_v);

				adjustCoeff = 1.0 * pyt / para_field.car_length;

				lumiaAdjust2D.glLumiaAdjust_L.push_back(adjustCoeff);
			}
			//objPoints2D.glObjPoints_F.pop_back();
			//vertexCoords2D.glVertex_F.pop_back();
			break;
		case 1:
			for( j = 1; j >= 0; j -= fp )
			{
				pxt = j * para_field.carWorldX;
				pyt = i * para_field.car_length;
					
				pxw = pxt - para_field.carWorldX + para_field.chessboard_width_corners * para_field.square_size;
				pyw = pyt - para_field.LRchess2carFront_distance;
				pzw = 0;

				pxv = (pxt - halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - (pyt + para_field.carWorldY)) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_L.push_back(pt3d_w);
				vertexCoords2D.glVertex_L.push_back(pt3d_v);

				adjustCoeff = 1.0 * pyt / para_field.car_length;

				lumiaAdjust2D.glLumiaAdjust_L.push_back(adjustCoeff);
				
				pxt = j * para_field.carWorldX;
				pyt = (i + fp) * para_field.car_length;
					
				pxw = pxt - para_field.carWorldX + para_field.chessboard_width_corners * para_field.square_size;
				pyw = pyt - para_field.LRchess2carFront_distance;
				pzw = 0;

				pxv = (pxt - halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - (pyt + para_field.carWorldY)) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_L.push_back(pt3d_w);
				vertexCoords2D.glVertex_L.push_back(pt3d_v);

				adjustCoeff = 1.0 * pyt / para_field.car_length;

				lumiaAdjust2D.glLumiaAdjust_L.push_back(adjustCoeff);
			}
			//objPoints.glObjPoints_F.pop_back();
			//vertexCoords.glVertex_F.pop_back();
			break;
		}
		// change direction!
		if(j - fp == 1 )
		{
			directionFlag = 1;
		}
		else
		{
			directionFlag = 0;
		}
	}
	printf("VTX_NUM_L: %lu\n", objPoints2D.glObjPoints_L.size());
	texCoords2D.glTexCoord_L.resize(objPoints2D.glObjPoints_L.size());
#endif


#if 1
	for (i = 0; i < 1; i += fp)
	{
		switch(directionFlag)
		{
		case 0:
			for (j = 0; j <= 1; j += fp)
			{
				pxt = j * para_field.carWorldX2;
				pyt = i * para_field.car_length;
					
				pxw = pxt;
				pyw = pyt - para_field.LRchess2carFront_distance;
				pzw = 0;

				pxv = (para_field.carWorldX + para_field.car_width + pxt - halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - (pyt + para_field.carWorldY)) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_R.push_back(pt3d_w);
				vertexCoords2D.glVertex_R.push_back(pt3d_v);

				adjustCoeff = 1.0 * pyt / para_field.car_length;

				lumiaAdjust2D.glLumiaAdjust_R.push_back(adjustCoeff);

				pxt = j * para_field.carWorldX2;
				pyt = (i + fp) * para_field.car_length;
					
				pxw = pxt;
				pyw = pyt - para_field.LRchess2carFront_distance;
				pzw = 0;

				pxv = (para_field.carWorldX + para_field.car_width + pxt - halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - (pyt + para_field.carWorldY)) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_R.push_back(pt3d_w);
				vertexCoords2D.glVertex_R.push_back(pt3d_v);

				adjustCoeff = 1.0 * pyt / para_field.car_length;

				lumiaAdjust2D.glLumiaAdjust_R.push_back(adjustCoeff);
			}
			//objPoints2D.glObjPoints_F.pop_back();
			//vertexCoords2D.glVertex_F.pop_back();
			break;
		case 1:
			for( j = 1; j >= 0; j -= fp )
			{
				pxt = j * para_field.carWorldX2;
				pyt = i * para_field.car_length;
					
				pxw = pxt;
				pyw = pyt - para_field.LRchess2carFront_distance;
				pzw = 0;

				pxv = (para_field.carWorldX + para_field.car_width + pxt - halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - (pyt + para_field.carWorldY)) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_R.push_back(pt3d_w);
				vertexCoords2D.glVertex_R.push_back(pt3d_v);

				adjustCoeff = 1.0 * pyt / para_field.car_length;

				lumiaAdjust2D.glLumiaAdjust_R.push_back(adjustCoeff);

				pxt = j * para_field.carWorldX2;
				pyt = (i + fp) * para_field.car_length;
					
				pxw = pxt;
				pyw = pyt - para_field.LRchess2carFront_distance;
				pzw = 0;

				pxv = (para_field.carWorldX + para_field.car_width + pxt - halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - (pyt + para_field.carWorldY)) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_R.push_back(pt3d_w);
				vertexCoords2D.glVertex_R.push_back(pt3d_v);

				adjustCoeff = 1.0 * pyt / para_field.car_length;

				lumiaAdjust2D.glLumiaAdjust_R.push_back(adjustCoeff);
			}
			//objPoints.glObjPoints_F.pop_back();
			//vertexCoords.glVertex_F.pop_back();
			break;
		}
		// change direction!
		if(j - fp == 1 )
		{
			directionFlag = 1;
		}
		else
		{
			directionFlag = 0;
		}
	}
	printf("VTX_NUM_R: %lu\n", objPoints2D.glObjPoints_R.size());
	texCoords2D.glTexCoord_R.resize(objPoints2D.glObjPoints_R.size());
#endif


#if 1
	frontXfuse = para_field.carWorldX * 0.5;
	frontYfuse = para_field.carWorldY * 0.5;

	f1.x = para_field.carWorldX;
	f1.y = para_field.carWorldY;

	f2.x = 0;
	f2.y = para_field.carWorldY - frontYfuse;

	f3.x = para_field.carWorldX - frontXfuse;
	f3.y = 0;

	theta = atan((float)(f1.y - f3.y) / (f1.x - f3.x)) - atan((float)(f1.y - f2.y) / (f1.x - f2.x));

	for (i = 0; i <= 1; i += fp)
	{
		switch(directionFlag)
		{
		case 0:
			for (j = 0; j <= 1; j += fp)
			{
				pxt = j * para_field.carWorldX;
				pyt = i * para_field.carWorldY;

				pxw = pxt - worldWidth / 2 + para_field.chessboard_length_corners * para_field.square_size / 2;
				pyw = para_field.chessboard_width_corners * para_field.square_size + (pyt - para_field.carWorldY);
				pzw = 0;

				pxv = (pxt- halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - pyt) / halfWorldHeight;
				pzv = 0;

				pt3d_w0 = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_FL_F.push_back(pt3d_w0);
				vertexCoords2D.glVertex_FL.push_back(pt3d_v);

				pxw = pxt - para_field.carWorldX + para_field.chessboard_width_corners * para_field.square_size;
				pyw = pyt - para_field.carWorldY - para_field.LRchess2carFront_distance;
				pzw = 0;

				pt3d_w1 = cvPoint3D32f(pxw, pyw, pzw);
				
				objPoints2D.glObjPoints_FL_L.push_back(pt3d_w1);

				if(getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) < 0)
			   	{
				   	blendAlpha2D.glAlpha_FL.push_back(0.0);
			   	}
			   	else if(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) > 0)
			   	{
				   	blendAlpha2D.glAlpha_FL.push_back(1.0);
			   	}
			   	else
			   	{
				   	dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
				   	dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);
	
				   	theta1 = asin(dist1 / dist2);
	
				   	blendAlpha2D.glAlpha_FL.push_back(1.0 - theta1 / theta);

				   	if((pxt > para_field.carWorldX / 2) && (pyt > para_field.carWorldY / 2))
	            	{
	                	objPointsStatistics2D.glObjPoints_FL_F.push_back(pt3d_w0);
	                	objPointsStatistics2D.glObjPoints_FL_L.push_back(pt3d_w1);
	            	}
	
			   	}


				pxt = j * para_field.carWorldX;
				pyt = (i + fp) * para_field.carWorldY;

				pxw = pxt - worldWidth / 2 + para_field.chessboard_length_corners * para_field.square_size / 2;
				pyw = para_field.chessboard_width_corners * para_field.square_size + (pyt - para_field.carWorldY);
				pzw = 0;

				pxv = (pxt- halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - pyt) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_FL_F.push_back(pt3d_w);
				vertexCoords2D.glVertex_FL.push_back(pt3d_v);

				pxw = pxt - para_field.carWorldX + para_field.chessboard_width_corners * para_field.square_size;
				pyw = pyt - para_field.carWorldY - para_field.LRchess2carFront_distance;
				pzw = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

				objPoints2D.glObjPoints_FL_L.push_back(pt3d_w);

				if(getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) < 0)
			   	{
				   blendAlpha2D.glAlpha_FL.push_back(0.0);
			   	}
			   	else if(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) > 0)
			   	{
				   blendAlpha2D.glAlpha_FL.push_back(1.0);
			   	}
			   	else
			   	{
				   dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
				   dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);
	
				   theta1 = asin(dist1 / dist2);
	
				   blendAlpha2D.glAlpha_FL.push_back(1.0 - theta1 / theta);
	
			   	}
			}
			//objPoints2D.glObjPoints_F.pop_back();
			//vertexCoords2D.glVertex_F.pop_back();
			break;
		case 1:
			for( j = 1; j >= 0; j -= fp )
			{
				pxt = j * para_field.carWorldX;
				pyt = i * para_field.carWorldY;

				pxw = pxt - worldWidth / 2 + para_field.chessboard_length_corners * para_field.square_size / 2;
				pyw = para_field.chessboard_width_corners * para_field.square_size + (pyt - para_field.carWorldY);
				pzw = 0;

				pxv = (pxt- halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - pyt) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_FL_F.push_back(pt3d_w);
				vertexCoords2D.glVertex_FL.push_back(pt3d_v);

				if((pxt > para_field.carWorldX / 2) && (pyt > para_field.carWorldY / 2))
            	{
                	objPointsStatistics.glObjPoints_FL_F.push_back(pt3d_w);
            	}

				pxw = pxt - para_field.carWorldX + para_field.chessboard_width_corners * para_field.square_size;
				pyw = pyt - para_field.carWorldY - para_field.LRchess2carFront_distance;
				pzw = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

				objPoints2D.glObjPoints_FL_L.push_back(pt3d_w);

				if(getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) < 0)
			   	{
				   blendAlpha2D.glAlpha_FL.push_back(0.0);
			   	}
			   	else if(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) > 0)
			   	{
				   blendAlpha2D.glAlpha_FL.push_back(1.0);
			   	}
			   	else
			   	{
				   dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
				   dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);
	
				   theta1 = asin(dist1 / dist2);
	
				   blendAlpha2D.glAlpha_FL.push_back(1.0 - theta1 / theta);
	
			   	}


				pxt = j * para_field.carWorldX;
				pyt = (i + fp) * para_field.carWorldY;				

				pxw = pxt - worldWidth / 2 + para_field.chessboard_length_corners * para_field.square_size / 2;
				pyw = para_field.chessboard_width_corners * para_field.square_size + (pyt - para_field.carWorldY);
				pzw = 0;

				pxv = (pxt- halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - pyt) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_FL_F.push_back(pt3d_w);
				vertexCoords2D.glVertex_FL.push_back(pt3d_v);

				pxw = pxt - para_field.carWorldX + para_field.chessboard_width_corners * para_field.square_size;
				pyw = pyt - para_field.carWorldY - para_field.LRchess2carFront_distance;
				pzw = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

				objPoints2D.glObjPoints_FL_L.push_back(pt3d_w);

				if(getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) < 0)
			   	{
				   blendAlpha2D.glAlpha_FL.push_back(0.0);
			   	}
			   	else if(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) > 0)
			   	{
				   blendAlpha2D.glAlpha_FL.push_back(1.0);
			   	}
			   	else
			   	{
				   dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
				   dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);
	
				   theta1 = asin(dist1 / dist2);
	
				   blendAlpha2D.glAlpha_FL.push_back(1.0 - theta1 / theta);
	
			   	}
			}
			//objPoints.glObjPoints_F.pop_back();
			//vertexCoords.glVertex_F.pop_back();
			break;
		}
		// change direction!
		if(j - fp == 1 )
		{
			directionFlag = 1;
		}
		else
		{
			directionFlag = 0;
		}
	}
	printf("VTX_NUM_FL: %lu %lu\n", objPoints2D.glObjPoints_FL_F.size(), objPoints2D.glObjPoints_FL_L.size());
	texCoords2D.glTexCoord_FL_F.resize(objPoints2D.glObjPoints_FL_F.size());
	texCoords2D.glTexCoord_FL_L.resize(objPoints2D.glObjPoints_FL_L.size());
	texCoordsStatistics2D.glTexCoord_FL_F.resize(objPointsStatistics2D.glObjPoints_FL_F.size());
	texCoordsStatistics2D.glTexCoord_FL_L.resize(objPointsStatistics2D.glObjPoints_FL_L.size());

#endif

#if 1
    f1.x = para_field.carWorldX + para_field.car_width;
    f1.y = para_field.carWorldY;

    f2.x = worldWidth;
    f2.y = para_field.carWorldY - frontYfuse;

    f3.x = para_field.carWorldX + para_field.car_width + frontXfuse;
    f3.y = 0;

    theta = atan((float)(f1.y - f3.y) / (f3.x - f1.x)) - atan((float)(f1.y - f2.y) / (f2.x - f1.x));

	printf("FR theta = %f\n",theta);

	for (i = 0; i <= 1; i += fp)
	{
		switch(directionFlag)
		{
		case 0:
			for (j = 0; j <= 1; j += fp)
			{
				pxt = para_field.carWorldX + para_field.car_width + j * para_field.carWorldX2;
				pyt = i * para_field.carWorldY;

				pxw = pxt - worldWidth / 2 + para_field.chessboard_length_corners * para_field.square_size / 2;
				pyw = para_field.chessboard_width_corners * para_field.square_size + (pyt - para_field.carWorldY);
				pzw = 0;

				pxv = (pxt- halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - pyt) / halfWorldHeight;
				pzv = 0;

				pt3d_w0 = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_FR_F.push_back(pt3d_w0);
				vertexCoords2D.glVertex_FR.push_back(pt3d_v);


				pxw = pxt - para_field.carWorldX - para_field.car_width;
				pyw = pyt - para_field.carWorldY - para_field.LRchess2carFront_distance;
				pzw = 0;

				pt3d_w1 = cvPoint3D32f(pxw, pyw, pzw);
				
				objPoints2D.glObjPoints_FR_R.push_back(pt3d_w1);

				if(getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) > 0)
	            {
	                blendAlpha2D.glAlpha_FR.push_back(0.0);
	            }
	            else if(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) < 0)
	            {
	                blendAlpha2D.glAlpha_FR.push_back(1.0);
	            }
	            else
	            {
	                dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
	                dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

	                theta1 = asin(dist1 / dist2);

	                blendAlpha2D.glAlpha_FR.push_back(1.0 - theta1 / theta);

	  				if((pxt < para_field.carWorldX + para_field.car_width + para_field.carWorldX2 / 2) && (pyt > para_field.carWorldY / 2))
					{
						objPointsStatistics2D.glObjPoints_FR_F.push_back(pt3d_w0);
						objPointsStatistics2D.glObjPoints_FR_R.push_back(pt3d_w1);
					}
	            }

				pxt = para_field.carWorldX + para_field.car_width + j * para_field.carWorldX2;
				pyt = (i + fp) * para_field.carWorldY;

				pxw = pxt - worldWidth / 2 + para_field.chessboard_length_corners * para_field.square_size / 2;
				pyw = para_field.chessboard_width_corners * para_field.square_size + (pyt - para_field.carWorldY);
				pzw = 0;

				pxv = (pxt- halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - pyt) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_FR_F.push_back(pt3d_w);
				vertexCoords2D.glVertex_FR.push_back(pt3d_v);

				pxw = pxt - para_field.carWorldX - para_field.car_width;
				pyw = pyt - para_field.carWorldY - para_field.LRchess2carFront_distance;
				pzw = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

				objPoints2D.glObjPoints_FR_R.push_back(pt3d_w);

				if(getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) > 0)
	            {
	                blendAlpha2D.glAlpha_FR.push_back(0.0);
	            }
	            else if(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) < 0)
	            {
	                blendAlpha2D.glAlpha_FR.push_back(1.0);
	            }
	            else
	            {
	                dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
	                dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

	                theta1 = asin(dist1 / dist2);

	                blendAlpha2D.glAlpha_FR.push_back(1.0 - theta1 / theta);
	            }
			}
			//objPoints2D.glObjPoints_F.pop_back();
			//vertexCoords2D.glVertex_F.pop_back();
			break;
		case 1:
			for( j = 1; j >= 0; j -= fp )
			{
				pxt = para_field.carWorldX + para_field.car_width + j * para_field.carWorldX2;
				pyt = i * para_field.carWorldY;

				pxw = pxt - worldWidth / 2 + para_field.chessboard_length_corners * para_field.square_size / 2;
				pyw = para_field.chessboard_width_corners * para_field.square_size + (pyt - para_field.carWorldY);
				pzw = 0;

				pxv = (pxt- halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - pyt) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_FR_F.push_back(pt3d_w);
				vertexCoords2D.glVertex_FR.push_back(pt3d_v);

				pxw = pxt - para_field.carWorldX - para_field.car_width;
				pyw = pyt - para_field.carWorldY - para_field.LRchess2carFront_distance;
				pzw = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

				objPoints2D.glObjPoints_FR_R.push_back(pt3d_w);

				if(getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) > 0)
	            {
	                blendAlpha2D.glAlpha_FR.push_back(0.0);
	            }
	            else if(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) < 0)
	            {
	                blendAlpha2D.glAlpha_FR.push_back(1.0);
	            }
	            else
	            {
	                dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
	                dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

	                theta1 = asin(dist1 / dist2);

	                blendAlpha2D.glAlpha_FR.push_back(1.0 - theta1 / theta);
	            }

				pxt = para_field.carWorldX + para_field.car_width + j * para_field.carWorldX2;
				pyt = (i + fp) * para_field.carWorldY;				

				pxw = pxt - worldWidth / 2 + para_field.chessboard_length_corners * para_field.square_size / 2;
				pyw = para_field.chessboard_width_corners * para_field.square_size + (pyt - para_field.carWorldY);
				pzw = 0;

				pxv = (pxt- halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - pyt) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_FR_F.push_back(pt3d_w);
				vertexCoords2D.glVertex_FR.push_back(pt3d_v);

				pxw = pxt - para_field.carWorldX - para_field.car_width;
				pyw = pyt - para_field.carWorldY - para_field.LRchess2carFront_distance;
				pzw = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

				objPoints2D.glObjPoints_FR_R.push_back(pt3d_w);

				if(getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) > 0)
	            {
	                blendAlpha2D.glAlpha_FR.push_back(0.0);
	            }
	            else if(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) < 0)
	            {
	                blendAlpha2D.glAlpha_FR.push_back(1.0);
	            }
	            else
	            {
	                dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
	                dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

	                theta1 = asin(dist1 / dist2);

	                blendAlpha2D.glAlpha_FR.push_back(1.0 - theta1 / theta);
	            }
			}
			//objPoints.glObjPoints_F.pop_back();
			//vertexCoords.glVertex_F.pop_back();
			break;
		}
		// change direction!
		if(j - fp == 1 )
		{
			directionFlag = 1;
		}
		else
		{
			directionFlag = 0;
		}
	}
	printf("VTX_NUM_FR: %lu %lu\n", objPoints2D.glObjPoints_FR_F.size(), objPoints2D.glObjPoints_FR_R.size());
	texCoords2D.glTexCoord_FR_F.resize(objPoints2D.glObjPoints_FR_F.size());
	texCoords2D.glTexCoord_FR_R.resize(objPoints2D.glObjPoints_FR_R.size());
	texCoordsStatistics2D.glTexCoord_FR_F.resize(objPointsStatistics2D.glObjPoints_FR_F.size());
	texCoordsStatistics2D.glTexCoord_FR_R.resize(objPointsStatistics2D.glObjPoints_FR_R.size());
#endif


#if 1
    rearXfuse = para_field.carWorldX * 0.8;
    rearYfuse = (worldHeight - para_field.carWorldY - para_field.car_length) * 0.2;

    f1.x = para_field.carWorldX;
    f1.y = para_field.carWorldY + para_field.car_length;

    f2.x = 0;
    f2.y = para_field.carWorldY + para_field.car_length + rearYfuse;

    f3.x = para_field.carWorldX - rearXfuse;
    f3.y = worldHeight;

    theta = atan((float)((f3.y - f1.y)) / (f1.x - f3.x)) - atan((float)((f2.y - f1.y)) / (f1.x - f2.x));

	printf("BL theta = %f\n",theta);

	for (i = 0; i <= 1; i += fp)
	{
		switch(directionFlag)
		{
		case 0:
			for (j = 0; j <= 1; j += fp)
			{
				pxt = j * para_field.carWorldX;
				pyt = para_field.carWorldY + para_field.car_length + i * para_field.carWorldY2;

				pxw = pxt - halfWorldWidth + para_field.chessboard_length_corners * para_field.square_size / 2;
				pyw = pyt - para_field.carWorldY - para_field.car_length;
				pzw = 0;

				pxv = (pxt- halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - pyt) / halfWorldHeight;
				pzv = 0;

				pt3d_w0 = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_BL_B.push_back(pt3d_w0);
				vertexCoords2D.glVertex_BL.push_back(pt3d_v);

				pxw = pxt - para_field.carWorldX + para_field.chessboard_width_corners * para_field.square_size;
				pyw = pyt - para_field.carWorldY - para_field.LRchess2carFront_distance;
				pzw = 0;

				pt3d_w1 = cvPoint3D32f(pxw, pyw, pzw);
				
				objPoints2D.glObjPoints_BL_L.push_back(pt3d_w1);

				if(getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) > 0)
	            {
	                blendAlpha2D.glAlpha_BL.push_back(0.0);
	            }
	            else if(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) < 0)
	            {
	                blendAlpha2D.glAlpha_BL.push_back(1.0);
	            }
	            else
	            {
	                dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
	                dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

	                theta1 = asin(dist1 / dist2);

	                weight = theta1 / theta;
	                blendAlpha2D.glAlpha_BL.push_back(1.0 - theta1 / theta);

	   				if((pxt > para_field.carWorldX / 2) && (pyt < para_field.carWorldY + para_field.car_length + para_field.carWorldY2 / 2))
					{
						objPointsStatistics2D.glObjPoints_BL_B.push_back(pt3d_w0);
						objPointsStatistics2D.glObjPoints_BL_L.push_back(pt3d_w1);
					}
	            }


				pxt = j * para_field.carWorldX;
				pyt = para_field.carWorldY + para_field.car_length + (i + fp) * para_field.carWorldY2;

				pxw = pxt - halfWorldWidth + para_field.chessboard_length_corners * para_field.square_size / 2;
				pyw = pyt - para_field.carWorldY - para_field.car_length;
				pzw = 0;

				pxv = (pxt- halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - pyt) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_BL_B.push_back(pt3d_w);
				vertexCoords2D.glVertex_BL.push_back(pt3d_v);

				pxw = pxt - para_field.carWorldX + para_field.chessboard_width_corners * para_field.square_size;
				pyw = pyt - para_field.carWorldY - para_field.LRchess2carFront_distance;
				pzw = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

				objPoints2D.glObjPoints_BL_L.push_back(pt3d_w);

				if(getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) > 0)
	            {
	                blendAlpha2D.glAlpha_BL.push_back(0.0);
	            }
	            else if(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) < 0)
	            {
	                blendAlpha2D.glAlpha_BL.push_back(1.0);
	            }
	            else
	            {
	                dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
	                dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

	                theta1 = asin(dist1 / dist2);

	                weight = theta1 / theta;
	                blendAlpha2D.glAlpha_BL.push_back(1.0 - theta1 / theta);
	            }
			}
			//objPoints2D.glObjPoints_F.pop_back();
			//vertexCoords2D.glVertex_F.pop_back();
			break;
		case 1:
			for( j = 1; j >= 0; j -= fp )
			{
				pxt = j * para_field.carWorldX;
				pyt = para_field.carWorldY + para_field.car_length + i * para_field.carWorldY2;

				pxw = pxt - halfWorldWidth + para_field.chessboard_length_corners * para_field.square_size / 2;
				pyw = pyt - para_field.carWorldY - para_field.car_length;
				pzw = 0;

				pxv = (pxt- halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - pyt) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_BL_B.push_back(pt3d_w);
				vertexCoords2D.glVertex_BL.push_back(pt3d_v);

				pxw = pxt - para_field.carWorldX + para_field.chessboard_width_corners * para_field.square_size;
				pyw = pyt - para_field.carWorldY - para_field.LRchess2carFront_distance;
				pzw = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

				objPoints2D.glObjPoints_BL_L.push_back(pt3d_w);

				if(getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) > 0)
	            {
	                blendAlpha2D.glAlpha_BL.push_back(0.0);
	            }
	            else if(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) < 0)
	            {
	                blendAlpha2D.glAlpha_BL.push_back(1.0);
	            }
	            else
	            {
	                dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
	                dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

	                theta1 = asin(dist1 / dist2);

	                weight = theta1 / theta;
	                blendAlpha2D.glAlpha_BL.push_back(1.0 - theta1 / theta);
	            }

				pxt = j * para_field.carWorldX;
				pyt = para_field.carWorldY + para_field.car_length + (i + fp) * para_field.carWorldY2;				

				pxw = pxt - halfWorldWidth + para_field.chessboard_length_corners * para_field.square_size / 2;
				pyw = pyt - para_field.carWorldY - para_field.car_length;
				pzw = 0;

				pxv = (pxt- halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - pyt) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_BL_B.push_back(pt3d_w);
				vertexCoords2D.glVertex_BL.push_back(pt3d_v);

				pxw = pxt - para_field.carWorldX + para_field.chessboard_width_corners * para_field.square_size;
				pyw = pyt - para_field.carWorldY - para_field.LRchess2carFront_distance;
				pzw = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

				objPoints2D.glObjPoints_BL_L.push_back(pt3d_w);
				
				if(getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) > 0)
	            {
	                blendAlpha2D.glAlpha_BL.push_back(0.0);
	            }
	            else if(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) < 0)
	            {
	                blendAlpha2D.glAlpha_BL.push_back(1.0);
	            }
	            else
	            {
	                dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
	                dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

	                theta1 = asin(dist1 / dist2);

	                weight = theta1 / theta;
	                blendAlpha2D.glAlpha_BL.push_back(1.0 - theta1 / theta);
	            }
			}
			//objPoints.glObjPoints_F.pop_back();
			//vertexCoords.glVertex_F.pop_back();
			break;
		}
		// change direction!
		if(j - fp == 1 )
		{
			directionFlag = 1;
		}
		else
		{
			directionFlag = 0;
		}
	}
	printf("VTX_NUM_BL: %lu %lu\n", objPoints2D.glObjPoints_BL_B.size(), objPoints2D.glObjPoints_BL_L.size());
	texCoords2D.glTexCoord_BL_B.resize(objPoints2D.glObjPoints_BL_B.size());
	texCoords2D.glTexCoord_BL_L.resize(objPoints2D.glObjPoints_BL_L.size());
	texCoordsStatistics2D.glTexCoord_BL_B.resize(objPointsStatistics2D.glObjPoints_BL_B.size());
	texCoordsStatistics2D.glTexCoord_BL_L.resize(objPointsStatistics2D.glObjPoints_BL_L.size());
#endif

#if 1	
	f1.x = para_field.carWorldX + para_field.car_width;
	f1.y = para_field.carWorldY + para_field.car_length;

	f2.x = worldHeight;
	f2.y = para_field.carWorldY + para_field.car_length + rearYfuse;

	f3.x = para_field.carWorldX + para_field.car_width + rearXfuse;
	f3.y = worldHeight;

	theta = atan((float)((f3.y - f1.y)) / (f3.x - f1.x)) - atan((float)((f2.y - f1.y)) / (f2.x - f1.x));

	for (i = 0; i <= 1; i += fp)
	{
		switch(directionFlag)
		{
		case 0:
			for (j = 0; j <= 1; j += fp)
			{
				pxt = para_field.carWorldX + para_field.car_width + j * para_field.carWorldX2;
				pyt = para_field.carWorldY + para_field.car_length + i * para_field.carWorldY2;

				pxw = pxt - halfWorldWidth + para_field.chessboard_length_corners * para_field.square_size / 2;
				pyw = pyt - para_field.carWorldY - para_field.car_length;
				pzw = 0;

				pxv = (pxt- halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - pyt) / halfWorldHeight;
				pzv = 0;

				pt3d_w0 = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_BR_B.push_back(pt3d_w0);
				vertexCoords2D.glVertex_BR.push_back(pt3d_v);

				pxw = pxt - para_field.carWorldX - para_field.car_width;
				pyw = pyt - para_field.carWorldY - para_field.LRchess2carFront_distance;
				pzw = 0;

				pt3d_w1 = cvPoint3D32f(pxw, pyw, pzw);
				
				objPoints2D.glObjPoints_BR_R.push_back(pt3d_w1);

				if(getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) < 0)
	            {
	                blendAlpha2D.glAlpha_BR.push_back(0.0);
	            }
	            else if(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) > 0)
	            {
	                blendAlpha2D.glAlpha_BR.push_back(1.0);
	            }
	            else
	            {
	                dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
	                dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

	                theta1 = asin(dist1 / dist2);

	                blendAlpha2D.glAlpha_BR.push_back(1.0 - theta1 / theta);

	                if((pxt < para_field.carWorldX + para_field.car_width + para_field.carWorldX2 / 2) && (pyt < para_field.carWorldY + para_field.car_length + para_field.carWorldY2 / 2))
					{
						objPointsStatistics2D.glObjPoints_BR_B.push_back(pt3d_w0);
						objPointsStatistics2D.glObjPoints_BR_R.push_back(pt3d_w1);
					}
	            }
	            

				pxt = para_field.carWorldX + para_field.car_width + j * para_field.carWorldX2;
				pyt = para_field.carWorldY + para_field.car_length + (i + fp) * para_field.carWorldY2;

				pxw = pxt - halfWorldWidth + para_field.chessboard_length_corners * para_field.square_size / 2;
				pyw = pyt - para_field.carWorldY - para_field.car_length;
				pzw = 0;

				pxv = (pxt- halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - pyt) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_BR_B.push_back(pt3d_w);
				vertexCoords2D.glVertex_BR.push_back(pt3d_v);

				pxw = pxt - para_field.carWorldX - para_field.car_width;
				pyw = pyt - para_field.carWorldY - para_field.LRchess2carFront_distance;
				pzw = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

				objPoints2D.glObjPoints_BR_R.push_back(pt3d_w);

				if(getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) < 0)
	            {
	                blendAlpha2D.glAlpha_BR.push_back(0.0);
	            }
	            else if(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) > 0)
	            {
	                blendAlpha2D.glAlpha_BR.push_back(1.0);
	            }
	            else
	            {
	                dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
	                dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

	                theta1 = asin(dist1 / dist2);

	                blendAlpha2D.glAlpha_BR.push_back(1.0 - theta1 / theta);
	            }
			}
			//objPoints2D.glObjPoints_F.pop_back();
			//vertexCoords2D.glVertex_F.pop_back();
			break;
		case 1:
			for( j = 1; j >= 0; j -= fp )
			{
				pxt = para_field.carWorldX + para_field.car_width + j * para_field.carWorldX2;
				pyt = para_field.carWorldY + para_field.car_length + i * para_field.carWorldY2;

				pxw = pxt - halfWorldWidth + para_field.chessboard_length_corners * para_field.square_size / 2;
				pyw = pyt - para_field.carWorldY - para_field.car_length;
				pzw = 0;

				pxv = (pxt- halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - pyt) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_BR_B.push_back(pt3d_w);
				vertexCoords2D.glVertex_BR.push_back(pt3d_v);

				pxw = pxt - para_field.carWorldX - para_field.car_width;
				pyw = pyt - para_field.carWorldY - para_field.LRchess2carFront_distance;
				pzw = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

				objPoints2D.glObjPoints_BR_R.push_back(pt3d_w);

				if(getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) < 0)
	            {
	                blendAlpha2D.glAlpha_BR.push_back(0.0);
	            }
	            else if(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) > 0)
	            {
	                blendAlpha2D.glAlpha_BR.push_back(1.0);
	            }
	            else
	            {
	                dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
	                dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

	                theta1 = asin(dist1 / dist2);

	                blendAlpha2D.glAlpha_BR.push_back(1.0 - theta1 / theta);
	            }

				pxt = para_field.carWorldX + para_field.car_width + j * para_field.carWorldX2;
				pyt = para_field.carWorldY + para_field.car_length + (i + fp) * para_field.carWorldY2;				

				pxw = pxt - halfWorldWidth + para_field.chessboard_length_corners * para_field.square_size / 2;
				pyw = pyt - para_field.carWorldY - para_field.car_length;
				pzw = 0;

				pxv = (pxt- halfWorldWidth) / halfWorldWidth;
				pyv = (halfWorldHeight - pyt) / halfWorldHeight;
				pzv = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);
				pt3d_v = cvPoint3D32f(pxv, pyv, pzv);
				
				objPoints2D.glObjPoints_BR_B.push_back(pt3d_w);
				vertexCoords2D.glVertex_BR.push_back(pt3d_v);

				pxw = pxt - para_field.carWorldX - para_field.car_width;
				pyw = pyt - para_field.carWorldY - para_field.LRchess2carFront_distance;
				pzw = 0;

				pt3d_w = cvPoint3D32f(pxw, pyw, pzw);

				objPoints2D.glObjPoints_BR_R.push_back(pt3d_w);

				if(getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) < 0)
	            {
	                blendAlpha2D.glAlpha_BR.push_back(0.0);
	            }
	            else if(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) > 0)
	            {
	                blendAlpha2D.glAlpha_BR.push_back(1.0);
	            }
	            else
	            {
	                dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
	                dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

	                theta1 = asin(dist1 / dist2);

	                blendAlpha2D.glAlpha_BR.push_back(1.0 - theta1 / theta);
	            }
			}
			//objPoints.glObjPoints_F.pop_back();
			//vertexCoords.glVertex_F.pop_back();
			break;
		}
		// change direction!
		if(j - fp == 1 )
		{
			directionFlag = 1;
		}
		else
		{
			directionFlag = 0;
		}
	}
	printf("VTX_NUM_BR: %lu %lu\n", objPoints2D.glObjPoints_BR_B.size(), objPoints2D.glObjPoints_BR_R.size());
	texCoords2D.glTexCoord_BR_B.resize(objPoints2D.glObjPoints_BR_B.size());
	texCoords2D.glTexCoord_BR_R.resize(objPoints2D.glObjPoints_BR_R.size());
	texCoordsStatistics2D.glTexCoord_BR_B.resize(objPointsStatistics2D.glObjPoints_BR_B.size());
	texCoordsStatistics2D.glTexCoord_BR_R.resize(objPointsStatistics2D.glObjPoints_BR_R.size());
#endif


#if 1
	glVertices2DCar[0] = 1.0 * (para_field.carWorldX - halfWorldWidth) / halfWorldWidth - SHADOW_X_OFFSET;
	glVertices2DCar[1] = 1.0 * (halfWorldHeight - para_field.carWorldY - para_field.car_length) / halfWorldHeight - SHADOW_Y_OFFSET;

	glVertices2DCar[3] = 1.0 * (para_field.carWorldX + para_field.car_width - halfWorldWidth) / halfWorldWidth + SHADOW_X_OFFSET;
	glVertices2DCar[4] = 1.0 * (halfWorldHeight - para_field.carWorldY - para_field.car_length) / halfWorldHeight - SHADOW_Y_OFFSET;

	glVertices2DCar[6] = 1.0 * (para_field.carWorldX - halfWorldWidth) / halfWorldWidth - SHADOW_X_OFFSET;
	glVertices2DCar[7] = 1.0 * (halfWorldHeight - para_field.carWorldY) / halfWorldHeight + SHADOW_Y_OFFSET;

	glVertices2DCar[9] = 1.0 * (para_field.carWorldX + para_field.car_width - halfWorldWidth) / halfWorldWidth + SHADOW_X_OFFSET;
	glVertices2DCar[10] = 1.0 * (halfWorldHeight - para_field.carWorldY) / halfWorldHeight + SHADOW_Y_OFFSET;
#endif

}


void projectPoints(const CvMat *obj_points,
                   const CvMat *r_vec,
                   const CvMat *t_vec,
                   const CvMat *A,
                   const CvMat *dist_coeffs,
                   CvMat *img_points)
{
    float a[9], t[3], R[9], k[4];
    float fx, fy, cx, cy;
    CvPoint3D32f *M;
    CvPoint2D32f *m;
    int i, j, count;
    CvPoint2D32f imgPoints;
    CvMat _rr;

    M = (CvPoint3D32f *)(obj_points->data.ptr);
    m = (CvPoint2D32f *)(img_points->data.ptr);
    count = MAX(obj_points->rows, obj_points->cols);

    //printf(" %s *************************************************\n", __FUNCTION__);
    //printf("OBJ POINTS counts [%d] \n", count);

    _rr = cvMat(3, 3, CV_32F, R);

    cvRodrigues2( r_vec, &_rr, NULL ) ;

    for(i = 0; i < 3; i++)
    {
        t[i] = cvmGet(t_vec, 0, i);

        for(j = 0; j < 3; j++)
        {
            a[i * 3 + j] = cvmGet(A, i, j);
            //R[i*3 + j] = (float)cvmGet(r_vec,i,j);
        }
    }

    for(i = 0; i < 4; i++)
    {
        if(!dist_coeffs)
        {
            k[i] = 0;
        }
        else
        {
            k[i] = cvmGet(dist_coeffs, 0, i);
        }
    }

    fx = 2 * a[0];
    fy = 2 * a[4];
    cx = 2 * a[2];
    cy = 2 * a[5];

    //printf("fx fy cx cy [%f %f %f %f]\n", a[0], a[4], a[2], a[5]);
    //printf("k [%f %f %f %f]\n", k[0], k[1], k[2], k[3]);
    //printf("rotate [%f %f %f]\n", cvmGet(r_vec, 0, 0),
    //cvmGet(r_vec, 0, 1), cvmGet(r_vec, 0, 2));
    //printf("trans  [%f %f %f]\n", t[0], t[1], t[2]);

    for( i = 0; i < count; i++ )
    {
        double X = M[i].x;
        double Y = M[i].y;
        double Z = M[i].z;
        double x = R[0] * X + R[1] * Y + R[2] * Z + t[0];
        double y = R[3] * X + R[4] * Y + R[5] * Z + t[1];
        double z = R[6] * X + R[7] * Y + R[8] * Z + t[2];
        double r, r2, xd, yd, theta, theta2, theta4, theta6, theta8, theta_d;

        if(z < 0)
        {
            imgPoints.x = 0.0001;
            imgPoints.y = 0.0001;
        }
        else
        {
            if(!dist_coeffs)
            {
                xd = x;
                yd = y;
            }
            else
            {
                z = z ? 1 / z : 1;
                x = x * z;
                y = y * z;
                r2 = x * x + y * y;
                r = sqrt(r2);
                theta = atan(r);
                theta2 = theta * theta;
                theta4 = theta2 * theta2;
                theta6 = theta4 * theta2;
                theta8 = theta6 * theta2;

                theta_d = theta * (1 + k[0] * theta2 + k[1] * theta4 + k[2] * theta6 + k[3] * theta8);

                if(r < 0.00001f)
                {
                    xd = 0;
                    yd = 0;
                }
                else
                {
                    xd = (x * theta_d) / r;
                    yd = (y * theta_d) / r;
                }
            }
            imgPoints.x = fx * xd + cx;
            imgPoints.y = fy * yd + cy;
#if 0
            if (imgPoints.x < 2)
            {
                imgPoints.x = 0.0001;
                imgPoints.y = 0.0001;
                //printf("1 %f\n",imgPoints.x);
            }
            else if (imgPoints.x > IMGWIDTH - 4)
            {
                imgPoints.x = 0.0001;//img_width - 4;
                imgPoints.y = 0.0001;
                //printf("2 %f\n",imgPoints.x);
            }

            if (imgPoints.y < 2)
            {
                imgPoints.x = 0.0001;
                imgPoints.y = 0.0001;
                //printf("3 %f\n",imgPoints.y);
            }
            else if (imgPoints.y > IMGHEIGHT - 4)
            {
                imgPoints.x = 0.0001;
                imgPoints.y = 0.0001;//img_height - 4;
                //printf("4 %f\n",imgPoints.y);
            }
#endif

            if(imgPoints.x < 0 || imgPoints.x > IMGWIDTH || imgPoints.y < 0 || imgPoints.y > IMGHEIGHT)
            {
                imgPoints.x = 0.0001;
                imgPoints.y = 0.0001;
            }
            else
            {
                m[i].x = imgPoints.x / IMGWIDTH;
                m[i].y = imgPoints.y / IMGHEIGHT;
            }
        }
    }
}


void projectPoints1(const CvMat *obj_points,
                    const CvMat *r_vec,
                    const CvMat *t_vec,
                    const CvMat *A,
                    const CvMat *dist_coeffs,
                    CvMat *img_points)
{
    float a[9], t[3], R[9], k[4];
    float fx, fy, cx, cy;
    CvPoint3D32f *M;
    int *m;
    int i, j, count;
    CvPoint2D32f imgPoints;
    CvMat _rr;

    M = (CvPoint3D32f *)(obj_points->data.ptr);
    m = (int *)(img_points->data.ptr);
    count = MAX(obj_points->rows, obj_points->cols);

    //printf(" %s *************************************************\n", __FUNCTION__);
    //printf("OBJ POINTS counts [%d] \n", count);

    _rr = cvMat(3, 3, CV_32F, R);

    cvRodrigues2( r_vec, &_rr, NULL ) ;

    for(i = 0; i < 3; i++)
    {
        t[i] = cvmGet(t_vec, 0, i);

        for(j = 0; j < 3; j++)
        {
            a[i * 3 + j] = cvmGet(A, i, j);
            //R[i*3 + j] = (float)cvmGet(r_vec,i,j);
        }
    }

    for(i = 0; i < 4; i++)
    {
        if(!dist_coeffs)
        {
            k[i] = 0;
        }
        else
        {
            k[i] = cvmGet(dist_coeffs, 0, i);
        }
    }

    fx = 2 * a[0];
    fy = 2 * a[4];
    cx = 2 * a[2];
    cy = 2 * a[5];

    //printf("fx fy cx cy [%f %f %f %f]\n", a[0], a[4], a[2], a[5]);
    //printf("k [%f %f %f %f]\n", k[0], k[1], k[2], k[3]);
    //printf("rotate [%f %f %f]\n", cvmGet(r_vec, 0, 0),
    //cvmGet(r_vec, 0, 1), cvmGet(r_vec, 0, 2));
    //printf("trans  [%f %f %f]\n", t[0], t[1], t[2]);

    for( i = 0; i < count; i++ )
    {
        double X = M[i].x;
        double Y = M[i].y;
        double Z = M[i].z;
        double x = R[0] * X + R[1] * Y + R[2] * Z + t[0];
        double y = R[3] * X + R[4] * Y + R[5] * Z + t[1];
        double z = R[6] * X + R[7] * Y + R[8] * Z + t[2];
        double r, r2, xd, yd, theta, theta2, theta4, theta6, theta8, theta_d;

        if(!dist_coeffs)
        {
            xd = x;
            yd = y;
        }
        else
        {
            z = z ? 1 / z : 1;
            x = x * z;
            y = y * z;
            r2 = x * x + y * y;
            r = sqrt(r2);
            theta = atan(r);
            theta2 = theta * theta;
            theta4 = theta2 * theta2;
            theta6 = theta4 * theta2;
            theta8 = theta6 * theta2;

            theta_d = theta * (1 + k[0] * theta2 + k[1] * theta4 + k[2] * theta6 + k[3] * theta8);

            if(r < 0.00001f)
            {
                xd = 0;
                yd = 0;
            }
            else
            {
                xd = (x * theta_d) / r;
                yd = (y * theta_d) / r;
            }
        }
        imgPoints.x = fx * xd + cx;
        imgPoints.y = fy * yd + cy;

        m[i] = cvRound(imgPoints.y) * IMGWIDTH + cvRound(imgPoints.x);

        //printf("%d\n",m[i]);
    }
}


CvPoint2D32f projectPoints2(const CvPoint3D32f obj_points,
                   const CvMat *r_vec,
                   const CvMat *t_vec)
{
    float t[3], R[9];
    int i, j, count;
    CvPoint2D32f imgPoints;
    CvMat _rr;
	double X, Y, Z, x, y, z;

    _rr = cvMat(3, 3, CV_32F, R);

    cvRodrigues2( r_vec, &_rr, NULL ) ;

    for(i = 0; i < 3; i++)
    {
        t[i] = cvmGet(t_vec, 0, i);
    }


    X = obj_points.x;
    Y = obj_points.y;
    Z = obj_points.z;
    x = R[0] * X + R[1] * Y + R[2] * Z + t[0];
    y = R[3] * X + R[4] * Y + R[5] * Z + t[1];
    z = R[6] * X + R[7] * Y + R[8] * Z + t[2];

    //printf("%f %f %f\n",x, y, z);
	
    z = z ? 1 / z : 1;
    imgPoints.x = x * z;
    imgPoints.y = y * z;

	return imgPoints;
}


CvPoint2D32f projectPoints3(const CvPoint3D32f obj_points,
                   const float *r_vec,
                   const float *t_vec)
{
    CvPoint2D32f imgPoints;
	double X, Y, Z, x, y, z;



    X = obj_points.x;
    Y = obj_points.y;
    Z = obj_points.z;
    x = r_vec[0] * X + r_vec[1] * Y + r_vec[2] * Z + t_vec[0];
    y = r_vec[3] * X + r_vec[4] * Y + r_vec[5] * Z + t_vec[1];
    z = r_vec[6] * X + r_vec[7] * Y + r_vec[8] * Z + t_vec[2];

    z = z ? 1 / z : 1;
    imgPoints.x = x * z;
    imgPoints.y = y * z;

	return imgPoints;
}


CvPoint2D32f function( float a1, float b1, float c1, float a2, float b2, float c2 )
{
	float a3,b3,c3;
	float a4,b4,c4;
	CvPoint2D32f res;

	a3=a1*a2; a4=a2*a1;
	b3=b1*a2; b4=b2*a1;
	c3=c1*a2; c4=c2*a1;

	res.y=(c3-c4)/(b3-b4);

	a3=a1*b2; a4=a2*b1;
	b3=b1*b2; b4=b2*b1;
	c3=c1*b2; c4=c2*b1;

	res.x=(c3-c4)/(a3-a4);

	return res;
}


CvPoint2D32f function1(CvPoint2D32f dist, const CvMat *A, undistortParams params, double *invR)
{
    float a[9];
    float fx, fy, cx, cy;
    int i, j, count;
    CvPoint2D32f imgPoints;
	float angle = 0; 
	float xr, yr;
	float a1, b1, c1, a2, b2, c2;
	CvPoint2D32f len, point;
	
    for(i = 0; i < 3; i++)
    {
        for(j = 0; j < 3; j++)
        {
            a[i * 3 + j] = cvmGet(A, i, j);
        }
    }

    fx = 2 * a[0];
    fy = 2 * a[4];
    cx = 2 * a[2];
    cy = 2 * a[5];

    //xr = (sin(angle) + dist.x * cos(angle)) / (cos(angle) - dist.x * sin(angle));
    //yr = dist.y * (xr * sin(angle) + cos(angle));

	a1 = dist.x * invR[6] - invR[0];
	b1 = dist.x * invR[7] - invR[1];
	c1 = invR[2] - dist.x * invR[8];

	a2 = dist.y * invR[6] - invR[3];
	b2 = dist.y * invR[7] - invR[4];
	c2 = invR[5] - dist.y * invR[8];
	
	point = function( a1, b1, c1, a2, b2, c2 );

    imgPoints.x = (cx + point.x * fx - params.x) / params.xZoom;
    imgPoints.y = (cy + point.y * fy - params.y) / params.yZoom;

    return imgPoints;
}


CvPoint2D32f function3(CvPoint2D32f dist, const float *intrinsic_matrix, float *distortTable, undistortParams params, double *invR)
{
    float a[9], k[4];
    float fx, fy, cx, cy;
    int i, j, count;
    CvPoint2D32f imgPoints, point;
	//float angle = params.angle + degree * RADIAN; 
	//float xr, yr;
	float a1, b1, c1, a2, b2, c2;
	double r, r2, xd, yd, theta, theta2, theta4, theta6, theta8, theta_d;
	double distort, undistort;
	
    fx = intrinsic_matrix[0] * 2;
    fy = intrinsic_matrix[1] * 2;
    cx = intrinsic_matrix[2] * 2;
   	cy = intrinsic_matrix[3] * 2;

    k[0] = distortTable[0];
	k[1] = distortTable[1];
	k[2] = distortTable[2];
	k[3] = distortTable[3];

	a1 = invR[0] - dist.x * invR[6];
	b1 = invR[1] - dist.x * invR[7];
	c1 = dist.x * invR[8] - invR[2];
	a2 = invR[3] - dist.y * invR[6];
	b2 = invR[4] - dist.y * invR[7];
	c2 = dist.y * invR[8] - invR[5];


	point = function( a1, b1, c1, a2, b2, c2 );

	/*r2 = point.x * point.x + point.y * point.y;
	r = sqrt(r2);
	theta = atan(r);
	theta2 = theta * theta;
	theta4 = theta2 * theta2;
	theta6 = theta4 * theta2;
	theta8 = theta6 * theta2;

	theta_d = theta * (1 + k[0] * theta2 + k[1] * theta4 + k[2] * theta6 + k[3] * theta8);
	distort = theta_d / r;*/

	xd = point.x;// * distort;
	yd = point.y;// * distort;

	/*$)A5C5=OqKXWx1j*/
    imgPoints.x = (cx + xd * fx - params.x) / params.xZoom;
    imgPoints.y = (cy + yd * fy - params.y) / params.yZoom;

    return imgPoints;
}


CvPoint2D32f function4(CvPoint2D32f dist, const float *intrinsic_matrix, float *distortTable, undistortParams params, double *invR)
{
    float a[9], k[4];
    float fx, fy, cx, cy;
    int i, j, count;
    CvPoint2D32f imgPoints, point;
	//float angle = params.angle + degree * RADIAN; 
	//float xr, yr;
	float a1, b1, c1, a2, b2, c2;
	float r, r2, r4, r6, xd, yd, x, y;
	float distort, undistort;
	float k1, k2, p1, p2, k3;
	
    cx = intrinsic_matrix[0];
    cy = intrinsic_matrix[1];
    fx = intrinsic_matrix[2];
    fy = intrinsic_matrix[3];

    k1 = distortTable[0];
	k2 = distortTable[1];
	p1 = distortTable[2];
	p2 = distortTable[3];
	k3 = distortTable[4];

	a1 = invR[0] - dist.x * invR[6];
	b1 = invR[1] - dist.x * invR[7];
	c1 = dist.x * invR[8] - invR[2];
	a2 = invR[3] - dist.y * invR[6];
	b2 = invR[4] - dist.y * invR[7];
	c2 = dist.y * invR[8] - invR[5];


	point = function( a1, b1, c1, a2, b2, c2 );
	x = point.x;
	y = point.y;
	r2 = x * x + y * y;
	r4 = r2 * r2;
	r6 = r4 * r2;

	//xd = x * (1 + k1 * r2 + k2 * r4 + k3 * r6);// + 2 * p1 * x * y + p2 * (r2 + 2 * x2);
	//yd = y * (1 + k1 * r2 + k2 * r4 + k3 * r6);// + 2 * p2 * x * y + p1 * (r2 + 2 * y2);
	xd = x;// * (1 + k1 * r2 + k2 * r4);// + 2 * p1 * x * y + p2 * (r2 + 2 * x2);
	yd = y;// * (1 + k1 * r2 + k2 * r4);// + 2 * p2 * x * y + p1 * (r2 + 2 * y2);
	//xd = x * (1 + k1 * r2 + k2 * r4) + 2 * p1 * x * y + p2 * (r2 + 2 * x * x);
	//yd = y * (1 + k1 * r2 + k2 * r4) + 2 * p2 * x * y + p1 * (r2 + 2 * y * y);

	/*$)A5C5=OqKXWx1j*/
    imgPoints.x = (cx + x * fx - params.x) / params.xZoom;
    imgPoints.y = (cy + y * fy - params.y) / params.yZoom;

    return imgPoints;
}

void generateTriangleStrip(std::vector<CvPoint2D32f> &triagnle, int width, int height)
{
	float p = 16.0;
	float i, j, fp;
	unsigned char directionFlag;
	CvPoint2D32f point;
	fp = 1.0 / p;

	directionFlag = 0;

	for (i = 0; i < 1; i += fp)
	{
		switch(directionFlag)
		{
		case 0:
			for (j = 0; j <= 1; j += fp)
			{
				point.x = j * width;
				point.y = i * height;
					
				triagnle.push_back(point);

				point.x = j * width;
				point.y = (i + fp) * height;
					
				triagnle.push_back(point);
			}
			//objPoints2D.glObjPoints_F.pop_back();
			//vertexCoords2D.glVertex_F.pop_back();
			break;
		case 1:
			for( j = 1; j >= 0; j -= fp )
			{
				point.x = j * width;
				point.y = i * height;
					
				triagnle.push_back(point);

				point.x = j * width;
				point.y = (i + fp) * height;
	
				triagnle.push_back(point);
			}
			//objPoints.glObjPoints_F.pop_back();
			//vertexCoords.glVertex_F.pop_back();
			break;
		}
		// change direction!
		if(j - fp == 1 )
		{
			directionFlag = 1;
		}
		else
		{
			directionFlag = 0;
		}
	}
}

#if 0
void initUndistortSingleView(void)
{
	int k, i;
    CvMat obj_points, img_points;
	int worldWith, worldHeight, xOffset, yOffset, angle;
	float scaleX, scaleY;
	float offsetWX, worldW;
	CvPoint3D32f *obj, *ver;
	CvPoint2D32f *img, *tex;

	float t[3], R[9];
	CvMat _rr;
	float a1, b1, c1, a2, b2, c2;
	float a, b, c, d, e, f;
	
	CvPoint3D32f tmpObj;
	float offsetX, offsetY;
	CvMat *Coeff, *Coeff1, *Coeff2;
	double cos_x, sin_x, cos_y, sin_y, cos_z, sin_z;
	double radians = (3.1415926535 / 180);
	float lineLen;
	CvPoint2D32f len;
	float scale;
	float step[17] = {0, 1.8, 3.6, 4.8, 6.4, 7.8, 9.2, 10.6, 12, 12.6, 13.2, 13.8, 14.4, 14.8, 15.2, 15.6, 16};
	float step1[17] = {0, 0.4, 0.8, 1.2, 1.6, 2.2, 2.8, 3.4, 4, 5.4, 6.8, 8.2, 9.6, 11.2, 12.8, 14.4, 16};
	float step2[17] = {0, 2, 4, 6, 8, 9.5, 11, 12.5, 14, 14.25, 14.5, 14.75, 15, 15.25, 15.5, 15.75, 16};
		
	frontUndistort.xZoom = 1.0;
	frontUndistort.yZoom = 1.0;
	frontUndistort.x = 0;//640 - 640;
	frontUndistort.y = 0;//360 - 360;

	generateTriangleStrip(glUndistortImage_F, bvs3DWidth, bvs3DHeight);
	float py;
	
	glUndistortTexCoord_F.resize(glUndistortImage_F.size());
	glUndistortVertex_F.resize(glUndistortImage_F.size());
	glUndistortObjPoints_F.resize(glUndistortImage_F.size());

	obj = &glUndistortObjPoints_F[0];
	ver = &glUndistortVertex_F[0];
	tex = &glUndistortTexCoord_F[0];
	img = &glUndistortImage_F[0];

	for (k = 0; k < glUndistortImage_F.size(); k++)
	{
		ver[k].x = -(1.0 - 2 * img[k].x / bvs3DWidth);
		ver[k].y = 1.0 - 2 * img[k].y / bvs3DHeight;
		ver[k].z = 0;
	}
	
	generateTriangleStrip(glUndistortImage_B, bvs3DWidth, bvs3DHeight);
		
	glUndistortTexCoord_B.resize(glUndistortImage_B.size());
	glUndistortVertex_B.resize(glUndistortImage_B.size());
	glUndistortObjPoints_B.resize(glUndistortImage_B.size());

	rearUndistort.xZoom = 1.0;
	rearUndistort.yZoom = 1.0;
	rearUndistort.x = 0;//640 - 640;
	rearUndistort.y = 0;//360 - 360;

	obj = &glUndistortObjPoints_B[0];
	ver = &glUndistortVertex_B[0];
	tex = &glUndistortTexCoord_B[0];
	img = &glUndistortImage_B[0];

	for (k = 0; k < glUndistortImage_B.size(); k++)
	{
		ver[k].x = -(1.0 - 2 * img[k].x / bvs3DWidth);
		ver[k].y = 1.0 - 2 * img[k].y / bvs3DHeight;
		ver[k].y = -ver[k].y;
		ver[k].z = 0;

	}


	obj_points = cvMat(1, glUndistortImage_B.size(), CV_32FC3, obj);
	img_points = cvMat(1, glUndistortImage_B.size(), CV_32FC2, tex);
	projectPoints(&obj_points, SimplifyrearCamParams.r_vec, SimplifyrearCamParams.t_vec, SimplifyrearCamParams.camera, SimplifyrearCamParams.dist_coeffs, &img_points);
}

#endif

#if 1
double towFac(double *b)
{
    return b[0] * b[3] - b[1] * b[2];
}

void copy3To2(double *a, double *b, int i, int j)
{
    int m = 0, n = 0;
    int count = 0;
    for(m = 0; m < 3; m++)
    {
        for(n = 0; n < 3; n++)
        {
            if(m != i && n != j)
            {
                count++;
                b[((count - 1) / 2) * 2 + (count - 1) % 2] = a[m * 3 + n];
            }
        }
    }
}

double threeSum(double *a, double *b)
{
    double sum = 0;
    for(int i = 0; i < 3; i++)
    {
        copy3To2(a, b, 0, i);
        if(i % 2 == 0)
        {
            sum += a[i] * towFac(b);
        }
        else
        {
            sum -= a[i] * towFac(b);
        }
    }
    return sum;
}

void calCArray(double *a, double *b, double *c)
{
    int i = 0;
    int j = 0;
    for(i = 0; i < 3; i++)
    {
        for(j = 0; j < 3; j++)
        {
            copy3To2(a, b, i, j);
            if((i + j) % 2 == 0)
            {
                c[j*3+i] = towFac(b);
            }
            else
            {
                c[j*3+i] = -towFac(b);
            }
        }
    }
}

void niArray(double *c, double A)
{
    int i = 0;
    int j = 0;
    for(i = 0; i < 3; i++)
    {
        for(j = 0; j < 3; j++)
        {
            c[i*3+j] /= A;
        }
    }
}

void getInvertMatrix(double *src, double *dst)
{
    int i, n = 0;
    long sum = 0;

    //printf("$)AT-@4>XUsHgOB#:\n");
    //printArray3(src, 3, 3);
    double b[2][2];
    double t = threeSum(src, &b[0][0]); //$)AGs|A|
    //printf("|A|=%lf\n", t);
    calCArray(src, &b[0][0], dst); //$)AGsA*
    niArray(dst, t); //$)AGsDf>XUs
    
}
#endif


void rotationMatrixToEulerAngles(CvMat *rVec, float *alpha, float *beta, float *gamma)
{
	float a[9], t[3], R[9], k[4];
	float fx, fy, cx, cy;
	CvPoint3D32f *M;
	CvPoint2D32f *m;
	int i, j, count;
	CvPoint2D32f imgPoints;
	CvMat _rr;

	_rr = cvMat(3, 3, CV_32F, R);
	
	cvRodrigues2( rVec, &_rr, NULL ) ;
	//cvTranspose(&_rr, &_rr);

    float sy = sqrt(R[7] * R[7] +  R[8] * R[8]);
 
    int singular = sy < 1e-6; // If
 
    float x, y, z;
    if (!singular) 
    {
        x = atan2(R[7], R[8]);
        y = atan2(-R[6], sy);
        z = atan2(R[3], R[0]);
    } 
    else 
    {
        x = atan2(-R[5], R[4]);
        y = atan2(-R[6], sy);
        z = 0;
    }

	*alpha = x;
	*beta = y;
	*gamma = z;
    //printf("%f %f %f\n", x, y, z);
    //return Vec3f(x, y, z);   
}

void matrixMul(double *srcA, double *srcB , double *dst)
{
	dst[0] = srcA[0] * srcB[0] + srcA[1] * srcB[3] + srcA[2] * srcB[6];
	dst[1] = srcA[0] * srcB[1] + srcA[1] * srcB[4] + srcA[2] * srcB[7];
	dst[2] = srcA[0] * srcB[2] + srcA[1] * srcB[5] + srcA[2] * srcB[8];

	dst[3] = srcA[3] * srcB[0] + srcA[4] * srcB[3] + srcA[5] * srcB[6];
	dst[4] = srcA[3] * srcB[1] + srcA[4] * srcB[4] + srcA[5] * srcB[7];
	dst[5] = srcA[3] * srcB[2] + srcA[4] * srcB[5] + srcA[5] * srcB[8];

	dst[6] = srcA[6] * srcB[0] + srcA[7] * srcB[3] + srcA[8] * srcB[6];
	dst[7] = srcA[6] * srcB[1] + srcA[7] * srcB[4] + srcA[8] * srcB[7];
	dst[8] = srcA[6] * srcB[2] + srcA[7] * srcB[5] + srcA[8] * srcB[8];
}


void findRearCurve(float wheelAngle, undistortParams params, float *camera, float *distortTable, float *rVec, float *tVec, double *invR, int width, int height)
{
    int wheel_base = 4675/20;
    int rear_wheel = 1375/20;
    float far_x;
    float ex_r, in_r, ex_r1, in_r1;
   	float tmpx, tmpy;
    float angle;
    int i, k;
    float j;
	float px, py;
	CvPoint3D32f pw;
	CvPoint2D32f len, point;
	float lineWidth, halfLineWidth, otherHalfLineWidth;
	double ex_r_in, ex_r_out, in_r_in, in_r_out;
	double ex_r1_in, ex_r1_out, in_r1_in, in_r1_out;
	double baseAngle[3][4], deltaAngle[3][4], radius[4];
	float fp = 1.0 / 64;
	CvPoint3D32f worldPoints[10][LENGTH * 2];
	float startx, length; 
	float startx0, length0;
	float maxAngle;
	float offsetY;
	int idx;
	int gap[3] = {7, 5, 3};
	int offset[2] = {1, 2};
	//float distance[4] = {800, 2140, 4124, 6000};
	float distance[4] = {0, 500/20, 1500/20, 4000/20};
	int car_width = 2556/20;
	float scale;

	lineWidth = 12;
	halfLineWidth = lineWidth / 2;
	otherHalfLineWidth = lineWidth - halfLineWidth;
	offsetY = 0;//1310;//+800;

	
	startx = -car_width/2;
	length = car_width;
	

	/*$)AW*;;3I;!6HVF*/
    if(wheelAngle >= 0 && wheelAngle < 0.001)
    {
        wheelAngle = 0.001;
    }
    else if(wheelAngle < 0 && wheelAngle > -0.001)
    {
        wheelAngle = -0.001;
    }
    angle = (90 - wheelAngle) * RADIAN;

    ex_r = fabs((float)wheel_base * tan(angle)) + car_width / 2;
    in_r = ex_r - car_width;

 	in_r1 = sqrt(in_r * in_r + rear_wheel * rear_wheel);
	ex_r1 = sqrt(ex_r * ex_r + rear_wheel * rear_wheel);

	in_r1_in = in_r1 - halfLineWidth;
	in_r1_out = in_r1 + otherHalfLineWidth;
	ex_r1_in = ex_r1 - halfLineWidth;
	ex_r1_out = ex_r1 + otherHalfLineWidth;

	radius[0] = in_r1_in;
	radius[1] = in_r1_out;
	radius[2] = ex_r1_in;
	radius[3] = ex_r1_out;


	/*for(idx=0; idx<3; idx++)
	{
		for(k=0; k<4; k++)
		{
			baseAngle[idx][k] = asin((rear_wheel + distance[idx]) / radius[k]);

			if(radius[k] > rear_wheel + distance[idx+1])
			{
				deltaAngle[idx][k] = asin((rear_wheel + distance[idx+1]) / radius[k]) - baseAngle[idx][k];
			}
			else
			{
				deltaAngle[idx][k] = PI / 2 - baseAngle[idx][k];
			}

			//PRINTF("angle0 %f %f\n",baseAngle[idx][k],deltaAngle[idx][k]);
		}
	}*/

	//ale = (1.0 - 0.6 * fabs(wheelAngle)/MAX_WHEEL_ANGLE);
	scale = (1.0 - 0.2* fabs(wheelAngle)/MAX_WHEEL_ANGLE);


	for(idx=0; idx<3; idx++)
	{
		for(k=0; k<4; k++)
		{
			if(k<2)
			{
				baseAngle[idx][k] = asin((rear_wheel + distance[idx] * scale) / radius[k]);

				if(radius[k] > rear_wheel + distance[idx+1] * scale)
				{
					deltaAngle[idx][k] = asin((rear_wheel + distance[idx+1] * scale) / radius[k]) - baseAngle[idx][k];
				}
				else
				{
					deltaAngle[idx][k] = PI / 2 - baseAngle[idx][k];
				}
			}
			else
			{
				baseAngle[idx][k] = asin((rear_wheel + distance[idx]) / radius[k]);

				if(radius[k] > rear_wheel + distance[idx+1])
				{
					deltaAngle[idx][k] = asin((rear_wheel + distance[idx+1]) / radius[k]) - baseAngle[idx][k];
				}
				else
				{
					deltaAngle[idx][k] = PI / 2 - baseAngle[idx][k];
				}
			}
		}
	}


	if(wheelAngle < 0)
	{
		for(idx=0; idx<3; idx++)
		{
			for (j = 0, k=0; j <= 1; j += fp, k++)
			{
				for(i=0; i<2; i++)
				{
					tmpx = car_width / 2 - ex_r + radius[i*2+0] * cos(baseAngle[idx][i*2+0] + deltaAngle[idx][i*2+0] * j); 
					tmpy = radius[i*2+0] * sin(baseAngle[idx][i*2+0] + deltaAngle[idx][i*2+0] * j) - rear_wheel;

					worldPoints[idx*2+i][k*2+0] = cvPoint3D32f(tmpx, tmpy+offsetY,0.0);


					tmpx = car_width / 2 - ex_r + radius[i*2+1] * cos(baseAngle[idx][i*2+1] + deltaAngle[idx][i*2+1] * j); 
					tmpy = radius[i*2+1] * sin(baseAngle[idx][i*2+1] + deltaAngle[idx][i*2+1] * j) - rear_wheel;

					worldPoints[idx*2+i][k*2+1] = cvPoint3D32f(tmpx, tmpy+offsetY,0.0);
				}
			}
		}

		for(idx=0; idx<3; idx++)
		{
			tmpx = car_width / 2 - ex_r + radius[1] * cos(baseAngle[idx][1] + deltaAngle[idx][1] * 1.0); 
			tmpy = radius[1] * sin(baseAngle[idx][1] + deltaAngle[idx][1] * 1.0) - rear_wheel;

			worldPoints[6][idx*4+0] = cvPoint3D32f(tmpx, tmpy+offsetY,0.0);

			tmpx = car_width / 2 - ex_r + radius[1] * cos(baseAngle[idx][1] + deltaAngle[idx][1] * (1.0 - gap[idx] * fp)); 
			tmpy = radius[1] * sin(baseAngle[idx][1] + deltaAngle[idx][1] * (1.0 - gap[idx] * fp)) - rear_wheel;

			worldPoints[6][idx*4+1] = cvPoint3D32f(tmpx, tmpy+offsetY,0.0);


			tmpx = car_width / 2 - ex_r + radius[2] * cos(baseAngle[idx][2] + deltaAngle[idx][2] * 1.0); 
			tmpy = radius[2] * sin(baseAngle[idx][2] + deltaAngle[idx][2] * 1.0) - rear_wheel;

			worldPoints[6][idx*4+2] = cvPoint3D32f(tmpx, tmpy+offsetY,0.0);

			tmpx = car_width / 2 - ex_r + radius[2] * cos(baseAngle[idx][2] + deltaAngle[idx][2] * (1.0 - gap[idx] * fp)); 
			tmpy = radius[2] * sin(baseAngle[idx][2] + deltaAngle[idx][2] * (1.0 - gap[idx] * fp)) - rear_wheel;

			worldPoints[6][idx*4+3] = cvPoint3D32f(tmpx, tmpy+offsetY,0.0);
		}


	}
	else
	{
		for(idx=0; idx<3; idx++)
		{
			for (j = 0, k=0; j <= 1; j += fp, k++)
			{
				for(i=0; i<2; i++)
				{
					tmpx = car_width / 2 + in_r - radius[i*2+0] * cos(baseAngle[idx][i*2+0] + deltaAngle[idx][i*2+0] * j); 
					tmpy = radius[i*2+0] * sin(baseAngle[idx][i*2+0] + deltaAngle[idx][i*2+0] * j) - rear_wheel;

					worldPoints[idx*2+i][k*2+0] = cvPoint3D32f(tmpx, tmpy+offsetY,0.0);

					tmpx = car_width / 2 + in_r - radius[i*2+1] * cos(baseAngle[idx][i*2+1] + deltaAngle[idx][i*2+1] * j); 
					tmpy = radius[i*2+1] * sin(baseAngle[idx][i*2+1] + deltaAngle[idx][i*2+1] * j) - rear_wheel;

					worldPoints[idx*2+i][k*2+1] = cvPoint3D32f(tmpx, tmpy+offsetY,0.0);
				}
			}
		}

		for(idx=0; idx<3; idx++)
		{
			tmpx = car_width / 2 + in_r - radius[1] * cos(baseAngle[idx][1] + deltaAngle[idx][1] * 1.0); 
			tmpy = radius[1] * sin(baseAngle[idx][1] + deltaAngle[idx][1] * 1.0) - rear_wheel;

			worldPoints[6][idx*4+0] = cvPoint3D32f(tmpx, tmpy+offsetY,0.0);

			tmpx = car_width / 2 + in_r - radius[1] * cos(baseAngle[idx][1] + deltaAngle[idx][1] * (1.0 - gap[idx] * fp)); 
			tmpy = radius[1] * sin(baseAngle[idx][1] + deltaAngle[idx][1] * (1.0 - gap[idx] * fp)) - rear_wheel;

			worldPoints[6][idx*4+1] = cvPoint3D32f(tmpx, tmpy+offsetY,0.0);


			tmpx = car_width / 2 + in_r - radius[2] * cos(baseAngle[idx][2] + deltaAngle[idx][2] * 1.0); 
			tmpy = radius[2] * sin(baseAngle[idx][2] + deltaAngle[idx][2] * 1.0) - rear_wheel;

			worldPoints[6][idx*4+2] = cvPoint3D32f(tmpx, tmpy+offsetY,0.0);

			tmpx = car_width / 2 + in_r - radius[2] * cos(baseAngle[idx][2] + deltaAngle[idx][2] * (1.0 - gap[idx] * fp)); 
			tmpy = radius[2] * sin(baseAngle[idx][2] + deltaAngle[idx][2] * (1.0 - gap[idx] * fp)) - rear_wheel;

			worldPoints[6][idx*4+3] = cvPoint3D32f(tmpx, tmpy+offsetY,0.0);
		}
	}

	for (j = 0, k=0; j <= 1; j += fp)
	{
		tmpx = startx + (length) * j;
        tmpy = 0-5; 

        worldPoints[7][k*2+0] = cvPoint3D32f(tmpx, tmpy, 0.0);

        tmpx = startx + (length) * j;
        tmpy = 0+5; 

        worldPoints[7][k*2+1] = cvPoint3D32f(tmpx, tmpy, 0.0);
        k++;
	}
	
	for(k=0; k<8; k++)
	{
		for(i = 0; i < LENGTH * 2; i++) /*$)A<FKcJ@=gWx1j5c*/
		{
			pw = worldPoints[k][i];
			//printf("%f %f\n",pw.x, pw.y);
			len = projectPoints3( pw, rVec, tVec ); 

			point = function3(len, camera, distortTable, params, invR);

			//printf("%d %d %f %f\n",k,i,point.x,point.y);

			verticesRearTrajLinePoint[k][i].x = -(1.0 - 2 * point.x / width);
			verticesRearTrajLinePoint[k][i].y = -(1.0 - 2 * point.y / height);
			verticesRearTrajLinePoint[k][i].z = 0.0;
		}
	}
}


void findRearCurve4(float wheelAngle, undistortParams params, float *camera, float *distortTable, float *rVec, float *tVec, double *invR, int width, int height)
{
    int wheel_base = 4675;
    int rear_wheel = 1375;
    float far_x;
    float ex_r, in_r, ex_r1, in_r1;
   	float tmpx, tmpy;
    float angle;
    int i, k;
    float j;
	float px, py;
	CvPoint3D32f pw;
	CvPoint2D32f len, point;
	float lineWidth, halfLineWidth, otherHalfLineWidth;
	double ex_r_in, ex_r_out, in_r_in, in_r_out;
	double ex_r1_in, ex_r1_out, in_r1_in, in_r1_out;
	double baseAngle[3][4], deltaAngle[3][4], radius[4];
	float fp = 1.0 / 64;
	CvPoint3D32f worldPoints[10][LENGTH * 2];
	float startx, length; 
	float startx0, length0;
	float maxAngle;
	float offsetY;
	int idx;
	int gap[3] = {7, 5, 3};
	int offset[2] = {1, 2};
	//float distance[4] = {800, 2140, 4124, 6000};
	float distance[4] = {0, 500, 1500, 4000};
	int car_width = 2556;
	float scale;

	lineWidth = 30;
	halfLineWidth = lineWidth / 2;
	otherHalfLineWidth = lineWidth - halfLineWidth;
	offsetY = 0;//1310;//+800;

	
	startx = -car_width/2;
	length = car_width;
	

	/*$)AW*;;3I;!6HVF*/
    if(wheelAngle >= 0 && wheelAngle < 0.05)
    {
        wheelAngle = 0.05;
    }
    else if(wheelAngle < 0 && wheelAngle > -0.05)
    {
        wheelAngle = -0.05;
    }
    angle = (90 - wheelAngle) * RADIAN;
    ex_r = fabs((float)wheel_base * tan(angle)) + car_width / 2;
    in_r = ex_r - car_width;

 	in_r1 = sqrt(in_r * in_r + rear_wheel * rear_wheel);
	ex_r1 = sqrt(ex_r * ex_r + rear_wheel * rear_wheel);
	

	in_r1_in = in_r1 - halfLineWidth;
	in_r1_out = in_r1 + otherHalfLineWidth;
	ex_r1_in = ex_r1 - halfLineWidth;
	ex_r1_out = ex_r1 + otherHalfLineWidth;

	radius[0] = in_r1_in;
	radius[1] = in_r1_out;
	radius[2] = ex_r1_in;
	radius[3] = ex_r1_out;
	

	/*for(idx=0; idx<3; idx++)
	{
		for(k=0; k<4; k++)
		{
			baseAngle[idx][k] = asin((rear_wheel + distance[idx]) / radius[k]);

			if(radius[k] > rear_wheel + distance[idx+1])
			{
				deltaAngle[idx][k] = asin((rear_wheel + distance[idx+1]) / radius[k]) - baseAngle[idx][k];
			}
			else
			{
				deltaAngle[idx][k] = PI / 2 - baseAngle[idx][k];
			}

			//PRINTF("angle0 %f %f\n",baseAngle[idx][k],deltaAngle[idx][k]);
		}
	}*/

	//ale = (1.0 - 0.6 * fabs(wheelAngle)/MAX_WHEEL_ANGLE);
	scale = (1.0 - 0.2* fabs(wheelAngle)/MAX_WHEEL_ANGLE);


	for(idx=0; idx<3; idx++)
	{
		for(k=0; k<4; k++)
		{
			if(k<2)
			{
				baseAngle[idx][k] = asin((rear_wheel + distance[idx] * scale) / radius[k]);

				if(radius[k] > rear_wheel + distance[idx+1] * scale)
				{
					deltaAngle[idx][k] = asin((rear_wheel + distance[idx+1] * scale) / radius[k]) - baseAngle[idx][k];
				}
				else
				{
					deltaAngle[idx][k] = PI / 2 - baseAngle[idx][k];
				}
			}
			else
			{
				baseAngle[idx][k] = asin((rear_wheel + distance[idx]) / radius[k]);

				if(radius[k] > rear_wheel + distance[idx+1])
				{
					deltaAngle[idx][k] = asin((rear_wheel + distance[idx+1]) / radius[k]) - baseAngle[idx][k];
				}
				else
				{
					deltaAngle[idx][k] = PI / 2 - baseAngle[idx][k];
				}
			}
		}
	}

	if(wheelAngle < 0)
	{
		for(idx=0; idx<3; idx++)
		{
			for (j = 0, k=0; j <= 1; j += fp, k++)
			{
				for(i=0; i<2; i++)
				{
					tmpx = car_width / 2 - ex_r + radius[i*2+0] * cos(baseAngle[idx][i*2+0] + deltaAngle[idx][i*2+0] * j); 
					tmpy = radius[i*2+0] * sin(baseAngle[idx][i*2+0] + deltaAngle[idx][i*2+0] * j) - rear_wheel;

					worldPoints[idx*2+i][k*2+0] = cvPoint3D32f(tmpx, tmpy+offsetY,0.0);


					tmpx = car_width / 2 - ex_r + radius[i*2+1] * cos(baseAngle[idx][i*2+1] + deltaAngle[idx][i*2+1] * j); 
					tmpy = radius[i*2+1] * sin(baseAngle[idx][i*2+1] + deltaAngle[idx][i*2+1] * j) - rear_wheel;

					worldPoints[idx*2+i][k*2+1] = cvPoint3D32f(tmpx, tmpy+offsetY,0.0);
				}
			}
		}

		for(idx=0; idx<3; idx++)
		{
			tmpx = car_width / 2 - ex_r + radius[1] * cos(baseAngle[idx][1] + deltaAngle[idx][1] * 1.0); 
			tmpy = radius[1] * sin(baseAngle[idx][1] + deltaAngle[idx][1] * 1.0) - rear_wheel;

			worldPoints[6][idx*4+0] = cvPoint3D32f(tmpx, tmpy+offsetY,0.0);

			tmpx = car_width / 2 - ex_r + radius[1] * cos(baseAngle[idx][1] + deltaAngle[idx][1] * (1.0 - gap[idx] * fp)); 
			tmpy = radius[1] * sin(baseAngle[idx][1] + deltaAngle[idx][1] * (1.0 - gap[idx] * fp)) - rear_wheel;

			worldPoints[6][idx*4+1] = cvPoint3D32f(tmpx, tmpy+offsetY,0.0);


			tmpx = car_width / 2 - ex_r + radius[2] * cos(baseAngle[idx][2] + deltaAngle[idx][2] * 1.0); 
			tmpy = radius[2] * sin(baseAngle[idx][2] + deltaAngle[idx][2] * 1.0) - rear_wheel;

			worldPoints[6][idx*4+2] = cvPoint3D32f(tmpx, tmpy+offsetY,0.0);

			tmpx = car_width / 2 - ex_r + radius[2] * cos(baseAngle[idx][2] + deltaAngle[idx][2] * (1.0 - gap[idx] * fp)); 
			tmpy = radius[2] * sin(baseAngle[idx][2] + deltaAngle[idx][2] * (1.0 - gap[idx] * fp)) - rear_wheel;

			worldPoints[6][idx*4+3] = cvPoint3D32f(tmpx, tmpy+offsetY,0.0);
		}


	}
	else
	{
		for(idx=0; idx<3; idx++)
		{
			for (j = 0, k=0; j <= 1; j += fp, k++)
			{
				for(i=0; i<2; i++)
				{
					tmpx = car_width / 2 + in_r - radius[i*2+0] * cos(baseAngle[idx][i*2+0] + deltaAngle[idx][i*2+0] * j); 
					tmpy = radius[i*2+0] * sin(baseAngle[idx][i*2+0] + deltaAngle[idx][i*2+0] * j) - rear_wheel;

					worldPoints[idx*2+i][k*2+0] = cvPoint3D32f(tmpx, tmpy+offsetY,0.0);

					tmpx = car_width / 2 + in_r - radius[i*2+1] * cos(baseAngle[idx][i*2+1] + deltaAngle[idx][i*2+1] * j); 
					tmpy = radius[i*2+1] * sin(baseAngle[idx][i*2+1] + deltaAngle[idx][i*2+1] * j) - rear_wheel;

					worldPoints[idx*2+i][k*2+1] = cvPoint3D32f(tmpx, tmpy+offsetY,0.0);
				}
			}
		}

		for(idx=0; idx<3; idx++)
		{
			tmpx = car_width / 2 + in_r - radius[1] * cos(baseAngle[idx][1] + deltaAngle[idx][1] * 1.0); 
			tmpy = radius[1] * sin(baseAngle[idx][1] + deltaAngle[idx][1] * 1.0) - rear_wheel;

			worldPoints[6][idx*4+0] = cvPoint3D32f(tmpx, tmpy+offsetY,0.0);

			tmpx = car_width / 2 + in_r - radius[1] * cos(baseAngle[idx][1] + deltaAngle[idx][1] * (1.0 - gap[idx] * fp)); 
			tmpy = radius[1] * sin(baseAngle[idx][1] + deltaAngle[idx][1] * (1.0 - gap[idx] * fp)) - rear_wheel;

			worldPoints[6][idx*4+1] = cvPoint3D32f(tmpx, tmpy+offsetY,0.0);


			tmpx = car_width / 2 + in_r - radius[2] * cos(baseAngle[idx][2] + deltaAngle[idx][2] * 1.0); 
			tmpy = radius[2] * sin(baseAngle[idx][2] + deltaAngle[idx][2] * 1.0) - rear_wheel;

			worldPoints[6][idx*4+2] = cvPoint3D32f(tmpx, tmpy+offsetY,0.0);

			tmpx = car_width / 2 + in_r - radius[2] * cos(baseAngle[idx][2] + deltaAngle[idx][2] * (1.0 - gap[idx] * fp)); 
			tmpy = radius[2] * sin(baseAngle[idx][2] + deltaAngle[idx][2] * (1.0 - gap[idx] * fp)) - rear_wheel;

			worldPoints[6][idx*4+3] = cvPoint3D32f(tmpx, tmpy+offsetY,0.0);
		}
	}

	for (j = 0, k=0; j <= 1; j += fp)
	{
		tmpx = startx + (length) * j;
        tmpy = 0-5; 

        worldPoints[7][k*2+0] = cvPoint3D32f(tmpx, tmpy, 0.0);

        tmpx = startx + (length) * j;
        tmpy = 0+5; 

        worldPoints[7][k*2+1] = cvPoint3D32f(tmpx, tmpy, 0.0);
        k++;
	}
	
	for(k=0; k<8; k++)
	{
		for(i = 0; i < LENGTH * 2; i++) /*$)A<FKcJ@=gWx1j5c*/
		{
			pw = worldPoints[k][i];
			//printf("%f %f\n",pw.x, pw.y);
			len = projectPoints3( pw, rVec, tVec ); 

			point = function4(len, camera, distortTable, params, invR);

			//printf("%d %d %f %f\n",k,i,point.x,point.y);

			verticesRearTrajLinePoint[k][i].x = -(1.0 - 2 * point.x / width);
			verticesRearTrajLinePoint[k][i].y = -(1.0 - 2 * point.y / height);
			verticesRearTrajLinePoint[k][i].z = 0.0;
		}
	}

	printf("\n\n");
}


void initTextureCoords(void)
{
    CvMat obj_points, img_points;
    int i;
    
#if 0
    // front cam
    obj_points = cvMat(1, objPoints.glObjPoints_F.size(), CV_32FC3, &objPoints.glObjPoints_F[0]);
    img_points = cvMat(1, texCoords.glTexCoord_F.size(), CV_32FC2, &texCoords.glTexCoord_F[0]);
    projectPoints(&obj_points, SimplifyfrontCamParams.r_vec, SimplifyfrontCamParams.t_vec, SimplifyfrontCamParams.camera, SimplifyfrontCamParams.dist_coeffs, &img_points);
    objPoints.glObjPoints_F.clear();

    // front left blend front part
    obj_points = cvMat(1, objPoints.glObjPoints_FL_F.size(), CV_32FC3, &objPoints.glObjPoints_FL_F[0]);
    img_points = cvMat(1, texCoords.glTexCoord_FL_F.size(), CV_32FC2, &texCoords.glTexCoord_FL_F[0]);
    projectPoints(&obj_points, SimplifyfrontCamParams.r_vec, SimplifyfrontCamParams.t_vec, SimplifyfrontCamParams.camera, SimplifyfrontCamParams.dist_coeffs, &img_points);
    objPoints.glObjPoints_FL_F.clear();

    // front right blend front part
    obj_points = cvMat(1, objPoints.glObjPoints_FR_F.size(), CV_32FC3, &objPoints.glObjPoints_FR_F[0]);
    img_points = cvMat(1, texCoords.glTexCoord_FR_F.size(), CV_32FC2, &texCoords.glTexCoord_FR_F[0]);
    projectPoints(&obj_points, SimplifyfrontCamParams.r_vec, SimplifyfrontCamParams.t_vec, SimplifyfrontCamParams.camera, SimplifyfrontCamParams.dist_coeffs, &img_points);
    objPoints.glObjPoints_FR_F.clear();
#endif

    obj_points = cvMat(1, objPoints2D.glObjPoints_F.size(), CV_32FC3, &objPoints2D.glObjPoints_F[0]);
    img_points = cvMat(1, texCoords2D.glTexCoord_F.size(), CV_32FC2, &texCoords2D.glTexCoord_F[0]);
    projectPoints(&obj_points, SimplifyfrontCamParams.r_vec, SimplifyfrontCamParams.t_vec, SimplifyfrontCamParams.camera, SimplifyfrontCamParams.dist_coeffs, &img_points);
    objPoints2D.glObjPoints_F.clear();

    // front left blend front part
    obj_points = cvMat(1, objPoints2D.glObjPoints_FL_F.size(), CV_32FC3, &objPoints2D.glObjPoints_FL_F[0]);
    img_points = cvMat(1, texCoords2D.glTexCoord_FL_F.size(), CV_32FC2, &texCoords2D.glTexCoord_FL_F[0]);
    projectPoints(&obj_points, SimplifyfrontCamParams.r_vec, SimplifyfrontCamParams.t_vec, SimplifyfrontCamParams.camera, SimplifyfrontCamParams.dist_coeffs, &img_points);
    objPoints2D.glObjPoints_FL_F.clear();

    // front right blend front part
    obj_points = cvMat(1, objPoints2D.glObjPoints_FR_F.size(), CV_32FC3, &objPoints2D.glObjPoints_FR_F[0]);
    img_points = cvMat(1, texCoords2D.glTexCoord_FR_F.size(), CV_32FC2, &texCoords2D.glTexCoord_FR_F[0]);
    projectPoints(&obj_points, SimplifyfrontCamParams.r_vec, SimplifyfrontCamParams.t_vec, SimplifyfrontCamParams.camera, SimplifyfrontCamParams.dist_coeffs, &img_points);
    objPoints2D.glObjPoints_FR_F.clear();

#if 0
    // rear cam
    obj_points = cvMat(1, objPoints.glObjPoints_B.size(), CV_32FC3, &objPoints.glObjPoints_B[0]);
    img_points = cvMat(1, texCoords.glTexCoord_B.size(), CV_32FC2, &texCoords.glTexCoord_B[0]);
    projectPoints(&obj_points, SimplifyrearCamParams.r_vec, SimplifyrearCamParams.t_vec, SimplifyrearCamParams.camera, SimplifyrearCamParams.dist_coeffs, &img_points);
    objPoints.glObjPoints_B.clear();

    // rear left blend rear part
    obj_points = cvMat(1, objPoints.glObjPoints_BL_B.size(), CV_32FC3, &objPoints.glObjPoints_BL_B[0]);
    img_points = cvMat(1, texCoords.glTexCoord_BL_B.size(), CV_32FC2, &texCoords.glTexCoord_BL_B[0]);
    projectPoints(&obj_points, SimplifyrearCamParams.r_vec, SimplifyrearCamParams.t_vec, SimplifyrearCamParams.camera, SimplifyrearCamParams.dist_coeffs, &img_points);
    objPoints.glObjPoints_BL_B.clear();

    // rear right blend rear part
    obj_points = cvMat(1, objPoints.glObjPoints_BR_B.size(), CV_32FC3, &objPoints.glObjPoints_BR_B[0]);
    img_points = cvMat(1, texCoords.glTexCoord_BR_B.size(), CV_32FC2, &texCoords.glTexCoord_BR_B[0]);
    projectPoints(&obj_points, SimplifyrearCamParams.r_vec, SimplifyrearCamParams.t_vec, SimplifyrearCamParams.camera, SimplifyrearCamParams.dist_coeffs, &img_points);
    objPoints.glObjPoints_BR_B.clear();
#endif

    obj_points = cvMat(1, objPoints2D.glObjPoints_B.size(), CV_32FC3, &objPoints2D.glObjPoints_B[0]);
    img_points = cvMat(1, texCoords2D.glTexCoord_B.size(), CV_32FC2, &texCoords2D.glTexCoord_B[0]);
    projectPoints(&obj_points, SimplifyrearCamParams.r_vec, SimplifyrearCamParams.t_vec, SimplifyrearCamParams.camera, SimplifyrearCamParams.dist_coeffs, &img_points);
    objPoints2D.glObjPoints_B.clear();

    // rear left blend rear part
    obj_points = cvMat(1, objPoints2D.glObjPoints_BL_B.size(), CV_32FC3, &objPoints2D.glObjPoints_BL_B[0]);
    img_points = cvMat(1, texCoords2D.glTexCoord_BL_B.size(), CV_32FC2, &texCoords2D.glTexCoord_BL_B[0]);
    projectPoints(&obj_points, SimplifyrearCamParams.r_vec, SimplifyrearCamParams.t_vec, SimplifyrearCamParams.camera, SimplifyrearCamParams.dist_coeffs, &img_points);
    objPoints2D.glObjPoints_BL_B.clear();

    // rear right blend rear part
    obj_points = cvMat(1, objPoints2D.glObjPoints_BR_B.size(), CV_32FC3, &objPoints2D.glObjPoints_BR_B[0]);
    img_points = cvMat(1, texCoords2D.glTexCoord_BR_B.size(), CV_32FC2, &texCoords2D.glTexCoord_BR_B[0]);
    projectPoints(&obj_points, SimplifyrearCamParams.r_vec, SimplifyrearCamParams.t_vec, SimplifyrearCamParams.camera, SimplifyrearCamParams.dist_coeffs, &img_points);
    objPoints2D.glObjPoints_BR_B.clear();

#if 0
    //left cam
    obj_points = cvMat(1, objPoints.glObjPoints_L.size(), CV_32FC3, &objPoints.glObjPoints_L[0]);
    img_points = cvMat(1, texCoords.glTexCoord_L.size(), CV_32FC2, &texCoords.glTexCoord_L[0]);
    projectPoints(&obj_points, SimplifyleftCamParams.r_vec, SimplifyleftCamParams.t_vec, SimplifyleftCamParams.camera, SimplifyleftCamParams.dist_coeffs, &img_points);
    objPoints.glObjPoints_L.clear();

    //left front blend left part
    obj_points = cvMat(1, objPoints.glObjPoints_FL_L.size(), CV_32FC3, &objPoints.glObjPoints_FL_L[0]);
    img_points = cvMat(1, texCoords.glTexCoord_FL_L.size(), CV_32FC2, &texCoords.glTexCoord_FL_L[0]);
    projectPoints(&obj_points, SimplifyleftCamParams.r_vec, SimplifyleftCamParams.t_vec, SimplifyleftCamParams.camera, SimplifyleftCamParams.dist_coeffs, &img_points);
    objPoints.glObjPoints_FL_L.clear();

    //left rear blend left part
    obj_points = cvMat(1, objPoints.glObjPoints_BL_L.size(), CV_32FC3, &objPoints.glObjPoints_BL_L[0]);
    img_points = cvMat(1, texCoords.glTexCoord_BL_L.size(), CV_32FC2, &texCoords.glTexCoord_BL_L[0]);
    projectPoints(&obj_points, SimplifyleftCamParams.r_vec, SimplifyleftCamParams.t_vec, SimplifyleftCamParams.camera, SimplifyleftCamParams.dist_coeffs, &img_points);
    objPoints.glObjPoints_BL_L.clear();
#endif

	//left cam
	obj_points = cvMat(1, objPoints2D.glObjPoints_L.size(), CV_32FC3, &objPoints2D.glObjPoints_L[0]);
	img_points = cvMat(1, texCoords2D.glTexCoord_L.size(), CV_32FC2, &texCoords2D.glTexCoord_L[0]);
	projectPoints(&obj_points, SimplifyleftCamParams.r_vec, SimplifyleftCamParams.t_vec, SimplifyleftCamParams.camera, SimplifyleftCamParams.dist_coeffs, &img_points);
	objPoints2D.glObjPoints_L.clear();

	//left front blend left part
	obj_points = cvMat(1, objPoints2D.glObjPoints_FL_L.size(), CV_32FC3, &objPoints2D.glObjPoints_FL_L[0]);
	img_points = cvMat(1, texCoords2D.glTexCoord_FL_L.size(), CV_32FC2, &texCoords2D.glTexCoord_FL_L[0]);
	projectPoints(&obj_points, SimplifyleftCamParams.r_vec, SimplifyleftCamParams.t_vec, SimplifyleftCamParams.camera, SimplifyleftCamParams.dist_coeffs, &img_points);
	objPoints2D.glObjPoints_FL_L.clear();

	//left rear blend left part
	obj_points = cvMat(1, objPoints2D.glObjPoints_BL_L.size(), CV_32FC3, &objPoints2D.glObjPoints_BL_L[0]);
	img_points = cvMat(1, texCoords2D.glTexCoord_BL_L.size(), CV_32FC2, &texCoords2D.glTexCoord_BL_L[0]);
	projectPoints(&obj_points, SimplifyleftCamParams.r_vec, SimplifyleftCamParams.t_vec, SimplifyleftCamParams.camera, SimplifyleftCamParams.dist_coeffs, &img_points);
	objPoints2D.glObjPoints_BL_L.clear();

#if 0
    //right cam
    obj_points = cvMat(1, objPoints.glObjPoints_R.size(), CV_32FC3, &objPoints.glObjPoints_R[0]);
    img_points = cvMat(1, texCoords.glTexCoord_R.size(), CV_32FC2, &texCoords.glTexCoord_R[0]);
    projectPoints(&obj_points, SimplifyrightCamParams.r_vec, SimplifyrightCamParams.t_vec, SimplifyrightCamParams.camera, SimplifyrightCamParams.dist_coeffs, &img_points);
    objPoints.glObjPoints_R.clear();

    //right front blend right part
    obj_points = cvMat(1, objPoints.glObjPoints_FR_R.size(), CV_32FC3, &objPoints.glObjPoints_FR_R[0]);
    img_points = cvMat(1, texCoords.glTexCoord_FR_R.size(), CV_32FC2, &texCoords.glTexCoord_FR_R[0]);
    projectPoints(&obj_points, SimplifyrightCamParams.r_vec, SimplifyrightCamParams.t_vec, SimplifyrightCamParams.camera, SimplifyrightCamParams.dist_coeffs, &img_points);
    objPoints.glObjPoints_FR_R.clear();

    //right rear blend right part
    obj_points = cvMat(1, objPoints.glObjPoints_BR_R.size(), CV_32FC3, &objPoints.glObjPoints_BR_R[0]);
    img_points = cvMat(1, texCoords.glTexCoord_BR_R.size(), CV_32FC2, &texCoords.glTexCoord_BR_R[0]);
    projectPoints(&obj_points, SimplifyrightCamParams.r_vec, SimplifyrightCamParams.t_vec, SimplifyrightCamParams.camera, SimplifyrightCamParams.dist_coeffs, &img_points);
    objPoints.glObjPoints_BR_R.clear();
#endif

    //right cam
    obj_points = cvMat(1, objPoints2D.glObjPoints_R.size(), CV_32FC3, &objPoints2D.glObjPoints_R[0]);
    img_points = cvMat(1, texCoords2D.glTexCoord_R.size(), CV_32FC2, &texCoords2D.glTexCoord_R[0]);
    projectPoints(&obj_points, SimplifyrightCamParams.r_vec, SimplifyrightCamParams.t_vec, SimplifyrightCamParams.camera, SimplifyrightCamParams.dist_coeffs, &img_points);
    objPoints2D.glObjPoints_R.clear();

    //right front blend right part
    obj_points = cvMat(1, objPoints2D.glObjPoints_FR_R.size(), CV_32FC3, &objPoints2D.glObjPoints_FR_R[0]);
    img_points = cvMat(1, texCoords2D.glTexCoord_FR_R.size(), CV_32FC2, &texCoords2D.glTexCoord_FR_R[0]);
    projectPoints(&obj_points, SimplifyrightCamParams.r_vec, SimplifyrightCamParams.t_vec, SimplifyrightCamParams.camera, SimplifyrightCamParams.dist_coeffs, &img_points);
    objPoints2D.glObjPoints_FR_R.clear();

    //right rear blend right part
    obj_points = cvMat(1, objPoints2D.glObjPoints_BR_R.size(), CV_32FC3, &objPoints2D.glObjPoints_BR_R[0]);
    img_points = cvMat(1, texCoords2D.glTexCoord_BR_R.size(), CV_32FC2, &texCoords2D.glTexCoord_BR_R[0]);
    projectPoints(&obj_points, SimplifyrightCamParams.r_vec, SimplifyrightCamParams.t_vec, SimplifyrightCamParams.camera, SimplifyrightCamParams.dist_coeffs, &img_points);
    objPoints2D.glObjPoints_BR_R.clear();

}


void getCamPixelPosition()
{
    CvMat obj_points, img_points;
    int i;

#if 0
    // front left blend front part
    obj_points = cvMat(1, objPointsStatistics.glObjPoints_FL_F.size(), CV_32FC3, &objPointsStatistics.glObjPoints_FL_F[0]);
    img_points = cvMat(1, texCoordsStatistics.glTexCoord_FL_F.size(), CV_32SC1, &texCoordsStatistics.glTexCoord_FL_F[0]);
    projectPoints1(&obj_points, SimplifyfrontCamParams.r_vec, SimplifyfrontCamParams.t_vec, SimplifyfrontCamParams.camera, SimplifyfrontCamParams.dist_coeffs, &img_points);
    objPointsStatistics.glObjPoints_FL_F.clear();

    // front right blend front part
    obj_points = cvMat(1, objPointsStatistics.glObjPoints_FR_F.size(), CV_32FC3, &objPointsStatistics.glObjPoints_FR_F[0]);
    img_points = cvMat(1, texCoordsStatistics.glTexCoord_FR_F.size(), CV_32SC1, &texCoordsStatistics.glTexCoord_FR_F[0]);
    projectPoints1(&obj_points, SimplifyfrontCamParams.r_vec, SimplifyfrontCamParams.t_vec, SimplifyfrontCamParams.camera, SimplifyfrontCamParams.dist_coeffs, &img_points);
    objPointsStatistics.glObjPoints_FR_F.clear();

    // front left blend front part
    obj_points = cvMat(1, objPointsStatistics.glObjPoints_BL_B.size(), CV_32FC3, &objPointsStatistics.glObjPoints_BL_B[0]);
    img_points = cvMat(1, texCoordsStatistics.glTexCoord_BL_B.size(), CV_32SC1, &texCoordsStatistics.glTexCoord_BL_B[0]);
    projectPoints1(&obj_points, SimplifyrearCamParams.r_vec, SimplifyrearCamParams.t_vec, SimplifyrearCamParams.camera, SimplifyrearCamParams.dist_coeffs, &img_points);
    objPointsStatistics.glObjPoints_BL_B.clear();

    // front right blend front part
    obj_points = cvMat(1, objPointsStatistics.glObjPoints_BR_B.size(), CV_32FC3, &objPointsStatistics.glObjPoints_BR_B[0]);
    img_points = cvMat(1, texCoordsStatistics.glTexCoord_BR_B.size(), CV_32SC1, &texCoordsStatistics.glTexCoord_BR_B[0]);
    projectPoints1(&obj_points, SimplifyrearCamParams.r_vec, SimplifyrearCamParams.t_vec, SimplifyrearCamParams.camera, SimplifyrearCamParams.dist_coeffs, &img_points);
    objPointsStatistics.glObjPoints_BR_B.clear();

    // front left blend front part
    obj_points = cvMat(1, objPointsStatistics.glObjPoints_FL_L.size(), CV_32FC3, &objPointsStatistics.glObjPoints_FL_L[0]);
    img_points = cvMat(1, texCoordsStatistics.glTexCoord_FL_L.size(), CV_32SC1, &texCoordsStatistics.glTexCoord_FL_L[0]);
    projectPoints1(&obj_points, SimplifyleftCamParams.r_vec, SimplifyleftCamParams.t_vec, SimplifyleftCamParams.camera, SimplifyleftCamParams.dist_coeffs, &img_points);
    objPointsStatistics.glObjPoints_FL_L.clear();

    // front right blend front part
    obj_points = cvMat(1, objPointsStatistics.glObjPoints_BL_L.size(), CV_32FC3, &objPointsStatistics.glObjPoints_BL_L[0]);
    img_points = cvMat(1, texCoordsStatistics.glTexCoord_BL_L.size(), CV_32SC1, &texCoordsStatistics.glTexCoord_BL_L[0]);
    projectPoints1(&obj_points, SimplifyleftCamParams.r_vec, SimplifyleftCamParams.t_vec, SimplifyleftCamParams.camera, SimplifyleftCamParams.dist_coeffs, &img_points);
    objPointsStatistics.glObjPoints_BL_L.clear();

    // front left blend front part
    obj_points = cvMat(1, objPointsStatistics.glObjPoints_FR_R.size(), CV_32FC3, &objPointsStatistics.glObjPoints_FR_R[0]);
    img_points = cvMat(1, texCoordsStatistics.glTexCoord_FR_R.size(), CV_32SC1, &texCoordsStatistics.glTexCoord_FR_R[0]);
    projectPoints1(&obj_points, SimplifyrightCamParams.r_vec, SimplifyrightCamParams.t_vec, SimplifyrightCamParams.camera, SimplifyrightCamParams.dist_coeffs, &img_points);
    objPointsStatistics.glObjPoints_FR_R.clear();

    // front right blend front part
    obj_points = cvMat(1, objPointsStatistics.glObjPoints_BR_R.size(), CV_32FC3, &objPointsStatistics.glObjPoints_BR_R[0]);
    img_points = cvMat(1, texCoordsStatistics.glTexCoord_BR_R.size(), CV_32SC1, &texCoordsStatistics.glTexCoord_BR_R[0]);
    projectPoints1(&obj_points, SimplifyrightCamParams.r_vec, SimplifyrightCamParams.t_vec, SimplifyrightCamParams.camera, SimplifyrightCamParams.dist_coeffs, &img_points);
    objPointsStatistics.glObjPoints_BR_R.clear();
#endif


    // front left blend front part
    obj_points = cvMat(1, objPointsStatistics2D.glObjPoints_FL_F.size(), CV_32FC3, &objPointsStatistics2D.glObjPoints_FL_F[0]);
    img_points = cvMat(1, texCoordsStatistics2D.glTexCoord_FL_F.size(), CV_32SC1, &texCoordsStatistics2D.glTexCoord_FL_F[0]);
    projectPoints1(&obj_points, SimplifyfrontCamParams.r_vec, SimplifyfrontCamParams.t_vec, SimplifyfrontCamParams.camera, SimplifyfrontCamParams.dist_coeffs, &img_points);
    objPointsStatistics2D.glObjPoints_FL_F.clear();

    printf("FL_F = %d\n",texCoordsStatistics2D.glTexCoord_FL_F.size());

    // front right blend front part
    obj_points = cvMat(1, objPointsStatistics2D.glObjPoints_FR_F.size(), CV_32FC3, &objPointsStatistics2D.glObjPoints_FR_F[0]);
    img_points = cvMat(1, texCoordsStatistics2D.glTexCoord_FR_F.size(), CV_32SC1, &texCoordsStatistics2D.glTexCoord_FR_F[0]);
    projectPoints1(&obj_points, SimplifyfrontCamParams.r_vec, SimplifyfrontCamParams.t_vec, SimplifyfrontCamParams.camera, SimplifyfrontCamParams.dist_coeffs, &img_points);
    objPointsStatistics2D.glObjPoints_FR_F.clear();

    // front left blend front part
    obj_points = cvMat(1, objPointsStatistics2D.glObjPoints_BL_B.size(), CV_32FC3, &objPointsStatistics2D.glObjPoints_BL_B[0]);
    img_points = cvMat(1, texCoordsStatistics2D.glTexCoord_BL_B.size(), CV_32SC1, &texCoordsStatistics2D.glTexCoord_BL_B[0]);
    projectPoints1(&obj_points, SimplifyrearCamParams.r_vec, SimplifyrearCamParams.t_vec, SimplifyrearCamParams.camera, SimplifyrearCamParams.dist_coeffs, &img_points);
    objPointsStatistics2D.glObjPoints_BL_B.clear();

    // front right blend front part
    obj_points = cvMat(1, objPointsStatistics2D.glObjPoints_BR_B.size(), CV_32FC3, &objPointsStatistics2D.glObjPoints_BR_B[0]);
    img_points = cvMat(1, texCoordsStatistics2D.glTexCoord_BR_B.size(), CV_32SC1, &texCoordsStatistics2D.glTexCoord_BR_B[0]);
    projectPoints1(&obj_points, SimplifyrearCamParams.r_vec, SimplifyrearCamParams.t_vec, SimplifyrearCamParams.camera, SimplifyrearCamParams.dist_coeffs, &img_points);
    objPointsStatistics2D.glObjPoints_BR_B.clear();

    // front left blend front part
    obj_points = cvMat(1, objPointsStatistics2D.glObjPoints_FL_L.size(), CV_32FC3, &objPointsStatistics2D.glObjPoints_FL_L[0]);
    img_points = cvMat(1, texCoordsStatistics2D.glTexCoord_FL_L.size(), CV_32SC1, &texCoordsStatistics2D.glTexCoord_FL_L[0]);
    projectPoints1(&obj_points, SimplifyleftCamParams.r_vec, SimplifyleftCamParams.t_vec, SimplifyleftCamParams.camera, SimplifyleftCamParams.dist_coeffs, &img_points);
    objPointsStatistics2D.glObjPoints_FL_L.clear();

    // front right blend front part
    obj_points = cvMat(1, objPointsStatistics2D.glObjPoints_BL_L.size(), CV_32FC3, &objPointsStatistics2D.glObjPoints_BL_L[0]);
    img_points = cvMat(1, texCoordsStatistics2D.glTexCoord_BL_L.size(), CV_32SC1, &texCoordsStatistics2D.glTexCoord_BL_L[0]);
    projectPoints1(&obj_points, SimplifyleftCamParams.r_vec, SimplifyleftCamParams.t_vec, SimplifyleftCamParams.camera, SimplifyleftCamParams.dist_coeffs, &img_points);
    objPointsStatistics2D.glObjPoints_BL_L.clear();

    // front left blend front part
    obj_points = cvMat(1, objPointsStatistics2D.glObjPoints_FR_R.size(), CV_32FC3, &objPointsStatistics2D.glObjPoints_FR_R[0]);
    img_points = cvMat(1, texCoordsStatistics2D.glTexCoord_FR_R.size(), CV_32SC1, &texCoordsStatistics2D.glTexCoord_FR_R[0]);
    projectPoints1(&obj_points, SimplifyrightCamParams.r_vec, SimplifyrightCamParams.t_vec, SimplifyrightCamParams.camera, SimplifyrightCamParams.dist_coeffs, &img_points);
    objPointsStatistics2D.glObjPoints_FR_R.clear();

    // front right blend front part
    obj_points = cvMat(1, objPointsStatistics2D.glObjPoints_BR_R.size(), CV_32FC3, &objPointsStatistics2D.glObjPoints_BR_R[0]);
    img_points = cvMat(1, texCoordsStatistics2D.glTexCoord_BR_R.size(), CV_32SC1, &texCoordsStatistics2D.glTexCoord_BR_R[0]);
    projectPoints1(&obj_points, SimplifyrightCamParams.r_vec, SimplifyrightCamParams.t_vec, SimplifyrightCamParams.camera, SimplifyrightCamParams.dist_coeffs, &img_points);
    objPointsStatistics2D.glObjPoints_BR_R.clear();

}


#if 0

void cacaluateRadar(stSdScaleLine radar)
{
    const float PIDIV2 = 1.57079632679489f;
    float i, fp, offset;
    unsigned frontRadius[3], backRadius[3];
    int j, k;

    int halfWorldWidth, halfWorldHeight;
    float x, y, theta, theta1, startTheta;
    float cosTheta1, sinTheta1;

    fp = (float)(1.0) / 32;
    halfWorldHeight = (para_field.carWorldY + para_field.car_length + para_field.carWorldY2) / 2;
    halfWorldWidth = (para_field.carWorldX + para_field.car_width + para_field.carWorldX2) / 2;

    offset = 20;
    frontRadius[0] = sqrtf((halfWorldHeight - para_field.carWorldY) * (halfWorldHeight - para_field.carWorldY) + para_field.car_width * para_field.car_width / 4) + offset;
    frontRadius[1] = (halfWorldHeight - para_field.carWorldY / 2);
    frontRadius[2] = halfWorldHeight - offset;

    backRadius[0] = sqrtf((halfWorldHeight - para_field.carWorldY2) * (halfWorldHeight - para_field.carWorldY2) + para_field.car_width * para_field.car_width / 4) + offset;
    backRadius[1] = (halfWorldHeight - para_field.carWorldY2 / 2);
    backRadius[2] = halfWorldHeight - offset;

    theta = myAtan(1.0 * para_field.car_width / 2 / (halfWorldHeight - para_field.carWorldY));
    startTheta = theta / 2;

    for(j = 0; j < 4; j++)
    {
        if(radar.radarDir[j] != 0)
        {
            for(i = 0, k = 0; i <= 1; i += fp)
            {
                theta1 = (PIDIV2 - theta) + j * startTheta + i * (startTheta);

                cosTheta1 = myCos(theta1);
                sinTheta1 = mySin(theta1);

                x = frontRadius[radar.radarDir[j] - 1] * cosTheta1;
                y = frontRadius[radar.radarDir[j] - 1] * sinTheta1;

                radarScaleLineVerticesPoints[j][k].x = (x / halfWorldWidth);
                radarScaleLineVerticesPoints[j][k].y = (y / halfWorldHeight);
                radarScaleLineVerticesPoints[j][k].z = 0.0;

                k++;


                x = (frontRadius[radar.radarDir[j] - 1] - offset) * cosTheta1;
                y = (frontRadius[radar.radarDir[j] - 1] - offset) * sinTheta1;

                radarScaleLineVerticesPoints[j][k].x = (x / halfWorldWidth);
                radarScaleLineVerticesPoints[j][k].y = (y / halfWorldHeight);
                radarScaleLineVerticesPoints[j][k].z = 0.0;

                k++;
            }
        }
    }

    theta = myAtan(1.0 * para_field.car_width / 2 / (halfWorldHeight - para_field.carWorldY2));
    startTheta = theta / 2;

    for(j = 4; j < 8; j++)
    {
        if(radar.radarDir[j] != 0)
        {
            for(i = 0, k = 0; i <= 1; i += fp)
            {
                theta1 = (-PIDIV2 - theta) + (j - 4) * startTheta + i * (startTheta);

                cosTheta1 = myCos(theta1);
                sinTheta1 = mySin(theta1);

                x = backRadius[radar.radarDir[j] - 1] * cosTheta1;
                y = backRadius[radar.radarDir[j] - 1] * sinTheta1;

                radarScaleLineVerticesPoints[j][k].x = (x / halfWorldWidth);
                radarScaleLineVerticesPoints[j][k].y = (y / halfWorldHeight);
                radarScaleLineVerticesPoints[j][k].z = 0.0;

                k++;

                x = (backRadius[radar.radarDir[j] - 1] - offset) * cosTheta1;
                y = (backRadius[radar.radarDir[j] - 1] - offset) * sinTheta1;

                radarScaleLineVerticesPoints[j][k].x = (x / halfWorldWidth);
                radarScaleLineVerticesPoints[j][k].y = (y / halfWorldHeight);
                radarScaleLineVerticesPoints[j][k].z = 0.0;

                k++;
            }
        }
    }
}

#endif


void initVBO()
{
    int i, j;

    glGenBuffers(4, VBO3DMosaicImageParams.CamVerticesPoints);
    glGenBuffers(4, VBO3DMosaicImageParams.CamImagePoints);

    glGenBuffers(4, VBO3DMosaicImageParams.MosaicCamVerticesPoints);
    glGenBuffers(2, VBO3DMosaicImageParams.MosaicFLCamImagePoints);
    glGenBuffers(2, VBO3DMosaicImageParams.MosaicFRCamImagePoints);
    glGenBuffers(2, VBO3DMosaicImageParams.MosaicBLCamImagePoints);
    glGenBuffers(2, VBO3DMosaicImageParams.MosaicBRCamImagePoints);

    glGenBuffers(4, VBO3DMosaicImageParams.LumiaBalance);

    glGenBuffers(4, VBO3DMosaicImageParams.Alpha);

    glGenBuffers(2, VBO3DMosaicImageParams.CarVerTexCoord);



    glGenBuffers(4, VBO2DMosaicImageParams.CamVerticesPoints);
    glGenBuffers(4, VBO2DMosaicImageParams.CamImagePoints);

    glGenBuffers(4, VBO2DMosaicImageParams.MosaicCamVerticesPoints);
    glGenBuffers(2, VBO2DMosaicImageParams.MosaicFLCamImagePoints);
    glGenBuffers(2, VBO2DMosaicImageParams.MosaicFRCamImagePoints);
    glGenBuffers(2, VBO2DMosaicImageParams.MosaicBLCamImagePoints);
    glGenBuffers(2, VBO2DMosaicImageParams.MosaicBRCamImagePoints);

    glGenBuffers(4, VBO2DMosaicImageParams.LumiaBalance);

    glGenBuffers(4, VBO2DMosaicImageParams.Alpha);

    glGenBuffers(2, VBO2DMosaicImageParams.CarVerTexCoord);

    glGenBuffers(carVertices.size(), VBO3DCarModelParams.vertices);
    glGenBuffers(carTextures.size(), VBO3DCarModelParams.textures);
    glGenBuffers(carNormals.size(), VBO3DCarModelParams.normals);
	glGenBuffers(carTangents.size(), VBO3DCarModelParams.tangents);
	glGenBuffers(carBitTangents.size(), VBO3DCarModelParams.bitTangents);

#if 1
    glGenBuffers(2, curveVerticesPoints);
    glGenBuffers(2, cameraVerTexCoord);
#endif

#if 1
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.CamVerticesPoints[0]);
    glBufferData(GL_ARRAY_BUFFER, texCoords.glTexCoord_F.size() * sizeof(CvPoint3D32f), &vertexCoords.glVertex_F[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.CamVerticesPoints[1]);
    glBufferData(GL_ARRAY_BUFFER, texCoords.glTexCoord_B.size() * sizeof(CvPoint3D32f), &vertexCoords.glVertex_B[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.CamVerticesPoints[2]);
    glBufferData(GL_ARRAY_BUFFER, texCoords.glTexCoord_L.size() * sizeof(CvPoint3D32f), &vertexCoords.glVertex_L[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.CamVerticesPoints[3]);
    glBufferData(GL_ARRAY_BUFFER, texCoords.glTexCoord_R.size() * sizeof(CvPoint3D32f), &vertexCoords.glVertex_R[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.CamImagePoints[0]);
    glBufferData(GL_ARRAY_BUFFER, texCoords.glTexCoord_F.size() * sizeof(CvPoint2D32f), &texCoords.glTexCoord_F[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.CamImagePoints[1]);
    glBufferData(GL_ARRAY_BUFFER, texCoords.glTexCoord_B.size() * sizeof(CvPoint2D32f), &texCoords.glTexCoord_B[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.CamImagePoints[2]);
    glBufferData(GL_ARRAY_BUFFER, texCoords.glTexCoord_L.size() * sizeof(CvPoint2D32f), &texCoords.glTexCoord_L[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.CamImagePoints[3]);
    glBufferData(GL_ARRAY_BUFFER, texCoords.glTexCoord_R.size() * sizeof(CvPoint2D32f), &texCoords.glTexCoord_R[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.MosaicCamVerticesPoints[0]);
    glBufferData(GL_ARRAY_BUFFER, vertexCoords.glVertex_FL.size() * sizeof(CvPoint3D32f), &vertexCoords.glVertex_FL[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.MosaicCamVerticesPoints[1]);
    glBufferData(GL_ARRAY_BUFFER, vertexCoords.glVertex_FR.size() * sizeof(CvPoint3D32f), &vertexCoords.glVertex_FR[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.MosaicCamVerticesPoints[2]);
    glBufferData(GL_ARRAY_BUFFER, vertexCoords.glVertex_BL.size() * sizeof(CvPoint3D32f), &vertexCoords.glVertex_BL[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.MosaicCamVerticesPoints[3]);
    glBufferData(GL_ARRAY_BUFFER, vertexCoords.glVertex_BR.size() * sizeof(CvPoint3D32f), &vertexCoords.glVertex_BR[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.MosaicFLCamImagePoints[0]);
    glBufferData(GL_ARRAY_BUFFER, vertexCoords.glVertex_FL.size() * sizeof(CvPoint2D32f), &texCoords.glTexCoord_FL_F[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.MosaicFLCamImagePoints[1]);
    glBufferData(GL_ARRAY_BUFFER, vertexCoords.glVertex_FL.size() * sizeof(CvPoint2D32f), &texCoords.glTexCoord_FL_L[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.MosaicFRCamImagePoints[0]);
    glBufferData(GL_ARRAY_BUFFER, vertexCoords.glVertex_FR.size() * sizeof(CvPoint2D32f), &texCoords.glTexCoord_FR_F[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.MosaicFRCamImagePoints[1]);
    glBufferData(GL_ARRAY_BUFFER, vertexCoords.glVertex_FR.size() * sizeof(CvPoint2D32f), &texCoords.glTexCoord_FR_R[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.MosaicBLCamImagePoints[0]);
    glBufferData(GL_ARRAY_BUFFER, vertexCoords.glVertex_BL.size() * sizeof(CvPoint2D32f), &texCoords.glTexCoord_BL_B[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.MosaicBLCamImagePoints[1]);
    glBufferData(GL_ARRAY_BUFFER, vertexCoords.glVertex_BL.size() * sizeof(CvPoint2D32f), &texCoords.glTexCoord_BL_L[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.MosaicBRCamImagePoints[0]);
    glBufferData(GL_ARRAY_BUFFER, vertexCoords.glVertex_BR.size() * sizeof(CvPoint2D32f), &texCoords.glTexCoord_BR_B[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.MosaicBRCamImagePoints[1]);
    glBufferData(GL_ARRAY_BUFFER, vertexCoords.glVertex_BR.size() * sizeof(CvPoint2D32f), &texCoords.glTexCoord_BR_R[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.LumiaBalance[0]);
    glBufferData(GL_ARRAY_BUFFER, lumiaAdjust.glLumiaAdjust_F.size() * sizeof(float), &lumiaAdjust.glLumiaAdjust_F[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.LumiaBalance[1]);
    glBufferData(GL_ARRAY_BUFFER, lumiaAdjust.glLumiaAdjust_B.size() * sizeof(float), &lumiaAdjust.glLumiaAdjust_B[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.LumiaBalance[2]);
    glBufferData(GL_ARRAY_BUFFER, lumiaAdjust.glLumiaAdjust_L.size() * sizeof(float), &lumiaAdjust.glLumiaAdjust_L[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.LumiaBalance[3]);
    glBufferData(GL_ARRAY_BUFFER, lumiaAdjust.glLumiaAdjust_R.size() * sizeof(float), &lumiaAdjust.glLumiaAdjust_R[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.Alpha[0]);
    glBufferData(GL_ARRAY_BUFFER, vertexCoords.glVertex_FL.size() * sizeof(float), &blendAlpha.glAlpha_FL[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.Alpha[1]);
    glBufferData(GL_ARRAY_BUFFER, vertexCoords.glVertex_FR.size() * sizeof(float), &blendAlpha.glAlpha_FR[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.Alpha[2]);
    glBufferData(GL_ARRAY_BUFFER, vertexCoords.glVertex_BL.size() * sizeof(float), &blendAlpha.glAlpha_BL[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.Alpha[3]);
    glBufferData(GL_ARRAY_BUFFER, vertexCoords.glVertex_BR.size() * sizeof(float), &blendAlpha.glAlpha_BR[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.CarVerTexCoord[0]);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), glVertices3DCar, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.CarVerTexCoord[1]);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), glTexCoordCar, GL_STATIC_DRAW);
#endif


#if 1
	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CamVerticesPoints[0]);
	glBufferData(GL_ARRAY_BUFFER, texCoords2D.glTexCoord_F.size() * sizeof(CvPoint3D32f), &vertexCoords2D.glVertex_F[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CamVerticesPoints[1]);
	glBufferData(GL_ARRAY_BUFFER, texCoords2D.glTexCoord_B.size() * sizeof(CvPoint3D32f), &vertexCoords2D.glVertex_B[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CamVerticesPoints[2]);
	glBufferData(GL_ARRAY_BUFFER, texCoords2D.glTexCoord_L.size() * sizeof(CvPoint3D32f), &vertexCoords2D.glVertex_L[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CamVerticesPoints[3]);
	glBufferData(GL_ARRAY_BUFFER, texCoords2D.glTexCoord_R.size() * sizeof(CvPoint3D32f), &vertexCoords2D.glVertex_R[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CamImagePoints[0]);
	glBufferData(GL_ARRAY_BUFFER, texCoords2D.glTexCoord_F.size() * sizeof(CvPoint2D32f), &texCoords2D.glTexCoord_F[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CamImagePoints[1]);
	glBufferData(GL_ARRAY_BUFFER, texCoords2D.glTexCoord_B.size() * sizeof(CvPoint2D32f), &texCoords2D.glTexCoord_B[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CamImagePoints[2]);
	glBufferData(GL_ARRAY_BUFFER, texCoords2D.glTexCoord_L.size() * sizeof(CvPoint2D32f), &texCoords2D.glTexCoord_L[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CamImagePoints[3]);
	glBufferData(GL_ARRAY_BUFFER, texCoords2D.glTexCoord_R.size() * sizeof(CvPoint2D32f), &texCoords2D.glTexCoord_R[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.MosaicCamVerticesPoints[0]);
	glBufferData(GL_ARRAY_BUFFER, vertexCoords2D.glVertex_FL.size() * sizeof(CvPoint3D32f), &vertexCoords2D.glVertex_FL[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.MosaicCamVerticesPoints[1]);
	glBufferData(GL_ARRAY_BUFFER, vertexCoords2D.glVertex_FR.size() * sizeof(CvPoint3D32f), &vertexCoords2D.glVertex_FR[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.MosaicCamVerticesPoints[2]);
	glBufferData(GL_ARRAY_BUFFER, vertexCoords2D.glVertex_BL.size() * sizeof(CvPoint3D32f), &vertexCoords2D.glVertex_BL[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.MosaicCamVerticesPoints[3]);
	glBufferData(GL_ARRAY_BUFFER, vertexCoords2D.glVertex_BR.size() * sizeof(CvPoint3D32f), &vertexCoords2D.glVertex_BR[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.MosaicFLCamImagePoints[0]);
	glBufferData(GL_ARRAY_BUFFER, vertexCoords2D.glVertex_FL.size() * sizeof(CvPoint2D32f), &texCoords2D.glTexCoord_FL_F[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.MosaicFLCamImagePoints[1]);
	glBufferData(GL_ARRAY_BUFFER, vertexCoords2D.glVertex_FL.size() * sizeof(CvPoint2D32f), &texCoords2D.glTexCoord_FL_L[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.MosaicFRCamImagePoints[0]);
	glBufferData(GL_ARRAY_BUFFER, vertexCoords2D.glVertex_FR.size() * sizeof(CvPoint2D32f), &texCoords2D.glTexCoord_FR_F[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.MosaicFRCamImagePoints[1]);
	glBufferData(GL_ARRAY_BUFFER, vertexCoords2D.glVertex_FR.size() * sizeof(CvPoint2D32f), &texCoords2D.glTexCoord_FR_R[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.MosaicBLCamImagePoints[0]);
	glBufferData(GL_ARRAY_BUFFER, vertexCoords2D.glVertex_BL.size() * sizeof(CvPoint2D32f), &texCoords2D.glTexCoord_BL_B[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.MosaicBLCamImagePoints[1]);
	glBufferData(GL_ARRAY_BUFFER, vertexCoords2D.glVertex_BL.size() * sizeof(CvPoint2D32f), &texCoords2D.glTexCoord_BL_L[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.MosaicBRCamImagePoints[0]);
	glBufferData(GL_ARRAY_BUFFER, vertexCoords2D.glVertex_BR.size() * sizeof(CvPoint2D32f), &texCoords2D.glTexCoord_BR_B[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.MosaicBRCamImagePoints[1]);
	glBufferData(GL_ARRAY_BUFFER, vertexCoords2D.glVertex_BR.size() * sizeof(CvPoint2D32f), &texCoords2D.glTexCoord_BR_R[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.LumiaBalance[0]);
	glBufferData(GL_ARRAY_BUFFER, lumiaAdjust2D.glLumiaAdjust_F.size() * sizeof(float), &lumiaAdjust2D.glLumiaAdjust_F[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.LumiaBalance[1]);
	glBufferData(GL_ARRAY_BUFFER, lumiaAdjust2D.glLumiaAdjust_B.size() * sizeof(float), &lumiaAdjust2D.glLumiaAdjust_B[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.LumiaBalance[2]);
	glBufferData(GL_ARRAY_BUFFER, lumiaAdjust2D.glLumiaAdjust_L.size() * sizeof(float), &lumiaAdjust2D.glLumiaAdjust_L[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.LumiaBalance[3]);
	glBufferData(GL_ARRAY_BUFFER, lumiaAdjust2D.glLumiaAdjust_R.size() * sizeof(float), &lumiaAdjust2D.glLumiaAdjust_R[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.Alpha[0]);
	glBufferData(GL_ARRAY_BUFFER, vertexCoords2D.glVertex_FL.size() * sizeof(float), &blendAlpha2D.glAlpha_FL[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.Alpha[1]);
	glBufferData(GL_ARRAY_BUFFER, vertexCoords2D.glVertex_FR.size() * sizeof(float), &blendAlpha2D.glAlpha_FR[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.Alpha[2]);
	glBufferData(GL_ARRAY_BUFFER, vertexCoords2D.glVertex_BL.size() * sizeof(float), &blendAlpha2D.glAlpha_BL[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.Alpha[3]);
	glBufferData(GL_ARRAY_BUFFER, vertexCoords2D.glVertex_BR.size() * sizeof(float), &blendAlpha2D.glAlpha_BR[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CarVerTexCoord[0]);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), glVertices2DCar, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CarVerTexCoord[1]);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), glTexCoordCar, GL_STATIC_DRAW);
#endif


#if 1
    for(i = 0; i < carVertices.size(); i++)
    {
        //printf("%d\n",i);
        glBindBuffer(GL_ARRAY_BUFFER, VBO3DCarModelParams.vertices[i]);
        glBufferData(GL_ARRAY_BUFFER, carVertices[i].size() * sizeof(vec3), &carVertices[i][0], GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, VBO3DCarModelParams.textures[i]);
        glBufferData(GL_ARRAY_BUFFER, carTextures[i].size() * sizeof(vec2), &carTextures[i][0], GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, VBO3DCarModelParams.normals[i]);
        glBufferData(GL_ARRAY_BUFFER, carNormals[i].size() * sizeof(vec3), &carNormals[i][0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, VBO3DCarModelParams.tangents[i]);
		glBufferData(GL_ARRAY_BUFFER, carTangents[i].size() * sizeof(vec3), &carTangents[i][0], GL_STATIC_DRAW);
			
		glBindBuffer(GL_ARRAY_BUFFER, VBO3DCarModelParams.bitTangents[i]);
		glBufferData(GL_ARRAY_BUFFER, carBitTangents[i].size() * sizeof(vec3), &carBitTangents[i][0], GL_STATIC_DRAW);
    }
#endif


    printf("Bind Finish\n");
}


void sd3DSetCVMatChnVal(CvMat *srcMat, unsigned int row, unsigned int col, unsigned char k, void *val, int type)
{
    unsigned int elemBytes = CV_ELEM_SIZE(type);
    unsigned char chnBytes = elemBytes / CV_MAT_CN(type);
    void *addr = srcMat->data.ptr + (size_t)srcMat->step * row + elemBytes * col + chnBytes * k;

    memcpy((unsigned char *)addr, (unsigned char *)val, chnBytes);
}

void sd3DSetCVMatDefVal(CvMat *srcMat, void *val, int type)
{
    unsigned int i = 0, j = 0, k = 0;

    for(i = 0; i < srcMat->rows; i++)
    {
        for(j = 0; j < srcMat->cols; j++)
        {
            for(k = 0; k < CV_MAT_CN(type); k++)
            {
                sd3DSetCVMatChnVal(srcMat, i, j, k, val, type);
            }
        }
    }
}



void initCamParaData(int flag)
{
    //para_field.car_width = 225;
    //para_field.car_length = 480;
    //para_field.LRchess2carFront_distance = 130;
    para_field.chessboard_width_corners = 5;
    para_field.chessboard_length_corners = 7;
    para_field.square_size = 20;
    //para_field.carWorldX = 200;
    //para_field.carWorldY = 200;
    //para_field.carWorldX2 = 200;
    //para_field.carWorldY2 = 300;

    readParamsXML(flag);

    float tmpVal32F = 0;

    SimplifyfrontCamParams.r_vec = cvCreateMat(1, 3, CV_32FC1);  //$)Ae$??o<??h=??o??
    cvmSet(SimplifyfrontCamParams.r_vec, 0, 0, (float)SimplifyfrontCamParams.mrInt[0] / SCALE3);
    cvmSet(SimplifyfrontCamParams.r_vec, 0, 1, (float)SimplifyfrontCamParams.mrInt[1] / SCALE3);
    cvmSet(SimplifyfrontCamParams.r_vec, 0, 2, (float)SimplifyfrontCamParams.mrInt[2] / SCALE3);

    SimplifyfrontCamParams.t_vec = cvCreateMat(1, 3, CV_32FC1);  //$)Ae$??o<?93g';e?o??
    cvmSet(SimplifyfrontCamParams.t_vec, 0, 0, (float)SimplifyfrontCamParams.mtInt[0] / SCALE2);
    cvmSet(SimplifyfrontCamParams.t_vec, 0, 1, (float)SimplifyfrontCamParams.mtInt[1] / SCALE2);
    cvmSet(SimplifyfrontCamParams.t_vec, 0, 2, (float)SimplifyfrontCamParams.mtInt[2] / SCALE2);

    SimplifyfrontCamParams.camera = cvCreateMat(3, 3, CV_32FC1); //?$)A8f????
    tmpVal32F = 0;
    sd3DSetCVMatDefVal(SimplifyfrontCamParams.camera, &tmpVal32F, CV_32FC1);
    cvmSet(SimplifyfrontCamParams.camera, 0, 0, (float)SimplifyfrontCamParams.mimdInt[0] / SCALE2); //fx
    cvmSet(SimplifyfrontCamParams.camera, 1, 1, (float)SimplifyfrontCamParams.mimdInt[1] / SCALE2); //fy
    cvmSet(SimplifyfrontCamParams.camera, 0, 2, (float)SimplifyfrontCamParams.mimdInt[2] / SCALE2); //cx
    cvmSet(SimplifyfrontCamParams.camera, 1, 2, (float)SimplifyfrontCamParams.mimdInt[3] / SCALE2); //cy
    cvmSet(SimplifyfrontCamParams.camera, 2, 2, 1.0);

    SimplifyfrontCamParams.dist_coeffs = cvCreateMat(1, 4, CV_32FC1); //?$)A8e?g3;f?
    cvmSet(SimplifyfrontCamParams.dist_coeffs, 0, 0, (float)SimplifyfrontCamParams.mimdInt[4] / SCALE1);
    cvmSet(SimplifyfrontCamParams.dist_coeffs, 0, 1, (float)SimplifyfrontCamParams.mimdInt[5] / SCALE1);
    cvmSet(SimplifyfrontCamParams.dist_coeffs, 0, 2, (float)SimplifyfrontCamParams.mimdInt[6] / SCALE1);
    cvmSet(SimplifyfrontCamParams.dist_coeffs, 0, 3, (float)SimplifyfrontCamParams.mimdInt[7] / SCALE1);


    SimplifyrearCamParams.r_vec = cvCreateMat(1, 3, CV_32FC1);  //$)Ae$??o<??h=??o??
    cvmSet(SimplifyrearCamParams.r_vec, 0, 0, (float)SimplifyrearCamParams.mrInt[0] / SCALE3);
    cvmSet(SimplifyrearCamParams.r_vec, 0, 1, (float)SimplifyrearCamParams.mrInt[1] / SCALE3);
    cvmSet(SimplifyrearCamParams.r_vec, 0, 2, (float)SimplifyrearCamParams.mrInt[2] / SCALE3);

    SimplifyrearCamParams.t_vec = cvCreateMat(1, 3, CV_32FC1);  //$)Ae$??o<?93g';e?o??
    cvmSet(SimplifyrearCamParams.t_vec, 0, 0, (float)SimplifyrearCamParams.mtInt[0] / SCALE2);
    cvmSet(SimplifyrearCamParams.t_vec, 0, 1, (float)SimplifyrearCamParams.mtInt[1] / SCALE2);
    cvmSet(SimplifyrearCamParams.t_vec, 0, 2, (float)SimplifyrearCamParams.mtInt[2] / SCALE2);

    SimplifyrearCamParams.camera = cvCreateMat(3, 3, CV_32FC1); //?$)A8f????
    tmpVal32F = 0;
    sd3DSetCVMatDefVal(SimplifyrearCamParams.camera, &tmpVal32F, CV_32FC1);
    cvmSet(SimplifyrearCamParams.camera, 0, 0, (float)SimplifyrearCamParams.mimdInt[0] / SCALE2); //fx
    cvmSet(SimplifyrearCamParams.camera, 1, 1, (float)SimplifyrearCamParams.mimdInt[1] / SCALE2); //fy
    cvmSet(SimplifyrearCamParams.camera, 0, 2, (float)SimplifyrearCamParams.mimdInt[2] / SCALE2); //cx
    cvmSet(SimplifyrearCamParams.camera, 1, 2, (float)SimplifyrearCamParams.mimdInt[3] / SCALE2); //cy
    cvmSet(SimplifyrearCamParams.camera, 2, 2, 1.0);

    SimplifyrearCamParams.dist_coeffs = cvCreateMat(1, 4, CV_32FC1); //?$)A8e?g3;f?
    cvmSet(SimplifyrearCamParams.dist_coeffs, 0, 0, (float)SimplifyrearCamParams.mimdInt[4] / SCALE1);
    cvmSet(SimplifyrearCamParams.dist_coeffs, 0, 1, (float)SimplifyrearCamParams.mimdInt[5] / SCALE1);
    cvmSet(SimplifyrearCamParams.dist_coeffs, 0, 2, (float)SimplifyrearCamParams.mimdInt[6] / SCALE1);
    cvmSet(SimplifyrearCamParams.dist_coeffs, 0, 3, (float)SimplifyrearCamParams.mimdInt[7] / SCALE1);


    SimplifyleftCamParams.r_vec = cvCreateMat(1, 3, CV_32FC1);  //$)Ae$??o<??h=??o??
    cvmSet(SimplifyleftCamParams.r_vec, 0, 0, (float)SimplifyleftCamParams.mrInt[0] / SCALE3);
    cvmSet(SimplifyleftCamParams.r_vec, 0, 1, (float)SimplifyleftCamParams.mrInt[1] / SCALE3);
    cvmSet(SimplifyleftCamParams.r_vec, 0, 2, (float)SimplifyleftCamParams.mrInt[2] / SCALE3);

    SimplifyleftCamParams.t_vec = cvCreateMat(1, 3, CV_32FC1);  //$)Ae$??o<?93g';e?o??
    cvmSet(SimplifyleftCamParams.t_vec, 0, 0, (float)SimplifyleftCamParams.mtInt[0] / SCALE2);
    cvmSet(SimplifyleftCamParams.t_vec, 0, 1, (float)SimplifyleftCamParams.mtInt[1] / SCALE2);
    cvmSet(SimplifyleftCamParams.t_vec, 0, 2, (float)SimplifyleftCamParams.mtInt[2] / SCALE2);

    SimplifyleftCamParams.camera = cvCreateMat(3, 3, CV_32FC1); //?$)A8f????
    tmpVal32F = 0;
    sd3DSetCVMatDefVal(SimplifyleftCamParams.camera, &tmpVal32F, CV_32FC1);
    cvmSet(SimplifyleftCamParams.camera, 0, 0, (float)SimplifyleftCamParams.mimdInt[0] / SCALE2); //fx
    cvmSet(SimplifyleftCamParams.camera, 1, 1, (float)SimplifyleftCamParams.mimdInt[1] / SCALE2); //fy
    cvmSet(SimplifyleftCamParams.camera, 0, 2, (float)SimplifyleftCamParams.mimdInt[2] / SCALE2); //cx
    cvmSet(SimplifyleftCamParams.camera, 1, 2, (float)SimplifyleftCamParams.mimdInt[3] / SCALE2); //cy
    cvmSet(SimplifyleftCamParams.camera, 2, 2, 1.0);

    SimplifyleftCamParams.dist_coeffs = cvCreateMat(1, 4, CV_32FC1); //?$)A8e?g3;f?
    cvmSet(SimplifyleftCamParams.dist_coeffs, 0, 0, (float)SimplifyleftCamParams.mimdInt[4] / SCALE1);
    cvmSet(SimplifyleftCamParams.dist_coeffs, 0, 1, (float)SimplifyleftCamParams.mimdInt[5] / SCALE1);
    cvmSet(SimplifyleftCamParams.dist_coeffs, 0, 2, (float)SimplifyleftCamParams.mimdInt[6] / SCALE1);
    cvmSet(SimplifyleftCamParams.dist_coeffs, 0, 3, (float)SimplifyleftCamParams.mimdInt[7] / SCALE1);


    SimplifyrightCamParams.r_vec = cvCreateMat(1, 3, CV_32FC1);  //$)Ae$??o<??h=??o??
    cvmSet(SimplifyrightCamParams.r_vec, 0, 0, (float)SimplifyrightCamParams.mrInt[0] / SCALE3);
    cvmSet(SimplifyrightCamParams.r_vec, 0, 1, (float)SimplifyrightCamParams.mrInt[1] / SCALE3);
    cvmSet(SimplifyrightCamParams.r_vec, 0, 2, (float)SimplifyrightCamParams.mrInt[2] / SCALE3);

    SimplifyrightCamParams.t_vec = cvCreateMat(1, 3, CV_32FC1);  //$)Ae$??o<?93g';e?o??
    cvmSet(SimplifyrightCamParams.t_vec, 0, 0, (float)SimplifyrightCamParams.mtInt[0] / SCALE2);
    cvmSet(SimplifyrightCamParams.t_vec, 0, 1, (float)SimplifyrightCamParams.mtInt[1] / SCALE2);
    cvmSet(SimplifyrightCamParams.t_vec, 0, 2, (float)SimplifyrightCamParams.mtInt[2] / SCALE2);

    SimplifyrightCamParams.camera = cvCreateMat(3, 3, CV_32FC1); //?$)A8f????
    tmpVal32F = 0;
    sd3DSetCVMatDefVal(SimplifyrightCamParams.camera, &tmpVal32F, CV_32FC1);
    cvmSet(SimplifyrightCamParams.camera, 0, 0, (float)SimplifyrightCamParams.mimdInt[0] / SCALE2); //fx
    cvmSet(SimplifyrightCamParams.camera, 1, 1, (float)SimplifyrightCamParams.mimdInt[1] / SCALE2); //fy
    cvmSet(SimplifyrightCamParams.camera, 0, 2, (float)SimplifyrightCamParams.mimdInt[2] / SCALE2); //cx
    cvmSet(SimplifyrightCamParams.camera, 1, 2, (float)SimplifyrightCamParams.mimdInt[3] / SCALE2); //cy
    cvmSet(SimplifyrightCamParams.camera, 2, 2, 1.0);

    SimplifyrightCamParams.dist_coeffs = cvCreateMat(1, 4, CV_32FC1); //?$)A8e?g3;f?
    cvmSet(SimplifyrightCamParams.dist_coeffs, 0, 0, (float)SimplifyrightCamParams.mimdInt[4] / SCALE1);
    cvmSet(SimplifyrightCamParams.dist_coeffs, 0, 1, (float)SimplifyrightCamParams.mimdInt[5] / SCALE1);
    cvmSet(SimplifyrightCamParams.dist_coeffs, 0, 2, (float)SimplifyrightCamParams.mimdInt[6] / SCALE1);
    cvmSet(SimplifyrightCamParams.dist_coeffs, 0, 3, (float)SimplifyrightCamParams.mimdInt[7] / SCALE1);
}





