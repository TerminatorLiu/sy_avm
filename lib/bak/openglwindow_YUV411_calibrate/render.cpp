/*************************************************************************/ /*!
@File           gles2test1.c
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/
#include <GLES3/gl3.h>
#include <EGL/egl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <vector>

#include "avmInit.hpp"

#include "esTransform.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


static int hProgramHandle[5];

safImgRect gAllView;
safImgRect gSingleView;

GLuint textureY[5];
GLuint textureU[5];
GLuint textureV[5];

GLuint textureRes[4];

float lumiaAve[12];
vec3 colorAve[12];  
vec3 colorCount[AVERAGE_COUNT][12];  

vec3 *verCoordPointFront;
vec2 *texCoordPointFront, *imgCoordPointFront;

vec3 *verCoordPointBack;
vec2 *texCoordPointBack, *imgCoordPointBack;

static int verCountFront, verCountBack;

vec3 verticesRearTrajLinePoint[11][LENGTH * 2];

GLuint bindTexture(GLuint texture, unsigned char *buffer, GLuint w , GLuint h, GLint type)
{
    glBindTexture ( GL_TEXTURE_2D, texture );
    glTexImage2D ( GL_TEXTURE_2D, 0, type, w, h, 0, type, GL_UNSIGNED_BYTE, buffer);
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    return texture;
}


#if 1
char vertexSource[] =
{
    "#version 300 es						  \n"
    "layout(location = 0) in vec4 av4position;              \n"
    "layout(location = 1) in vec2 av2texcoord;              \n"
    "layout(location = 2) in float coeff;              \n"
    "uniform mat4 mvp;                        \n"
    "smooth centroid out vec2 vv2texcoord;                \n"
    "out float vCoeff;                \n"
    "void main()                            \n"
    "{                                        \n"
    "    vv2texcoord = av2texcoord;           \n"
    "    vCoeff = coeff;           \n"
    "    gl_Position = mvp * av4position;     \n"
    "}                                        \n"
};

char fragmentSource[] =
{
    "#version 300 es						  \n"
    "precision mediump float;                                      \n"
    "uniform sampler2D textureY;                               \n"
    "uniform sampler2D textureU;                              \n"
    "uniform sampler2D textureV;                              \n"
    "uniform vec3 color0Adjust;                                   \n"
    "uniform vec3 color1Adjust;                                   \n"
    "smooth centroid in vec2 vv2texcoord;                        \n"
    "in float vCoeff;                					 	 \n"
    "out vec4 fragColor;                          				 \n"
    "void main()                                                 \n"
    "{                                                           \n"
    "    vec3 yuv;                                       \n"
    "    vec3 rgb;                                       \n"
    "    float lumia = texture(textureY, vv2texcoord).r;     \n"
    "    float cr = texture(textureU, vv2texcoord).r - 0.5; \n"
    "	 float cb = texture(textureV, vv2texcoord).r - 0.5; \n"
    "	 yuv = vec3(lumia, cb, cr);                              \n"
    "	 rgb = yuv * mat3(1, 1.779, 0,                           \n"
    "                     1, -0.3455, -0.7169,                   \n"
    "                     1, 0, 1.4075);                         \n"
    "  	 vec3 gain = color0Adjust * (1.0 - vCoeff) + color1Adjust * vCoeff;      \n"
    "    fragColor = vec4(rgb + gain, 1.0);        \n"
    "}                                                           \n"

};

char vertexMosaicSource[] =
{
    "#version 300 es						  \n"
    "layout(location = 0) in vec4 av4position;                      \n"
    "layout(location = 1) in vec2 av2texcoord1;                     \n"
    "layout(location = 2) in vec2 av2texcoord2;                     \n"
    "layout(location = 3) in float avalpha;                         \n"
    "uniform mat4 mvp;                                              \n"
    "smooth centroid out vec2 vv2texcoord1;                         \n"
    "smooth centroid out vec2 vv2texcoord2;                         \n"
    "out float vvalpha;                                         	\n"
    "void main()                                                    \n"
    "{                                                              \n"
    "    vv2texcoord1 = av2texcoord1;                               \n"
    "    vv2texcoord2 = av2texcoord2;                               \n"
    "    vvalpha = avalpha;                                         \n"
    "    gl_Position = mvp * av4position;                           \n"
    "}                                                              \n"
};

char fragmentMosaicSource[] =
{
    "#version 300 es						  \n"
    "precision mediump float;                                         \n"
    "uniform sampler2D texture1Y;                               \n"
    "uniform sampler2D texture1U;                              \n"
    "uniform sampler2D texture1V;                              \n"
    "uniform sampler2D texture2Y;                               \n"
    "uniform sampler2D texture2U;                              \n"
    "uniform sampler2D texture2V;                              \n"
    "uniform vec3 color0Adjust;                                   \n"
    "uniform vec3 color1Adjust;                                   \n"
    "smooth centroid in vec2 vv2texcoord1;                          \n"
    "smooth centroid in vec2 vv2texcoord2;                          \n"
    "in float vvalpha;                                         		\n"
    "out vec4 fragColor;                          					\n"
    "void main()                                                    \n"
    "{                                                              \n"
    "    vec3 yuv1, yuv2;                                       \n"
    "    vec3 rgb1, rgb2;                                       \n"
    "    float lumia, cb, cr;                                       \n"
    "    lumia = texture(texture1Y, vv2texcoord1).r;     \n"
    "    cr = texture(texture1U, vv2texcoord1).r - 0.5; \n"
    "	 cb= texture(texture1V, vv2texcoord1).r - 0.5; \n"
    "	 yuv1 = vec3(lumia, cb, cr);                              \n"
    "    lumia = texture(texture2Y, vv2texcoord2).r;     \n"
    "    cr = texture(texture2U, vv2texcoord2).r - 0.5; \n"
    "	 cb = texture(texture2V, vv2texcoord2).r - 0.5; \n"
    "	 yuv2 = vec3(lumia, cb, cr);                              \n"
    "	 rgb1 = yuv1 * mat3(1, 1.779, 0,                           \n"
    "                     1, -0.3455, -0.7169,                   \n"
    "                     1, 0, 1.4075);                         \n"
    "	 rgb2 = yuv2 * mat3(1, 1.779, 0,                           \n"
    "                     1, -0.3455, -0.7169,                   \n"
    "                     1, 0, 1.4075);                         \n"
    "    fragColor = vec4(mix(rgb1 + color0Adjust, rgb2 + color1Adjust, vvalpha), 1.0);\n"
    //"    fragColor = vec4(mix(rgb1 , rgb2 , vvalpha), 1.0);\n"
    "}                                                              \n"
};

#if 1
char vertexBlendSource[] =
{
	"#version 300 es										\n"
	"layout(location = 0) in vec4 vPosition;			  \n"
	"uniform mat4 mvp;										\n"
	"void main()											\n"
	"{														\n"
	"	 gl_Position = mvp * vPosition;		 \n"
	"}														\n"
};

char fragmentBlendSource[] =
{
	"#version 300 es											\n"
	"precision mediump float; 									\n"
	"uniform vec4 outColor; 					  \n"
	"out vec4 fragColor;										\n"
	"void main()												\n"
	"{															\n"
	"	 fragColor = outColor;			\n"
	"}															\n"
};

#endif


char vertexBmpShowSource[] =
{
    "#version 300 es						  				\n"
    "layout(location = 0) in vec4 av4position;              \n"
    "layout(location = 1) in vec2 av2texcoord;              \n"
    "uniform mat4 mvp;                        				\n"
    "smooth centroid out vec2 vv2texcoord;                	\n"
    "void main()                              				\n"
    "{                                        				\n"
    "    vv2texcoord = av2texcoord;           				\n"
    "    gl_Position = mvp * av4position;     				\n"
    "}                                        				\n"
};

char fragmentBmpShowSource[] =
{
    "#version 300 es						  					\n"
    "precision mediump float;                                     \n"
    "uniform sampler2D textureImg;                            \n"
    "smooth centroid in vec2 vv2texcoord;                       \n"
    "out vec4 fragColor;                          				\n"
    "void main()                                                \n"
    "{                                                          \n"
    "    fragColor = texture(textureImg, vv2texcoord);       	\n"
    "}                                                          \n"
};

#endif


char vertexCameraSource[] =
{
    "#version 300 es						  \n"
    "layout(location = 0) in vec4 av4position;              \n"
    "layout(location = 1) in vec2 av2texcoord;              \n"
    "uniform mat4 mvp;                        \n"
    "smooth centroid out vec2 vv2texcoord;                \n"
    "void main()                            \n"
    "{                                        \n"
    "    vv2texcoord = av2texcoord;           \n"
    "    gl_Position = mvp * av4position;     \n"
    "}                                        \n"
};

char fragmentCameraSource[] =
{
    "#version 300 es						  \n"
    "precision mediump float;                                      \n"
    "uniform sampler2D textureY;                               \n"
    "uniform sampler2D textureU;                              \n"
    "uniform sampler2D textureV;                              \n"
    "smooth centroid in vec2 vv2texcoord;                        \n"
    "in float vCoeff;                					 	 \n"
    "out vec4 fragColor;                          				 \n"
    "void main()                                                 \n"
    "{                                                           \n"
    "    vec3 yuv;                                       \n"
    "    vec3 rgb;                                       \n"
    "    float lumia = texture(textureY, vv2texcoord).r;     \n"
    "    float cr = texture(textureU, vv2texcoord).r - 0.5; \n"
    "	 float cb= texture(textureV, vv2texcoord).r - 0.5; \n"
    "	 yuv = vec3(lumia, cb, cr);                              \n"
    "	 rgb = yuv * mat3(1, 1.779, 0,                           \n"
    "                     1, -0.3455, -0.7169,                   \n"
    "                     1, 0, 1.4075);                         \n"
    "    fragColor = vec4(rgb , 1.0);        \n"
    "}                                                           \n"

};






void caculateColorCoeff2D(unsigned char **imageBuffer)
{

    int i;
    unsigned int lumiaSum0, lumiaSum1;

    int index0, index1;
    vec3 rgbColor[12];

    unsigned int count;
    static unsigned int runCount = 0;
    int index;


    index = runCount % AVERAGE_COUNT;

    lumiaSum0 = 0;
	lumiaSum1 = 0;

    for(i = 0; i < texCoordsStatistics2D.glTexCoord_FL_F.size(); i++)
    {
        index0 = texCoordsStatistics2D.glTexCoord_FL_F[i];
		lumiaSum0 += imageBuffer[0][index0];

        index1 = texCoordsStatistics2D.glTexCoord_FL_L[i];
        lumiaSum1 += imageBuffer[2][index1];
    }

    count = texCoordsStatistics2D.glTexCoord_FL_F.size();
    rgbColor[0].x = 1.0 * lumiaSum0 / count;
    rgbColor[0].y = 1.0 * lumiaSum0 / count;
    rgbColor[0].z = 1.0 * lumiaSum0 / count;

    rgbColor[1].x = 1.0 * lumiaSum1 / count;
    rgbColor[1].y = 1.0 * lumiaSum1 / count;
    rgbColor[1].z = 1.0 * lumiaSum1 / count;


    lumiaSum0 = 0;
	lumiaSum1 = 0;

    for(i = 0; i < texCoordsStatistics2D.glTexCoord_FR_F.size(); i++)
    {
        index0 = texCoordsStatistics2D.glTexCoord_FR_F[i];
        lumiaSum0 += imageBuffer[0][index0];

        index1 = texCoordsStatistics2D.glTexCoord_FR_R[i];
        lumiaSum1 += imageBuffer[3][index1];
    }

    count = texCoordsStatistics2D.glTexCoord_FR_F.size();

    rgbColor[2].x = 1.0 * lumiaSum0 / count;
    rgbColor[2].y = 1.0 * lumiaSum0 / count;
    rgbColor[2].z = 1.0 * lumiaSum0 / count;

    rgbColor[3].x = 1.0 * lumiaSum1 / count;
    rgbColor[3].y = 1.0 * lumiaSum1 / count;
    rgbColor[3].z = 1.0 * lumiaSum1 / count;


    lumiaSum0 = 0;
	lumiaSum1 = 0;

    for(i = 0; i < texCoordsStatistics2D.glTexCoord_BL_B.size(); i++)
    {
        index0 = texCoordsStatistics2D.glTexCoord_BL_B[i];
        lumiaSum0 += imageBuffer[1][index0];

        index1 = texCoordsStatistics2D.glTexCoord_BL_L[i];
        lumiaSum1 += imageBuffer[2][index1];
    }

    count = texCoordsStatistics2D.glTexCoord_BL_B.size();

    rgbColor[4].x = 1.0 * lumiaSum0 / count;
    rgbColor[4].y = 1.0 * lumiaSum0 / count;
    rgbColor[4].z = 1.0 * lumiaSum0 / count;

    rgbColor[5].x = 1.0 * lumiaSum1 / count;
    rgbColor[5].y = 1.0 * lumiaSum1 / count;
    rgbColor[5].z = 1.0 * lumiaSum1 / count;


	lumiaSum0 = 0;
	lumiaSum1 = 0;

    for(i = 0; i < texCoordsStatistics2D.glTexCoord_BR_B.size(); i++)
    {
        index0 = texCoordsStatistics2D.glTexCoord_BR_B[i];
        lumiaSum0 += imageBuffer[1][index0];


        index1 = texCoordsStatistics2D.glTexCoord_BR_R[i];
        lumiaSum1 += imageBuffer[3][index1];
    }

    count = texCoordsStatistics2D.glTexCoord_BR_B.size();

    rgbColor[6].x = 1.0 * lumiaSum0 / count;
    rgbColor[6].y = 1.0 * lumiaSum0 / count;
    rgbColor[6].z = 1.0 * lumiaSum0 / count;

    rgbColor[7].x = 1.0 * lumiaSum1 / count;
    rgbColor[7].y = 1.0 * lumiaSum1 / count;
    rgbColor[7].z = 1.0 * lumiaSum1 / count;

	for(i = 0; i < 8; i++)
	{
		colorAve[i].x = rgbColor[i].x / 255.0;
		colorAve[i].y = rgbColor[i].y / 255.0;
		colorAve[i].z = rgbColor[i].z / 255.0;
	}

    colorAve[8].x = (colorAve[0].x + colorAve[1].x) / 2;
    colorAve[8].y = (colorAve[0].y + colorAve[1].y) / 2;
    colorAve[8].z = (colorAve[0].z + colorAve[1].z) / 2;

    colorAve[9].x = (colorAve[2].x + colorAve[3].x) / 2;
    colorAve[9].y = (colorAve[2].y + colorAve[3].y) / 2;
    colorAve[9].z = (colorAve[2].z + colorAve[3].z) / 2;

    colorAve[10].x = (colorAve[4].x + colorAve[5].x) / 2;
    colorAve[10].y = (colorAve[4].y + colorAve[5].y) / 2;
    colorAve[10].z = (colorAve[4].z + colorAve[5].z) / 2;

    colorAve[11].x = (colorAve[6].x + colorAve[7].x) / 2;
    colorAve[11].y = (colorAve[6].y + colorAve[7].y) / 2;
    colorAve[11].z = (colorAve[6].z + colorAve[7].z) / 2;

}


unsigned char *yuvBuffer[5];

void loadRes(int w, int h)
{
    int i;

    int width, height, channel;
	//unsigned char *yuvBuffer[5];
	unsigned char *resBuffer[4];
	unsigned char *pSrcY, *pSrcU, *pSrcV;
	char filePath[256] = {0};
	FILE *fp;

	for(i = 0; i < 4; i++)
    {
        yuvBuffer[i] = (unsigned char *)malloc(w * h * 3 / 2);
		sprintf(filePath, "./test/%d.yuv", i);
		fp = fopen(filePath, "rb");
            fread(yuvBuffer[i], 1, w*h*3/2, fp);
            fclose(fp);

        pSrcY = yuvBuffer[i];
        pSrcU = pSrcY + w * h;
        pSrcV = pSrcU + w * h / 4;

        glGenTextures(1, &textureY[i]);
        bindTexture(textureY[i], pSrcY, w, h, GL_LUMINANCE);

        glGenTextures(1, &textureU[i]);
        bindTexture(textureU[i], pSrcU, w / 2, h / 2, GL_LUMINANCE);

        glGenTextures(1, &textureV[i]);
        bindTexture(textureV[i], pSrcV, w / 2, h / 2, GL_LUMINANCE);
    }


	stbi_set_flip_vertically_on_load(0);

	resBuffer[1] = stbi_load("./test/car.png", &width, &height, &channel, 0);
    glGenTextures( 1, &textureRes[0]);
    bindTexture(textureRes[0], resBuffer[1], width, height, GL_RGBA);

#if 0
   	pu8CamMBvir[0] = yuvBuffer[0];
   	pu8CamMBvir[1] = yuvBuffer[1];
   	pu8CamMBvir[2] = yuvBuffer[2];
   	pu8CamMBvir[3] = yuvBuffer[3];

   	cameraCalib(1046, 370, 57);
#endif
       
}


#if 1
GLuint esLoadShader ( GLenum type, const char *shaderSrc )
{
   GLuint shader;
   GLint compiled;
   char *infoLog;

   // Create the shader object
   shader = glCreateShader ( type );

   if ( shader == 0 )
   {
      return 0;
   }

   // Load the shader source
   glShaderSource ( shader, 1, &shaderSrc, NULL );

   // Compile the shader
   glCompileShader ( shader );

   // Check the compile status
   glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

   if ( !compiled )
   {
      GLint infoLen = 0;

      glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );

      if ( infoLen > 1 )
      {
         infoLog = (char*)malloc ( sizeof ( char ) * infoLen );

         glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
         printf ( "Error compiling shader:\n%s\n", infoLog );

         free ( infoLog );
      }

      glDeleteShader ( shader );
      return 0;
   }

   return shader;

}


GLuint esLoadProgram ( const char *vertShaderSrc, const char *fragShaderSrc )
{
   GLuint vertexShader;
   GLuint fragmentShader;
   GLuint programObject;
   GLint linked;
   char *infoLog;

   // Load the vertex/fragment shaders
   vertexShader = esLoadShader ( GL_VERTEX_SHADER, vertShaderSrc );

   if ( vertexShader == 0 )
   {
      return 0;
   }

   fragmentShader = esLoadShader ( GL_FRAGMENT_SHADER, fragShaderSrc );

   if ( fragmentShader == 0 )
   {
      glDeleteShader ( vertexShader );
      return 0;
   }

   // Create the program object
   programObject = glCreateProgram ( );

   if ( programObject == 0 )
   {
      return 0;
   }

   glAttachShader ( programObject, vertexShader );
   glAttachShader ( programObject, fragmentShader );

   // Link the program
   glLinkProgram ( programObject );

   // Check the link status
   glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

   if ( !linked )
   {
      GLint infoLen = 0;

      glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );

      if ( infoLen > 1 )
      {
         infoLog = (char*)malloc ( sizeof ( char ) * infoLen );
         
         glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
         printf ( "Error linking program:\n%s\n", infoLog );

         free ( infoLog );
      }

      glDeleteProgram ( programObject );
      return 0;
   }

   // Free up no longer needed shader resources
   glDeleteShader ( vertexShader );
   glDeleteShader ( fragmentShader );

   return programObject;
}

#endif



static void setBool(GLuint programId, const char *name, bool value) 
{
    glUniform1i(glGetUniformLocation(programId, name), (int) value);
}


static void setInt(GLuint programId, const char *name, int value) 
{
    glUniform1i(glGetUniformLocation(programId, name), value);
}

static void setFloat(GLuint programId, const char *name, float value) 
{
    glUniform1f(glGetUniformLocation(programId, name), value);
}

static void setVec2(GLuint programId, const char *name, const vec2 value) 
{
    glUniform2fv(glGetUniformLocation(programId, name), 1, &value.x);
}

static void setVec2(GLuint programId, const char *name, float x, float y) 
{
    glUniform2f(glGetUniformLocation(programId, name), x, y);
}

static void setVec3(GLuint programId, const char *name, const vec3 value) 
{
    glUniform3fv(glGetUniformLocation(programId, name), 1, &value.x);
}

static void setVec3(GLuint programId, const char *name, float x, float y, float z) 
{
    glUniform3f(glGetUniformLocation(programId, name), x, y, z);
}

static void setVec4(GLuint programId, const char *name, const vec4 value) 
{
    glUniform4fv(glGetUniformLocation(programId, name), 1, &value.r);
}

static void setVec4(GLuint programId, const char *name, float x, float y, float z, float w) 
{
    glUniform4f(glGetUniformLocation(programId, name), x, y, z, w);
}



static void setMat4(GLuint programId, const char *name, ESMatrix mat) 
{
    glUniformMatrix4fv(glGetUniformLocation(programId, name), 1, GL_FALSE, &mat.m[0][0]);
}




void rotationMatrixToEulerAngle(float *R, float *alpha, float *beta, float *gamma)
{
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
}

#if 1
double towFac(double *b) { return b[0] * b[3] - b[1] * b[2]; }

void copy3To2(double *a, double *b, int i, int j) {
  int m = 0, n = 0;
  int count = 0;
  for (m = 0; m < 3; m++) {
    for (n = 0; n < 3; n++) {
      if (m != i && n != j) {
        count++;
        b[((count - 1) / 2) * 2 + (count - 1) % 2] = a[m * 3 + n];
      }
    }
  }
}

double threeSum(double *a, double *b) {
  double sum = 0;
  for (int i = 0; i < 3; i++) {
    copy3To2(a, b, 0, i);
    if (i % 2 == 0) {
      sum += a[i] * towFac(b);
    } else {
      sum -= a[i] * towFac(b);
    }
  }
  return sum;
}

void calCArray(double *a, double *b, double *c) {
  int i = 0;
  int j = 0;
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      copy3To2(a, b, i, j);
      if ((i + j) % 2 == 0) {
        c[j * 3 + i] = towFac(b);
      } else {
        c[j * 3 + i] = -towFac(b);
      }
    }
  }
}

void niArray(double *c, double A) {
  int i = 0;
  int j = 0;
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      c[i * 3 + j] /= A;
    }
  }
}

void invertMatrix(double *src, double *dst) {

  double b[2][2];
  double t = threeSum(src, &b[0][0]);
  calCArray(src, &b[0][0], dst);
  niArray(dst, t);
}
#endif


/*====================================================================
?$)A=f???'0:   initShader
?$)A=f????:   ??;:???2e?
$)Ag.??e.??:
?$)A(e????:
$)Ah>?????:   ??
$)Ah???g;??:   ??
$)Ad???h.0e?o<?
====================================================================*/
static int initShader(void)
{
    hProgramHandle[0] = esLoadProgram ( vertexSource, fragmentSource );
    hProgramHandle[1] = esLoadProgram ( vertexMosaicSource, fragmentMosaicSource );
    hProgramHandle[2] = esLoadProgram ( vertexBmpShowSource, fragmentBmpShowSource );
	hProgramHandle[3] = esLoadProgram ( vertexBlendSource, fragmentBlendSource );
	hProgramHandle[4] = esLoadProgram ( vertexCameraSource, fragmentCameraSource );

    printf("%d %d %d %d %d\n",hProgramHandle[0],hProgramHandle[1],hProgramHandle[2],hProgramHandle[3],hProgramHandle[4]);

    return GL_TRUE;
}



void show2DCar()
{
	vec3 colorAdjust[2];
	ESMatrix matrixMVP;

	esMatrixLoadIdentity ( &matrixMVP );	 

	glViewport(gAllView.x, gAllView.y, gAllView.width, gAllView.height);
	
#if 1
    glUseProgram(hProgramHandle[0]);
    
    /* Enable attributes for position, color and texture coordinates etc. */
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

	setMat4(hProgramHandle[0], "mvp", matrixMVP); 

    //Front
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CamVerticesPoints[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CamImagePoints[0]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.LumiaBalance[0]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);

    colorAdjust[0].x = colorAve[8].x - colorAve[0].x;
    colorAdjust[0].y = colorAve[8].y - colorAve[0].y;
    colorAdjust[0].z = colorAve[8].z - colorAve[0].z;
    
    colorAdjust[1].x = colorAve[9].x - colorAve[2].x;
    colorAdjust[1].y = colorAve[9].y - colorAve[2].y;
    colorAdjust[1].z = colorAve[9].z - colorAve[2].z;
    
    setVec3(hProgramHandle[0], "color0Adjust", colorAdjust[0]); 
    setVec3(hProgramHandle[0], "color1Adjust", colorAdjust[1]); 

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureY[0]);
    setInt(hProgramHandle[0], "textureY", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureU[0]);
    setInt(hProgramHandle[0], "textureU", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureV[0]);
    setInt(hProgramHandle[0], "textureV", 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCoords2D.glVertex_F.size());//GL_TRIANGLE_STRIP


    //back
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CamVerticesPoints[1]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CamImagePoints[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.LumiaBalance[1]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);

    colorAdjust[0].x = colorAve[10].x - colorAve[4].x;
    colorAdjust[0].y = colorAve[10].y - colorAve[4].y;
    colorAdjust[0].z = colorAve[10].z - colorAve[4].z;
    
    colorAdjust[1].x = colorAve[11].x - colorAve[6].x;
    colorAdjust[1].y = colorAve[11].y - colorAve[6].y;
    colorAdjust[1].z = colorAve[11].z - colorAve[6].z;
    
    setVec3(hProgramHandle[0], "color0Adjust", colorAdjust[0]); 
    setVec3(hProgramHandle[0], "color1Adjust", colorAdjust[1]); 

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureY[1]);
    setInt(hProgramHandle[0], "textureY", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureU[1]);
    setInt(hProgramHandle[0], "textureU", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureV[1]);
    setInt(hProgramHandle[0], "textureV", 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCoords2D.glVertex_B.size());

    //left
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CamVerticesPoints[2]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CamImagePoints[2]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.LumiaBalance[2]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);

    colorAdjust[0].x = colorAve[8].x - colorAve[1].x;
	colorAdjust[0].y = colorAve[8].y - colorAve[1].y;
	colorAdjust[0].z = colorAve[8].z - colorAve[1].z;
	
	colorAdjust[1].x = colorAve[10].x - colorAve[5].x;
	colorAdjust[1].y = colorAve[10].y - colorAve[5].y;
	colorAdjust[1].z = colorAve[10].z - colorAve[5].z;
	
    setVec3(hProgramHandle[0], "color0Adjust", colorAdjust[0]); 
	setVec3(hProgramHandle[0], "color1Adjust", colorAdjust[1]); 

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureY[2]);
    setInt(hProgramHandle[0], "textureY", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureU[2]);
    setInt(hProgramHandle[0], "textureU", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureV[2]);
    setInt(hProgramHandle[0], "textureV", 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCoords2D.glVertex_L.size());

    //right
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CamVerticesPoints[3]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CamImagePoints[3]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.LumiaBalance[3]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);

    colorAdjust[0].x = colorAve[9].x - colorAve[3].x;
	colorAdjust[0].y = colorAve[9].y - colorAve[3].y;
	colorAdjust[0].z = colorAve[9].z - colorAve[3].z;
	
	colorAdjust[1].x = colorAve[11].x - colorAve[7].x;
	colorAdjust[1].y = colorAve[11].y - colorAve[7].y;
	colorAdjust[1].z = colorAve[11].z - colorAve[7].z;

    setVec3(hProgramHandle[0], "color0Adjust", colorAdjust[0]); 
	setVec3(hProgramHandle[0], "color1Adjust", colorAdjust[1]); 

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureY[3]);
    setInt(hProgramHandle[0], "textureY", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureU[3]);
    setInt(hProgramHandle[0], "textureU", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureV[3]);
    setInt(hProgramHandle[0], "textureV", 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCoords2D.glVertex_R.size());
#endif

#if 1
    glUseProgram(hProgramHandle[1]);
    
    /* Enable attributes for position, color and texture coordinates etc. */
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

	setMat4(hProgramHandle[1], "mvp", matrixMVP);


    //front  left
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.Alpha[0]);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.MosaicCamVerticesPoints[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.MosaicFLCamImagePoints[0]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.MosaicFLCamImagePoints[1]);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    colorAdjust[0].x = colorAve[8].x - colorAve[0].x;
	colorAdjust[0].y = colorAve[8].y - colorAve[0].y;
	colorAdjust[0].z = colorAve[8].z - colorAve[0].z;
	
	colorAdjust[1].x = colorAve[8].x - colorAve[1].x;
	colorAdjust[1].y = colorAve[8].y - colorAve[1].y;
	colorAdjust[1].z = colorAve[8].z - colorAve[1].z;
	
	setVec3(hProgramHandle[1], "color0Adjust", colorAdjust[0]); 
	setVec3(hProgramHandle[1], "color1Adjust", colorAdjust[1]); 

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureY[0]);
    setInt(hProgramHandle[1], "texture1Y", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureU[0]);
    setInt(hProgramHandle[1], "texture1U", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureV[0]);
    setInt(hProgramHandle[1], "texture1V", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, textureY[2]);
    setInt(hProgramHandle[1], "texture2Y", 3);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, textureU[2]);
    setInt(hProgramHandle[1], "texture2U", 4);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, textureV[2]);
    setInt(hProgramHandle[1], "texture2V", 5);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCoords2D.glVertex_FL.size());


    //front  right
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.Alpha[1]);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.MosaicCamVerticesPoints[1]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.MosaicFRCamImagePoints[0]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.MosaicFRCamImagePoints[1]);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);


    colorAdjust[0].x = colorAve[9].x - colorAve[2].x;
	colorAdjust[0].y = colorAve[9].y - colorAve[2].y;
	colorAdjust[0].z = colorAve[9].z - colorAve[2].z;
	
	colorAdjust[1].x = colorAve[9].x - colorAve[3].x;
	colorAdjust[1].y = colorAve[9].y - colorAve[3].y;
	colorAdjust[1].z = colorAve[9].z - colorAve[3].z;
	
	setVec3(hProgramHandle[1], "color0Adjust", colorAdjust[0]); 
	setVec3(hProgramHandle[1], "color1Adjust", colorAdjust[1]); 

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureY[0]);
    setInt(hProgramHandle[1], "texture1Y", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureU[0]);
    setInt(hProgramHandle[1], "texture1U", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureV[0]);
    setInt(hProgramHandle[1], "texture1V", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, textureY[3]);
    setInt(hProgramHandle[1], "texture2Y", 3);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, textureU[3]);
    setInt(hProgramHandle[1], "texture2U", 4);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, textureV[3]);
    setInt(hProgramHandle[1], "texture2V", 5);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCoords2D.glVertex_FR.size());

    //back  left
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.Alpha[2]);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.MosaicCamVerticesPoints[2]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.MosaicBLCamImagePoints[0]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.MosaicBLCamImagePoints[1]);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);


    colorAdjust[0].x = colorAve[10].x - colorAve[4].x;
	colorAdjust[0].y = colorAve[10].y - colorAve[4].y;
	colorAdjust[0].z = colorAve[10].z - colorAve[4].z;
	
	colorAdjust[1].x = colorAve[10].x - colorAve[5].x;
	colorAdjust[1].y = colorAve[10].y - colorAve[5].y;
	colorAdjust[1].z = colorAve[10].z - colorAve[5].z;
	
	setVec3(hProgramHandle[1], "color0Adjust", colorAdjust[0]); 
	setVec3(hProgramHandle[1], "color1Adjust", colorAdjust[1]); 

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureY[1]);
    setInt(hProgramHandle[1], "texture1Y", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureU[1]);
    setInt(hProgramHandle[1], "texture1U", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureV[1]);
    setInt(hProgramHandle[1], "texture1V", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, textureY[2]);
    setInt(hProgramHandle[1], "texture2Y", 3);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, textureU[2]);
    setInt(hProgramHandle[1], "texture2U", 4);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, textureV[2]);
    setInt(hProgramHandle[1], "texture2V", 5);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCoords2D.glVertex_BL.size());


    //back  right
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.Alpha[3]);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.MosaicCamVerticesPoints[3]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.MosaicBRCamImagePoints[0]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.MosaicBRCamImagePoints[1]);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    colorAdjust[0].x = colorAve[11].x - colorAve[6].x;
	colorAdjust[0].y = colorAve[11].y - colorAve[6].y;
	colorAdjust[0].z = colorAve[11].z - colorAve[6].z;
	
	colorAdjust[1].x = colorAve[11].x - colorAve[7].x;
	colorAdjust[1].y = colorAve[11].y - colorAve[7].y;
	colorAdjust[1].z = colorAve[11].z - colorAve[7].z;

	setVec3(hProgramHandle[1], "color0Adjust", colorAdjust[0]); 
	setVec3(hProgramHandle[1], "color1Adjust", colorAdjust[1]); 

	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureY[1]);
    setInt(hProgramHandle[1], "texture1Y", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureU[1]);
    setInt(hProgramHandle[1], "texture1U", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureV[1]);
    setInt(hProgramHandle[1], "texture1V", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, textureY[3]);
    setInt(hProgramHandle[1], "texture2Y", 3);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, textureU[3]);
    setInt(hProgramHandle[1], "texture2U", 4);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, textureV[3]);
    setInt(hProgramHandle[1], "texture2V", 5);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCoords2D.glVertex_BR.size());
#endif


#if 1
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	
	glUseProgram(hProgramHandle[2]);

	setMat4(hProgramHandle[2], "mvp", matrixMVP);

    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CarVerTexCoord[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CarVerTexCoord[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureRes[0]);
	setInt(hProgramHandle[1], "textureImg", 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#endif

}


void drawUndistortBackCurve(ESMatrix orthoMatrix)
{
	int i;
	vec4 color[4] = {{1.0, 0.0, 0.0, 0.5}, {1.0, 1.0, 0.0, 0.5}, {0.0, 1.0, 0.0, 0.5}, {0.0, 0.0, 0.0, 0.5}};

	glUseProgram(hProgramHandle[3]);

	setMat4(hProgramHandle[3], "mvp", orthoMatrix); 


	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

	glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

	for (i = 0; i < 6; i++)
	{
		setVec4(hProgramHandle[3], "outColor", color[i / 2]); 

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, verticesRearTrajLinePoint[i]);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, LENGTH * 2);
	}

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, verticesRearTrajLinePoint[6]);
	for (i = 0; i < 3; i++)
	{
		setVec4(hProgramHandle[3], "outColor", color[i]); 
		glDrawArrays(GL_TRIANGLE_STRIP, i * 4, 4);
	}

}

vec2 undistort(float i, float j, const float *A, const float *dist_coeffs, undistortParams params, double *invR)
{
    float u, v;
    double x, y, x2, y2, r2, r, theta, theta2, theta4, theta6, theta8, theta_d, xd, yd;
    vec2 imgPoints;
    float p3D[3], oldP3D[3];
    float z;

    float fx = A[0];
  	float fy = A[4];
  	float cx = A[2];
  	float cy = A[5];

    float k1 = dist_coeffs[0];
    float k2 = dist_coeffs[1];
    float p1 = dist_coeffs[2];
    float p2 = dist_coeffs[3];

    p3D[0] = (params.xZoom * j + params.x - cx) / fx;
    p3D[1] = (params.yZoom * i + params.y - cy) / fy;
    p3D[2] = 1.0;

    oldP3D[0] = invR[0] * p3D[0] + invR[1] * p3D[1] + invR[2] * p3D[2];
    oldP3D[1] = invR[3] * p3D[0] + invR[4] * p3D[1] + invR[5] * p3D[2];
    oldP3D[2] = invR[6] * p3D[0] + invR[7] * p3D[1] + invR[8] * p3D[2];

    if(oldP3D[2] > 0.001)
    {
        x = oldP3D[0] / oldP3D[2];
        y = oldP3D[1] / oldP3D[2];
        z = 1.0;

        /*$)AM8JS3}7(#,M<OqWx1j*/
        x = oldP3D[0] / oldP3D[2];
        y = oldP3D[1] / oldP3D[2];

        y2 = y * y;
        x2 = x * x;
        r2 = x2 + y2;
        r = sqrt(r2);
        theta = atan(r);
        theta2 = theta * theta;
        theta4 = theta2 * theta2;
        theta6 = theta4 * theta2;
        theta8 = theta6 * theta2;
        theta_d = theta * (1 + k1 * theta2 + k2 * theta4 + p1 * theta6 + p2 * theta8);

        if(r < 0.000001)
        {
            xd = 0;
            yd = 0;
        }
        else
        {
            /*$)A5C5==CU}5DM<OqWx1j*/
            xd = x * theta_d / r;
            yd = y * theta_d / r;
        }

        /*$)A5C5=OqKXWx1j*/
        u  = fx * xd + cx;
        v  = fy * yd + cy;

        if (u < 0)
        {
            u = 0;
        }
        else if (u > IMAGE_WIDTH)
        {
            u = IMAGE_WIDTH;
        }

        if (v < 0)
        {
            v = 0;
        }
        else if (v > IMAGE_HEIGHT)
        {
            v = IMAGE_HEIGHT;
        }
    }
    else
    {
        u = 0;
        v = 0;
    }

    imgPoints.x = u / IMAGE_WIDTH;
    imgPoints.y = v / IMAGE_HEIGHT;

    return imgPoints;
}




void showSingleView(int viewMode, float wheelAngle)
{
    int whichCamera, cnt;
    float verticesView[] =
    {
        -1.0f, -1.0f, 0.0f,  // left-buttom
        1.0f, -1.0f, 0.0f,	// right- buttom
        -1.0f,	1.0f, 0.0f,   // right-top
        1.0f,  1.0f, 0.0f,	 // left-top
    };


    float texCoordView[] =
    {
        0.0f,  1.0f,  // left-top
        1.0f,  1.0f,  // right-top
        0.0f,  0.0f,  // left-buttom
        1.0f,  0.0f,  // right- buttom
    };
    
    float texCoordViewLR[] =
    {
        0.1f,  0.9f,  // left-top
        0.9f,  0.9f,  // right-top
        0.1f,  0.1f,  // left-buttom
        0.9f,  0.1f,  // right- buttom
    };

	float texCoordViewDMS[] =
	{
		0.0f,  0.0f,  // left-top
		0.0f,  1.0f,  // right-top
		1.0f,  0.0f,  // left-buttom
		1.0f,  1.0f,  // right- buttom
	};


    float *verAddr, *texAddr;
    ESMatrix orthoMatrix;

    switch(viewMode)
    {
    case VIEW_FRONT:
        whichCamera = 0;
        cnt = 4;
        verAddr = verticesView;
        texAddr = texCoordView;
        break;
    case VIEW_BACK:
        whichCamera = 1;
        cnt = 4;
        verAddr = verticesView;
        texAddr = texCoordView;
        break;
    case VIEW_LEFT:
        whichCamera = 2;
        cnt = 4;
        verAddr = verticesView;
        texAddr = texCoordViewLR;
        break;
    case VIEW_RIGHT:
        whichCamera = 3;
        cnt = 4;
        verAddr = verticesView;
        texAddr = texCoordViewLR;
        break;
    case VIEW_UNDISTORT_FRONT:
        whichCamera = 0;
        cnt = verCountFront;
        verAddr = (float *)verCoordPointFront;
        texAddr = (float *)texCoordPointFront;
        break;
    case VIEW_UNDISTORT_BACK:
        whichCamera = 1;
        cnt = verCountBack;
        verAddr = (float *)verCoordPointBack;
        texAddr = (float *)texCoordPointBack;
        break;
    case VIEW_DMS:
        whichCamera = 4;
        cnt = 4;
        verAddr = verticesView;
        texAddr = texCoordViewDMS;
        break;
  	case VIEW_CONTAINER:
        whichCamera = 4;
        cnt = 4;
        verAddr = verticesView;
        texAddr = texCoordView;
        break;
    default:
        whichCamera = 4;
        cnt = 4;
        verAddr = verticesView;
        texAddr = texCoordView;
        break;
    }


    esMatrixLoadIdentity ( &orthoMatrix );

    glViewport(gSingleView.x, gSingleView.y, gSingleView.width, gSingleView.height);

    glUseProgram(hProgramHandle[4]);

    /* Enable attributes for position, color and texture coordinates etc. */
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

    setMat4(hProgramHandle[4], "mvp", orthoMatrix);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, verAddr);
 	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, texAddr);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureY[whichCamera]);
    setInt(hProgramHandle[0], "textureY", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureU[whichCamera]);
    setInt(hProgramHandle[0], "textureU", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureV[whichCamera]);
    setInt(hProgramHandle[0], "textureV", 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, cnt);

	if(viewMode == VIEW_UNDISTORT_BACK)
	{
		drawUndistortBackCurve(orthoMatrix);
	}
}

void showFourView()
{
    float verticesViewFront[] =
    {
        -1.0f, 0.0f, 0.0f,  // left-buttom
        0.0f, 0.0f, 0.0f,	// right- buttom
        -1.0f,	1.0f, 0.0f,   // right-top
        0.0f,  1.0f, 0.0f,	 // left-top
    };

    float verticesViewBack[] =
    {
        0.0f, 0.0f, 0.0f,  // left-buttom
        1.0f, 0.0f, 0.0f,	// right- buttom
        0.0f,	1.0f, 0.0f,   // right-top
        1.0f,  1.0f, 0.0f,	 // left-top
    };

    float verticesViewLeft[] =
    {
        -1.0f, -1.0f, 0.0f,  // left-buttom
        0.0f, -1.0f, 0.0f,	// rLeftight- buttom
        -1.0f,	0.0f, 0.0f,   // right-top
        0.0f,  0.0f, 0.0f,	 // left-top
    };

    float verticesViewRight[] =
    {
        0.0f, -1.0f, 0.0f,  // left-buttom
        1.0f, -1.0f, 0.0f,	// right- buttom
        0.0f,	0.0f, 0.0f,   // right-top
        1.0f,  0.0f, 0.0f,	 // left-top
    };


    float texCoordView[] =
    {
        0.0f,  1.0f,  // left-top
        1.0f,  1.0f,  // right-top
        0.0f,  0.0f,  // left-buttom
        1.0f,  0.0f,  // right- buttom
    };

    ESMatrix orthoMatrix;
    esMatrixLoadIdentity ( &orthoMatrix );

	glBindBuffer(GL_ARRAY_BUFFER, 0);

    glViewport(gSingleView.x, gSingleView.y, gSingleView.width, gSingleView.height);

    glUseProgram(hProgramHandle[4]);

    /* Enable attributes for position, color and texture coordinates etc. */
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    setMat4(hProgramHandle[4], "mvp", orthoMatrix); 

    //Front
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, verticesViewFront);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, texCoordView);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureY[0]);
    setInt(hProgramHandle[0], "textureY", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureU[0]);
    setInt(hProgramHandle[0], "textureU", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureV[0]);
    setInt(hProgramHandle[0], "textureV", 2);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


    //Back
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, verticesViewBack);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, texCoordView);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureY[1]);
    setInt(hProgramHandle[0], "textureY", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureU[1]);
    setInt(hProgramHandle[0], "textureU", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureV[1]);
    setInt(hProgramHandle[0], "textureV", 2);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


    //Left
 	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, verticesViewLeft);
 	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, texCoordView);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureY[2]);
    setInt(hProgramHandle[0], "textureY", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureU[2]);
    setInt(hProgramHandle[0], "textureU", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureV[2]);
    setInt(hProgramHandle[0], "textureV", 2);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


    //Right
  	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, verticesViewRight);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, texCoordView);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureY[3]);
    setInt(hProgramHandle[0], "textureY", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureU[3]);
    setInt(hProgramHandle[0], "textureU", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureV[3]);
    setInt(hProgramHandle[0], "textureV", 2);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}



int generateTriangle(vec2 *triagnle, int width, int height)
{
	float p = 64.0;
	float i, j, fp;
	unsigned char directionFlag;
    vec2 point;
	int count;
	fp = 1.0 / p;

	directionFlag = 0;

	count = 0;

	for (i = 0; i < 1; i += fp)
	{
		switch(directionFlag)
		{
		case 0:
			for (j = 0; j <= 1; j += fp)
			{
				point.x = j * width;
				point.y = i * height;
					
				triagnle[count++] = point;

				point.x = j * width;
				point.y = (i + fp) * height;

				triagnle[count++] = point;
			}
			break;
		case 1:
			for( j = 1; j >= 0; j -= fp )
			{
				point.x = j * width;
				point.y = i * height;

				triagnle[count++] = point;

				point.x = j * width;
				point.y = (i + fp) * height;
	
				triagnle[count++] = point;

			}
			break;
		}
		// change direction!

		if( fabs(j - fp - 1) < FLOAT_ZERO )
		{
			directionFlag = 1;
		}
		else
		{
			directionFlag = 0;
		}
	}
	
	return count;
}

vec2 function(float a1, float b1, float c1, float a2, float b2, float c2) {
  float a3, b3, c3;
  float a4, b4, c4;
  vec2 res;

  a3 = a1 * a2;
  a4 = a2 * a1;
  b3 = b1 * a2;
  b4 = b2 * a1;
  c3 = c1 * a2;
  c4 = c2 * a1;

  res.y = (c3 - c4) / (b3 - b4);

  a3 = a1 * b2;
  a4 = a2 * b1;
  b3 = b1 * b2;
  b4 = b2 * b1;
  c3 = c1 * b2;
  c4 = c2 * b1;

  res.x = (c3 - c4) / (a3 - a4);

  return res;
}



vec2 function2(vec2 dist, const float *A, undistortParams params, double *invR)
{
    float fx, fy, cx, cy;
    vec2 imgPoints, point;
	float a1, b1, c1, a2, b2, c2;
	
    fx = A[0];
  	fy = A[4];
  	cx = A[2];
  	cy = A[5];

	a1 = invR[0] - dist.x * invR[6];
	b1 = invR[1] - dist.x * invR[7];
	c1 = dist.x * invR[8] - invR[2];
	a2 = invR[3] - dist.y * invR[6];
	b2 = invR[4] - dist.y * invR[7];
	c2 = dist.y * invR[8] - invR[5];


	point = function( a1, b1, c1, a2, b2, c2 );
	
	/*$)A5C5=OqKXWx1j*/
    imgPoints.x = (cx + point.x * fx - params.x) / params.xZoom;
    imgPoints.y = (cy + point.y * fy - params.y) / params.yZoom;

    return imgPoints;
}


vec2 projectPoints3(const vec3 obj_points, const float *r_vec,
                    const float *t_vec) {
  vec2 imgPoints;
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



void findRearCurve(float wheelAngle, undistortParams params, float *camera, float *rVec, float *tVec, double *invR, int width, int height)
{
    float wheel_base = 4675/20.0;
    float rear_wheel = 1375/20.0;
    float ex_r, in_r, ex_r1, in_r1;
   	float tmpx, tmpy;
    float angle;
    int i, k;
    float j;
	vec3 pw;
	vec2 len, point;
	float lineWidth, halfLineWidth, otherHalfLineWidth;
    double ex_r1_in, ex_r1_out, in_r1_in, in_r1_out;
	double baseAngle[3][4], deltaAngle[3][4], radius[4];
	float fp = 1.0 / 64;
	vec3 worldPoints[10][LENGTH * 2];
	float startx, length; 
	float offsetY;
	int idx;
	int gap[3] = {3, 4, 5};
	float distance[4] = {0, 500/20.0, 1500/20.0, 4000/20.0 };
	float car_width = 2556/20.0;
	float scale;

	lineWidth = 3;
	halfLineWidth = lineWidth / 2;
	otherHalfLineWidth = lineWidth - halfLineWidth;
	offsetY = 0;

	
	startx = -car_width/2 + 70;
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
					tmpx = car_width - ex_r + radius[i*2+0] * cos(baseAngle[idx][i*2+0] + deltaAngle[idx][i*2+0] * j); 
					tmpy = radius[i*2+0] * sin(baseAngle[idx][i*2+0] + deltaAngle[idx][i*2+0] * j) - rear_wheel;

					worldPoints[idx*2+i][k*2+0] = Vec3(tmpx, tmpy+offsetY,0.0);


					tmpx = car_width - ex_r + radius[i*2+1] * cos(baseAngle[idx][i*2+1] + deltaAngle[idx][i*2+1] * j); 
					tmpy = radius[i*2+1] * sin(baseAngle[idx][i*2+1] + deltaAngle[idx][i*2+1] * j) - rear_wheel;

					worldPoints[idx*2+i][k*2+1] = Vec3(tmpx, tmpy+offsetY,0.0);
				}
			}
		}

		for(idx=0; idx<3; idx++)
		{
			tmpx = car_width - ex_r + radius[1] * cos(baseAngle[idx][1] + deltaAngle[idx][1] * 1.0); 
			tmpy = radius[1] * sin(baseAngle[idx][1] + deltaAngle[idx][1] * 1.0) - rear_wheel;

			worldPoints[6][idx*4+0] = Vec3(tmpx, tmpy+offsetY,0.0);

			tmpx = car_width - ex_r + radius[1] * cos(baseAngle[idx][1] + deltaAngle[idx][1] * (1.0 - gap[idx] * fp)); 
			tmpy = radius[1] * sin(baseAngle[idx][1] + deltaAngle[idx][1] * (1.0 - gap[idx] * fp)) - rear_wheel;

			worldPoints[6][idx*4+1] = Vec3(tmpx, tmpy+offsetY,0.0);


			tmpx = car_width - ex_r + radius[2] * cos(baseAngle[idx][2] + deltaAngle[idx][2] * 1.0); 
			tmpy = radius[2] * sin(baseAngle[idx][2] + deltaAngle[idx][2] * 1.0) - rear_wheel;

			worldPoints[6][idx*4+2] = Vec3(tmpx, tmpy+offsetY,0.0);

			tmpx = car_width - ex_r + radius[2] * cos(baseAngle[idx][2] + deltaAngle[idx][2] * (1.0 - gap[idx] * fp)); 
			tmpy = radius[2] * sin(baseAngle[idx][2] + deltaAngle[idx][2] * (1.0 - gap[idx] * fp)) - rear_wheel;

			worldPoints[6][idx*4+3] = Vec3(tmpx, tmpy+offsetY,0.0);
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
					tmpx = car_width + in_r - radius[i*2+0] * cos(baseAngle[idx][i*2+0] + deltaAngle[idx][i*2+0] * j); 
					tmpy = radius[i*2+0] * sin(baseAngle[idx][i*2+0] + deltaAngle[idx][i*2+0] * j) - rear_wheel;

					worldPoints[idx*2+i][k*2+0] = Vec3(tmpx, tmpy+offsetY,0.0);

					tmpx = car_width + in_r - radius[i*2+1] * cos(baseAngle[idx][i*2+1] + deltaAngle[idx][i*2+1] * j); 
					tmpy = radius[i*2+1] * sin(baseAngle[idx][i*2+1] + deltaAngle[idx][i*2+1] * j) - rear_wheel;

					worldPoints[idx*2+i][k*2+1] = Vec3(tmpx, tmpy+offsetY,0.0);
				}
			}
		}

		for(idx=0; idx<3; idx++)
		{
			tmpx = car_width + in_r - radius[1] * cos(baseAngle[idx][1] + deltaAngle[idx][1] * 1.0); 
			tmpy = radius[1] * sin(baseAngle[idx][1] + deltaAngle[idx][1] * 1.0) - rear_wheel;

			worldPoints[6][idx*4+0] = Vec3(tmpx, tmpy+offsetY,0.0);

			tmpx = car_width + in_r - radius[1] * cos(baseAngle[idx][1] + deltaAngle[idx][1] * (1.0 - gap[idx] * fp)); 
			tmpy = radius[1] * sin(baseAngle[idx][1] + deltaAngle[idx][1] * (1.0 - gap[idx] * fp)) - rear_wheel;

			worldPoints[6][idx*4+1] = Vec3(tmpx, tmpy+offsetY,0.0);


			tmpx = car_width + in_r - radius[2] * cos(baseAngle[idx][2] + deltaAngle[idx][2] * 1.0); 
			tmpy = radius[2] * sin(baseAngle[idx][2] + deltaAngle[idx][2] * 1.0) - rear_wheel;

			worldPoints[6][idx*4+2] = Vec3(tmpx, tmpy+offsetY,0.0);

			tmpx = car_width + in_r - radius[2] * cos(baseAngle[idx][2] + deltaAngle[idx][2] * (1.0 - gap[idx] * fp)); 
			tmpy = radius[2] * sin(baseAngle[idx][2] + deltaAngle[idx][2] * (1.0 - gap[idx] * fp)) - rear_wheel;

			worldPoints[6][idx*4+3] = Vec3(tmpx, tmpy+offsetY,0.0);
		}
	}

	for (j = 0, k=0; j <= 1; j += fp)
	{
		tmpx = startx + (length) * j;
        tmpy = 0-5; 

        worldPoints[7][k*2+0] = Vec3(tmpx, tmpy, 0.0);

        tmpx = startx + (length) * j;
        tmpy = 0+5; 

        worldPoints[7][k*2+1] = Vec3(tmpx, tmpy, 0.0);
        k++;
	}
	
	for(k=0; k<8; k++)
	{
		for(i = 0; i < LENGTH * 2; i++) /*$)A<FKcJ@=gWx1j5c*/
		{
			pw = worldPoints[k][i];
	
			len = projectPoints3( pw, rVec, tVec ); 

			point = function2(len, camera, params, invR);

			verticesRearTrajLinePoint[k][i].x = -(1.0 - 2 * point.x / width);
			verticesRearTrajLinePoint[k][i].y = -(1.0 - 2 * point.y / height);
			verticesRearTrajLinePoint[k][i].z = 0.0;
		}
	}
}


void initUndistort(int width, int height)
{
    float alpha, beta, gamma;
    double R[9], invR[9];
    undistortParams resizer;
    int k, p;
    p = 64;

#if 1
    resizer.x = 100;
    resizer.y = 200;
    resizer.xZoom = 1.0 * 1080 / width;
    resizer.yZoom = 1.0 * 820 / height;

    imgCoordPointFront = (vec2 *)malloc(sizeof(vec2) * p * (p + 1) * 2);

    verCountFront = generateTriangle(imgCoordPointFront, width, height);

    texCoordPointFront = (vec2 *)malloc(sizeof(vec2) * verCountFront);
    verCoordPointFront = (vec3 *)malloc(sizeof(vec3) * verCountFront);

    rotationMatrixToEulerAngle(frontCamParams.mr, &alpha, &beta, &gamma);

    alpha = 0 * RADIAN;

    R[0] = cos(beta) * cos(gamma);
    R[1] = cos(beta) * sin(gamma);
    R[2] = -sin(beta);
    R[3] = sin(alpha) * sin(beta) * cos(gamma) - cos(alpha) * sin(gamma);
    R[4] = sin(alpha) * sin(beta) * sin(gamma) + cos(alpha) * cos(gamma);
    R[5] = sin(alpha) * cos(beta);
    R[6] = cos(alpha) * sin(beta) * cos(gamma) + sin(alpha) * sin(gamma);
    R[7] = cos(alpha) * sin(beta) * sin(gamma) - sin(alpha) * cos(gamma);
    R[8] = cos(alpha) * cos(beta);

    invertMatrix(R, invR);

    for (k = 0; k < verCountFront; k++)
    {
        verCoordPointFront[k].x = -(1.0 - 2 * imgCoordPointFront[k].x / width);
        verCoordPointFront[k].y = (1.0 - 2 * imgCoordPointFront[k].y / height);
        verCoordPointFront[k].z = 0;

        texCoordPointFront[k] = undistort(imgCoordPointFront[k].y, imgCoordPointFront[k].x,
                                          frontCamParams.mi, frontCamParams.md, resizer, invR);
    }
#endif

#if 1
    resizer.x = 0;
    resizer.y = 20;
    resizer.xZoom = 1.0 * 1280 / width;
    resizer.yZoom = 1.0 * 620 / height;


    imgCoordPointBack = (vec2 *)malloc(sizeof(vec2) * p * (p + 1) * 2);

    verCountBack = generateTriangle(imgCoordPointBack, width, height);

    texCoordPointBack = (vec2 *)malloc(sizeof(vec2) * verCountBack);
    verCoordPointBack = (vec3 *)malloc(sizeof(vec3) * verCountBack);

    rotationMatrixToEulerAngle(rearCamParams.mr, &alpha, &beta, &gamma);

    alpha = 0 * RADIAN;

    R[0] = cos(beta) * cos(gamma);
    R[1] = cos(beta) * sin(gamma);
    R[2] = -sin(beta);
    R[3] = sin(alpha) * sin(beta) * cos(gamma) - cos(alpha) * sin(gamma);
    R[4] = sin(alpha) * sin(beta) * sin(gamma) + cos(alpha) * cos(gamma);
    R[5] = sin(alpha) * cos(beta);
    R[6] = cos(alpha) * sin(beta) * cos(gamma) + sin(alpha) * sin(gamma);
    R[7] = cos(alpha) * sin(beta) * sin(gamma) - sin(alpha) * cos(gamma);
    R[8] = cos(alpha) * cos(beta);

    invertMatrix(R, invR);

    for (k = 0; k < verCountBack; k++)
    {
        verCoordPointBack[k].x = -(1.0 - 2 * imgCoordPointBack[k].x / width);
        verCoordPointBack[k].y = -(1.0 - 2 * imgCoordPointBack[k].y / height);
        verCoordPointBack[k].z = 0;

        texCoordPointBack[k] = undistort(imgCoordPointBack[k].y, imgCoordPointBack[k].x,
                                         rearCamParams.mi, rearCamParams.md, resizer, invR);
    }
    
    findRearCurve(0.0, resizer, rearCamParams.mi, rearCamParams.mr, rearCamParams.mt, invR, width, height);
#endif

}

int avmInit(safImgRect allView, safImgRect singleView)
{
    gAllView = allView;

    gSingleView = singleView;

	initShader();

	initCamParaData();

	init2DModel();

	initTextureCoords();

	getCamPixelPosition();
	
	loadRes(IMAGE_WIDTH, IMAGE_HEIGHT);

	initVBO();

	initUndistort(gSingleView.width, gSingleView.height);

    return 0;
}

extern void updateTexture(unsigned char **src)
{
    int i;
    unsigned char *pSrcY, *pSrcU, *pSrcV;
    for(i = 0; i < 5; i++)
    {

        pSrcY = src[i];
        pSrcU = pSrcY + IMAGE_WIDTH * IMAGE_HEIGHT;
        pSrcV = pSrcU + IMAGE_WIDTH * IMAGE_HEIGHT / 4;

        glBindTexture( GL_TEXTURE_2D, textureY[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, IMAGE_WIDTH, IMAGE_HEIGHT, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pSrcY);

        glBindTexture( GL_TEXTURE_2D, textureU[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pSrcU);

        glBindTexture( GL_TEXTURE_2D, textureV[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pSrcV);
    }
    
    pu8CamMBvir[0] = src[0];
   	pu8CamMBvir[1] = src[1];
   	pu8CamMBvir[2] = src[2];
   	pu8CamMBvir[3] = src[3];


    caculateColorCoeff2D(src);
}


void runRender(int viewMode, float steeringWheelAngle)
{
    // Clear the colorbuffer and depth-buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    show2DCar();
    
    if(viewMode == VIEW_OVERALL)
    {
        showFourView();
    }
    else
    {
       	showSingleView(viewMode, steeringWheelAngle);
    }
}


#if 0
void imageFilp(unsigned char *src, unsigned char *dst)
{
 unsigned char *pSrcY, *pSrcU, *pSrcV;
 unsigned char *pDstY, *pDstU, *pDstV;
 CvMat srcY, srcU, srcV;
 CvMat dstY, dstU, dstV;

 pSrcY = src;
 pSrcU = pSrcY + IMAGE_WIDTH * IMAGE_HEIGHT;
 pSrcV = pSrcU + IMAGE_WIDTH * IMAGE_HEIGHT / 4;

 pDstY = dst;
 pDstU = pDstY + IMAGE_WIDTH * IMAGE_HEIGHT;
 pDstV = pDstU + IMAGE_WIDTH * IMAGE_HEIGHT / 4;

 srcY = cvMat(IMAGE_WIDTH, IMAGE_HEIGHT, CV_8UC1, pSrcY);
 srcU = cvMat(IMAGE_WIDTH/2, IMAGE_HEIGHT/2, CV_8UC1, pSrcU);
 srcV = cvMat(IMAGE_WIDTH/2, IMAGE_HEIGHT/2, CV_8UC1, pSrcV);

 dstY = cvMat(IMAGE_WIDTH, IMAGE_HEIGHT, CV_8UC1, pDstY);
 dstU = cvMat(IMAGE_WIDTH/2, IMAGE_HEIGHT/2, CV_8UC1, pDstU);
 dstV = cvMat(IMAGE_WIDTH/2, IMAGE_HEIGHT/2, CV_8UC1, pDstV);

 cvFlip(&srcY, &dstY, -1);
 cvFlip(&srcU, &dstU, -1);
 cvFlip(&srcV, &dstV, -1);
}
#endif

