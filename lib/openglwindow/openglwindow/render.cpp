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

#include "InitMosaic.hpp"
#include "mtlloader.hpp"

#include "esTransform.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

//#include "stb_image.h"
//#include "stb_image_write.h"
#include "stb_image.h"


//static int mvp_pos[4];
static int hProgramHandle[5];

//volatile double wheelangle = 0.0;
volatile double gDoorAngle = 0.0;


//GLint iLocColor0Adjust = -1;
//GLint iLocColor1Adjust = -1;

//GLint iLocColor0AdjustMosaic = -1;
//GLint iLocColor1AdjustMosaic = -1;
#if 0
GLint iLocColorBlend = -1;
GLint iLocPositionAdjust = -1;
GLint iLocTexcoordAdjust = -1;
GLint iLocPositionLumia = -1;

GLint iLocTextureY = -1;
GLint iLocTextureUV = -1;
GLint iLocTextureMosaicY1 = -1;
GLint iLocTextureMosaicY2 = -1;
GLint iLocTextureMosaicUV1 = -1;
GLint iLocTextureMosaicUV2 = -1;
//GLint iLocTextureConvertY = -1;
//GLint iLocTextureConvertUV = -1;

//GLint iLocTextureYUYV = -1;
//GLint iLocTextureMosaic1YUYV = -1;
//GLint iLocTextureMosaic2YUYV = -1;

//GLint iLocTextureRGB = -1;
#endif

int bvs2DWidth = 360;//288;
int bvs2DHeight = 720;
int bvs2DoffsetX = 920;
int bvs2DoffsetY = 0;

int bvs3DWidth = 920;//1280;//288;
int bvs3DHeight = 720;
int bvs3DoffsetX = 0;
int bvs3DoffsetY = 0;


//GLuint textureRGBA[5];
GLuint textureY[5];
GLuint textureU[5];
GLuint textureV[5];

GLuint textureRes[4];


GLuint cubeTexture;

float lumiaAve[12];
CvPoint3D32f colorAve[12];  //$)Ad???d:?:&??!!h0????
CvPoint3D32f colorCount[AVERAGE_COUNT][12];  //$)Ad???d:?:&??!!h0????

CvPoint3D32f *verCoordPointFront;
CvPoint2D32f *texCoordPointFront, *imgCoordPointFront;

CvPoint3D32f *verCoordPointBack;
CvPoint2D32f *texCoordPointBack, *imgCoordPointBack;

static int verCountFront, verCountBack;

extern SimplifyCamParams SimplifyfrontCamParams;
extern SimplifyCamParams SimplifyrearCamParams;

extern "C" void convertImage(char *filePath, unsigned char *pYBuf, unsigned char *pUVBuf)
{
    //IplImage* image;
    unsigned char *pY, *pUV;
    unsigned char *p;
    unsigned char b, g, r;
    int i, j, widthStep;
    unsigned char TmpU[4], TmpV[4];
    int width, height, channel;
    unsigned char *imageBuffer;

    //image = cvLoadImage( filePath, 1 );
    imageBuffer = stbi_load(filePath, &width, &height, &channel, 0);

    p = (unsigned char *)malloc((width) * (height) * channel);
    pY	  = pYBuf;
    pUV   = pUVBuf;

    widthStep = (width) * channel;

    for( i = 0; i < height; i++ )
    {
        for( j = 0; j < width; j++ )
        {
            b = (unsigned char)(imageBuffer[i * widthStep + 3 * j]);
            g = (unsigned char)(imageBuffer[i * widthStep + 3 * j + 1]);
            r = (unsigned char)(imageBuffer[i * widthStep + 3 * j + 2]);

            p[i * widthStep + 3 * j] = (unsigned char)(0.2990 * r + 0.5870 * g + 0.1140 * b);
            p[i * widthStep + 3 * j + 1] = (unsigned char)(-0.1687 * r - 0.3313 * g + 0.5000 * b + 128);
            p[i * widthStep + 3 * j + 2] = (unsigned char)(0.5000 * r - 0.4187 * g - 0.0813 * b + 128);
        }
    }

    for( i = 0; i < height; i++ )
    {
        for( j = 0; j < width; j++ )
        {
            *(pY++) = p[i * widthStep + 3 * j];
        }
    }

    for( i = 0; i < height; i += 2 )
    {
        for( j = 0; j < width; j += 2 )
        {
            TmpU[0] = p[i * widthStep + 3 * j + 1];
            TmpU[1] = p[i * widthStep + 3 * (j + 1) + 1];
            TmpU[2] = p[(i + 1) * widthStep + 3 * j + 1];
            TmpU[3] = p[(i + 1) * widthStep + 3 * (j + 1) + 1];

            TmpV[0] = p[i * widthStep + 3 * j + 2];
            TmpV[1] = p[i * widthStep + 3 * (j + 1) + 2];
            TmpV[2] = p[(i + 1) * widthStep + 3 * j + 2];
            TmpV[3] = p[(i + 1) * widthStep + 3 * (j + 1) + 2];

            *(pUV++) = (TmpV[0] + TmpV[1] + TmpV[2] + TmpV[3]) / 4;
            *(pUV++) = (TmpU[0] + TmpU[1] + TmpU[2] + TmpU[3]) / 4;
        }
    }

    free(p);
}

unsigned char interpolate(unsigned char *src, int widthStep, int x, int y, float ix, float iy, int offset)
{
    int channel;
    unsigned char pixelValue[4];
    int address;
    unsigned char result;

    channel = 3;
    address = y * widthStep + x * channel + offset;

    pixelValue[0] = src[address];
    pixelValue[1] = src[address + channel];
    pixelValue[2] = src[address + widthStep];
    pixelValue[3] = src[address + widthStep + channel];

    result = pixelValue[0] * (1 - ix) * (1 - iy) + pixelValue[1] * (ix) * (1 - iy) + pixelValue[2] * (1 - ix) * (iy) + pixelValue[3] * (ix) * (iy);

    return result;
}

void resizeImage(unsigned char *src, int srcWidth, int srcHeight, unsigned char *dst, int dstWidth, int dstHeight)
{
    float scaleX, scaleY;
    float ix, iy;
    float srcX, srcY;
    int srcWidthStep, dstWidthStep;
    int i, j;
    int x, y;
    int channel;

    scaleX = 1.0 * srcWidth / dstWidth;
    scaleY = 1.0 * srcHeight / dstHeight;

    channel = 3;
    srcWidthStep = srcWidth * channel;
    dstWidthStep = dstWidth * channel;


    for(i = 0; i < dstHeight; i++)
    {
        srcY = i * scaleY;
        y = int(srcY);
        iy = srcY - y;
        for(j = 0; j < dstWidth; j++)
        {
            srcX = j * scaleX;
            x = int(srcX);
            ix = srcX - x;

            dst[i * dstWidthStep + j * channel + 0] = interpolate(src, srcWidthStep, x, y, ix, iy, 0);
            dst[i * dstWidthStep + j * channel + 1] = interpolate(src, srcWidthStep, x, y, ix, iy, 1);
            dst[i * dstWidthStep + j * channel + 2] = interpolate(src, srcWidthStep, x, y, ix, iy, 2);
        }
    }
}


GLuint bindTexture(GLuint texture, unsigned char *buffer, GLuint w , GLuint h, GLint type)
{
    glBindTexture ( GL_TEXTURE_2D, texture );
    glTexImage2D ( GL_TEXTURE_2D, 0, type/*GL_LUMINANCE*/, w, h, 0, type/*GL_LUMINANCE*/, GL_UNSIGNED_BYTE, buffer);
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


char fragmentSourceRGBA[] =
{
    "#version 300 es						  \n"
    "precision highp float;                                      \n"
    "uniform sampler2D s_textureRGBA;                               \n"
    "uniform vec3 color0Adjust;                                   \n"
    "uniform vec3 color1Adjust;                                   \n"
    "smooth centroid in vec2 vv2texcoord;                        \n"
    "in float vCoeff;                					 	 \n"
    "out vec4 fragColor;                          				 \n"
    "void main()                                                 \n"
    "{                                                           \n"
    "  	 vec3 origin = texture(s_textureRGBA, vv2texcoord).rgb;\n"
    "  	 vec3 gain = color0Adjust * (1.0 - vCoeff) + color1Adjust * vCoeff;      \n"
    "    fragColor = vec4(origin + gain, 1.0);        \n"
    "}                                                           \n"

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
    "precision highp float; 									\n"
    "uniform vec4 outColor; 					  \n"
    "out vec4 fragColor;										\n"
    "void main()												\n"
    "{															\n"
    "	 fragColor = outColor;			\n"
    "}															\n"
};

#endif


#if 0
char vertexConvertSource[] =
{
    "#version 300 es						  \n"
    "layout(location = 0) in vec4 av4position;              \n"
    "layout(location = 1) in vec2 av2texcoord;              \n"
    "uniform mat4 mvp;                        \n"
    "smooth centroid out vec2 vv2texcoord;    \n"
    "void main()                              \n"
    "{                                        \n"
    "    vv2texcoord = av2texcoord;           \n"
    "    gl_Position = mvp * av4position;     \n"
    "}                                        \n"
};

char fragmentConvertSource[] =
{
    "#version 300 es						  \n"
    "precision highp float;                                      \n"
    "uniform sampler2D s_textureY;                               \n"
    "uniform sampler2D s_textureUV;                              \n"
    "smooth centroid in vec2 vv2texcoord;                        \n"
    "out vec4 fragColor;                          				\n"
    "void main()                                                 \n"
    "{                                                           \n"
    "    mediump vec3 yuv;                                       \n"
    "    mediump vec3 rgb;                                       \n"
    "    float lumia = texture(s_textureY, vv2texcoord).r;     \n"
    "    float cb = texture(s_textureUV, vv2texcoord).a - 0.5; \n"
    "	 float cr = texture(s_textureUV, vv2texcoord).r - 0.5; \n"
    "	 yuv = vec3(lumia, cb, cr);                              \n"
    "	 rgb = yuv * mat3(1, 1.779, 0,                           \n"
    "                     1, -0.3455, -0.7169,                   \n"
    "                     1, 0, 1.4075);                         \n"
    "    fragColor = vec4(rgb, 1.0);                          \n"
    "}                                                           \n"

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
    "precision highp float;                                     \n"
    "uniform sampler2D s_textureRGB;                            \n"
    "smooth centroid in vec2 vv2texcoord;                       \n"
    "out vec4 fragColor;                          				\n"
    "void main()                                                \n"
    "{                                                          \n"
    "    fragColor = texture(s_textureRGB, vv2texcoord);       	\n"
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
#if 0
char fragmentCameraSource[] =
{
    "#version 300 es						  \n"
    "precision highp float;                                      \n"
    "uniform sampler2D s_textureRGBA;                               \n"
    "smooth centroid in vec2 vv2texcoord;                        \n"
    "out vec4 fragColor;                          				 \n"
    "void main()                                                 \n"
    "{                                                           \n"
    "    fragColor = texture(s_textureRGBA, vv2texcoord);        \n"
    "}                                                           \n"

};
#endif

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

/*====================================================================
?$)A=f???'0:   caculateColorCoeff
?$)A=f????:   h.!g???8*?:e?????2h??4e??
$)Ag.??e.??:
?$)A(e????:
$)Ah>?????:   imageBuffer:??7/???e$4e????i&????
$)Ah???g;??:   ??
$)Ad???h.0e?o<?
====================================================================*/
void caculateColorCoeff(unsigned char **imageBuffer)
{

    int i, j;
    unsigned int rSum0, rSum1;
    unsigned int gSum0, gSum1;
    unsigned int bSum0, bSum1;

    int index0, index1;
    CvPoint3D32f rgbColor[12];

    unsigned int count;
    static unsigned int runCount = 0;
    int index;
    CvPoint3D32f addSum[8];

    index = runCount % AVERAGE_COUNT;

    rSum0 = 0;
    rSum1 = 0;
    gSum0 = 0;
    gSum1 = 0;
    bSum0 = 0;
    bSum1 = 0;

    for(i = 0; i < texCoordsStatistics.glTexCoord_FL_F.size(); i++)
    {
        index0 = texCoordsStatistics.glTexCoord_FL_F[i] * 4;
        rSum0 += imageBuffer[0][index0 + 0];
        gSum0 += imageBuffer[0][index0 + 1];
        bSum0 += imageBuffer[0][index0 + 2];


        index1 = texCoordsStatistics.glTexCoord_FL_L[i] * 4;
        rSum1 += imageBuffer[2][index1 + 0];
        gSum1 += imageBuffer[2][index1 + 1];
        bSum1 += imageBuffer[2][index1 + 2];

    }

    count = texCoordsStatistics.glTexCoord_FL_F.size();
    rgbColor[0].x = 1.0 * rSum0 / count;
    rgbColor[0].y = 1.0 * gSum0 / count;
    rgbColor[0].z = 1.0 * bSum0 / count;

    rgbColor[1].x = 1.0 * rSum1 / count;
    rgbColor[1].y = 1.0 * gSum1 / count;
    rgbColor[1].z = 1.0 * bSum1 / count;


    rSum0 = 0;
    rSum1 = 0;
    gSum0 = 0;
    gSum1 = 0;
    bSum0 = 0;
    bSum1 = 0;

    for(i = 0; i < texCoordsStatistics.glTexCoord_FR_F.size(); i++)
    {
        index0 = texCoordsStatistics.glTexCoord_FR_F[i] * 4;
        rSum0 += imageBuffer[0][index0 + 0];
        gSum0 += imageBuffer[0][index0 + 1];
        bSum0 += imageBuffer[0][index0 + 2];



        index1 = texCoordsStatistics.glTexCoord_FR_R[i] * 4;
        rSum1 += imageBuffer[3][index1 + 0];
        gSum1 += imageBuffer[3][index1 + 1];
        bSum1 += imageBuffer[3][index1 + 2];

    }

    count = texCoordsStatistics.glTexCoord_FR_F.size();

    rgbColor[2].x = 1.0 * rSum0 / count;
    rgbColor[2].y = 1.0 * gSum0 / count;
    rgbColor[2].z = 1.0 * bSum0 / count;

    rgbColor[3].x = 1.0 * rSum1 / count;
    rgbColor[3].y = 1.0 * gSum1 / count;
    rgbColor[3].z = 1.0 * bSum1 / count;


    rSum0 = 0;
    rSum1 = 0;
    gSum0 = 0;
    gSum1 = 0;
    bSum0 = 0;
    bSum1 = 0;

    for(i = 0; i < texCoordsStatistics.glTexCoord_BL_B.size(); i++)
    {
        index0 = texCoordsStatistics.glTexCoord_BL_B[i] * 4;
        rSum0 += imageBuffer[1][index0 + 0];
        gSum0 += imageBuffer[1][index0 + 1];
        bSum0 += imageBuffer[1][index0 + 2];



        index1 = texCoordsStatistics.glTexCoord_BL_L[i] * 4;
        rSum1 += imageBuffer[2][index1 + 0];
        gSum1 += imageBuffer[2][index1 + 1];
        bSum1 += imageBuffer[2][index1 + 2];

    }

    count = texCoordsStatistics.glTexCoord_BL_B.size();

    rgbColor[4].x = 1.0 * rSum0 / count;
    rgbColor[4].y = 1.0 * gSum0 / count;
    rgbColor[4].z = 1.0 * bSum0 / count;

    rgbColor[5].x = 1.0 * rSum1 / count;
    rgbColor[5].y = 1.0 * gSum1 / count;
    rgbColor[5].z = 1.0 * bSum1 / count;


    rSum0 = 0;
    rSum1 = 0;
    gSum0 = 0;
    gSum1 = 0;
    bSum0 = 0;
    bSum1 = 0;

    for(i = 0; i < texCoordsStatistics.glTexCoord_BR_B.size(); i++)
    {
        index0 = texCoordsStatistics.glTexCoord_BR_B[i] * 4;
        rSum0 += imageBuffer[1][index0 + 0];
        gSum0 += imageBuffer[1][index0 + 1];
        bSum0 += imageBuffer[1][index0 + 2];


        index1 = texCoordsStatistics.glTexCoord_BR_R[i] * 4;
        rSum1 += imageBuffer[3][index1 + 0];
        gSum1 += imageBuffer[3][index1 + 1];
        bSum1 += imageBuffer[3][index1 + 2];
    }

    count = texCoordsStatistics.glTexCoord_BR_B.size();

    rgbColor[6].x = 1.0 * rSum0 / count;
    rgbColor[6].y = 1.0 * gSum0 / count;
    rgbColor[6].z = 1.0 * bSum0 / count;

    rgbColor[7].x = 1.0 * rSum1 / count;
    rgbColor[7].y = 1.0 * gSum1 / count;
    rgbColor[7].z = 1.0 * bSum1 / count;

    for(i = 0; i < 8; i ++)
    {
        colorCount[index][i].x = rgbColor[i].x / 255;
        colorCount[index][i].y = rgbColor[i].y / 255;
        colorCount[index][i].z = rgbColor[i].z / 255;
    }

    runCount++;

    if(runCount < AVERAGE_COUNT)
    {
        memset(colorAve, 0, 4 * 3 * 12);
        return;
    }
    else
    {
        for(i = 0; i < 8; i++)
        {
            addSum[i].x = 0;
            addSum[i].y = 0;
            addSum[i].z = 0;
            for(j = 0; j < AVERAGE_COUNT; j++)
            {
                addSum[i].x += colorCount[j][i].x;
                addSum[i].y += colorCount[j][i].y;
                addSum[i].z += colorCount[j][i].z;
            }
            colorAve[i].x = addSum[i].x / AVERAGE_COUNT;
            colorAve[i].y = addSum[i].y / AVERAGE_COUNT;
            colorAve[i].z = addSum[i].z / AVERAGE_COUNT;
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

    //memset(colorAve, 0, 4*3*12);

    //for(i=0; i<12; i++)
    //printf("count = %d, idx=%d ===== %f %f %f\n", runCount, i, colorAve[i].x, colorAve[i].y, colorAve[i].z);
}


#if 0
/*====================================================================
?$)A=f???'0:   caculateColorCoeff2D
?$)A=f????:   h.!g???8*?:e?????2h??4e??
$)Ag.??e.??:
?$)A(e????:
$)Ah>?????:   imageBuffer:??7/???e$4e????i&????
$)Ah???g;??:   ??
$)Ad???h.0e?o<?
====================================================================*/
void caculateColorCoeff2D(unsigned char **imageBuffer)
{

    int i, j;
    unsigned int rSum0, rSum1;
    unsigned int gSum0, gSum1;
    unsigned int bSum0, bSum1;
	unsigned int lumiaSum0, lumiaSum1;

    int index0, index1;
    CvPoint3D32f rgbColor[12];

    unsigned int count;
    static unsigned int runCount = 0;
    int index;
    CvPoint3D32f addSum[8];

    index = runCount % AVERAGE_COUNT;

    rSum0 = 0;
    rSum1 = 0;
    gSum0 = 0;
    gSum1 = 0;
    bSum0 = 0;
    bSum1 = 0;

    lumiaSum0 = 0;
	lumiaSum1 = 0;

    for(i = 0; i < texCoordsStatistics2D.glTexCoord_FL_F.size(); i++)
    {
        index0 = texCoordsStatistics2D.glTexCoord_FL_F[i] * 4;
        rSum0 += imageBuffer[0][index0 + 0];
        gSum0 += imageBuffer[0][index0 + 1];
        bSum0 += imageBuffer[0][index0 + 2];


        index1 = texCoordsStatistics2D.glTexCoord_FL_L[i] * 4;
        rSum1 += imageBuffer[2][index1 + 0];
        gSum1 += imageBuffer[2][index1 + 1];
        bSum1 += imageBuffer[2][index1 + 2];

    }

    count = texCoordsStatistics2D.glTexCoord_FL_F.size();
    rgbColor[0].x = 1.0 * rSum0 / count;
    rgbColor[0].y = 1.0 * gSum0 / count;
    rgbColor[0].z = 1.0 * bSum0 / count;

    rgbColor[1].x = 1.0 * rSum1 / count;
    rgbColor[1].y = 1.0 * gSum1 / count;
    rgbColor[1].z = 1.0 * bSum1 / count;

    printf("%d\n", count);
    //fflush(stdout);

    rSum0 = 0;
    rSum1 = 0;
    gSum0 = 0;
    gSum1 = 0;
    bSum0 = 0;
    bSum1 = 0;

    for(i = 0; i < texCoordsStatistics2D.glTexCoord_FR_F.size(); i++)
    {
        index0 = texCoordsStatistics2D.glTexCoord_FR_F[i] * 4;
        rSum0 += imageBuffer[0][index0 + 0];
        gSum0 += imageBuffer[0][index0 + 1];
        bSum0 += imageBuffer[0][index0 + 2];



        index1 = texCoordsStatistics2D.glTexCoord_FR_R[i] * 4;
        rSum1 += imageBuffer[3][index1 + 0];
        gSum1 += imageBuffer[3][index1 + 1];
        bSum1 += imageBuffer[3][index1 + 2];

    }

    count = texCoordsStatistics2D.glTexCoord_FR_F.size();

    rgbColor[2].x = 1.0 * rSum0 / count;
    rgbColor[2].y = 1.0 * gSum0 / count;
    rgbColor[2].z = 1.0 * bSum0 / count;

    rgbColor[3].x = 1.0 * rSum1 / count;
    rgbColor[3].y = 1.0 * gSum1 / count;
    rgbColor[3].z = 1.0 * bSum1 / count;


    rSum0 = 0;
    rSum1 = 0;
    gSum0 = 0;
    gSum1 = 0;
    bSum0 = 0;
    bSum1 = 0;

    for(i = 0; i < texCoordsStatistics2D.glTexCoord_BL_B.size(); i++)
    {
        index0 = texCoordsStatistics2D.glTexCoord_BL_B[i] * 4;
        rSum0 += imageBuffer[1][index0 + 0];
        gSum0 += imageBuffer[1][index0 + 1];
        bSum0 += imageBuffer[1][index0 + 2];



        index1 = texCoordsStatistics2D.glTexCoord_BL_L[i] * 4;
        rSum1 += imageBuffer[2][index1 + 0];
        gSum1 += imageBuffer[2][index1 + 1];
        bSum1 += imageBuffer[2][index1 + 2];

    }

    count = texCoordsStatistics2D.glTexCoord_BL_B.size();

    rgbColor[4].x = 1.0 * rSum0 / count;
    rgbColor[4].y = 1.0 * gSum0 / count;
    rgbColor[4].z = 1.0 * bSum0 / count;

    rgbColor[5].x = 1.0 * rSum1 / count;
    rgbColor[5].y = 1.0 * gSum1 / count;
    rgbColor[5].z = 1.0 * bSum1 / count;


    rSum0 = 0;
    rSum1 = 0;
    gSum0 = 0;
    gSum1 = 0;
    bSum0 = 0;
    bSum1 = 0;

    for(i = 0; i < texCoordsStatistics2D.glTexCoord_BR_B.size(); i++)
    {
        index0 = texCoordsStatistics2D.glTexCoord_BR_B[i] * 4;
        rSum0 += imageBuffer[1][index0 + 0];
        gSum0 += imageBuffer[1][index0 + 1];
        bSum0 += imageBuffer[1][index0 + 2];


        index1 = texCoordsStatistics2D.glTexCoord_BR_R[i] * 4;
        rSum1 += imageBuffer[3][index1 + 0];
        gSum1 += imageBuffer[3][index1 + 1];
        bSum1 += imageBuffer[3][index1 + 2];
    }

    count = texCoordsStatistics2D.glTexCoord_BR_B.size();

    rgbColor[6].x = 1.0 * rSum0 / count;
    rgbColor[6].y = 1.0 * gSum0 / count;
    rgbColor[6].z = 1.0 * bSum0 / count;

    rgbColor[7].x = 1.0 * rSum1 / count;
    rgbColor[7].y = 1.0 * gSum1 / count;
    rgbColor[7].z = 1.0 * bSum1 / count;

    for(i = 0; i < 8; i ++)
    {
        colorCount[index][i].x = rgbColor[i].x / 255;
        colorCount[index][i].y = rgbColor[i].y / 255;
        colorCount[index][i].z = rgbColor[i].z / 255;
    }

    runCount++;

    if(runCount < AVERAGE_COUNT)
    {
        memset(colorAve, 0, 4 * 3 * 12);
        return;
    }
    else
    {
        for(i = 0; i < 8; i++)
        {
            addSum[i].x = 0;
            addSum[i].y = 0;
            addSum[i].z = 0;
            for(j = 0; j < AVERAGE_COUNT; j++)
            {
                addSum[i].x += colorCount[j][i].x;
                addSum[i].y += colorCount[j][i].y;
                addSum[i].z += colorCount[j][i].z;
            }
            colorAve[i].x = addSum[i].x / AVERAGE_COUNT;
            colorAve[i].y = addSum[i].y / AVERAGE_COUNT;
            colorAve[i].z = addSum[i].z / AVERAGE_COUNT;
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

    //memset(colorAve, 0, 4*3*12);

    for(i = 0; i < 12; i++)
        printf("count = %d, idx=%d ===== %f %f %f\n", runCount, i, colorAve[i].x, colorAve[i].y, colorAve[i].z);
}

#else

/*====================================================================
?$)A=f???'0:   caculateColorCoeff2D
?$)A=f????:   h.!g???8*?:e?????2h??4e??
$)Ag.??e.??:
?$)A(e????:
$)Ah>?????:   imageBuffer:??7/???e$4e????i&????
$)Ah???g;??:   ??
$)Ad???h.0e?o<?
====================================================================*/
void caculateColorCoeff2D(unsigned char **imageBuffer)
{

    int i, j;
	unsigned int lumiaSum0, lumiaSum1;

    int index0, index1;
    CvPoint3D32f rgbColor[12];

    unsigned int count;
    static unsigned int runCount = 0;
    int index;
    CvPoint3D32f addSum[8];

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

    //printf("lumiaSum0 = %d lumiaSum1 = %d\n",lumiaSum0, lumiaSum1);

    /*for(i = 0; i < 8; i ++)
    {
        colorCount[index][i].x = rgbColor[i].x / 255;
        colorCount[index][i].y = rgbColor[i].y / 255;
        colorCount[index][i].z = rgbColor[i].z / 255;
    }

    runCount++;

    if(runCount < AVERAGE_COUNT)
    {
        memset(colorAve, 0, 4 * 3 * 12);
        return;
    }
    else
    {
        for(i = 0; i < 8; i++)
        {
            addSum[i].x = 0;
            addSum[i].y = 0;
            addSum[i].z = 0;
            for(j = 0; j < AVERAGE_COUNT; j++)
            {
                addSum[i].x += colorCount[j][i].x;
                addSum[i].y += colorCount[j][i].y;
                addSum[i].z += colorCount[j][i].z;
            }
            colorAve[i].x = addSum[i].x / AVERAGE_COUNT;
            colorAve[i].y = addSum[i].y / AVERAGE_COUNT;
            colorAve[i].z = addSum[i].z / AVERAGE_COUNT;
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
    }*/

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

    //memset(colorAve, 0, 4*3*12);

    //for(i = 0; i < 12; i++)
        //printf("count = %d, idx=%d ===== %f %f %f\n", runCount, i, colorAve[i].x, colorAve[i].y, colorAve[i].z);
}
#endif

/*====================================================================
?$)A=f???'0:   loadImage
?$)A=f????:   ??==????????g;??g:9g?
$)Ag.??e.??:
?$)A(e????:
$)Ah>?????:g:9g????i+?
$)Ah???g;??:   ??
$)Ad???h.0e?o<?
====================================================================*/
void loadImage(int w, int h)
{
    int i, j;

    int width, height, channel;
    unsigned char *yuvBuffer[5];
    unsigned char *resBuffer[4];
    FILE *fp;
    char filePath[256] = {0};

    unsigned char *pSrcY, *pSrcU, *pSrcV;


    /*for(i = 0; i < 5; i++)
    {
    	sprintf(filePath, "./test/jiaobanche/%d.png", i+10);

    	imageBuffer[i] = stbi_load(filePath, &width, &height, &channel, 0);
    	//printf("image %d %d %d\n", width, height, channel);
    	glGenTextures( 1, &textureRGBA[i]);
    	bindTexture(textureRGBA[i], imageBuffer[i], w, h, GL_RGBA);
    }*/

    for(i = 0; i < 5; i++)
    {
        sprintf(filePath, "./test/jiaobanche/%d.yuv", i);

        yuvBuffer[i] = (unsigned char *)malloc(IMGWIDTH * IMGHEIGHT * 3 / 2);
        fp = fopen(filePath, "rb");
        fread(yuvBuffer[i], 1, IMGWIDTH * IMGHEIGHT * 3 / 2, fp);
        fclose(fp);

        pSrcY = yuvBuffer[i];
        pSrcU = pSrcY + IMGWIDTH * IMGHEIGHT;
        pSrcV = pSrcU + IMGWIDTH * IMGHEIGHT / 4;

        //imageBuffer[i] = stbi_load(filePath, &width, &height, &channel, 0);
        //printf("image %d %d %d\n", width, height, channel);
        //glGenTextures( 1, &textureYUV[i]);
        //bindTexture(textureRGBA[i], imageBuffer[i], w, h, GL_RGBA);

        glGenTextures(1, &textureY[i]);
        bindTexture(textureY[i], pSrcY, IMGWIDTH, IMGHEIGHT, GL_LUMINANCE);

        glGenTextures(1, &textureU[i]);
        bindTexture(textureU[i], pSrcU, IMGWIDTH / 2, IMGHEIGHT / 2, GL_LUMINANCE);

        glGenTextures(1, &textureV[i]);
        bindTexture(textureV[i], pSrcV, IMGWIDTH / 2, IMGHEIGHT / 2, GL_LUMINANCE);
    }


    stbi_set_flip_vertically_on_load(0);

#if 1
    resBuffer[0] = stbi_load("./test/Car.png", &width, &height, &channel, 0);
    glGenTextures( 1, &textureRes[0]);
    bindTexture(textureRes[0], resBuffer[0], width, height, GL_RGBA);

    // resBuffer[1] = stbi_load("../test/zebra.bmp", &width, &height, &channel, 0);
    resBuffer[1] = stbi_load("./test/car_new.png", &width, &height, &channel, 0);

    for(i = 0; i < width * height * channel; i += channel)
    {
        if(resBuffer[1][i] >= 250 &&
                resBuffer[1][i + 1] >= 250 &&
                resBuffer[1][i + 2] >= 250)
        {
            resBuffer[1][i + 3] = 0;
        }
    }
    glGenTextures( 1, &textureRes[1]);
    bindTexture(textureRes[1], resBuffer[1], width, height, GL_RGBA);

    //printf("$%^&* %d %d %d\n", width, height, channel);
#endif

    //for(i=0;i<22;i++)
    //caculateColorCoeff2D(&imageBuffer[0]);

}


#if 0
///
// Create a simple cubemap with a 1x1 face with a different
// color for each face
GLuint CreateSimpleTextureCubemap( )
{
    int width, height, channel;
    unsigned char *pImg;

    GLuint textureId;

    // Generate a texture object
    glGenTextures ( 1, &textureId );

    // Bind the texture object
    glBindTexture ( GL_TEXTURE_CUBE_MAP, textureId );

    pImg = stbi_load("D:/test/cube-1.jpg", &width, &height, &channel, 0);

    printf("%d %d\n", width, height);
    // Load the cube face - Positive X
    glTexImage2D ( GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0,
                   GL_RGB, GL_UNSIGNED_BYTE, pImg );

    pImg = stbi_load("D:/test/cube-2.jpg", &width, &height, &channel, 0);
    // Load the cube face - Negative X
    glTexImage2D ( GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0,
                   GL_RGB, GL_UNSIGNED_BYTE, pImg );

    pImg = stbi_load("D:/test/cube-3.jpg", &width, &height, &channel, 0);
    // Load the cube face - Positive Y
    glTexImage2D ( GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0,
                   GL_RGB, GL_UNSIGNED_BYTE, pImg );

    pImg = stbi_load("D:/test/cube-4.jpg", &width, &height, &channel, 0);
    // Load the cube face - Negative Y
    glTexImage2D ( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0,
                   GL_RGB, GL_UNSIGNED_BYTE, pImg );

    pImg = stbi_load("D:/test/cube-5.jpg", &width, &height, &channel, 0);
    // Load the cube face - Positive Z
    glTexImage2D ( GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0,
                   GL_RGB, GL_UNSIGNED_BYTE, pImg );

    pImg = stbi_load("D:/test/cube-6.jpg", &width, &height, &channel, 0);
    // Load the cube face - Negative Z
    glTexImage2D ( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0,
                   GL_RGB, GL_UNSIGNED_BYTE, pImg );

    // Set the filtering mode
    glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    return textureId;

}


///
// Create a simple cubemap with a 1x1 face with a different
// color for each face
GLuint CreateSimpleTextureCubemap1( )
{
    int width, height, channel;
    unsigned char *pImg;

    GLuint textureId;

    // Generate a texture object
    glGenTextures ( 1, &textureId );

    // Bind the texture object
    glBindTexture ( GL_TEXTURE_CUBE_MAP, textureId );

    pImg = stbi_load("../test/skyBox/CoitTower/posx.jpg", &width, &height, &channel, 0);

    printf("%d %d\n", width, height);
    // Load the cube face - Positive X
    glTexImage2D ( GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0,
                   GL_RGB, GL_UNSIGNED_BYTE, pImg );

    pImg = stbi_load("../test/skyBox/CoitTower/negx.jpg", &width, &height, &channel, 0);
    // Load the cube face - Negative X
    glTexImage2D ( GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0,
                   GL_RGB, GL_UNSIGNED_BYTE, pImg );

    pImg = stbi_load("../test/skyBox/CoitTower/posy.jpg", &width, &height, &channel, 0);
    // Load the cube face - Positive Y
    glTexImage2D ( GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0,
                   GL_RGB, GL_UNSIGNED_BYTE, pImg );

    pImg = stbi_load("../test/skyBox/CoitTower/negy.jpg", &width, &height, &channel, 0);
    // Load the cube face - Negative Y
    glTexImage2D ( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0,
                   GL_RGB, GL_UNSIGNED_BYTE, pImg );

    pImg = stbi_load("../test/skyBox/CoitTower/posz.jpg", &width, &height, &channel, 0);
    // Load the cube face - Positive Z
    glTexImage2D ( GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0,
                   GL_RGB, GL_UNSIGNED_BYTE, pImg );

    pImg = stbi_load("../test/skyBox/CoitTower/negz.jpg", &width, &height, &channel, 0);
    // Load the cube face - Negative Z
    glTexImage2D ( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0,
                   GL_RGB, GL_UNSIGNED_BYTE, pImg );

    // Set the filtering mode
    glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    return textureId;

}
#endif

#if 0
/*====================================================================
?$)A=f???'0:   esLoadShader
?$)A=f????:   ??==???2e?
$)Ag.??e.??:
?$)A(e????:
$)Ah>?????:   type:???2e?g1;e?o<?haderSrc:???2e?f:??
$)Ah???g;??:   g<??e%=g????2e?
$)Ad???h.0e?o<?
====================================================================*/
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
            infoLog = (char *)malloc ( sizeof ( char ) * infoLen );

            glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
            printf ( "Error compiling shader:\n%s\n", infoLog );

            free ( infoLog );
        }

        glDeleteShader ( shader );
        return 0;
    }

    return shader;

}


/*====================================================================
?$)A=f???'0:   esLoadProgram
?$)A=f????:   ??;:???2e??%f?
$)Ag.??e.??:
?$)A(e????:
$)Ah>?????:   vertShaderSrc:i!6g????2e?f:??o<?ragShaderSrc:??????2e?f:??
$)Ah???g;??:   ??;:e%=g??%f?
$)Ad???h.0e?o<?
====================================================================*/
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
            infoLog = (char *)malloc ( sizeof ( char ) * infoLen );

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


/*====================================================================
?$)A=f???'0:   setBool
?$)A=f????:   g;???2e??????88?????
$)Ag.??e.??:
?$)A(e????:
$)Ah>?????:   programId:???2e??%f?o<?ame:?????'0o<?alue:???????
$)Ah???g;??:   ??
$)Ad???h.0e?o<?
====================================================================*/
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



/*====================================================================
?$)A=f???'0:   rotationMatrixToEulerAngles
?$)A=f????:   ??=,?)i?h=????????
$)Ag.??e.??:
?$)A(e????:
$)Ah>?????:   Ro<??h=???5o?alpha\beta\gamma h>??d8?8*f,'f?h'?
$)Ah???g;??:   ??
$)Ad???h.0e?o<?
====================================================================*/
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

    printf("%d %d %d %d %d\n", hProgramHandle[0], hProgramHandle[1], hProgramHandle[2], hProgramHandle[3], hProgramHandle[4]);

    return GL_TRUE;
}


#if 0
unsigned char *carResBuffer[10];

unsigned char *turnSignalBuffer[10];

void loadTurnSignalPic1()
{
    int width, height, channel;

    turnSignalBuffer[0] = stbi_load("D:/test/LAMP_HEAD_OFF_AND_TURN_0.bmp", &width, &height, &channel, 0);
    turnSignalBuffer[1] = stbi_load("D:/test/LAMP_HEAD_OFF_AND_TURN_1.bmp", &width, &height, &channel, 0);
    turnSignalBuffer[2] = stbi_load("D:/test/LAMP_HEAD_OFF_AND_TURN_2.bmp", &width, &height, &channel, 0);
    turnSignalBuffer[3] = stbi_load("D:/test/LAMP_HEAD_OFF_AND_TURN_3.bmp", &width, &height, &channel, 0);
    turnSignalBuffer[4] = stbi_load("D:/test/LAMP_HEAD_ON_AND_TURN_0.bmp", &width, &height, &channel, 0);
    turnSignalBuffer[5] = stbi_load("D:/test/LAMP_HEAD_ON_AND_TURN_1.bmp", &width, &height, &channel, 0);
    turnSignalBuffer[6] = stbi_load("D:/test/LAMP_HEAD_ON_AND_TURN_2.bmp", &width, &height, &channel, 0);
    turnSignalBuffer[7] = stbi_load("D:/test/LAMP_HEAD_ON_AND_TURN_3.bmp", &width, &height, &channel, 0);
    turnSignalBuffer[8] = stbi_load("D:/test/LAMP_TAIL_OFF.bmp", &width, &height, &channel, 0);
    turnSignalBuffer[9] = stbi_load("D:/test/LAMP_TAIL_ON.bmp", &width, &height, &channel, 0);
}

void loadCarPic1()
{

    int width, height, channel;
    int i;


    //carResBuffer[1] = stbi_load("D:/test/door.bmp", &width, &height, &channel, 0);
    //carResBuffer[2] = stbi_load("D:/test/door.bmp", &width, &height, &channel, 0);
    //carResBuffer[3] = stbi_load("D:/test/door.bmp", &width, &height, &channel, 0);

    for(i = 0; i < carMaterialList.size(); i++)
    {
        if(strcmp(carMaterialList[i].name, "BODY") == 0)
        {
            carResBuffer[0] = stbi_load("D:/test/BODY5.bmp", &width, &height, &channel, 0);
            glGenTextures( 1, &carTextureIDList[i]);
            bindTexture(carTextureIDList[i], carResBuffer[0], width, height, GL_RGB);
        }
        else if(strcmp(carMaterialList[i].name, "GLASS") == 0)
        {
            carResBuffer[1] = stbi_load("D:/test/GLASS8.bmp", &width, &height, &channel, 0);
            glGenTextures( 1, &carTextureIDList[i]);
            bindTexture(carTextureIDList[i], carResBuffer[1], width, height, GL_RGB);
        }
        else if(strcmp(carMaterialList[i].name, "WHEEL_HUB") == 0)
        {
            carResBuffer[2] = stbi_load("D:/test/WHEEL_HUB.bmp", &width, &height, &channel, 0);
            glGenTextures( 1, &carTextureIDList[i]);
            bindTexture(carTextureIDList[i], carResBuffer[2], width, height, GL_RGB);
        }
        else if(strcmp(carMaterialList[i].name, "LINE") == 0)
        {
            carResBuffer[3] = stbi_load("D:/test/LINE.bmp", &width, &height, &channel, 0);
            glGenTextures( 1, &carTextureIDList[i]);
            bindTexture(carTextureIDList[i], carResBuffer[3], width, height, GL_RGB);
        }
        else if(strcmp(carMaterialList[i].name, "WHEEL") == 0)
        {
            carResBuffer[4] = stbi_load("D:/test/WHEEL.bmp", &width, &height, &channel, 0);
            glGenTextures( 1, &carTextureIDList[i]);
            bindTexture(carTextureIDList[i], carResBuffer[4], width, height, GL_RGB);
        }
        else if(strcmp(carMaterialList[i].name, "LAMP_TAIL") == 0)
        {
            carResBuffer[5] = stbi_load("D:/test/LAMP_TAIL_OFF.bmp", &width, &height, &channel, 0);
            glGenTextures( 1, &carTextureIDList[i]);
            bindTexture(carTextureIDList[i], carResBuffer[5], width, height, GL_RGB);
        }
        else if(strcmp(carMaterialList[i].name, "LAMP_HEAD_AND_TURN") == 0)
        {
            carResBuffer[6] = stbi_load("D:/test/LAMP_HEAD_OFF_AND_TURN_0.bmp", &width, &height, &channel, 0);
            glGenTextures( 1, &carTextureIDList[i]);
            bindTexture(carTextureIDList[i], carResBuffer[6], width, height, GL_RGB);
        }
        else
        {
            carResBuffer[7] = stbi_load("D:/test/sky4.bmp", &width, &height, &channel, 0);
            glGenTextures( 1, &carTextureIDList[i]);
            bindTexture(carTextureIDList[i], carResBuffer[7], width, height, GL_RGB);
        }
    }

    loadTurnSignalPic1();
}

void loadTurnSignalPic()
{
    int width, height, channel;

    turnSignalBuffer[0] = stbi_load(CAR_PATH"Map/LAMP_HEAD_OFF_AND_TURN_0.bmp", &width, &height, &channel, 0);
    turnSignalBuffer[1] = stbi_load(CAR_PATH"Map/LAMP_HEAD_OFF_AND_TURN_1.bmp", &width, &height, &channel, 0);
    turnSignalBuffer[2] = stbi_load(CAR_PATH"Map/LAMP_HEAD_OFF_AND_TURN_2.bmp", &width, &height, &channel, 0);
    turnSignalBuffer[3] = stbi_load(CAR_PATH"Map/LAMP_HEAD_OFF_AND_TURN_3.bmp", &width, &height, &channel, 0);
    turnSignalBuffer[4] = stbi_load(CAR_PATH"Map/LAMP_HEAD_ON_AND_TURN_0.bmp", &width, &height, &channel, 0);
    turnSignalBuffer[5] = stbi_load(CAR_PATH"Map/LAMP_HEAD_ON_AND_TURN_1.bmp", &width, &height, &channel, 0);
    turnSignalBuffer[6] = stbi_load(CAR_PATH"Map/LAMP_HEAD_ON_AND_TURN_2.bmp", &width, &height, &channel, 0);
    turnSignalBuffer[7] = stbi_load(CAR_PATH"Map/LAMP_HEAD_ON_AND_TURN_3.bmp", &width, &height, &channel, 0);
    turnSignalBuffer[8] = stbi_load(CAR_PATH"Map/LAMP_TAIL_OFF.bmp", &width, &height, &channel, 0);
    turnSignalBuffer[9] = stbi_load(CAR_PATH"Map/LAMP_TAIL_ON.bmp", &width, &height, &channel, 0);
}


/*====================================================================
?$)A=f???'0:   loadCarPic
?$)A=f????:   ?9f?h=?(!???h4(e?g'0e?h==f?e/9e???44?>f?d;?
$)Ag.??e.??:
?$)A(e????:
$)Ah>?????:   ??
$)Ah???g;??:   ??
$)Ad???h.0e?o<?
====================================================================*/
void loadCarPic()
{

    int width, height, channel;
    int i;
    unsigned char *carResBuffer[16];

    for(i = 0; i < carMaterialList.size(); i++)
    {
        if(strcmp(carMaterialList[i].name, "WHEEL_HUB") == 0)
        {
            carResBuffer[0] = stbi_load(CAR_PATH"Map/WHEEL_HUB.bmp", &width, &height, &channel, 0);
            glGenTextures( 1, &carTextureIDList[i]);
            bindTexture(carTextureIDList[i], carResBuffer[0], width, height, GL_RGB);
        }
        else if(strcmp(carMaterialList[i].name, "LINE") == 0)
        {
            carResBuffer[1] = stbi_load(CAR_PATH"Map/LINE.bmp", &width, &height, &channel, 0);
            glGenTextures( 1, &carTextureIDList[i]);
            bindTexture(carTextureIDList[i], carResBuffer[1], width, height, GL_RGB);
        }
        else if(strcmp(carMaterialList[i].name, "WHEEL") == 0)
        {
            carResBuffer[2] = stbi_load(CAR_PATH"Map/WHEEL.bmp", &width, &height, &channel, 0);
            glGenTextures( 1, &carTextureIDList[i]);
            bindTexture(carTextureIDList[i], carResBuffer[2], width, height, GL_RGB);
        }
        else if(strcmp(carMaterialList[i].name, "PLASTIC") == 0)
        {
            carResBuffer[3] = stbi_load(CAR_PATH"Map/PLASTIC.bmp", &width, &height, &channel, 0);
            glGenTextures( 1, &carTextureIDList[i]);
            bindTexture(carTextureIDList[i], carResBuffer[3], width, height, GL_RGB);
        }
        else if(strcmp(carMaterialList[i].name, "FENCE") == 0)
        {
            carResBuffer[4] = stbi_load(CAR_PATH"Map/FENCE.bmp", &width, &height, &channel, 0);
            glGenTextures( 1, &carTextureIDList[i]);
            bindTexture(carTextureIDList[i], carResBuffer[4], width, height, GL_RGB);
        }
        else if(strcmp(carMaterialList[i].name, "LICENSE_PLATE") == 0)
        {
            carResBuffer[5] = stbi_load(CAR_PATH"Map/LICENSE.bmp", &width, &height, &channel, 0);
            glGenTextures( 1, &carTextureIDList[i]);
            bindTexture(carTextureIDList[i], carResBuffer[5], width, height, GL_RGB);
        }
        else if(strcmp(carMaterialList[i].name, "LAMP_TAIL") == 0)
        {
            carResBuffer[6] = stbi_load(CAR_PATH"Map/LAMP_TAIL_OFF.bmp", &width, &height, &channel, 0);
            glGenTextures( 1, &carTextureIDList[i]);
            bindTexture(carTextureIDList[i], carResBuffer[6], width, height, GL_RGB);

            carResBuffer[7] = stbi_load(CAR_PATH"Map/LAMP_TAIL_ON.bmp", &width, &height, &channel, 0);
            glGenTextures( 1, &carTextureIDList[i + 1]);
            bindTexture(carTextureIDList[i + 1], carResBuffer[7], width, height, GL_RGB);
        }
        else if(strcmp(carMaterialList[i].name, "LAMP_HEAD_AND_TURN") == 0)
        {
            carResBuffer[8] = stbi_load(CAR_PATH"Map/LAMP_HEAD_OFF_AND_TURN_0.bmp", &width, &height, &channel, 0);
            glGenTextures( 1, &carTextureIDList[i + 1]);
            bindTexture(carTextureIDList[i + 1], carResBuffer[8], width, height, GL_RGB);

            carResBuffer[9] = stbi_load(CAR_PATH"Map/LAMP_HEAD_OFF_AND_TURN_1.bmp", &width, &height, &channel, 0);
            glGenTextures( 1, &carTextureIDList[i + 2]);
            bindTexture(carTextureIDList[i + 2], carResBuffer[9], width, height, GL_RGB);

            carResBuffer[10] = stbi_load(CAR_PATH"Map/LAMP_HEAD_OFF_AND_TURN_2.bmp", &width, &height, &channel, 0);
            glGenTextures( 1, &carTextureIDList[i + 3]);
            bindTexture(carTextureIDList[i + 3], carResBuffer[10], width, height, GL_RGB);

            carResBuffer[11] = stbi_load(CAR_PATH"Map/LAMP_HEAD_OFF_AND_TURN_3.bmp", &width, &height, &channel, 0);
            glGenTextures( 1, &carTextureIDList[i + 4]);
            bindTexture(carTextureIDList[i + 4], carResBuffer[11], width, height, GL_RGB);

            carResBuffer[12] = stbi_load(CAR_PATH"Map/LAMP_HEAD_ON_AND_TURN_0.bmp", &width, &height, &channel, 0);
            glGenTextures( 1, &carTextureIDList[i + 5]);
            bindTexture(carTextureIDList[i + 5], carResBuffer[12], width, height, GL_RGB);

            carResBuffer[13] = stbi_load(CAR_PATH"Map/LAMP_HEAD_ON_AND_TURN_1.bmp", &width, &height, &channel, 0);
            glGenTextures( 1, &carTextureIDList[i + 6]);
            bindTexture(carTextureIDList[i + 6], carResBuffer[13], width, height, GL_RGB);

            carResBuffer[14] = stbi_load(CAR_PATH"Map/LAMP_HEAD_ON_AND_TURN_2.bmp", &width, &height, &channel, 0);
            glGenTextures( 1, &carTextureIDList[i + 7]);
            bindTexture(carTextureIDList[i + 7], carResBuffer[14], width, height, GL_RGB);

            carResBuffer[15] = stbi_load(CAR_PATH"Map/LAMP_HEAD_ON_AND_TURN_3.bmp", &width, &height, &channel, 0);
            glGenTextures( 1, &carTextureIDList[i + 8]);
            bindTexture(carTextureIDList[i + 8], carResBuffer[15], width, height, GL_RGB);
        }

    }
}

#endif
#if 0
float matrix3x3Determinant(float *matrix)
{
    float result = 0.0f;

    result  = matrix[0] * (matrix[4] * matrix[8] - matrix[7] * matrix[5]);
    result -= matrix[3] * (matrix[1] * matrix[8] - matrix[7] * matrix[2]);
    result += matrix[6] * (matrix[1] * matrix[5] - matrix[4] * matrix[2]);

    return result;
}

float matrixDeterminant(ESMatrix *matrix)
{
    float matrix3x3[9];
    float determinant3x3 = 0.0f;
    float result = 0.0f;
    float *elements;

    elements = &matrix->m[0][0];
    /* Remove (i, j) (1, 1) to form new 3x3 matrix. */
    matrix3x3[0] = elements[ 5];
    matrix3x3[1] = elements[ 6];
    matrix3x3[2] = elements[ 7];
    matrix3x3[3] = elements[ 9];
    matrix3x3[4] = elements[10];
    matrix3x3[5] = elements[11];
    matrix3x3[6] = elements[13];
    matrix3x3[7] = elements[14];
    matrix3x3[8] = elements[15];
    determinant3x3 = matrix3x3Determinant(matrix3x3);
    result += elements[0] * determinant3x3;

    /* Remove (i, j) (1, 2) to form new 3x3 matrix. */
    matrix3x3[0] = elements[ 1];
    matrix3x3[1] = elements[ 2];
    matrix3x3[2] = elements[ 3];
    matrix3x3[3] = elements[ 9];
    matrix3x3[4] = elements[10];
    matrix3x3[5] = elements[11];
    matrix3x3[6] = elements[13];
    matrix3x3[7] = elements[14];
    matrix3x3[8] = elements[15];
    determinant3x3 = matrix3x3Determinant(matrix3x3);
    result -= elements[4] * determinant3x3;

    /* Remove (i, j) (1, 3) to form new 3x3 matrix. */
    matrix3x3[0] = elements[ 1];
    matrix3x3[1] = elements[ 2];
    matrix3x3[2] = elements[ 3];
    matrix3x3[3] = elements[ 5];
    matrix3x3[4] = elements[ 6];
    matrix3x3[5] = elements[ 7];
    matrix3x3[6] = elements[13];
    matrix3x3[7] = elements[14];
    matrix3x3[8] = elements[15];
    determinant3x3 = matrix3x3Determinant(matrix3x3);
    result += elements[8] * determinant3x3;

    /* Remove (i, j) (1, 4) to form new 3x3 matrix. */
    matrix3x3[0] = elements[ 1];
    matrix3x3[1] = elements[ 2];
    matrix3x3[2] = elements[ 3];
    matrix3x3[3] = elements[ 5];
    matrix3x3[4] = elements[ 6];
    matrix3x3[5] = elements[ 7];
    matrix3x3[6] = elements[ 9];
    matrix3x3[7] = elements[10];
    matrix3x3[8] = elements[11];
    determinant3x3 = matrix3x3Determinant(matrix3x3);
    result -= elements[12] * determinant3x3;

    return result;
}

void esMatrixTranspose(ESMatrix *matrix)
{
    float temp;
    float *elements;

    elements = &matrix->m[0][0];

    temp = elements[1];
    elements[1] = elements[4];
    elements[4] = temp;

    temp = elements[2];
    elements[2] = elements[8];
    elements[8] = temp;

    temp = elements[3];
    elements[3] = elements[12];
    elements[12] = temp;

    temp = elements[6];
    elements[6] = elements[9];
    elements[9] = temp;

    temp = elements[7];
    elements[7] = elements[13];
    elements[13] = temp;

    temp = elements[11];
    elements[11] = elements[14];
    elements[14] = temp;
}

ESMatrix esMatrixScale(ESMatrix *matrix, float scale)
{
    ESMatrix result;
    int allElements;

    for(allElements = 0; allElements < 16; allElements ++)
    {
        result.m[allElements / 4][allElements % 4] = matrix->m[allElements / 4][allElements % 4] * scale;
    }

    return result;
}


ESMatrix esMatrixInvert(ESMatrix *matrix)
{
    ESMatrix result;
    float matrix3x3[9];
    float *elements;

    elements = &matrix->m[0][0];

    /* Find the cofactor of each element. */
    /* Element (i, j) (1, 1) */
    matrix3x3[0] = elements[ 5];
    matrix3x3[1] = elements[ 6];
    matrix3x3[2] = elements[ 7];
    matrix3x3[3] = elements[ 9];
    matrix3x3[4] = elements[10];
    matrix3x3[5] = elements[11];
    matrix3x3[6] = elements[13];
    matrix3x3[7] = elements[14];
    matrix3x3[8] = elements[15];
    result.m[0][0] = matrix3x3Determinant(matrix3x3);

    /* Element (i, j) (1, 2) */
    matrix3x3[0] = elements[ 1];
    matrix3x3[1] = elements[ 2];
    matrix3x3[2] = elements[ 3];
    matrix3x3[3] = elements[ 9];
    matrix3x3[4] = elements[10];
    matrix3x3[5] = elements[11];
    matrix3x3[6] = elements[13];
    matrix3x3[7] = elements[14];
    matrix3x3[8] = elements[15];
    result.m[1][0] = -matrix3x3Determinant(matrix3x3);

    /* Element (i, j) (1, 3) */
    matrix3x3[0] = elements[ 1];
    matrix3x3[1] = elements[ 2];
    matrix3x3[2] = elements[ 3];
    matrix3x3[3] = elements[ 5];
    matrix3x3[4] = elements[ 6];
    matrix3x3[5] = elements[ 7];
    matrix3x3[6] = elements[13];
    matrix3x3[7] = elements[14];
    matrix3x3[8] = elements[15];
    result.m[2][0] = matrix3x3Determinant(matrix3x3);

    /* Element (i, j) (1, 4) */
    matrix3x3[0] = elements[ 1];
    matrix3x3[1] = elements[ 2];
    matrix3x3[2] = elements[ 3];
    matrix3x3[3] = elements[ 5];
    matrix3x3[4] = elements[ 6];
    matrix3x3[5] = elements[ 7];
    matrix3x3[6] = elements[ 9];
    matrix3x3[7] = elements[10];
    matrix3x3[8] = elements[11];
    result.m[3][0] = -matrix3x3Determinant(matrix3x3);

    /* Element (i, j) (2, 1) */
    matrix3x3[0] = elements[ 4];
    matrix3x3[1] = elements[ 6];
    matrix3x3[2] = elements[ 7];
    matrix3x3[3] = elements[ 8];
    matrix3x3[4] = elements[10];
    matrix3x3[5] = elements[11];
    matrix3x3[6] = elements[12];
    matrix3x3[7] = elements[14];
    matrix3x3[8] = elements[15];
    result.m[0][1] = -matrix3x3Determinant(matrix3x3);

    /* Element (i, j) (2, 2) */
    matrix3x3[0] = elements[ 0];
    matrix3x3[1] = elements[ 2];
    matrix3x3[2] = elements[ 3];
    matrix3x3[3] = elements[ 8];
    matrix3x3[4] = elements[10];
    matrix3x3[5] = elements[11];
    matrix3x3[6] = elements[12];
    matrix3x3[7] = elements[14];
    matrix3x3[8] = elements[15];
    result.m[1][1] = matrix3x3Determinant(matrix3x3);

    /* Element (i, j) (2, 3) */
    matrix3x3[0] = elements[ 0];
    matrix3x3[1] = elements[ 2];
    matrix3x3[2] = elements[ 3];
    matrix3x3[3] = elements[ 4];
    matrix3x3[4] = elements[ 6];
    matrix3x3[5] = elements[ 7];
    matrix3x3[6] = elements[12];
    matrix3x3[7] = elements[14];
    matrix3x3[8] = elements[15];
    result.m[2][1] = -matrix3x3Determinant(matrix3x3);

    /* Element (i, j) (2, 4) */
    matrix3x3[0] = elements[ 0];
    matrix3x3[1] = elements[ 2];
    matrix3x3[2] = elements[ 3];
    matrix3x3[3] = elements[ 4];
    matrix3x3[4] = elements[ 6];
    matrix3x3[5] = elements[ 7];
    matrix3x3[6] = elements[ 8];
    matrix3x3[7] = elements[10];
    matrix3x3[8] = elements[11];
    result.m[3][1] = matrix3x3Determinant(matrix3x3);

    /* Element (i, j) (3, 1) */
    matrix3x3[0] = elements[ 4];
    matrix3x3[1] = elements[ 5];
    matrix3x3[2] = elements[ 7];
    matrix3x3[3] = elements[ 8];
    matrix3x3[4] = elements[ 9];
    matrix3x3[5] = elements[11];
    matrix3x3[6] = elements[12];
    matrix3x3[7] = elements[13];
    matrix3x3[8] = elements[15];
    result.m[0][2] = matrix3x3Determinant(matrix3x3);

    /* Element (i, j) (3, 2) */
    matrix3x3[0] = elements[ 0];
    matrix3x3[1] = elements[ 1];
    matrix3x3[2] = elements[ 3];
    matrix3x3[3] = elements[ 8];
    matrix3x3[4] = elements[ 9];
    matrix3x3[5] = elements[11];
    matrix3x3[6] = elements[12];
    matrix3x3[7] = elements[13];
    matrix3x3[8] = elements[15];
    result.m[1][2] = -matrix3x3Determinant(matrix3x3);

    /* Element (i, j) (3, 3) */
    matrix3x3[0] = elements[ 0];
    matrix3x3[1] = elements[ 1];
    matrix3x3[2] = elements[ 3];
    matrix3x3[3] = elements[ 4];
    matrix3x3[4] = elements[ 5];
    matrix3x3[5] = elements[ 7];
    matrix3x3[6] = elements[12];
    matrix3x3[7] = elements[13];
    matrix3x3[8] = elements[15];
    result.m[2][2] = matrix3x3Determinant(matrix3x3);

    /* Element (i, j) (3, 4) */
    matrix3x3[0] = elements[ 0];
    matrix3x3[1] = elements[ 1];
    matrix3x3[2] = elements[ 3];
    matrix3x3[3] = elements[ 4];
    matrix3x3[4] = elements[ 5];
    matrix3x3[5] = elements[ 7];
    matrix3x3[6] = elements[ 8];
    matrix3x3[7] = elements[ 9];
    matrix3x3[8] = elements[11];
    result.m[3][2] = -matrix3x3Determinant(matrix3x3);

    /* Element (i, j) (4, 1) */
    matrix3x3[0] = elements[ 4];
    matrix3x3[1] = elements[ 5];
    matrix3x3[2] = elements[ 6];
    matrix3x3[3] = elements[ 8];
    matrix3x3[4] = elements[ 9];
    matrix3x3[5] = elements[10];
    matrix3x3[6] = elements[12];
    matrix3x3[7] = elements[13];
    matrix3x3[8] = elements[14];
    result.m[0][3] = -matrix3x3Determinant(matrix3x3);

    /* Element (i, j) (4, 2) */
    matrix3x3[0] = elements[ 0];
    matrix3x3[1] = elements[ 1];
    matrix3x3[2] = elements[ 2];
    matrix3x3[3] = elements[ 8];
    matrix3x3[4] = elements[ 9];
    matrix3x3[5] = elements[10];
    matrix3x3[6] = elements[12];
    matrix3x3[7] = elements[13];
    matrix3x3[8] = elements[14];
    result.m[1][3] = matrix3x3Determinant(matrix3x3);

    /* Element (i, j) (4, 3) */
    matrix3x3[0] = elements[ 0];
    matrix3x3[1] = elements[ 1];
    matrix3x3[2] = elements[ 2];
    matrix3x3[3] = elements[ 4];
    matrix3x3[4] = elements[ 5];
    matrix3x3[5] = elements[ 6];
    matrix3x3[6] = elements[12];
    matrix3x3[7] = elements[13];
    matrix3x3[8] = elements[14];
    result.m[2][3] = -matrix3x3Determinant(matrix3x3);

    /* Element (i, j) (4, 4) */
    matrix3x3[0] = elements[ 0];
    matrix3x3[1] = elements[ 1];
    matrix3x3[2] = elements[ 2];
    matrix3x3[3] = elements[ 4];
    matrix3x3[4] = elements[ 5];
    matrix3x3[5] = elements[ 6];
    matrix3x3[6] = elements[ 8];
    matrix3x3[7] = elements[ 9];
    matrix3x3[8] = elements[10];
    result.m[3][3] = matrix3x3Determinant(matrix3x3);

    /* The adjoint is the transpose of the cofactor matrix. */
    esMatrixTranspose(&result);

    /* The inverse is the adjoint divided by the determinant. */
    result = esMatrixScale(&result, 1.0f / matrixDeterminant(matrix));

    return result;
}
#endif


#if 0
vec3 changeColor[6] = {{0, 0, 128}, {128, 128, 128}, {64, 64, 64}, {64, 32, 0}, {16, 16, 16}, {128, 0, 0}};

#define ON_OFF 1

static unsigned char bodyColor[3] = {16, 16, 128};	//$)Ah=?(!i"??

/*====================================================================
?$)A=f???'0:   show3DCar
?$)A=f????:   3Df82f??;i??>g$:
$)Ag.??e.??:
?$)A(e????:
$)Ah>?????:   DisplayChannelID  ?>g$:f(!e?
$)Ah???g;??:   ??
$)Ad???h.0e?o<?
====================================================================*/
void show3DCar(int displayChannelID)
{
    FILE *fp;
    static FILE *fpSetup;
    static float angleX = 0, angleY = 0, angleZ = 0;
    vec3 colorAdjust[2];
    int i, j;
    float leftDoorAngle = gDoorAngle;  //$)Ae7??h=??h'?:&
    float rightDoorAngle = -gDoorAngle;
    float theta0, theta1, radius, tx, ty, tz, fov;
    float lightTheta0, lightRadius;
    static unsigned int Count = 0;
    static unsigned int ChangeLamp = 0;
    int turnTexID = 0;  //$)Ah=??g:9g?h44e?g4"e?
    int lampTailStatus = 8;  //$)Ae0>g??6f??44?>g4"e<?
    //static unsigned char bodyColor[3] = {64, 64, 64};  //$)Ah=?(!i"??
    float lightStrength[3];
    float sinPhi, cosPhi;
    float sinTheta, cosTheta;
    GLfloat lightSinTheta, lightCosTheta, lightSinPhi, lightCosPhi;
    float x, y, z;
    unsigned char *imageBuffer;
    time_t tt;
    tm t;
    char filePath[256] = {0};
    int writeFlag = 0;
    int clipX, clipY, clipWidth, clipHeight;
    float isTex, isReflect;
    static int fileOpenFlag = 1;
    static int fileCloseFlag = 1;
    unsigned int faceCount = 0;
    int vertexBlock;
    vec4 scaleLineColor = {1.0, 165.0 / 255.0, 0.0, 0.5}; //$)Ah=(h?9i"??
    vec4 radarColor[3] = {{1.0, 0.0, 0.0, 0.5}, {1.0, 1.0, 0.0, 0.5}, {0.0, 1.0, 0.0, 0.5}};  //?$)A7h>>i"??
    float radarFrontTx[3] = { -0.01, 0.0, 0.01}; //???$)Ah>>e??"g??5h=&??f????g'?
    float radarBackTx[3] = {0.01, 0.0, -0.01};  //???$)Ah>>e??"g??5h=&??f????g'?
    static int radarInfo[8] = {0, 0, 0, 0};
    vec4 zebraColor = {0.0, 0.0, 0.0, 1.0};
    color3 blackGlass = {0.3, 0.3, 0.3};
    int colorIdx = 0;
    int turnTexIDOffset = 1;

    //angleY += 0.25;  //$)Ag;?h=4f?h=?4/??
    angleY = 0;//30 * Count;

    theta0 = 0.0001;
    theta1 = angleY;
    radius = 1.0;
    lightTheta0 = theta0;
    lightRadius = radius;
    fov = 45.0;
    tx = 0.0;
    ty = -0.0;
    tz = 0.0;
    angleZ += 1.0;

    //getchar();
    //printf("%f\n",angleY);

    if(ChangeLamp % 2)
    {
        turnTexID = 1;
        lampTailStatus = 0;
    }
    else
    {
        turnTexID = 6;
        lampTailStatus = 1;
    }
    lightStrength[0] = 0.1;
    lightStrength[1] = 1.5;
    lightStrength[2] = 0.5;


    //$)Ah.>g=.h'???)i?
    sinTheta = (float)sin((theta1 - 90) * PI / 180.0f);
    cosTheta = (float)cos((theta1 - 90) * PI / 180.0f);

    sinPhi = (float)sin(theta0 * PI / 180.0f);
    cosPhi = (float)cos(theta0 * PI / 180.0f);

    //$)Ah.>g=.??????g=?
    lightSinTheta = (float)sin((theta1 - 90) * PI / 180.0f);
    lightCosTheta = (float)cos((theta1 - 90) * PI / 180.0f);

    lightSinPhi = (float)sin(lightTheta0 * PI / 180.0f);
    lightCosPhi = (float)cos(lightTheta0 * PI / 180.0f);


    //?$)A7e???=,?6d;6??8-e???
#if 0
    fp = fopen(CAR_PATH"coordinate.txt", "r");

    fscanf(fp, "WHEEL_F_L:%f,%f,%f\n", &x, &y, &z);

    vec3 LFPWheel = {x, y, z};

    fscanf(fp, "WHEEL_F_R:%f,%f,%f\n", &x, &y, &z);
    vec3 RFPWheel = {x, y, z};

    fscanf(fp, "WHEEL_B_L:%f,%f,%f\n", &x, &y, &z);
    vec3 LBPWheel = {x, y, z};

    fscanf(fp, "WHEEL_B_R:%f,%f,%f\n", &x, &y, &z);
    vec3 RBPWheel = {x, y, z};

    fscanf(fp, "DOOR_F_L:%f,%f,%f\n", &x, &y, &z);
    vec3 FLDoor = {x, y, z};

    fscanf(fp, "DOOR_F_R:%f,%f,%f\n", &x, &y, &z);
    vec3 FRDoor = {x, y, z};

    fscanf(fp, "DOOR_B_L:%f,%f,%f\n", &x, &y, &z);
    vec3 BLDoor = {x, y, z};

    fscanf(fp, "DOOR_B_R:%f,%f,%f\n", &x, &y, &z);
    vec3 BRDoor = {x, y, z};

    fscanf(fp, "TRUNKLID:%f,%f,%f\n", &x, &y, &z);
    vec3 TRUNKLID = {x, y, z};

    fscanf(fp, "TRUNK_LID_DOWN:%f,%f,%f\n", &x, &y, &z);
    vec3 TRUNK_LID_DOWN = {x, y, z};

    fclose(fp);
#else
    vec3 LFPWheel = {x, y, z};

    vec3 RFPWheel = {x, y, z};


    vec3 LBPWheel = {x, y, z};

    vec3 RBPWheel = {x, y, z};

    vec3 FLDoor = {x, y, z};

    vec3 FRDoor = {x, y, z};

    vec3 BLDoor = {x, y, z};

    vec3 BRDoor = {x, y, z};

    vec3 TRUNKLID = {x, y, z};

    vec3 TRUNK_LID_DOWN = {x, y, z};

#endif

    ESMatrix matrixModel, matrixModelT, matrixModelS, matrixView, matrixProjection, matrixModelView, matrixMVP;
    ESMatrix matrixCarModel, matrixCarView, matrixCarProjection, matrixCarModelView, matrixCarMVP;
    ESMatrix matrixCarModelT, matrixCarModelR, matrixCarModelR0, matrixCarModelR1, matrixCarModelScale;

    ESMatrix matrixCarModelInv, identity;

    //$)Ah.>g=.??=1?)i?
    esMatrixLoadIdentity(&matrixProjection);
    esPerspective(&matrixProjection, fov, 1.0f * bvs2DWidth / (float)bvs2DHeight, 0.1f, 10.0f);


    //$)Ai"????????h>>d?g=??h!???:e???
    if(Count % 100 == 0)
    {
        ChangeLamp++;

        colorIdx = (ChangeLamp % 6);

        //bodyColor[0] = changeColor[colorIdx].x;
        //bodyColor[1] = changeColor[colorIdx].y;
        //bodyColor[2] = changeColor[colorIdx].z;

        //bodyColor[0] = (rand() % 255);
        //bodyColor[1] = (rand() % 255);
        //bodyColor[2] = (rand() % 255);

        radarInfo[0] = (rand() % 3);
        radarInfo[1] = (rand() % 3);
        radarInfo[2] = (rand() % 3);
        radarInfo[3] = (rand() % 3);

        radarInfo[4] = (rand() % 3);
        radarInfo[5] = (rand() % 3);
        radarInfo[6] = (rand() % 3);
        radarInfo[7] = (rand() % 3);
    }


    //$)Ah.>g=.h'???)i?
    esMatrixLookAt(	&matrixView,
                    radius * sinPhi * sinTheta, radius * cosPhi, radius * sinPhi * cosTheta,
                    0.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f);


    //$)Ah6???$f?
    if(angleX >= 360) angleX -= 360;
    if(angleX < 0) angleX += 360;
    if(angleY >= 360) angleY -= 360;
    if(angleY < 0) angleY += 360;
    if(angleZ >= 360) angleZ -= 360;
    if(angleZ < 0) angleZ += 360;

    //$)Ah.>g=.h'?????g=?
    glViewport(bvs2DoffsetX, bvs2DoffsetY, bvs2DWidth, bvs2DHeight);

    //$)Ah.>g=.f(!e??)i?
    esMatrixLoadIdentity(&matrixModel);
    esTranslate (&matrixModel, tx, ty, tz );


    //$)Af(!e??)i?Xh'???)i?X??=1?)i?o<???0f?;e??"g???
    esMatrixMultiply(&matrixModelView, &matrixModel, &matrixView);
    esMatrixMultiply(&matrixMVP, &matrixModelView, &matrixProjection);



#if 1
    //?$)A;i?????:e?
    glUseProgram(hProgramHandle[0]);
    /* Enable attributes for position, color and texture coordinates etc. */
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    //glUniformMatrix4fv(mvp_pos[0], 1, GL_FALSE, &matrixMVP.m[0][0]);
    setMat4(hProgramHandle[0], "mvp", matrixMVP);

    //Front
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.CamVerticesPoints[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.CamImagePoints[0]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.LumiaBalance[0]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);

    colorAdjust[0].x = colorAve[8].x - colorAve[0].x;
    colorAdjust[0].y = colorAve[8].y - colorAve[0].y;
    colorAdjust[0].z = colorAve[8].z - colorAve[0].z;

    colorAdjust[1].x = colorAve[9].x - colorAve[2].x;
    colorAdjust[1].y = colorAve[9].y - colorAve[2].y;
    colorAdjust[1].z = colorAve[9].z - colorAve[2].z;

    //glUniform3fv(iLocColor0Adjust, 1, &colorAdjust[0].x);
    //glUniform3fv(iLocColor1Adjust, 1, &colorAdjust[1].x);
    setVec3(hProgramHandle[0], "color0Adjust", colorAdjust[0]);
    setVec3(hProgramHandle[0], "color1Adjust", colorAdjust[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureRGBA[0]);
    setInt(hProgramHandle[0], "s_textureRBGA", 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, texCoords.glTexCoord_F.size());//GL_TRIANGLE_STRIP


    //back
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.CamVerticesPoints[1]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.CamImagePoints[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.LumiaBalance[1]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);

    colorAdjust[0].x = colorAve[10].x - colorAve[4].x;
    colorAdjust[0].y = colorAve[10].y - colorAve[4].y;
    colorAdjust[0].z = colorAve[10].z - colorAve[4].z;

    colorAdjust[1].x = colorAve[11].x - colorAve[6].x;
    colorAdjust[1].y = colorAve[11].y - colorAve[6].y;
    colorAdjust[1].z = colorAve[11].z - colorAve[6].z;

    //glUniform3fv(iLocColor0Adjust, 1, &colorAdjust[0].x);
    //glUniform3fv(iLocColor1Adjust, 1, &colorAdjust[1].x);

    setVec3(hProgramHandle[0], "color0Adjust", colorAdjust[0]);
    setVec3(hProgramHandle[0], "color1Adjust", colorAdjust[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureRGBA[1]);
    setInt(hProgramHandle[0], "s_textureRBGA", 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, texCoords.glTexCoord_B.size());


    //left
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.CamVerticesPoints[2]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.CamImagePoints[2]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.LumiaBalance[2]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);

    colorAdjust[0].x = colorAve[8].x - colorAve[1].x;
    colorAdjust[0].y = colorAve[8].y - colorAve[1].y;
    colorAdjust[0].z = colorAve[8].z - colorAve[1].z;

    colorAdjust[1].x = colorAve[10].x - colorAve[5].x;
    colorAdjust[1].y = colorAve[10].y - colorAve[5].y;
    colorAdjust[1].z = colorAve[10].z - colorAve[5].z;

    //glUniform3fv(iLocColor0Adjust, 1, &colorAdjust[0].x);
    //glUniform3fv(iLocColor1Adjust, 1, &colorAdjust[1].x);

    setVec3(hProgramHandle[0], "color0Adjust", colorAdjust[0]);
    setVec3(hProgramHandle[0], "color1Adjust", colorAdjust[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureRGBA[2]);
    setInt(hProgramHandle[0], "s_textureRBGA", 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, texCoords.glTexCoord_L.size());

    //right
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.CamVerticesPoints[3]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.CamImagePoints[3]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.LumiaBalance[3]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);

    colorAdjust[0].x = colorAve[9].x - colorAve[3].x;
    colorAdjust[0].y = colorAve[9].y - colorAve[3].y;
    colorAdjust[0].z = colorAve[9].z - colorAve[3].z;

    colorAdjust[1].x = colorAve[11].x - colorAve[7].x;
    colorAdjust[1].y = colorAve[11].y - colorAve[7].y;
    colorAdjust[1].z = colorAve[11].z - colorAve[7].z;

    //glUniform3fv(iLocColor0Adjust, 1, &colorAdjust[0].x);
    //glUniform3fv(iLocColor1Adjust, 1, &colorAdjust[1].x);

    setVec3(hProgramHandle[0], "color0Adjust", colorAdjust[0]);
    setVec3(hProgramHandle[0], "color1Adjust", colorAdjust[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureRGBA[3]);
    setInt(hProgramHandle[0], "s_textureRBGA", 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, texCoords.glTexCoord_R.size());
#endif

    //printf("run independ\n");
#if 1
    //?$)A;h??????
    glUseProgram(hProgramHandle[1]);

    /* Enable attributes for position, color and texture coordinates etc. */
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    //glUniformMatrix4fv(mvp_pos[1], 1, GL_FALSE, &matrixMVP.m[0][0]);
    setMat4(hProgramHandle[1], "mvp", matrixMVP);


    //front  left
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.Alpha[0]);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.MosaicCamVerticesPoints[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.MosaicFLCamImagePoints[0]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.MosaicFLCamImagePoints[1]);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    colorAdjust[0].x = colorAve[8].x - colorAve[0].x;
    colorAdjust[0].y = colorAve[8].y - colorAve[0].y;
    colorAdjust[0].z = colorAve[8].z - colorAve[0].z;

    colorAdjust[1].x = colorAve[8].x - colorAve[1].x;
    colorAdjust[1].y = colorAve[8].y - colorAve[1].y;
    colorAdjust[1].z = colorAve[8].z - colorAve[1].z;

    //glUniform3fv(iLocColor0AdjustMosaic, 1, &colorAdjust[0].x);
    //glUniform3fv(iLocColor1AdjustMosaic, 1, &colorAdjust[1].x);
    setVec3(hProgramHandle[1], "color0Adjust", colorAdjust[0]);
    setVec3(hProgramHandle[1], "color1Adjust", colorAdjust[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureRGBA[0]);
    setInt(hProgramHandle[1], "s_texture1RGBA", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureRGBA[2]);
    setInt(hProgramHandle[1], "s_texture2RGBA", 1);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCoords.glVertex_FL.size());


    //front  right
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.Alpha[1]);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.MosaicCamVerticesPoints[1]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.MosaicFRCamImagePoints[0]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.MosaicFRCamImagePoints[1]);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);


    colorAdjust[0].x = colorAve[9].x - colorAve[2].x;
    colorAdjust[0].y = colorAve[9].y - colorAve[2].y;
    colorAdjust[0].z = colorAve[9].z - colorAve[2].z;

    colorAdjust[1].x = colorAve[9].x - colorAve[3].x;
    colorAdjust[1].y = colorAve[9].y - colorAve[3].y;
    colorAdjust[1].z = colorAve[9].z - colorAve[3].z;

    //glUniform3fv(iLocColor0AdjustMosaic, 1, &colorAdjust[0].x);
    //glUniform3fv(iLocColor1AdjustMosaic, 1, &colorAdjust[1].x);
    setVec3(hProgramHandle[1], "color0Adjust", colorAdjust[0]);
    setVec3(hProgramHandle[1], "color1Adjust", colorAdjust[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureRGBA[0]);
    setInt(hProgramHandle[1], "s_texture1RGBA", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureRGBA[3]);
    setInt(hProgramHandle[1], "s_texture2RGBA", 1);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCoords.glVertex_FR.size());

    //back  left
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.Alpha[2]);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.MosaicCamVerticesPoints[2]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.MosaicBLCamImagePoints[0]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.MosaicBLCamImagePoints[1]);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);


    colorAdjust[0].x = colorAve[10].x - colorAve[4].x;
    colorAdjust[0].y = colorAve[10].y - colorAve[4].y;
    colorAdjust[0].z = colorAve[10].z - colorAve[4].z;

    colorAdjust[1].x = colorAve[10].x - colorAve[5].x;
    colorAdjust[1].y = colorAve[10].y - colorAve[5].y;
    colorAdjust[1].z = colorAve[10].z - colorAve[5].z;

    //glUniform3fv(iLocColor0AdjustMosaic, 1, &colorAdjust[0].x);
    //glUniform3fv(iLocColor1AdjustMosaic, 1, &colorAdjust[1].x);
    setVec3(hProgramHandle[1], "color0Adjust", colorAdjust[0]);
    setVec3(hProgramHandle[1], "color1Adjust", colorAdjust[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureRGBA[1]);
    setInt(hProgramHandle[1], "s_texture1RGBA", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureRGBA[2]);
    setInt(hProgramHandle[1], "s_texture2RGBA", 1);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCoords.glVertex_BL.size());


    //back  right
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.Alpha[3]);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.MosaicCamVerticesPoints[3]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.MosaicBRCamImagePoints[0]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.MosaicBRCamImagePoints[1]);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    colorAdjust[0].x = colorAve[11].x - colorAve[6].x;
    colorAdjust[0].y = colorAve[11].y - colorAve[6].y;
    colorAdjust[0].z = colorAve[11].z - colorAve[6].z;

    colorAdjust[1].x = colorAve[11].x - colorAve[7].x;
    colorAdjust[1].y = colorAve[11].y - colorAve[7].y;
    colorAdjust[1].z = colorAve[11].z - colorAve[7].z;

    //glUniform3fv(iLocColor0AdjustMosaic, 1, &colorAdjust[0].x);
    //glUniform3fv(iLocColor1AdjustMosaic, 1, &colorAdjust[1].x);
    setVec3(hProgramHandle[1], "color0Adjust", colorAdjust[0]);
    setVec3(hProgramHandle[1], "color1Adjust", colorAdjust[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureRGBA[1]);
    setInt(hProgramHandle[1], "s_texture1RGBA", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureRGBA[3]);
    setInt(hProgramHandle[1], "s_texture2RGBA", 1);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCoords.glVertex_BR.size());
#endif

#if 0
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);


    glUseProgram(hProgramHandle[3]);

    /* Enable attributes for position, color and texture coordinates etc. */
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glUniformMatrix4fv(mvp_pos[3], 1, GL_FALSE, &matrixMVP.m[0][0]);
    glUniform4fv(iLocColorBlend, 1, &zebraColor.r);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.CarVerTexCoord[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.CarVerTexCoord[2]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);//GL_TRIANGLE_STRIP
    //printf("faceCount = %d\n", radarVertex.glVertex_F.size());
#endif

#if 1
    //?$)A;f?i)?:?
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(hProgramHandle[2]);

    //glUniformMatrix4fv(mvp_pos[2], 1, GL_FALSE, &matrixMVP.m[0][0]);
    setMat4(hProgramHandle[2], "mvp", matrixMVP);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.CarVerTexCoord[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3DMosaicImageParams.CarVerTexCoord[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureRes[1]);
    setInt(hProgramHandle[1], "s_textureRGB", 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);//GL_TRIANGLE_STRIP
#endif

#if 0
    //??D$)Ah=?(!
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //???$)Af77e?f(!e?
    glEnable(GL_DEPTH_TEST);  //???$)Af71e:&f5??

    angleZ += 1.0;  //$)Ah=?=.h=??h'?:&
    //angleZ = 0.0;


    //$)Ae0?=&f(!g<)?>e?-1.0~1.0d9??
    esMatrixLoadIdentity(&matrixCarModelScale);
    esScale ( &matrixCarModelScale, carScaleX, carScaleY, carScaleZ );

    // ?$)A9f?obj??;6d8????e.?????d;6d?d8??d8???6d;6d>???%g?
    for (i = 0; i < carVertices.size(); i++)
    {
        glUseProgram(carProgramIDList[carMaterialIdx[i]]);

        //$)Ae>?????2e?d8?!6?9g4"e<??e88d:.g4"e???
        carVertexPositionIDList[carMaterialIdx[i]] = glGetAttribLocation(
                    carProgramIDList[carMaterialIdx[i]],
                    "aPos");

        // Get a handle for our buffers
        carNormalPositionIDList[carMaterialIdx[i]] = glGetAttribLocation(
                    carProgramIDList[carMaterialIdx[i]],
                    "aNormal");

        // Get a handle for our buffers
        carTexturePositionIDList[carMaterialIdx[i]] = glGetAttribLocation(
                    carProgramIDList[carMaterialIdx[i]],
                    "aTexcoord");

        // Get a handle for our buffers
        carTangentPositionIDList[carMaterialIdx[i]] = glGetAttribLocation(
                    carProgramIDList[carMaterialIdx[i]],
                    "aTangent");

        // Get a handle for our buffers
        carBitTangentPositionIDList[carMaterialIdx[i]] = glGetAttribLocation(
                    carProgramIDList[carMaterialIdx[i]],
                    "aBitTangent");

        //printf("%d %d %d %d %d\n", carVertexPositionIDList[carMaterialIdx[i]],
        //carNormalPositionIDList[carMaterialIdx[i]],
        //carTexturePositionIDList[carMaterialIdx[i]],
        //carTangentPositionIDList[carMaterialIdx[i]],
        //carBitTangentPositionIDList[carMaterialIdx[i]]);

        // Get a handle for our "MVP" uniform
        carMatrixModelIDList[carMaterialIdx[i]] = glGetUniformLocation(
                    carProgramIDList[carMaterialIdx[i]],
                    "model");

        // Get a handle for our "MVP" uniform
        carMatrixMVPIDList[carMaterialIdx[i]] = glGetUniformLocation(
                carProgramIDList[carMaterialIdx[i]],
                "mvp");

        // Get a handle for our "MVP" uniform
        carMatrixNormalIDList[carMaterialIdx[i]] = glGetUniformLocation(
                    carProgramIDList[carMaterialIdx[i]],
                    "normalMatrix");

        // Get a handle for our "MVP" uniform
        carFragLightPosIDList[carMaterialIdx[i]] = glGetUniformLocation(
                    carProgramIDList[carMaterialIdx[i]],
                    "lightPos");

        // Get a handle for our "MVP" uniform
        carFragViewPosIDList[carMaterialIdx[i]] = glGetUniformLocation(
                    carProgramIDList[carMaterialIdx[i]],
                    "viewPos");

        // Get a handle for our "MVP" uniform
        carFragKdColorIDList[carMaterialIdx[i]] = glGetUniformLocation(
                    carProgramIDList[carMaterialIdx[i]],
                    "KdColor");

        // Get a handle for our "MVP" uniform
        carFragAlphaIDList[carMaterialIdx[i]] = glGetUniformLocation(
                carProgramIDList[carMaterialIdx[i]],
                "alpha");

        carLocTextureIDList[carMaterialIdx[i]] = glGetUniformLocation(
                    carProgramIDList[carMaterialIdx[i]],
                    "s_texture");

        carLocCubeTextureIDList[carMaterialIdx[i]] = glGetUniformLocation(
                    carProgramIDList[carMaterialIdx[i]],
                    "cubeTexture");

        carLocNormalTextureIDList[carMaterialIdx[i]] = glGetUniformLocation(
                    carProgramIDList[carMaterialIdx[i]],
                    "normalTexture");


        // Get a handle for our "MVP" uniform
        carFragLightStrengthIDList[carMaterialIdx[i]] = glGetUniformLocation(
                    carProgramIDList[carMaterialIdx[i]],
                    "lightStrength");

        //printf("%d %d\n",i, carLocTextureIDList[carMaterialIdx[i]]);
        glUniform1i(carLocTextureIDList[carMaterialIdx[i]], 0);
        glUniform1i(carLocCubeTextureIDList[carMaterialIdx[i]], 1);
        glUniform1i(carLocNormalTextureIDList[carMaterialIdx[i]], 2);

#if 1
        //?$)A'e?e/9e????d;6e01h!??h=??h>>e?e<??3h=&?(e?h=????=,g-?????e.??
        esMatrixLoadIdentity(&matrixCarModel);
        esMatrixLoadIdentity(&matrixCarModelR0);
        esMatrixLoadIdentity(&matrixCarModelR1);
        esMatrixLoadIdentity(&matrixCarModelT);

        if( strcmp(carObjectList[i].name, "WHEEL_HUB_F_L") == 0 ||
                strcmp(carObjectList[i].name, "WHEEL_F_L") == 0 ||
                strcmp(carObjectList[i].name, "DISC_F_L") == 0 )
        {
            esTranslate ( &matrixCarModelT, -LFPWheel.x, -LFPWheel.y, -LFPWheel.z );
            esRotate( &matrixCarModelR0, angleZ, 0.0f, 0.0f, 1.0f );
            esRotate( &matrixCarModelR1, wheelangle, 0.0f, 1.0f, 0.0f );

            esMatrixMultiply(&matrixCarModelR, &matrixCarModelR0, &matrixCarModelR1);
            esMatrixMultiply(&matrixCarModel, &matrixCarModelT, &matrixCarModelR);

            esMatrixLoadIdentity(&matrixCarModelT);
            esTranslate ( &matrixCarModelT, LFPWheel.x, LFPWheel.y, LFPWheel.z );
            esMatrixMultiply(&matrixCarModel, &matrixCarModel, &matrixCarModelT);
        }
        else if(strcmp(carObjectList[i].name, "WHEEL_HUB_F_R") == 0 ||
                strcmp(carObjectList[i].name, "WHEEL_F_R") == 0 ||
                strcmp(carObjectList[i].name, "DISC_F_R") == 0)
        {
            esTranslate ( &matrixCarModelT, -RFPWheel.x, -RFPWheel.y, -RFPWheel.z );
            esRotate( &matrixCarModelR0, angleZ, 0.0f, 0.0f, 1.0f );
            esRotate( &matrixCarModelR1, wheelangle, 0.0f, 1.0f, 0.0f );

            esMatrixMultiply(&matrixCarModelR, &matrixCarModelR0, &matrixCarModelR1);
            esMatrixMultiply(&matrixCarModel, &matrixCarModelT, &matrixCarModelR);

            esMatrixLoadIdentity(&matrixCarModelT);
            esTranslate ( &matrixCarModelT, RFPWheel.x, RFPWheel.y, RFPWheel.z);
            esMatrixMultiply(&matrixCarModel, &matrixCarModel, &matrixCarModelT);
        }
        else if(strcmp(carObjectList[i].name, "WHEEL_HUB_B_L") == 0 ||
                strcmp(carObjectList[i].name, "WHEEL_B_L") == 0 ||
                strcmp(carObjectList[i].name, "DISC_B_L") == 0)
        {
            esTranslate ( &matrixCarModelT, -LBPWheel.x, -LBPWheel.y, -LBPWheel.z );
            esRotate( &matrixCarModelR0, angleZ, 0.0f, 0.0f, 1.0f );
            //esRotate( &matrixCarModelR0, wheelangle, 0.0f, 1.0f, 0.0f );

            esMatrixMultiply(&matrixCarModel, &matrixCarModelT, &matrixCarModelR0);

            esMatrixLoadIdentity(&matrixCarModelT);
            esTranslate ( &matrixCarModelT, LBPWheel.x, LBPWheel.y, LBPWheel.z );
            esMatrixMultiply(&matrixCarModel, &matrixCarModel, &matrixCarModelT);
        }
        else if(strcmp(carObjectList[i].name, "WHEEL_HUB_B_R") == 0 ||
                strcmp(carObjectList[i].name, "WHEEL_B_R") == 0 ||
                strcmp(carObjectList[i].name, "DISC_B_R") == 0)
        {
            esTranslate ( &matrixCarModelT, -RBPWheel.x, -RBPWheel.y, -RBPWheel.z );
            esRotate( &matrixCarModelR0, angleZ, 0.0f, 0.0f, 1.0f );
            //esRotate( &matrixCarModelR0, wheelangle, 0.0f, 1.0f, 0.0f );

            esMatrixMultiply(&matrixCarModel, &matrixCarModelT, &matrixCarModelR0);

            esMatrixLoadIdentity(&matrixCarModelT);
            esTranslate ( &matrixCarModelT, RBPWheel.x, RBPWheel.y, RBPWheel.z );
            esMatrixMultiply(&matrixCarModel, &matrixCarModel, &matrixCarModelT);
        }
        else if(strcmp(carObjectList[i].name, "DOOR_F_L") == 0 ||
                strcmp(carObjectList[i].name, "DOOR_BOX_F_L") == 0 ||
                strcmp(carObjectList[i].name, "DOOR_TRIM_F_L") == 0 ||
                strcmp(carObjectList[i].name, "DOOR_GLASS_F_L") == 0 ||
                strcmp(carObjectList[i].name, "DOOR_FRAME_OUTSIDE_F_L") == 0 ||
                strcmp(carObjectList[i].name, "MIRROR_BODY_PEDESTAL_L") == 0 ||
                strcmp(carObjectList[i].name, "MIRROR_LAMPSHADER_L") == 0 ||
                strcmp(carObjectList[i].name, "MIRROR_GLASS_L") == 0 )
        {
            esTranslate ( &matrixCarModelT, -FLDoor.x, -FLDoor.y, -FLDoor.z );
            esRotate( &matrixCarModelR0, leftDoorAngle, 0.0f, 1.0f, 0.0f );

            esMatrixMultiply(&matrixCarModel, &matrixCarModelT, &matrixCarModelR0);

            esMatrixLoadIdentity(&matrixCarModelT);
            esTranslate ( &matrixCarModelT, FLDoor.x, FLDoor.y, FLDoor.z );
            esMatrixMultiply(&matrixCarModel, &matrixCarModel, &matrixCarModelT);
        }
        else if(strcmp(carObjectList[i].name, "DOOR_F_R") == 0 ||
                strcmp(carObjectList[i].name, "DOOR_BOX_F_R") == 0 ||
                strcmp(carObjectList[i].name, "DOOR_TRIM_F_R") == 0 ||
                strcmp(carObjectList[i].name, "DOOR_GLASS_F_R") == 0 ||
                strcmp(carObjectList[i].name, "DOOR_FRAME_OUTSIDE_F_R") == 0 ||
                strcmp(carObjectList[i].name, "MIRROR_BODY_PEDESTAL_R") == 0 ||
                strcmp(carObjectList[i].name, "MIRROR_LAMPSHADER_R") == 0 ||
                strcmp(carObjectList[i].name, "MIRROR_GLASS_R") == 0 )
        {
            esTranslate ( &matrixCarModelT, -FRDoor.x, -FRDoor.y, -FRDoor.z );
            esRotate( &matrixCarModelR0, rightDoorAngle, 0.0f, 1.0f, 0.0f );

            esMatrixMultiply(&matrixCarModel, &matrixCarModelT, &matrixCarModelR0);

            esMatrixLoadIdentity(&matrixCarModelT);
            esTranslate ( &matrixCarModelT, FRDoor.x, FRDoor.y, FRDoor.z );
            esMatrixMultiply(&matrixCarModel, &matrixCarModel, &matrixCarModelT);
        }
        else if(strcmp(carObjectList[i].name, "DOOR_B_L") == 0 ||
                strcmp(carObjectList[i].name, "DOOR_BOX_B_L") == 0 ||
                strcmp(carObjectList[i].name, "DOOR_TRIM_B_L") == 0 ||
                strcmp(carObjectList[i].name, "DOOR_GLASS_B_L") == 0 ||
                strcmp(carObjectList[i].name, "DOOR_FRAME_OUTSIDE_B_L") == 0 ||
                strcmp(carObjectList[i].name, "FENDER_B_L") == 0 )
        {
            esTranslate ( &matrixCarModelT, -BLDoor.x, -BLDoor.y, -BLDoor.z );
            esRotate( &matrixCarModelR0, leftDoorAngle, 0.0f, 1.0f, 0.0f );

            esMatrixMultiply(&matrixCarModel, &matrixCarModelT, &matrixCarModelR0);

            esMatrixLoadIdentity(&matrixCarModelT);
            esTranslate ( &matrixCarModelT, BLDoor.x, BLDoor.y, BLDoor.z );
            esMatrixMultiply(&matrixCarModel, &matrixCarModel, &matrixCarModelT);
        }
        else if(strcmp(carObjectList[i].name, "DOOR_B_R") == 0 ||
                strcmp(carObjectList[i].name, "DOOR_BOX_B_R") == 0 ||
                strcmp(carObjectList[i].name, "DOOR_TRIM_B_R") == 0 ||
                strcmp(carObjectList[i].name, "DOOR_GLASS_B_R") == 0 ||
                strcmp(carObjectList[i].name, "DOOR_FRAME_OUTSIDE_B_R") == 0 ||
                strcmp(carObjectList[i].name, "FENDER_B_R") == 0 )
        {
            esTranslate ( &matrixCarModelT, -BRDoor.x, -BRDoor.y, -BRDoor.z );
            esRotate( &matrixCarModelR0, rightDoorAngle, 0.0f, 1.0f, 0.0f );

            esMatrixMultiply(&matrixCarModel, &matrixCarModelT, &matrixCarModelR0);

            esMatrixLoadIdentity(&matrixCarModelT);
            esTranslate ( &matrixCarModelT, BRDoor.x, BRDoor.y, BRDoor.z );
            esMatrixMultiply(&matrixCarModel, &matrixCarModel, &matrixCarModelT);
        }
        else if(strcmp(carObjectList[i].name, "TRUNK_LID") == 0 ||
                strcmp(carObjectList[i].name, "TRUNK_LID_H") == 0 ||
                strcmp(carObjectList[i].name, "TAIL_LOGO") == 0 ||
                strcmp(carObjectList[i].name, "TAIL_LOGO_1") == 0 ||
                strcmp(carObjectList[i].name, "TAIL_LOGO_2") == 0 ||
                strcmp(carObjectList[i].name, "TAIL_LOGO_3") == 0 ||
                strcmp(carObjectList[i].name, "TAIL_LICENSE_PLATE") == 0 ||
                strcmp(carObjectList[i].name, "MODEL") == 0 ||
                strcmp(carObjectList[i].name, "TRUNK_LID_TRIM") == 0 ||
                strcmp(carObjectList[i].name, "TAIL_PLASTIC") == 0 ||
                strcmp(carObjectList[i].name, "TAIL_GLASS") == 0 ||
                strcmp(carObjectList[i].name, "TAIL_WIPER") == 0 ||
                strcmp(carObjectList[i].name, "TAIL_PLASTIC") == 0 ||
                strcmp(carObjectList[i].name, "LAMP_TAIL") == 0 ||
                strcmp(carObjectList[i].name, "LAMPSHADER_TAIL_UP") == 0 ||
                strcmp(carObjectList[i].name, "LAMPSHADER_TAIL_DOWN") == 0 ||
                strcmp(carObjectList[i].name, "LAMPSHADER_TAIL_IN") == 0 ||
                strcmp(carObjectList[i].name, "LAMPSHADER_TAIL") == 0 ||
                strcmp(carObjectList[i].name, "LAMPSHADER_TAIL_B") == 0 ||
                strcmp(carObjectList[i].name, "TRUNK_LIGHT") == 0 ||
                strcmp(carObjectList[i].name, "WING") == 0)
        {
            esTranslate ( &matrixCarModelT, -TRUNKLID.x, -TRUNKLID.y, -TRUNKLID.z );
            esRotate( &matrixCarModelR0, 0, 0.0f, 0.0f, 1.0f );
            //esRotate( &matrixCarModelR0, wheelangle, 0.0f, 1.0f, 0.0f );

            esMatrixMultiply(&matrixCarModel, &matrixCarModelT, &matrixCarModelR0);

            esMatrixLoadIdentity(&matrixCarModelT);
            esTranslate ( &matrixCarModelT, TRUNKLID.x, TRUNKLID.y, TRUNKLID.z );
            esMatrixMultiply(&matrixCarModel, &matrixCarModel, &matrixCarModelT);
        }
        /*else if(strcmp(carObjectList[i].name, "TRUNK_LID_DOWN") == 0)
        {
        	esTranslate ( &matrixCarModelT, -TRUNK_LID_DOWN.x, -TRUNK_LID_DOWN.y, -TRUNK_LID_DOWN.z );
        	esRotate( &matrixCarModelR0, -90, 0.0f, 0.0f, 1.0f );
        	//esRotate( &matrixCarModelR0, wheelangle, 0.0f, 1.0f, 0.0f );

        	esMatrixMultiply(&matrixCarModel, &matrixCarModelT, &matrixCarModelR0);

        	esMatrixLoadIdentity(&matrixCarModelT);
        	esTranslate ( &matrixCarModelT, TRUNK_LID_DOWN.x, TRUNK_LID_DOWN.y, TRUNK_LID_DOWN.z );
        	esMatrixMultiply(&matrixCarModel, &matrixCarModel, &matrixCarModelT);
        }*/
        else
        {
            esMatrixLoadIdentity(&matrixCarModel);
        }
#endif

        //$)Ae>???;e??"g??5e?f3?:??)i?
        //esMatrixLoadIdentity(&matrixCarModel);
        esMatrixLoadIdentity(&matrixCarModelT);
        esTranslate (&matrixCarModelT, tx, ty, tz);

        esMatrixMultiply(&matrixCarModel, &matrixCarModel, &matrixCarModelScale);
        esMatrixMultiply(&matrixCarModel, &matrixCarModel, &matrixCarModelT);

        esMatrixMultiply(&matrixCarModelView, &matrixCarModel, &matrixView);
        esMatrixMultiply(&matrixCarMVP, &matrixCarModelView, &matrixProjection);

        matrixCarModelInv = esMatrixInvert(&matrixCarModel);
        esMatrixTranspose(&matrixCarModelInv);

        //esMatrixLoadIdentity(&matrixCarModel);
        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        glUniformMatrix4fv(carMatrixModelIDList[carMaterialIdx[i]],
                           1,
                           GL_FALSE,
                           &matrixCarModel.m[0][0]);

        glUniformMatrix4fv(carMatrixMVPIDList[carMaterialIdx[i]],
                           1,
                           GL_FALSE,
                           &matrixCarMVP.m[0][0]);

        glUniformMatrix4fv(carMatrixNormalIDList[carMaterialIdx[i]],
                           1,
                           GL_FALSE,
                           &matrixCarModelInv.m[0][0]);

        glEnableVertexAttribArray(carVertexPositionIDList[carMaterialIdx[i]]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO3DCarModelParams.vertices[i]);
        glVertexAttribPointer(carVertexPositionIDList[carMaterialIdx[i]],
                              3,
                              GL_FLOAT,
                              GL_FALSE,
                              0,
                              0);


        glEnableVertexAttribArray(carNormalPositionIDList[carMaterialIdx[i]]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO3DCarModelParams.normals[i]);
        glVertexAttribPointer(carNormalPositionIDList[carMaterialIdx[i]],
                              3,
                              GL_FLOAT,
                              GL_FALSE,
                              0,
                              0);

        glEnableVertexAttribArray(carTexturePositionIDList[carMaterialIdx[i]]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO3DCarModelParams.textures[i]);
        glVertexAttribPointer(carTexturePositionIDList[carMaterialIdx[i]],
                              2,
                              GL_FLOAT,
                              GL_FALSE,
                              0,
                              0);

        glEnableVertexAttribArray(carTangentPositionIDList[carMaterialIdx[i]]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO3DCarModelParams.tangents[i]);
        glVertexAttribPointer(carTangentPositionIDList[carMaterialIdx[i]],
                              3,
                              GL_FLOAT,
                              GL_FALSE,
                              0,
                              0);


        glEnableVertexAttribArray(carBitTangentPositionIDList[carMaterialIdx[i]]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO3DCarModelParams.bitTangents[i]);
        glVertexAttribPointer(carBitTangentPositionIDList[carMaterialIdx[i]],
                              3,
                              GL_FLOAT,
                              GL_FALSE,
                              0,
                              0);


        glUniform3fv(carFragKdColorIDList[carMaterialIdx[i]],
                     1,
                     (float *)&carMaterialList[carMaterialIdx[i]].ColorKd.r);

        glUniform1f(carFragAlphaIDList[carMaterialIdx[i]], carMaterialList[carMaterialIdx[i]].Alpha.r);

        glUniform3f(carFragLightPosIDList[carMaterialIdx[i]], lightRadius * lightSinPhi * lightSinTheta, lightRadius * lightCosPhi, lightRadius * lightSinPhi * lightCosTheta);

        glUniform3f(carFragLightStrengthIDList[carMaterialIdx[i]], lightStrength[0], lightStrength[1], lightStrength[2]);

        glUniform3f(carFragViewPosIDList[carMaterialIdx[i]], radius * sinPhi * sinTheta, radius * cosPhi, radius * sinPhi * cosTheta);

        if( strcmp(carMaterialList[carMaterialIdx[i]].name, "BODY") == 0) //$)Ae&????=&h:?????????9e?i"??
        {
            glUniform3f(carFragKdColorIDList[carMaterialIdx[i]], bodyColor[0] / 255.0, bodyColor[1] / 255.0, bodyColor[2] / 255.0);
        }
        else if( strcmp(carMaterialList[carMaterialIdx[i]].name, "GLASS") == 0 )
        {
            glUniform3f(carFragKdColorIDList[carMaterialIdx[i]], blackGlass.r, blackGlass.g, blackGlass.b);
        }
        else
        {
            glUniform3fv(carFragKdColorIDList[carMaterialIdx[i]],
                         1,
                         (float *)&carMaterialList[carMaterialIdx[i]].ColorKd.r);
        }


        //$)Af??f4;h=&f(!h44?>e???==??e/9e???44??
        glActiveTexture(GL_TEXTURE0);
        if(strcmp(carMaterialList[carMaterialIdx[i]].name, "LAMP_HEAD_AND_TURN") == 0)
        {
            glBindTexture(GL_TEXTURE_2D, carTextureIDList[carMaterialIdx[i] + turnTexID]); //g_textureID[0])
        }
        else if(strcmp(carMaterialList[carMaterialIdx[i]].name, "LAMP_TAIL") == 0)
        {
            glBindTexture(GL_TEXTURE_2D, carTextureIDList[carMaterialIdx[i] + lampTailStatus]); //g_textureID[0])
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, carTextureIDList[carMaterialIdx[i]]);
        }

        //$)Af??f4;g?e"??e0?44??
        glActiveTexture ( GL_TEXTURE1 );
        glBindTexture ( GL_TEXTURE_CUBE_MAP, cubeTexture );

        glActiveTexture ( GL_TEXTURE2 );
        glBindTexture ( GL_TEXTURE_2D, carNormalTextureIDList[0] );

        //if(	strcmp(carObjectList[i].name, "INTER") == 0)
        //if( strcmp(carMaterialList[carMaterialIdx[i]].name, "WHEEL") == 0 ||
        //strcmp(carMaterialList[carMaterialIdx[i]].name, "WHEEL_HUB") == 0)
        //if( strcmp(carMaterialList[carMaterialIdx[i]].name, "BODY") == 0 )
        {
            glDrawArrays(GL_TRIANGLES, 0, carVertices[i].size());

            //faceCount += carVertices[i].size() / 3;
        }
    }

    //printf("faceCount = %d\n",faceCount);

    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif

#if 0
    //?$)A;e?h=(h?9
    esMatrixLoadIdentity(&matrixModel);
    esTranslate (&matrixModel, 0.0, 0.0, 0.0 );

    esMatrixMultiply(&matrixModelView, &matrixModel, &matrixView);
    esMatrixMultiply(&matrixMVP, &matrixModelView, &matrixProjection);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    glUseProgram(hProgramHandle[3]);

    glLineWidth(4);

    /* Enable attributes for position, color and texture coordinates etc. */
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, scaleLineVertex.vboVerticesFPoints[0]);
    glBufferData(GL_ARRAY_BUFFER, scaleLineVertex.glVertex_F[0].size() * sizeof(CvPoint3D32f), &scaleLineVertex.glVertex_F[0][0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glUniformMatrix4fv(mvp_pos[3], 1, GL_FALSE, &matrixMVP.m[0][0]);
    glUniform4fv(iLocColorBlend, 1, &scaleLineColor.r);

    glDrawArrays(GL_LINE_STRIP, 0, scaleLineVertex.glVertex_F[0].size());


    glBindBuffer(GL_ARRAY_BUFFER, scaleLineVertex.vboVerticesFPoints[1]);
    glBufferData(GL_ARRAY_BUFFER, scaleLineVertex.glVertex_F[1].size() * sizeof(CvPoint3D32f), &scaleLineVertex.glVertex_F[1][0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_LINE_STRIP, 0, scaleLineVertex.glVertex_F[1].size());


    glBindBuffer(GL_ARRAY_BUFFER, scaleLineVertex.vboVerticesFPoints[2]);
    glBufferData(GL_ARRAY_BUFFER, scaleLineVertex.glVertex_F[2].size() * sizeof(CvPoint3D32f), &scaleLineVertex.glVertex_F[2][0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_LINE_STRIP, 0, scaleLineVertex.glVertex_F[2].size());

    glUniform4fv(iLocColorBlend, 1, &radarColor[1].r);

    glBindBuffer(GL_ARRAY_BUFFER, scaleLineVertex.vboVerticescombFPoints[0]);
    glBufferData(GL_ARRAY_BUFFER, scaleLineVertex.glVertex_combF[0].size() * sizeof(CvPoint3D32f), &scaleLineVertex.glVertex_combF[0][0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    for(i = 0; i < scaleLineVertex.glVertex_combF[0].size(); i += 2)
        glDrawArrays(GL_LINES, i, 2);

    glBindBuffer(GL_ARRAY_BUFFER, scaleLineVertex.vboVerticescombFPoints[1]);
    glBufferData(GL_ARRAY_BUFFER, scaleLineVertex.glVertex_combF[1].size() * sizeof(CvPoint3D32f), &scaleLineVertex.glVertex_combF[1][0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    for(i = 0; i < scaleLineVertex.glVertex_combF[1].size(); i += 2)
        glDrawArrays(GL_LINES, i, 2);

    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
#endif

#if 0
    //?$)A;e?h=(h?9
    esMatrixLoadIdentity(&matrixModel);
    esTranslate (&matrixModel, 0.0, 0.0, 0.0 );

    esMatrixMultiply(&matrixModelView, &matrixModel, &matrixView);
    esMatrixMultiply(&matrixMVP, &matrixModelView, &matrixProjection);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    glUseProgram(hProgramHandle[3]);

    glLineWidth(4);

    /* Enable attributes for position, color and texture coordinates etc. */
    glEnableVertexAttribArray(0);

    glUniformMatrix4fv(mvp_pos[3], 1, GL_FALSE, &matrixMVP.m[0][0]);
    glUniform4fv(iLocColorBlend, 1, &scaleLineColor.r);

    glBindBuffer(GL_ARRAY_BUFFER, scaleLineVertex.vboVerticesBPoints[0]);
    glBufferData(GL_ARRAY_BUFFER, scaleLineVertex.glVertex_B[0].size() * sizeof(CvPoint3D32f), &scaleLineVertex.glVertex_B[0][0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_LINE_STRIP, 0, scaleLineVertex.glVertex_B[0].size());


    glBindBuffer(GL_ARRAY_BUFFER, scaleLineVertex.vboVerticesBPoints[1]);
    glBufferData(GL_ARRAY_BUFFER, scaleLineVertex.glVertex_B[1].size() * sizeof(CvPoint3D32f), &scaleLineVertex.glVertex_B[1][0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_LINE_STRIP, 0, scaleLineVertex.glVertex_B[1].size());

    glBindBuffer(GL_ARRAY_BUFFER, scaleLineVertex.vboVerticesBPoints[2]);
    glBufferData(GL_ARRAY_BUFFER, scaleLineVertex.glVertex_B[2].size() * sizeof(CvPoint3D32f), &scaleLineVertex.glVertex_B[2][0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_LINE_STRIP, 0, scaleLineVertex.glVertex_B[2].size());

    glUniform4fv(iLocColorBlend, 1, &radarColor[1].r);

    glBindBuffer(GL_ARRAY_BUFFER, scaleLineVertex.vboVerticescombBPoints[0]);
    glBufferData(GL_ARRAY_BUFFER, scaleLineVertex.glVertex_combB[0].size() * sizeof(CvPoint3D32f), &scaleLineVertex.glVertex_combB[0][0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    for(i = 0; i < scaleLineVertex.glVertex_combB[0].size(); i += 2)
        glDrawArrays(GL_LINES, i, 2);

    glBindBuffer(GL_ARRAY_BUFFER, scaleLineVertex.vboVerticescombBPoints[1]);
    glBufferData(GL_ARRAY_BUFFER, scaleLineVertex.glVertex_combB[1].size() * sizeof(CvPoint3D32f), &scaleLineVertex.glVertex_combB[1][0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    for(i = 0; i < scaleLineVertex.glVertex_combB[1].size(); i += 2)
        glDrawArrays(GL_LINES, i, 2);

    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

#endif

#if 0
    //?$)A;e??7h>>
    float scaleY = 0.03;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    vertexBlock = radarVertex.glVertex_F.size() / 4;

    glUseProgram(hProgramHandle[3]);

    /* Enable attributes for position, color and texture coordinates etc. */
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, radarVertex.vboVerticesPoints[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    esMatrixLoadIdentity(&matrixModelS);
    esScale (&matrixModelS, 1.0, scaleY, 1.0 );
    esMatrixLoadIdentity(&matrixModelT);
    esTranslate (&matrixModelT, radarFrontTx[radarInfo[0]], ty, tz );

    esMatrixMultiply(&matrixModel, &matrixModelS, &matrixModelT);

    esMatrixMultiply(&matrixModelView, &matrixModel, &matrixView);
    esMatrixMultiply(&matrixMVP, &matrixModelView, &matrixProjection);

    glUniformMatrix4fv(mvp_pos[3], 1, GL_FALSE, &matrixMVP.m[0][0]);

    glUniform4fv(iLocColorBlend, 1, &radarColor[radarInfo[0]].r);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexBlock);



    esMatrixLoadIdentity(&matrixModelS);
    esScale (&matrixModelS, 1.0, scaleY, 1.0 );
    esMatrixLoadIdentity(&matrixModelT);
    esTranslate (&matrixModelT, radarFrontTx[radarInfo[1]], ty, tz );

    esMatrixMultiply(&matrixModel, &matrixModelS, &matrixModelT);

    esMatrixMultiply(&matrixModelView, &matrixModel, &matrixView);
    esMatrixMultiply(&matrixMVP, &matrixModelView, &matrixProjection);

    glUniformMatrix4fv(mvp_pos[3], 1, GL_FALSE, &matrixMVP.m[0][0]);

    glUniform4fv(iLocColorBlend, 1, &radarColor[radarInfo[1]].r);
    glDrawArrays(GL_TRIANGLE_STRIP, vertexBlock, vertexBlock);


    esMatrixLoadIdentity(&matrixModelS);
    esScale (&matrixModelS, 1.0, scaleY, 1.0 );
    esMatrixLoadIdentity(&matrixModelT);
    esTranslate (&matrixModelT, radarFrontTx[radarInfo[2]], ty, tz );

    esMatrixMultiply(&matrixModel, &matrixModelS, &matrixModelT);

    esMatrixMultiply(&matrixModelView, &matrixModel, &matrixView);
    esMatrixMultiply(&matrixMVP, &matrixModelView, &matrixProjection);

    glUniformMatrix4fv(mvp_pos[3], 1, GL_FALSE, &matrixMVP.m[0][0]);

    glUniform4fv(iLocColorBlend, 1, &radarColor[radarInfo[2]].r);
    glDrawArrays(GL_TRIANGLE_STRIP, vertexBlock * 2, vertexBlock);



    esMatrixLoadIdentity(&matrixModelS);
    esScale (&matrixModelS, 1.0, scaleY, 1.0 );
    esMatrixLoadIdentity(&matrixModelT);
    esTranslate (&matrixModelT, radarFrontTx[radarInfo[0]], ty, tz );

    esMatrixMultiply(&matrixModel, &matrixModelS, &matrixModelT);

    esMatrixMultiply(&matrixModelView, &matrixModel, &matrixView);
    esMatrixMultiply(&matrixMVP, &matrixModelView, &matrixProjection);

    glUniformMatrix4fv(mvp_pos[3], 1, GL_FALSE, &matrixMVP.m[0][0]);


    glUniform4fv(iLocColorBlend, 1, &radarColor[radarInfo[3]].r);
    glDrawArrays(GL_TRIANGLE_STRIP, vertexBlock * 3, vertexBlock);

    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    //printf("faceCount = %d\n", radarVertex.glVertex_F.size());
#endif


#if 0
    //?$)A;e??7h>>
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    vertexBlock = radarVertex.glVertex_B.size() / 4;

    glUseProgram(hProgramHandle[3]);

    /* Enable attributes for position, color and texture coordinates etc. */
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, radarVertex.vboVerticesPoints[1]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    esMatrixLoadIdentity(&matrixModelS);
    esScale (&matrixModelS, 1.0, scaleY, 1.0 );
    esMatrixLoadIdentity(&matrixModelT);
    esTranslate (&matrixModelT, radarBackTx[radarInfo[4]], ty, tz );

    esMatrixMultiply(&matrixModel, &matrixModelS, &matrixModelT);

    esMatrixMultiply(&matrixModelView, &matrixModel, &matrixView);
    esMatrixMultiply(&matrixMVP, &matrixModelView, &matrixProjection);

    glUniformMatrix4fv(mvp_pos[3], 1, GL_FALSE, &matrixMVP.m[0][0]);

    glUniform4fv(iLocColorBlend, 1, &radarColor[radarInfo[4]].r);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexBlock);



    esMatrixLoadIdentity(&matrixModelS);
    esScale (&matrixModelS, 1.0, scaleY, 1.0 );
    esMatrixLoadIdentity(&matrixModelT);
    esTranslate (&matrixModelT, radarBackTx[radarInfo[5]], ty, tz );

    esMatrixMultiply(&matrixModel, &matrixModelS, &matrixModelT);


    esMatrixMultiply(&matrixModelView, &matrixModel, &matrixView);
    esMatrixMultiply(&matrixMVP, &matrixModelView, &matrixProjection);

    glUniformMatrix4fv(mvp_pos[3], 1, GL_FALSE, &matrixMVP.m[0][0]);

    glUniform4fv(iLocColorBlend, 1, &radarColor[radarInfo[5]].r);
    glDrawArrays(GL_TRIANGLE_STRIP, vertexBlock, vertexBlock);



    esMatrixLoadIdentity(&matrixModelS);
    esScale (&matrixModelS, 1.0, scaleY, 1.0 );
    esMatrixLoadIdentity(&matrixModelT);
    esTranslate (&matrixModelT, radarBackTx[radarInfo[6]], ty, tz );

    esMatrixMultiply(&matrixModel, &matrixModelS, &matrixModelT);


    esMatrixMultiply(&matrixModelView, &matrixModel, &matrixView);
    esMatrixMultiply(&matrixMVP, &matrixModelView, &matrixProjection);

    glUniformMatrix4fv(mvp_pos[3], 1, GL_FALSE, &matrixMVP.m[0][0]);

    glUniform4fv(iLocColorBlend, 1, &radarColor[radarInfo[6]].r);
    glDrawArrays(GL_TRIANGLE_STRIP, vertexBlock * 2, vertexBlock);



    esMatrixLoadIdentity(&matrixModelS);
    esScale (&matrixModelS, 1.0, scaleY, 1.0 );
    esMatrixLoadIdentity(&matrixModelT);
    esTranslate (&matrixModelT, radarBackTx[radarInfo[7]], ty, tz );

    esMatrixMultiply(&matrixModel, &matrixModelS, &matrixModelT);


    esMatrixMultiply(&matrixModelView, &matrixModel, &matrixView);
    esMatrixMultiply(&matrixMVP, &matrixModelView, &matrixProjection);

    glUniformMatrix4fv(mvp_pos[3], 1, GL_FALSE, &matrixMVP.m[0][0]);


    glUniform4fv(iLocColorBlend, 1, &radarColor[radarInfo[7]].r);
    glDrawArrays(GL_TRIANGLE_STRIP, vertexBlock * 3, vertexBlock);

    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    //printf("faceCount = %d\n", radarVertex.glVertex_F.size());
#endif



#if 0
    //??????
    clipX = 0;//+246;
    clipY = 0;//+47;
    clipWidth = 1280;
    clipHeight = 720;

    //fp = fopen("D:/test/writeFlag.txt","r");
    //fscanf(fp, "%d\n", &writeFlag);
    //fclose(fp);

    //if(writeFlag == 1)
    if(Count < 12)
    {
        //printf("%d-%02d-%02d %02d:%02d:%02d",t.tm_year + 1900, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);

        memset(filePath, 0x0, sizeof(filePath));
        //sprintf(filePath, "D:/test/%d-%02d-%02d-%02d-%02d-%02d.png",t.tm_year + 1900, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
        sprintf(filePath, "D:/test/%d.png", Count + 12);

        imageBuffer = (unsigned char *)malloc(clipWidth * clipHeight * 4);

        glReadPixels(clipX, clipY, clipWidth, clipHeight, GL_RGBA, GL_UNSIGNED_BYTE, imageBuffer);

        stbi_flip_vertically_on_write(1);

        //stbi_write_png("D:/test/write3.png", bvs3DWidth-200, bvs3DHeight-100, 4, imageBuffer, (bvs3DWidth-200) * 4);
        stbi_write_png(filePath, clipWidth, clipHeight, 4, imageBuffer, clipWidth * 4);

        stbi_image_free(imageBuffer);

        //fp = fopen("D:/test/writeFlag.txt","w");
        //fprintf(fp, "%d\n", writeFlag);
        //fclose(fp);

        printf("new write finish\n");
    }
#endif


    Count++;

}
#endif


/*====================================================================
?$)A=f???'0:   show2DCar
?$)A=f????:   3Df82f??;i??>g$:
$)Ag.??e.??:
?$)A(e????:
$)Ah>?????:   DisplayChannelID  ?>g$:f(!e?
$)Ah???g;??:   ??
$)Ad???h.0e?o<?
====================================================================*/
void show2DCar(int displayChannelID)
{
    vec3 colorAdjust[2];
    int i, j;
    ESMatrix matrixMVP;

    esMatrixLoadIdentity ( &matrixMVP );


    glViewport(bvs2DoffsetX, bvs2DoffsetY, bvs2DWidth, bvs2DHeight);
#if 1
    //?$)A;i?????:e?
    glUseProgram(hProgramHandle[0]);
    /* Enable attributes for position, color and texture coordinates etc. */
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    //glUniformMatrix4fv(mvp_pos[0], 1, GL_FALSE, &matrixMVP.m[0][0]);
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

    glDrawArrays(GL_TRIANGLE_STRIP, 0, texCoords2D.glTexCoord_F.size());//GL_TRIANGLE_STRIP


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

    glDrawArrays(GL_TRIANGLE_STRIP, 0, texCoords2D.glTexCoord_B.size());

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
    setInt(hProgramHandle[4], "textureY", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureU[2]);
    setInt(hProgramHandle[4], "textureU", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureV[2]);
    setInt(hProgramHandle[4], "textureV", 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, texCoords2D.glTexCoord_L.size());

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

    glDrawArrays(GL_TRIANGLE_STRIP, 0, texCoords2D.glTexCoord_R.size());
#endif

    //printf("run independ\n");
#if 1
    //?$)A;h??????
    glUseProgram(hProgramHandle[1]);

    /* Enable attributes for position, color and texture coordinates etc. */
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    //glUniformMatrix4fv(mvp_pos[1], 1, GL_FALSE, &matrixMVP.m[0][0]);
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
    //printf("run depend\n");


#if 1
    //$)Ah44h=&f(?
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
    glBindTexture(GL_TEXTURE_2D, textureRes[1]);
    setInt(hProgramHandle[1], "s_textureRGB", 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#endif

}

void drawUndistortBackCurve(ESMatrix orthoMatrix)
{
	int i;
	vec4 color[4] = {{1.0, 0.0, 0.0, 0.35}, {1.0, 1.0, 0.0, 0.35}, {0.0, 1.0, 0.0, 0.35}, {0.0, 0.0, 0.0, 0.35}};

	float verticesView[] =
		{
			-0.5f, -0.5f, 0.0f,  // left-buttom
			0.5f, -0.5f, 0.0f,	// right- buttom
			-0.5f,	0.5f, 0.0f,   // right-top
			0.5f,  0.5f, 0.0f,	 // left-top
		};

	glUseProgram(hProgramHandle[3]);

	setMat4(hProgramHandle[3], "mvp", orthoMatrix); 

	//?$)A3i?f71e:&f5?????e<?i"??f77e?
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

	glEnableVertexAttribArray(0);

	
#if 1
	for (i = 0; i < 6; i++)
	{
		setVec4(hProgramHandle[3], "outColor", color[i / 2]); 

        glBindBuffer(GL_ARRAY_BUFFER, curveVerticesPoints[0]);
		glBufferData(GL_ARRAY_BUFFER, LENGTH * 2 * sizeof(CvPoint3D32f), verticesRearTrajLinePoint[i], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, LENGTH * 2);
	}

    glBindBuffer(GL_ARRAY_BUFFER, curveVerticesPoints[0]);
	glBufferData(GL_ARRAY_BUFFER, LENGTH * 2 * sizeof(CvPoint3D32f), verticesRearTrajLinePoint[6], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	for (i = 0; i < 3; i++)
	{
		setVec4(hProgramHandle[3], "outColor", color[i]); 
		glDrawArrays(GL_TRIANGLE_STRIP, i * 4, 4);
	}
#endif

	setVec4(hProgramHandle[3], "outColor", color[0]); 
	
    glBindBuffer(GL_ARRAY_BUFFER, curveVerticesPoints[0]);
	glBufferData(GL_ARRAY_BUFFER, LENGTH * 2 * sizeof(CvPoint3D32f), verticesRearTrajLinePoint[7], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//glDrawArrays(GL_TRIANGLE_STRIP, 0, LENGTH * 2);

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

    float alpha, beta, gamma;
    double R[9], invR[9], outR[9];
    undistortParams resizer;
	float rVec[3];
	CvMat rVEC, rMAT;
	float camera[4], distortTable[4], rMat[9], tVec[3];

    /*whichCamera = viewMode;

    if(whichCamera < ORIRIN_VIEW_FRONT_STATE || whichCamera > ORIRIN_VIEW_RIGHT_STATE)
    {
    	whichCamera = ORIRIN_VIEW_FRONT_STATE;
    }*/

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

    ESMatrix orthoMatrix;
    vec4 scaleLineColor = {1.0, 165.0 / 255.0, 0.0, 0.5}; //$)Ah=(h?9i"??
    vec4 radarColor[3] = {{1.0, 0.0, 0.0, 0.5}, {1.0, 1.0, 0.0, 0.5}, {0.0, 1.0, 0.0, 0.5}};  //?$)A7h>>i"??
    esMatrixLoadIdentity ( &orthoMatrix );

    glViewport(bvs3DoffsetX, bvs3DoffsetY, bvs3DWidth, bvs3DHeight);

    glUseProgram(hProgramHandle[4]);

    /* Enable attributes for position, color and texture coordinates etc. */
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    setMat4(hProgramHandle[4], "mvp", orthoMatrix);

    //Front
    glBindBuffer(GL_ARRAY_BUFFER, cameraVerTexCoord[0]);
    glBufferData(GL_ARRAY_BUFFER, cnt * sizeof(CvPoint3D32f), verAddr, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, cameraVerTexCoord[1]);
    glBufferData(GL_ARRAY_BUFFER, cnt * sizeof(CvPoint2D32f), texAddr, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);


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
	#if 0
    	camera[0] = (float)SimplifyrearCamParams.mimdInt[0] / SCALE2;
		camera[1] = (float)SimplifyrearCamParams.mimdInt[1] / SCALE2;
		camera[2] = (float)SimplifyrearCamParams.mimdInt[2] / SCALE2;
		camera[3] = (float)SimplifyrearCamParams.mimdInt[3] / SCALE2;
	
		distortTable[0] = (float)SimplifyrearCamParams.mimdInt[4] / SCALE1;
		distortTable[1] = (float)SimplifyrearCamParams.mimdInt[5] / SCALE1;
		distortTable[2] = (float)SimplifyrearCamParams.mimdInt[6] / SCALE1;
		distortTable[3] = (float)SimplifyrearCamParams.mimdInt[7] / SCALE1;
	
		rVec[0] = (float)SimplifyrearCamParams.mrInt[0] / SCALE3;
		rVec[1] = (float)SimplifyrearCamParams.mrInt[1] / SCALE3;
		rVec[2] = (float)SimplifyrearCamParams.mrInt[2] / SCALE3;

		tVec[0] = (float)SimplifyrearCamParams.mtInt[0] / SCALE2;
		tVec[1] = (float)SimplifyrearCamParams.mtInt[1] / SCALE2;
		tVec[2] = (float)SimplifyrearCamParams.mtInt[2] / SCALE2;
	
		rVEC = cvMat(1, 3, CV_32F, rVec);
		rMAT = cvMat(3, 3, CV_32F, rMat);
	
		cvRodrigues2( &rVEC, &rMAT, NULL );

		rotationMatrixToEulerAngle(rMat, &alpha, &beta, &gamma);

		alpha = 0 * RADIAN;

		R[0] = cos(beta) * cos(gamma);
		R[1] = cos(beta) * sin(gamma);
		R[2] = -sin(beta);
		R[3] = sin(alpha) * sin(beta) * cos(gamma) - cos(alpha) * sin(gamma);
		R[4]= sin(alpha) * sin(beta) * sin(gamma) + cos(alpha) * cos(gamma);
		R[5] = sin(alpha) * cos(beta);
		R[6]= cos(alpha) * sin(beta) * cos(gamma) + sin(alpha) * sin(gamma);
		R[7] = cos(alpha) * sin(beta) * sin(gamma) - sin(alpha) * cos(gamma);
		R[8] = cos(alpha) * cos(beta);

		getInvertMatrix(R, invR);
		
		resizer.x = 100;
		resizer.y = 220;
		resizer.xZoom = 1.0 * 1080 / bvs3DWidth;
		resizer.yZoom = 1.0 * 620 / bvs3DHeight;

		findRearCurve(wheelAngle, resizer, camera, distortTable, rMat, tVec, invR, bvs3DWidth, bvs3DHeight);
#endif
		drawUndistortBackCurve(orthoMatrix);
	}
}

void showFullScreeen()
{
    float verticesView[] =
    {
        -1.0f, -1.0f, 0.0f,  // left-buttom
        1.0f, -1.0f, 0.0f,	// right- buttom
        -1.0f,	1.0f, 0.0f,   // right-top
        1.0f,  1.0f, 0.0f,	 // left-top
    };

    const int cnt = 4;

    float texCoordView[] =
    {
        0.0f,  1.0f,  // left-top
        1.0f,  1.0f,  // right-top
        0.0f,  0.0f,  // left-buttom
        1.0f,  0.0f,  // right- buttom
    };


    ESMatrix orthoMatrix;
    esMatrixLoadIdentity ( &orthoMatrix );

    glViewport(0, 0, 1280, 720);

    glUseProgram(hProgramHandle[4]);

    /* Enable attributes for position, color and texture coordinates etc. */
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    setMat4(hProgramHandle[4], "mvp", orthoMatrix);

    //Front
    glBindBuffer(GL_ARRAY_BUFFER, cameraVerTexCoord[0]);
    glBufferData(GL_ARRAY_BUFFER, cnt * sizeof(CvPoint3D32f), verticesView, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, cameraVerTexCoord[1]);
    glBufferData(GL_ARRAY_BUFFER, cnt * sizeof(CvPoint2D32f), texCoordView, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureY[0]);
    setInt(hProgramHandle[4], "textureY", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureU[0]);
    setInt(hProgramHandle[4], "textureU", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureV[0]);
    setInt(hProgramHandle[4], "textureV", 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, cnt);
}

void showFourView()
{
    vec3 colorAdjust[2] = {{0, 0, 0}, {0, 0, 0}};

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
    \

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
    float lumia[2] = {0.0, 0.0};
    esMatrixLoadIdentity ( &orthoMatrix );

    glViewport(bvs3DoffsetX, bvs3DoffsetY, bvs3DWidth, bvs3DHeight);

    glUseProgram(hProgramHandle[4]);

    /* Enable attributes for position, color and texture coordinates etc. */
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    //glUniformMatrix4fv(mvp_pos[0], 1, GL_FALSE, &orthoMatrix.m[0][0]);
    setMat4(hProgramHandle[4], "mvp", orthoMatrix);

    //Front
    glBindBuffer(GL_ARRAY_BUFFER, cameraVerTexCoord[0]);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(CvPoint3D32f), verticesViewFront, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, cameraVerTexCoord[1]);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(CvPoint2D32f), texCoordView, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, textureRGBA[0]);
    //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

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
    glBindBuffer(GL_ARRAY_BUFFER, cameraVerTexCoord[0]);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(CvPoint3D32f), verticesViewBack, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, cameraVerTexCoord[1]);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(CvPoint2D32f), texCoordView, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, textureRGBA[1]);
    //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

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
    glBindBuffer(GL_ARRAY_BUFFER, cameraVerTexCoord[0]);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(CvPoint3D32f), verticesViewLeft, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, cameraVerTexCoord[1]);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(CvPoint2D32f), texCoordView, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, textureRGBA[2]);
    //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

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
    glBindBuffer(GL_ARRAY_BUFFER, cameraVerTexCoord[0]);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(CvPoint3D32f), verticesViewRight, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, cameraVerTexCoord[1]);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(CvPoint2D32f), texCoordView, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, textureRGBA[3]);

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



int generateTriangle(CvPoint2D32f *triagnle, int width, int height)
{
    float p = 64.0;
    float i, j, fp;
    unsigned char directionFlag;
    CvPoint2D32f point;
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

                //triagnle.push_back(point);
                triagnle[count++] = point;

                point.x = j * width;
                point.y = (i + fp) * height;

                //triagnle.push_back(point);
                triagnle[count++] = point;

                //count += 2;
            }
            //objPoints2D.glObjPoints_F.pop_back();
            //vertexCoords2D.glVertex_F.pop_back();
            break;
        case 1:
            for( j = 1; j >= 0; j -= fp )
            {
                point.x = j * width;
                point.y = i * height;

                //triagnle.push_back(point);
                triagnle[count++] = point;

                point.x = j * width;
                point.y = (i + fp) * height;

                //triagnle.push_back(point);
                triagnle[count++] = point;

                //count += 2;
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

    //printf("count = %d\n",count);

    return count;
}


CvPoint2D32f undistort(float i, float j, const float *intrinsic_matrix, const float *dist_coeffs, undistortParams params, double *invR)
{
    float u, v;
    double x, y, x2, y2, r2, r, r4, r6, theta, theta2, theta4, theta6, theta8, theta_d, xd, yd, kr, _2xy;
    float xc, yc, zc, xr, yr;
    CvPoint2D32f imgPoints;
    float p3D[3], oldP3D[3];
    float z;

    float fx = intrinsic_matrix[0] * 2;
    float fy = intrinsic_matrix[1] * 2;
    float cx = intrinsic_matrix[2] * 2;
    float cy = intrinsic_matrix[3] * 2;

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
#if 0
        y = y / z;
        x = x / z;

        y2 = y * y;
        x2 = x * x;
        r2 = x2 + y2;
        r4 = r2 * r2;
        r6 = r4 * r2;

        //xd = x * (1 + k1 * r2 + k2 * r4 + k3 * r6);// + 2 * p1 * x * y + p2 * (r2 + 2 * x2);
        //yd = y * (1 + k1 * r2 + k2 * r4 + k3 * r6);// + 2 * p2 * x * y + p1 * (r2 + 2 * y2);

        //x = x + 2 * p1 * x * y + p2 * (r2 + 2 * x2);
        //y = y + 2 * p2 * x * y + p1 * (r2 + 2 * y2);
#if 0
        xd = x * (1 + k1 * r2 + k2 * r4) + 2 * p1 * x * y + p2 * (r2 + 2 * x2);
        yd = y * (1 + k1 * r2 + k2 * r4) + 2 * p2 * x * y + p1 * (r2 + 2 * y2);

        /*$)Ae>????????*/
        u  = fx * xd + cx;
        v  = fy * yd + cy;
#endif

        _2xy = 2 * x * y;

        kr = (1 + ((k3 * r2 + k2) * r2 + k1) * r2);
        u = fx * (x * kr + p1 * _2xy + p2 * (r2 + 2 * x2)) + cx;
        v = fy * (y * kr + p1 * (r2 + 2 * y2) + p2 * _2xy) + cy;
#endif

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
        else if (u > IMGWIDTH)
        {
            u = IMGWIDTH;//IMGWIDTH - 2;
        }

        if (v < 0)
        {
            v = 0;//2;
        }
        else if (v > IMGHEIGHT)
        {
            v = IMGHEIGHT;//IMGHEIGHT - 2;
        }
    }
    else
    {
        u = 0;
        v = 0;
    }

    imgPoints.x = u / IMGWIDTH;
    imgPoints.y = v / IMGHEIGHT;

    return imgPoints;
}

void initUndistort(int width, int height)
{
    float alpha, beta, gamma;
    double R[9], invR[9], outR[9];
    undistortParams resizer;
    float rVec[3];
    CvMat rVEC, rMAT;
    static float camera[4], distortTable[4], rMat[9], tVec[3];
    int k, p;
    p = 64;

#if 1
    camera[0] = (float)SimplifyfrontCamParams.mimdInt[0] / SCALE2;
    camera[1] = (float)SimplifyfrontCamParams.mimdInt[1] / SCALE2;
    camera[2] = (float)SimplifyfrontCamParams.mimdInt[2] / SCALE2;
    camera[3] = (float)SimplifyfrontCamParams.mimdInt[3] / SCALE2;

    distortTable[0] = (float)SimplifyfrontCamParams.mimdInt[4] / SCALE1;
    distortTable[1] = (float)SimplifyfrontCamParams.mimdInt[5] / SCALE1;
    distortTable[2] = (float)SimplifyfrontCamParams.mimdInt[6] / SCALE1;
    distortTable[3] = (float)SimplifyfrontCamParams.mimdInt[7] / SCALE1;

    rVec[0] = (float)SimplifyfrontCamParams.mrInt[0] / SCALE3;
    rVec[1] = (float)SimplifyfrontCamParams.mrInt[1] / SCALE3;
    rVec[2] = (float)SimplifyfrontCamParams.mrInt[2] / SCALE3;

    tVec[0] = (float)SimplifyfrontCamParams.mtInt[0] / SCALE2;
    tVec[1] = (float)SimplifyfrontCamParams.mtInt[1] / SCALE2;
    tVec[2] = (float)SimplifyfrontCamParams.mtInt[2] / SCALE2;

    rVEC = cvMat(1, 3, CV_32F, rVec);
    rMAT = cvMat(3, 3, CV_32F, rMat);

    cvRodrigues2( &rVEC, &rMAT, NULL );

    resizer.x = 100;
    resizer.y = 50;
    resizer.xZoom = 1.0 * 1080 / width;
    resizer.yZoom = 1.0 * 620 / height;

    imgCoordPointFront = (CvPoint2D32f *)malloc(sizeof(CvPoint2D32f) * p * (p + 1) * 2);

    verCountFront = generateTriangle(imgCoordPointFront, width, height);

    texCoordPointFront = (CvPoint2D32f *)malloc(sizeof(CvPoint2D32f) * verCountFront);
    verCoordPointFront = (CvPoint3D32f *)malloc(sizeof(CvPoint3D32f) * verCountFront);

    rotationMatrixToEulerAngle(rMat, &alpha, &beta, &gamma);
    //printf("%f %f %f\n",alpha, beta, gamma);

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

    getInvertMatrix(R, invR);

    for (k = 0; k < verCountFront; k++)
    {
        verCoordPointFront[k].x = -(1.0 - 2 * imgCoordPointFront[k].x / width);
        verCoordPointFront[k].y = (1.0 - 2 * imgCoordPointFront[k].y / height);
        verCoordPointFront[k].z = 0;

        texCoordPointFront[k] = undistort(imgCoordPointFront[k].y, imgCoordPointFront[k].x,
                                          camera, distortTable, resizer, invR);
    }
#endif

#if 1
    camera[0] = (float)SimplifyrearCamParams.mimdInt[0] / SCALE2;
    camera[1] = (float)SimplifyrearCamParams.mimdInt[1] / SCALE2;
    camera[2] = (float)SimplifyrearCamParams.mimdInt[2] / SCALE2;
    camera[3] = (float)SimplifyrearCamParams.mimdInt[3] / SCALE2;

    distortTable[0] = (float)SimplifyrearCamParams.mimdInt[4] / SCALE1;
    distortTable[1] = (float)SimplifyrearCamParams.mimdInt[5] / SCALE1;
    distortTable[2] = (float)SimplifyrearCamParams.mimdInt[6] / SCALE1;
    distortTable[3] = (float)SimplifyrearCamParams.mimdInt[7] / SCALE1;

    rVec[0] = (float)SimplifyrearCamParams.mrInt[0] / SCALE3;
    rVec[1] = (float)SimplifyrearCamParams.mrInt[1] / SCALE3;
    rVec[2] = (float)SimplifyrearCamParams.mrInt[2] / SCALE3;

    tVec[0] = (float)SimplifyrearCamParams.mtInt[0] / SCALE2;
    tVec[1] = (float)SimplifyrearCamParams.mtInt[1] / SCALE2;
    tVec[2] = (float)SimplifyrearCamParams.mtInt[2] / SCALE2;

    rVEC = cvMat(1, 3, CV_32F, rVec);
    rMAT = cvMat(3, 3, CV_32F, rMat);

    cvRodrigues2( &rVEC, &rMAT, NULL );

    resizer.x = 100;
    resizer.y = 220;
    resizer.xZoom = 1.0 * 1080 / width;
    resizer.yZoom = 1.0 * 620 / height;
#endif


    imgCoordPointBack = (CvPoint2D32f *)malloc(sizeof(CvPoint2D32f) * p * (p + 1) * 2);

    verCountBack = generateTriangle(imgCoordPointBack, width, height);

    texCoordPointBack = (CvPoint2D32f *)malloc(sizeof(CvPoint2D32f) * verCountBack);
    verCoordPointBack = (CvPoint3D32f *)malloc(sizeof(CvPoint3D32f) * verCountBack);

    rotationMatrixToEulerAngle(rMat, &alpha, &beta, &gamma);
    //printf("%f %f %f\n",alpha, beta, gamma);

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

    getInvertMatrix(R, invR);

    for (k = 0; k < verCountBack; k++)
    {
        verCoordPointBack[k].x = -(1.0 - 2 * imgCoordPointBack[k].x / width);
        verCoordPointBack[k].y = -(1.0 - 2 * imgCoordPointBack[k].y / height);
        verCoordPointBack[k].z = 0;

        texCoordPointBack[k] = undistort(imgCoordPointBack[k].y, imgCoordPointBack[k].x,
                                         camera, distortTable, resizer, invR);
    }
#if 1
	//$)AWTP635
	/*camera[0] = 661.5395857819783;
	camera[1] = 371.8842023536371;
	camera[2] = 366.6896908038024;
	camera[3] = 356.1124332021072;

	distortTable[0] = -0.03647013101842009;
	distortTable[1] = 0.008072033197747085;
	distortTable[2] = -0.01052453285682819;
	distortTable[3] = 0.003438989378898739;*/


	/*tVec[0] = -407.442383; tVec[1] = 229.544632; tVec[2] = 1819.767090;

	//tVec[0] = -369.596161; tVec[1] = 226.227524; tVec[2] = 1593.657959;
	rMat[0] = -0.982189; rMat[1] = -0.123113; rMat[2] = -0.141947;
	rMat[3] = -0.041112; rMat[4] = -0.596337; rMat[5] = 0.801681;
	rMat[6] = -0.183345; rMat[7] = 0.793237; rMat[8] = 0.580654;*/


	/*tVec[0] = -57.863224; tVec[1] = -7.968931; tVec[2] = 1477.134521;
	rMat[0] = -0.996468; rMat[1] = -0.073354; rMat[2] = -0.040881;
	rMat[3] = 0.027985; rMat[4] = -0.749052; rMat[5] = 0.661920;
	rMat[6] = -0.079177; rMat[7] = 0.658437; rMat[8] = 0.748459;*/

	/*camera[0] = 320.08*2;
	camera[1] = 177.11*2;
	camera[2] = 187.36*2;
	camera[3] = 156.63*2;

	distortTable[0] = -0.03647013101842009;
	distortTable[1] = 0.008072033197747085;
	distortTable[2] = -0.01052453285682819;
	distortTable[3] = 0.003438989378898739;

	tVec[0] = 25.699928; tVec[1] = 60.056995; tVec[2] = 1577.340210;
	rMat[0] = -0.999639; rMat[1] = -0.026536; rMat[2] = 0.004318;
	rMat[3] = 0.023784; rMat[4] = -0.798004; rMat[5] = 0.602182;
	rMat[6] = -0.012534; rMat[7] = 0.602067; rMat[8] = 0.798347;*/


	camera[0] = 330.23*2;
	camera[1] = 183.49*2;
	camera[2] = 177.36*2;
	camera[3] = 171.53*2;


	distortTable[0]   = -0.0070845942839427014;
	distortTable[1]   = -0.019164490233966703;
	distortTable[2]   =  0.0047466118871461417;
	distortTable[3]   =  -0.00062181781886237492;

	tVec[0] = 155.305710; tVec[1] = 559.879395; tVec[2] = 748.974854;
	rMat[0] = -0.989676; rMat[1] = -0.079691; rMat[2] = 0.119123;
	rMat[3] = 0.142066; rMat[4] = -0.655218; rMat[5] = 0.741961;
	rMat[6] = 0.018924; rMat[7] = 0.751224; rMat[8] = 0.659775;
#endif

#if 0
	//408$)A=A0h35
	camera[0] = 328.63*2;
	camera[1] = 167.79*2;
	camera[2] = 185.77*2;
	camera[3] = 174.16*2;

	distortTable[0] = -0.03647013101842009;
	distortTable[1] = 0.008072033197747085;
	distortTable[2] = -0.01052453285682819;
	distortTable[3] = 0.003438989378898739;

	tVec[0] = -314.301331; tVec[1] = -174.368835; tVec[2] = 4120.027832;
	rMat[0] = 0.986385; rMat[1] = -0.063298; rMat[2] = -0.151782;
	rMat[3] = 0.143237; rMat[4] = 0.784122; rMat[5] = 0.603851;
	rMat[6] = 0.080793; rMat[7] = -0.617370; rMat[8] = 0.782513;
#endif

    findRearCurve4(0.0, resizer, camera, distortTable, rMat, tVec, invR, bvs3DWidth, bvs3DHeight);


}

int initMosaic(safImgRect allView, safImgRect singleView, int flag)
{
    //bvs2DWidth = allView.width;
    //bvs2DHeight = allView.height;
    //bvs2DoffsetX = allView.x;
    //bvs2DoffsetY = allView.y;

    //bvs3DWidth = singleView.width;
    //bvs3DHeight = singleView.height;
    //bvs3DoffsetX = singleView.x;
    //bvs3DoffsetY = singleView.y;

    initCamParaData(flag);

    //init3DModel();

    init2DModel();

    initTextureCoords();

    initShader();

    //init3DCar();

    //loadCarPic();

    //cubeTexture = CreateSimpleTextureCubemap1( );

    //initUndistortSingleView();

    getCamPixelPosition();

    loadImage(IMGWIDTH, IMGHEIGHT);

    //initScaleLine();

    //findRuleLine();


    //caculateLumiaAdjust(srcY);

    //caculateLumiaCoeff();

    initVBO();

    initUndistort(bvs3DWidth, bvs3DHeight);

    //pu8CamMBvir[0] = NULL;
    //pu8CamMBvir[1] = NULL;
    //pu8CamMBvir[2] = NULL;
    //pu8CamMBvir[3] = NULL;

    srand(0);

    return 0;
}


#if 0
extern void updateTexture(unsigned char **src)
{
    int i;
    unsigned char *pSrcY,

             pSrcY = src[i];
    pSrcU = pSrcY + IMGWIDTH * IMGHEIGHT;
    pSrcV = pSrcU + IMGWIDTH * IMGHEIGHT / 4;

    //imageBuffer[i] = stbi_load(filePath, &width, &height, &channel, 0);
    //printf("image %d %d %d\n", width, height, channel);
    //glGenTextures( 1, &textureYUV[i]);
    //bindTexture(textureRGBA[i], imageBuffer[i], w, h, GL_RGBA);

    glGenTextures(1, &textureY[i]);
    bindTexture(textureY[i], pSrcY, IMGWIDTH, IMGHEIGHT, GL_LUMINANCE);

    glGenTextures(1, &textureU[i]);
    bindTexture(textureU[i], pSrcU, IMGWIDTH / 2, IMGHEIGHT / 2, GL_LUMINANCE);

    glGenTextures(1, &textureV[i]);
    bindTexture(textureV[i], pSrcV, IMGWIDTH / 2, IMGHEIGHT / 2, GL_LUMINANCE);
    for(i = 0; i < 5; i++)
    {
        glBindTexture( GL_TEXTURE_2D, textureRGBA[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, IMGWIDTH, IMGHEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, src[i]);
    }

    //caculateColorCoeff2D(src);
}

#endif

extern void updateTexture(unsigned char **src)
{
    int i;
    unsigned char *pSrcY, *pSrcU, *pSrcV;
    for(i = 0; i < 5; i++)
    {

        pSrcY = src[i];
        pSrcU = pSrcY + IMGWIDTH * IMGHEIGHT;
        pSrcV = pSrcU + IMGWIDTH * IMGHEIGHT / 4;

        glBindTexture( GL_TEXTURE_2D, textureY[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, IMGWIDTH, IMGHEIGHT, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pSrcY);

        glBindTexture( GL_TEXTURE_2D, textureU[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, IMGWIDTH / 2, IMGHEIGHT / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pSrcU);

        glBindTexture( GL_TEXTURE_2D, textureV[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, IMGWIDTH / 2, IMGHEIGHT / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pSrcV);
    }
    extern unsigned char* pu8CamMBvir[4];
    for(i = 0; i < 4; i++)
    {
        pu8CamMBvir[i] = src[i];
    }
    caculateColorCoeff2D(src);
}




#if 0
void BVSAutoChgAngleTest(void)
{
    static double step = 0.05;

    wheelangle += step;
    if(wheelangle >= 40)
    {
        step = -0.05;
    }
    else if(wheelangle <= -40)
    {
        step = 0.05;
    }
}
#endif


void BVSAutoChgDoorAngle(void)
{
    static double step = 0.5;

    gDoorAngle += step;
    if(gDoorAngle >= 40)
    {
        step = -0.5;
    }
    else if(gDoorAngle <= 0)
    {
        step = 0.5;
    }
}


void runRender(int viewMode, float steeringWheelAngle)
{
    //BVSAutoChgAngleTest();
    //BVSAutoChgDoorAngle();

    //printf("viewmode = %d\n", viewMode);
    // Clear the colorbuffer and depth-buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    show2DCar(viewMode);
    if(viewMode == VIEW_OVERALL)
    {
        showFourView();
    }
    else
    {
        showSingleView(viewMode, steeringWheelAngle);
    }

    //show3DCar(DisplayChannelID);

    //show2DCar(DisplayChannelID);
    //rotateView(wheelangle * 0.5);

    //showUndistortSingleView();

    //glDisable(GL_CULL_FACE);

    //eglSwapBuffers();
}








