
#include <GLES3/gl3.h>
#include <EGL/egl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <vector>

#include "avmInit.hpp"

#include "cv.h"
#include "cxcore.h"

#include "esTransform.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TEXTURE(N) "texture" #N
#define ADJUST(N) "color" #N "Adjust"

static int h_program_handle[5];

safImgRect g_all_view;
safImgRect g_single_view;

GLuint texture_y[5];
GLuint texture_u[5];
GLuint texture_v[5];

GLuint texture_res[4];

float lumia_ave[12];
vec3 color_ave[12];
vec3 color_count[AVERAGE_COUNT][12];

vec3 *ver_coord_point_front;
vec2 *tex_coord_point_front;
vec2 *img_coord_point_front;

vec3 *ver_coord_point_back;
vec2 *tex_coord_point_back;
vec2 *img_coord_point_back;

int ver_count_front;
int ver_count_back;

vec3 vertices_rear_traj_line_point[11][LENGTH * 2];

safImgRect front_resizer;
safImgRect rear_resizer;

GLuint BindTexture(GLuint texture, unsigned char *buffer, GLuint w, GLuint h, GLint type)
{
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, type, w, h, 0, type, GL_UNSIGNED_BYTE, buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return texture;
}

const char vertex_source[] =
    {
        "#version 300 es						  \n"
        "layout(location = 0) in vec3 iVexPos;              \n"
        "layout(location = 1) in vec2 iTexPos;              \n"
        "layout(location = 2) in float iCoeff;              \n"
        "uniform mat4 mvp;                        \n"
        "smooth centroid out vec2 gTexPos;                \n"
        "out float vCoeff;                \n"
        "void main()                            \n"
        "{                                        \n"
        "    gTexPos = iTexPos;           \n"
        "    vCoeff = iCoeff;           \n"
        "    gl_Position = mvp * vec4(iVexPos, 1.0);     \n"
        "}                                        \n"};

const char fragment_source[] =
    {
        "#version 300 es						  \n"
        "precision mediump float;                                      \n"
        "uniform sampler2D textureY;                               \n"
        "uniform sampler2D textureU;                              \n"
        "uniform sampler2D textureV;                              \n"
        "uniform vec3 color0Adjust;                                   \n"
        "uniform vec3 color1Adjust;                                   \n"
        "smooth centroid in vec2 gTexPos;                        \n"
        "in float vCoeff;                					 	 \n"
        "out vec4 fragColor;                          				 \n"
        "void main()                                                 \n"
        "{                                                           \n"
        "    vec3 yuv;                                       \n"
        "    vec3 rgb;                                       \n"
        "    float lumia = texture(textureY, gTexPos).r;     \n"
        "    float cr = texture(textureU, gTexPos).r - 0.5; \n"
        "	 float cb = texture(textureV, gTexPos).r - 0.5; \n"
        "	 yuv = vec3(lumia, cb, cr);                              \n"
        "	 rgb = yuv * mat3(1, 1.779, 0,                           \n"
        "                     1, -0.3455, -0.7169,                   \n"
        "                     1, 0, 1.4075);                         \n"
        "  	 vec3 gain = color0Adjust * (1.0 - vCoeff) + color1Adjust * vCoeff;      \n"
        "    fragColor = vec4(rgb + gain, 1.0);        \n"
        "}                                                           \n"

};

const char vertex_mosaic_source[] =
    {
        "#version 300 es						  \n"
        "layout(location = 0) in vec3 iVexPos;                      \n"
        "layout(location = 1) in vec2 iTexPos1;                     \n"
        "layout(location = 2) in vec2 iTexPos2;                     \n"
        "layout(location = 3) in float iAlpha;                         \n"
        "uniform mat4 mvp;                                              \n"
        "smooth centroid out vec2 gTexPos1;                         \n"
        "smooth centroid out vec2 gTexPos2;                         \n"
        "out float gAlpha;                                         	\n"
        "void main()                                                    \n"
        "{                                                              \n"
        "    gTexPos1 = iTexPos1;                               \n"
        "    gTexPos2 = iTexPos2;                               \n"
        "    gAlpha = iAlpha;                                         \n"
        "    gl_Position = gl_Position = mvp * vec4(iVexPos, 1.0);                           \n"
        "}                                                              \n"};

const char fragment_mosaic_source[] =
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
        "smooth centroid in vec2 gTexPos1;                          \n"
        "smooth centroid in vec2 gTexPos2;                          \n"
        "in float gAlpha;                                         		\n"
        "out vec4 fragColor;                          					\n"
        "void main()                                                    \n"
        "{                                                              \n"
        "    vec3 yuv1, yuv2;                                       \n"
        "    vec3 rgb1, rgb2;                                       \n"
        "    float lumia, cb, cr;                                       \n"
        "    lumia = texture(texture1Y, gTexPos1).r;     \n"
        "    cr = texture(texture1U, gTexPos1).r - 0.5; \n"
        "	 cb= texture(texture1V, gTexPos1).r - 0.5; \n"
        "	 yuv1 = vec3(lumia, cb, cr);                              \n"
        "    lumia = texture(texture2Y, gTexPos2).r;     \n"
        "    cr = texture(texture2U, gTexPos2).r - 0.5; \n"
        "	 cb = texture(texture2V, gTexPos2).r - 0.5; \n"
        "	 yuv2 = vec3(lumia, cb, cr);                              \n"
        "	 rgb1 = yuv1 * mat3(1, 1.779, 0,                           \n"
        "                     1, -0.3455, -0.7169,                   \n"
        "                     1, 0, 1.4075);                         \n"
        "	 rgb2 = yuv2 * mat3(1, 1.779, 0,                           \n"
        "                     1, -0.3455, -0.7169,                   \n"
        "                     1, 0, 1.4075);                         \n"
        "    fragColor = vec4(mix(rgb1 + color0Adjust, rgb2 + color1Adjust, gAlpha), 1.0);\n"
        "}                                                              \n"};

const char vertex_blend_source[] =
    {
        "#version 300 es										\n"
        "layout(location = 0) in vec3 vPosition;			  \n"
        "uniform mat4 mvp;										\n"
        "void main()											\n"
        "{														\n"
        "	 gl_Position = mvp * vec4(vPosition, 1.0);		 \n"
        "}														\n"};

const char fragment_blend_source[] =
    {
        "#version 300 es											\n"
        "precision mediump float; 									\n"
        "uniform vec4 outColor; 					  \n"
        "out vec4 fragColor;										\n"
        "void main()												\n"
        "{															\n"
        "	 fragColor = outColor;			\n"
        "}															\n"};

const char vertex_bmp_show_source[] =
    {
        "#version 300 es						  				\n"
        "layout(location = 0) in vec3 iVexPos;              \n"
        "layout(location = 1) in vec2 iTexPos;              \n"
        "uniform mat4 mvp;                        				\n"
        "smooth centroid out vec2 gTexPos;                	\n"
        "void main()                              				\n"
        "{                                        				\n"
        "    gTexPos = iTexPos;           				\n"
        "    gl_Position = mvp * vec4(iVexPos, 1.0);     				\n"
        "}                                        				\n"};

const char fragment_bmp_show_source[] =
    {
        "#version 300 es						  					\n"
        "precision mediump float;                                     \n"
        "uniform sampler2D textureImg;                            \n"
        "smooth centroid in vec2 gTexPos;                       \n"
        "out vec4 fragColor;                          				\n"
        "void main()                                                \n"
        "{                                                          \n"
        "    fragColor = texture(textureImg, gTexPos);       	\n"
        "}                                                          \n"};

const char vertex_camera_source[] =
    {
        "#version 300 es						  \n"
        "layout(location = 0) in vec3 iVexPos;              \n"
        "layout(location = 1) in vec2 iTexPos;              \n"
        "uniform mat4 mvp;                        \n"
        "smooth centroid out vec2 gTexPos;                \n"
        "void main()                            \n"
        "{                                        \n"
        "    gTexPos = iTexPos;           \n"
        "    gl_Position = mvp * vec4(iVexPos, 1.0);     \n"
        "}                                        \n"};

const char fragment_camera_source[] =
    {
        "#version 300 es						  \n"
        "precision mediump float;                                      \n"
        "uniform sampler2D textureY;                               \n"
        "uniform sampler2D textureU;                              \n"
        "uniform sampler2D textureV;                              \n"
        "smooth centroid in vec2 gTexPos;                        \n"
        "in float vCoeff;                					 	 \n"
        "out vec4 fragColor;                          				 \n"
        "void main()                                                 \n"
        "{                                                           \n"
        "    vec3 yuv;                                       \n"
        "    vec3 rgb;                                       \n"
        "    float lumia = texture(textureY, gTexPos).r;     \n"
        "    float cr = texture(textureU, gTexPos).r - 0.5; \n"
        "	 float cb= texture(textureV, gTexPos).r - 0.5; \n"
        "	 yuv = vec3(lumia, cb, cr);                              \n"
        "	 rgb = yuv * mat3(1, 1.779, 0,                           \n"
        "                     1, -0.3455, -0.7169,                   \n"
        "                     1, 0, 1.4075);                         \n"
        "    fragColor = vec4(rgb , 1.0);        \n"
        "}                                                           \n"

};

void CaculateColorCoeff2D(unsigned char **image_buffer)
{
    int i;
    unsigned int lumia_sum0;
    unsigned int lumia_sum1;

    int index0, index1;
    vec3 rgb_color[12];

    unsigned int count;
    static unsigned int run_count = 0;
    int index;

    if ((NULL == image_buffer[0]) || (NULL == image_buffer[1]) || (NULL == image_buffer[2]) || (NULL == image_buffer[3]))
    {
        /*check image address*/
        return;
    }

    index = run_count % AVERAGE_COUNT;

    lumia_sum0 = 0;
    lumia_sum1 = 0;

    for (i = 0; i < tex_coords_statistics.gl_tex_coord_fl_f.size(); ++i)
    {
        index0 = tex_coords_statistics.gl_tex_coord_fl_f[i];
        lumia_sum0 += image_buffer[0][index0];

        index1 = tex_coords_statistics.gl_tex_coord_fl_l[i];
        lumia_sum1 += image_buffer[2][index1];
    }

    count = tex_coords_statistics.gl_tex_coord_fl_f.size();
    rgb_color[0].x = 1.0 * lumia_sum0 / count;
    rgb_color[0].y = 1.0 * lumia_sum0 / count;
    rgb_color[0].z = 1.0 * lumia_sum0 / count;

    rgb_color[1].x = 1.0 * lumia_sum1 / count;
    rgb_color[1].y = 1.0 * lumia_sum1 / count;
    rgb_color[1].z = 1.0 * lumia_sum1 / count;

    lumia_sum0 = 0;
    lumia_sum1 = 0;

    for (i = 0; i < tex_coords_statistics.gl_tex_coord_fr_f.size(); ++i)
    {
        index0 = tex_coords_statistics.gl_tex_coord_fr_f[i];
        lumia_sum0 += image_buffer[0][index0];

        index1 = tex_coords_statistics.gl_tex_coord_fr_r[i];
        lumia_sum1 += image_buffer[3][index1];
    }

    count = tex_coords_statistics.gl_tex_coord_fr_f.size();

    rgb_color[2].x = 1.0 * lumia_sum0 / count;
    rgb_color[2].y = 1.0 * lumia_sum0 / count;
    rgb_color[2].z = 1.0 * lumia_sum0 / count;

    rgb_color[3].x = 1.0 * lumia_sum1 / count;
    rgb_color[3].y = 1.0 * lumia_sum1 / count;
    rgb_color[3].z = 1.0 * lumia_sum1 / count;

    lumia_sum0 = 0;
    lumia_sum1 = 0;

    for (i = 0; i < tex_coords_statistics.gl_tex_coord_bl_b.size(); ++i)
    {
        index0 = tex_coords_statistics.gl_tex_coord_bl_b[i];
        lumia_sum0 += image_buffer[1][index0];

        index1 = tex_coords_statistics.gl_tex_coord_bl_l[i];
        lumia_sum1 += image_buffer[2][index1];
    }

    count = tex_coords_statistics.gl_tex_coord_bl_b.size();

    rgb_color[4].x = 1.0 * lumia_sum0 / count;
    rgb_color[4].y = 1.0 * lumia_sum0 / count;
    rgb_color[4].z = 1.0 * lumia_sum0 / count;

    rgb_color[5].x = 1.0 * lumia_sum1 / count;
    rgb_color[5].y = 1.0 * lumia_sum1 / count;
    rgb_color[5].z = 1.0 * lumia_sum1 / count;

    lumia_sum0 = 0;
    lumia_sum1 = 0;

    for (i = 0; i < tex_coords_statistics.gl_tex_coord__br_b.size(); ++i)
    {
        index0 = tex_coords_statistics.gl_tex_coord__br_b[i];
        lumia_sum0 += image_buffer[1][index0];

        index1 = tex_coords_statistics.gl_tex_coord__br_r[i];
        lumia_sum1 += image_buffer[3][index1];
    }

    count = tex_coords_statistics.gl_tex_coord__br_b.size();

    rgb_color[6].x = 1.0 * lumia_sum0 / count;
    rgb_color[6].y = 1.0 * lumia_sum0 / count;
    rgb_color[6].z = 1.0 * lumia_sum0 / count;

    rgb_color[7].x = 1.0 * lumia_sum1 / count;
    rgb_color[7].y = 1.0 * lumia_sum1 / count;
    rgb_color[7].z = 1.0 * lumia_sum1 / count;

    for (i = 0; i < 8; ++i)
    {
        color_ave[i].x = rgb_color[i].x / 255.0;
        color_ave[i].y = rgb_color[i].y / 255.0;
        color_ave[i].z = rgb_color[i].z / 255.0;
        // printf("color_ave[%d] = %f %f %f\n",i,color_ave[i].x,color_ave[i].y,color_ave[i].z);
    }

    color_ave[8].x = (color_ave[0].x + color_ave[1].x) / 2;
    color_ave[8].y = (color_ave[0].y + color_ave[1].y) / 2;
    color_ave[8].z = (color_ave[0].z + color_ave[1].z) / 2;

    color_ave[9].x = (color_ave[2].x + color_ave[3].x) / 2;
    color_ave[9].y = (color_ave[2].y + color_ave[3].y) / 2;
    color_ave[9].z = (color_ave[2].z + color_ave[3].z) / 2;

    color_ave[10].x = (color_ave[4].x + color_ave[5].x) / 2;
    color_ave[10].y = (color_ave[4].y + color_ave[5].y) / 2;
    color_ave[10].z = (color_ave[4].z + color_ave[5].z) / 2;

    color_ave[11].x = (color_ave[6].x + color_ave[7].x) / 2;
    color_ave[11].y = (color_ave[6].y + color_ave[7].y) / 2;
    color_ave[11].z = (color_ave[6].z + color_ave[7].z) / 2;
}

extern "C" void imageFilp(unsigned char *src, unsigned char *dst)
{
    unsigned char *p_src_y, *p_src_u, *p_src_v;
    unsigned char *pDstY, *pDstU, *pDstV;
    CvMat srcY, srcU, srcV;
    CvMat dstY, dstU, dstV;

    p_src_y = src;
    p_src_u = p_src_y + IMAGE_WIDTH * IMAGE_HEIGHT;
    p_src_v = p_src_u + IMAGE_WIDTH * IMAGE_HEIGHT / 4;

    pDstY = dst;
    pDstU = pDstY + IMAGE_WIDTH * IMAGE_HEIGHT;
    pDstV = pDstU + IMAGE_WIDTH * IMAGE_HEIGHT / 4;

    srcY = cvMat(IMAGE_WIDTH, IMAGE_HEIGHT, CV_8UC1, p_src_y);
    //srcU = cvMat(IMAGE_WIDTH/2, IMAGE_HEIGHT/2, CV_8UC1, p_src_u);
    //srcV = cvMat(IMAGE_WIDTH/2, IMAGE_HEIGHT/2, CV_8UC1, p_src_v);

    dstY = cvMat(IMAGE_WIDTH, IMAGE_HEIGHT, CV_8UC1, pDstY);
    //dstU = cvMat(IMAGE_WIDTH/2, IMAGE_HEIGHT/2, CV_8UC1, pDstU);
    //dstV = cvMat(IMAGE_WIDTH/2, IMAGE_HEIGHT/2, CV_8UC1, pDstV);

    cvFlip(&srcY, &dstY, -1);
    //cvFlip(&srcU, &dstU, -1);
    //cvFlip(&srcV, &dstV, -1);
}

extern void UpdateCarImage(char car_name[])
{
    int width;
    int height;
    int channel;
    unsigned char *res_buffer;

    char file_path[256] = {0};
    strcpy((char *)parking_assistant_params.car_name, car_name);
    sprintf(file_path, "./test/%s", parking_assistant_params.car_name);
    printf("file_path = %s res_buffer = %p\n", file_path, res_buffer);
    res_buffer = stbi_load(file_path, &width, &height, &channel, 0);
    printf("res_buffer = %p,w = %d, h = %d,c = %d\n", res_buffer, width, height, channel);
    //memset(res_buffer, 0, width * height * channel);
    //glGenTextures( 1, &texture_res[0]);
    glBindTexture(GL_TEXTURE_2D, texture_res[0]);
    if (channel == 4)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, res_buffer);
    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, res_buffer);
    }
    free(res_buffer);
}

// void UpdateCarImage(int status)
// {
//     int width;
//     int height;
//     int channel;
//     unsigned char *res_buffer;
//     char file_path[256] = {0};

//     //sprintf(file_path, "./test/%s", parking_assistant_params.car_name);
//     if (status)
//         res_buffer = stbi_load("./test/mixer.png", &width, &height, &channel, 0);
//     else
//         res_buffer = stbi_load("./test/dump.png", &width, &height, &channel, 0);

//     printf("car = %x %d %d %d\n", res_buffer, width, height, channel);
//     glBindTexture(GL_TEXTURE_2D, texture_res[0]);

//     if (channel == 4)
//     {
//         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, res_buffer);
//     }
//     else
//     {
//         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, res_buffer);
//     }
// }

void LoadRes(int w, int h)
{
    int i;

    int width;
    int height;
    int channel;
    unsigned char *res_buffer[4];
    unsigned char *p_src_y;
    unsigned char *p_src_u;
    unsigned char *p_src_v;

    unsigned char *yuv_buffer[5];
    char file_path[256] = {0};
    FILE *fp;

    for (i = 0; i < 5; ++i)
    {
        yuv_buffer[i] = (unsigned char *)malloc(w * h * 3 / 2);
        /*sprintf(file_path, "../test/b%d.YUV", i);
        fp = fopen(file_path, "rb");
        fread(yuv_buffer[i], 1, w * h * 3 / 2, fp);
        fclose(fp);

        if(i > 1)
        {
            imageFilp(yuv_buffer[i], yuv_buffer[i]);
        }*/

        p_src_y = yuv_buffer[i];
        p_src_u = p_src_y + w * h;
        p_src_v = p_src_u + w * h / 4;

        glGenTextures(1, &texture_y[i]);
        BindTexture(texture_y[i], p_src_y, w, h, GL_LUMINANCE);

        glGenTextures(1, &texture_u[i]);
        BindTexture(texture_u[i], p_src_u, w / 2, h / 2, GL_LUMINANCE);

        glGenTextures(1, &texture_v[i]);
        BindTexture(texture_v[i], p_src_v, w / 2, h / 2, GL_LUMINANCE);
    }

    stbi_set_flip_vertically_on_load(0);

    //res_buffer[1] = stbi_load("./test/car.png", &width, &height, &channel, 0);
    sprintf(file_path, "./test/%s", parking_assistant_params.car_name);
    res_buffer[1] = stbi_load(file_path, &width, &height, &channel, 0);

    glGenTextures(1, &texture_res[0]);

    if (channel == 4)
    {
        BindTexture(texture_res[0], res_buffer[1], width, height, GL_RGBA);
    }
    else
    {
        BindTexture(texture_res[0], res_buffer[1], width, height, GL_RGB);
    }

    //pu8_cam_vir[0] = yuv_buffer[0];
    //pu8_cam_vir[1] = yuv_buffer[1];
    //pu8_cam_vir[2] = yuv_buffer[2];
    //pu8_cam_vir[3] = yuv_buffer[3];

    //cameraCalib(920, 310, 116);
}

GLuint EsLoadShader(GLenum type, const char *shaderSrc)
{
    GLuint shader;
    GLint compiled;
    char *infoLog;

    // Create the shader object
    shader = glCreateShader(type);

    if (shader == 0)
    {
        return 0;
    }

    // Load the shader source
    glShaderSource(shader, 1, &shaderSrc, NULL);

    // Compile the shader
    glCompileShader(shader);

    // Check the compile status
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
        GLint infoLen = 0;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1)
        {
            infoLog = (char *)malloc(sizeof(char) * infoLen);

            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            printf("Error compiling shader:\n%s\n", infoLog);

            free(infoLog);
        }

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint EsLoadProgram(const char *vertShaderSrc, const char *fragShaderSrc)
{
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint programObject;
    GLint linked;
    char *infoLog;

    // Load the vertex/fragment shaders
    vertexShader = EsLoadShader(GL_VERTEX_SHADER, vertShaderSrc);

    if (vertexShader == 0)
    {
        return 0;
    }

    fragmentShader = EsLoadShader(GL_FRAGMENT_SHADER, fragShaderSrc);

    if (fragmentShader == 0)
    {
        glDeleteShader(vertexShader);
        return 0;
    }

    // Create the program object
    programObject = glCreateProgram();

    if (programObject == 0)
    {
        return 0;
    }

    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);

    // Link the program
    glLinkProgram(programObject);

    // Check the link status
    glGetProgramiv(programObject, GL_LINK_STATUS, &linked);

    if (!linked)
    {
        GLint infoLen = 0;

        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1)
        {
            infoLog = (char *)malloc(sizeof(char) * infoLen);

            glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
            printf("Error linking program:\n%s\n", infoLog);

            free(infoLog);
        }

        glDeleteProgram(programObject);
        return 0;
    }

    // Free up no longer needed shader resources
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return programObject;
}

static void SetBool(GLuint programId, const char *name, bool value)
{
    glUniform1i(glGetUniformLocation(programId, name), (int)value);
}

static void SetInt(GLuint programId, const char *name, int value)
{
    glUniform1i(glGetUniformLocation(programId, name), value);
}

static void SetFloat(GLuint programId, const char *name, float value)
{
    glUniform1f(glGetUniformLocation(programId, name), value);
}

static void SetVec2(GLuint programId, const char *name, const vec2 value)
{
    glUniform2fv(glGetUniformLocation(programId, name), 1, &value.x);
}

static void SetVec2(GLuint programId, const char *name, float x, float y)
{
    glUniform2f(glGetUniformLocation(programId, name), x, y);
}

static void SetVec3(GLuint programId, const char *name, const vec3 value)
{
    glUniform3fv(glGetUniformLocation(programId, name), 1, &value.x);
}

static void SetVec3(GLuint programId, const char *name, float x, float y, float z)
{
    glUniform3f(glGetUniformLocation(programId, name), x, y, z);
}

static void SetVec4(GLuint programId, const char *name, const vec4 value)
{
    glUniform4fv(glGetUniformLocation(programId, name), 1, &value.r);
}

static void SetVec4(GLuint programId, const char *name, float x, float y, float z, float w)
{
    glUniform4f(glGetUniformLocation(programId, name), x, y, z, w);
}

static void SetMat4(GLuint programId, const char *name, ESMatrix mat)
{
    glUniformMatrix4fv(glGetUniformLocation(programId, name), 1, GL_FALSE, &mat.m[0][0]);
}

void RotationMatrixToEulerAngle(float *R, float *alpha, float *beta, float *gamma)
{
    float sy = sqrt(R[7] * R[7] + R[8] * R[8]);

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

double TowFac(double *b)
{
    return b[0] * b[3] - b[1] * b[2];
}

void Copy3To2(double *a, double *b, int i, int j)
{
    int m = 0;
    int n = 0;
    int count = 0;
    for (m = 0; m < 3; ++m)
    {
        for (n = 0; n < 3; ++n)
        {
            if (m != i && n != j)
            {
                count++;
                b[((count - 1) / 2) * 2 + (count - 1) % 2] = a[m * 3 + n];
            }
        }
    }
}

double ThreeSum(double *a, double *b)
{
    double sum = 0;
    for (int i = 0; i < 3; ++i)
    {
        Copy3To2(a, b, 0, i);
        if (i % 2 == 0)
        {
            sum += a[i] * TowFac(b);
        }
        else
        {
            sum -= a[i] * TowFac(b);
        }
    }
    return sum;
}

void CalCArray(double *a, double *b, double *c)
{
    int i = 0;
    int j = 0;
    for (i = 0; i < 3; ++i)
    {
        for (j = 0; j < 3; ++j)
        {
            Copy3To2(a, b, i, j);
            if ((i + j) % 2 == 0)
            {
                c[j * 3 + i] = TowFac(b);
            }
            else
            {
                c[j * 3 + i] = -TowFac(b);
            }
        }
    }
}

void InvertArray(double *c, double A)
{
    int i = 0;
    int j = 0;
    for (i = 0; i < 3; ++i)
    {
        for (j = 0; j < 3; ++j)
        {
            c[i * 3 + j] /= A;
        }
    }
}

void InvertMatrix(double *src, double *dst)
{

    double b[2][2];
    double t = ThreeSum(src, &b[0][0]);
    CalCArray(src, &b[0][0], dst);
    InvertArray(dst, t);
}

int InitShader(void)
{
    h_program_handle[0] = EsLoadProgram(vertex_source, fragment_source);
    h_program_handle[1] = EsLoadProgram(vertex_mosaic_source, fragment_mosaic_source);
    h_program_handle[2] = EsLoadProgram(vertex_bmp_show_source, fragment_bmp_show_source);
    h_program_handle[3] = EsLoadProgram(vertex_blend_source, fragment_blend_source);
    h_program_handle[4] = EsLoadProgram(vertex_camera_source, fragment_camera_source);

    printf("%d %d %d %d %d\n", h_program_handle[0], h_program_handle[1], h_program_handle[2], h_program_handle[3], h_program_handle[4]);

    return GL_TRUE;
}

void Show2DCar()
{
    vec3 color_adjust[2];
    ESMatrix matrix_MVP;

    esMatrixLoadIdentity(&matrix_MVP);

    glViewport(g_all_view.x, g_all_view.y, g_all_view.width, g_all_view.height);

    glUseProgram(h_program_handle[0]);

    /* Enable attributes for position, color and texture coordinates etc. */
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    SetMat4(h_program_handle[0], "mvp", matrix_MVP);

    //Front
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.cam_vertices_points[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.cam_texture_points[0]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.LumiaBalance[0]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);

    color_adjust[0].x = color_ave[8].x - color_ave[0].x;
    color_adjust[0].y = color_ave[8].y - color_ave[0].y;
    color_adjust[0].z = color_ave[8].z - color_ave[0].z;

    color_adjust[1].x = color_ave[9].x - color_ave[2].x;
    color_adjust[1].y = color_ave[9].y - color_ave[2].y;
    color_adjust[1].z = color_ave[9].z - color_ave[2].z;

    SetVec3(h_program_handle[0], ADJUST(0), color_adjust[0]);
    SetVec3(h_program_handle[0], ADJUST(1), color_adjust[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_y[0]);
    SetInt(h_program_handle[0], TEXTURE(Y), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_u[0]);
    SetInt(h_program_handle[0], TEXTURE(U), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture_v[0]);
    SetInt(h_program_handle[0], TEXTURE(V), 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_coords.glVertex_F.size());

    //back
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.cam_vertices_points[1]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.cam_texture_points[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.LumiaBalance[1]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);

    color_adjust[0].x = color_ave[10].x - color_ave[4].x;
    color_adjust[0].y = color_ave[10].y - color_ave[4].y;
    color_adjust[0].z = color_ave[10].z - color_ave[4].z;

    color_adjust[1].x = color_ave[11].x - color_ave[6].x;
    color_adjust[1].y = color_ave[11].y - color_ave[6].y;
    color_adjust[1].z = color_ave[11].z - color_ave[6].z;

    SetVec3(h_program_handle[0], ADJUST(0), color_adjust[0]);
    SetVec3(h_program_handle[0], ADJUST(1), color_adjust[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_y[1]);
    SetInt(h_program_handle[0], TEXTURE(Y), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_u[1]);
    SetInt(h_program_handle[0], TEXTURE(U), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture_v[1]);
    SetInt(h_program_handle[0], TEXTURE(V), 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_coords.glVertex_B.size());

    //left
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.cam_vertices_points[2]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.cam_texture_points[2]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.LumiaBalance[2]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);

    color_adjust[0].x = color_ave[8].x - color_ave[1].x;
    color_adjust[0].y = color_ave[8].y - color_ave[1].y;
    color_adjust[0].z = color_ave[8].z - color_ave[1].z;

    color_adjust[1].x = color_ave[10].x - color_ave[5].x;
    color_adjust[1].y = color_ave[10].y - color_ave[5].y;
    color_adjust[1].z = color_ave[10].z - color_ave[5].z;

    SetVec3(h_program_handle[0], ADJUST(0), color_adjust[0]);
    SetVec3(h_program_handle[0], ADJUST(1), color_adjust[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_y[2]);
    SetInt(h_program_handle[0], TEXTURE(Y), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_u[2]);
    SetInt(h_program_handle[0], TEXTURE(U), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture_v[2]);
    SetInt(h_program_handle[0], TEXTURE(V), 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_coords.glVertex_L.size());

    //right
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.cam_vertices_points[3]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.cam_texture_points[3]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.LumiaBalance[3]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);

    color_adjust[0].x = color_ave[9].x - color_ave[3].x;
    color_adjust[0].y = color_ave[9].y - color_ave[3].y;
    color_adjust[0].z = color_ave[9].z - color_ave[3].z;

    color_adjust[1].x = color_ave[11].x - color_ave[7].x;
    color_adjust[1].y = color_ave[11].y - color_ave[7].y;
    color_adjust[1].z = color_ave[11].z - color_ave[7].z;

    SetVec3(h_program_handle[0], ADJUST(0), color_adjust[0]);
    SetVec3(h_program_handle[0], ADJUST(1), color_adjust[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_y[3]);
    SetInt(h_program_handle[0], TEXTURE(Y), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_u[3]);
    SetInt(h_program_handle[0], TEXTURE(U), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture_v[3]);
    SetInt(h_program_handle[0], TEXTURE(V), 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_coords.glVertex_R.size());

    glUseProgram(h_program_handle[1]);

    /* Enable attributes for position, color and texture coordinates etc. */
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    SetMat4(h_program_handle[1], "mvp", matrix_MVP);

    //front  left
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.alpha[0]);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.fuse_cam_vertices_points[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.fuse_fl_cam_texture_points[0]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.fuse_fl_cam_texture_points[1]);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    color_adjust[0].x = color_ave[8].x - color_ave[0].x;
    color_adjust[0].y = color_ave[8].y - color_ave[0].y;
    color_adjust[0].z = color_ave[8].z - color_ave[0].z;

    color_adjust[1].x = color_ave[8].x - color_ave[1].x;
    color_adjust[1].y = color_ave[8].y - color_ave[1].y;
    color_adjust[1].z = color_ave[8].z - color_ave[1].z;

    SetVec3(h_program_handle[1], ADJUST(0), color_adjust[0]);
    SetVec3(h_program_handle[1], ADJUST(1), color_adjust[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_y[0]);
    SetInt(h_program_handle[1], TEXTURE(1Y), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_u[0]);
    SetInt(h_program_handle[1], TEXTURE(1U), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture_v[0]);
    SetInt(h_program_handle[1], TEXTURE(1V), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texture_y[2]);
    SetInt(h_program_handle[1], TEXTURE(2Y), 3);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, texture_u[2]);
    SetInt(h_program_handle[1], TEXTURE(2U), 4);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, texture_v[2]);
    SetInt(h_program_handle[1], TEXTURE(2V), 5);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_coords.glVertex_FL.size());

    //front  right
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.alpha[1]);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.fuse_cam_vertices_points[1]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.fuse_fr_cam_texture_points[0]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.fuse_fr_cam_texture_points[1]);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    color_adjust[0].x = color_ave[9].x - color_ave[2].x;
    color_adjust[0].y = color_ave[9].y - color_ave[2].y;
    color_adjust[0].z = color_ave[9].z - color_ave[2].z;

    color_adjust[1].x = color_ave[9].x - color_ave[3].x;
    color_adjust[1].y = color_ave[9].y - color_ave[3].y;
    color_adjust[1].z = color_ave[9].z - color_ave[3].z;

    SetVec3(h_program_handle[1], ADJUST(0), color_adjust[0]);
    SetVec3(h_program_handle[1], ADJUST(1), color_adjust[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_y[0]);
    SetInt(h_program_handle[1], TEXTURE(1Y), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_u[0]);
    SetInt(h_program_handle[1], TEXTURE(1U), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture_v[0]);
    SetInt(h_program_handle[1], TEXTURE(1V), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texture_y[3]);
    SetInt(h_program_handle[1], TEXTURE(2Y), 3);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, texture_u[3]);
    SetInt(h_program_handle[1], TEXTURE(2U), 4);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, texture_v[3]);
    SetInt(h_program_handle[1], TEXTURE(2V), 5);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_coords.glVertex_FR.size());

    //back  left
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.alpha[2]);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.fuse_cam_vertices_points[2]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.fuse_bl_cam_texture_points[0]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.fuse_bl_cam_texture_points[1]);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    color_adjust[0].x = color_ave[10].x - color_ave[4].x;
    color_adjust[0].y = color_ave[10].y - color_ave[4].y;
    color_adjust[0].z = color_ave[10].z - color_ave[4].z;

    color_adjust[1].x = color_ave[10].x - color_ave[5].x;
    color_adjust[1].y = color_ave[10].y - color_ave[5].y;
    color_adjust[1].z = color_ave[10].z - color_ave[5].z;

    SetVec3(h_program_handle[1], ADJUST(0), color_adjust[0]);
    SetVec3(h_program_handle[1], ADJUST(1), color_adjust[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_y[1]);
    SetInt(h_program_handle[1], TEXTURE(1Y), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_u[1]);
    SetInt(h_program_handle[1], TEXTURE(1U), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture_v[1]);
    SetInt(h_program_handle[1], TEXTURE(1V), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texture_y[2]);
    SetInt(h_program_handle[1], TEXTURE(2Y), 3);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, texture_u[2]);
    SetInt(h_program_handle[1], TEXTURE(2U), 4);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, texture_v[2]);
    SetInt(h_program_handle[1], TEXTURE(2V), 5);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_coords.glVertex_BL.size());

    //back  right
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.alpha[3]);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.fuse_cam_vertices_points[3]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.fuse_br_cam_texture_points[0]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.fuse_br_cam_texture_points[1]);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    color_adjust[0].x = color_ave[11].x - color_ave[6].x;
    color_adjust[0].y = color_ave[11].y - color_ave[6].y;
    color_adjust[0].z = color_ave[11].z - color_ave[6].z;

    color_adjust[1].x = color_ave[11].x - color_ave[7].x;
    color_adjust[1].y = color_ave[11].y - color_ave[7].y;
    color_adjust[1].z = color_ave[11].z - color_ave[7].z;

    SetVec3(h_program_handle[1], ADJUST(0), color_adjust[0]);
    SetVec3(h_program_handle[1], ADJUST(1), color_adjust[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_y[1]);
    SetInt(h_program_handle[1], TEXTURE(1Y), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_u[1]);
    SetInt(h_program_handle[1], TEXTURE(1U), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture_v[1]);
    SetInt(h_program_handle[1], TEXTURE(1V), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texture_y[3]);
    SetInt(h_program_handle[1], TEXTURE(2Y), 3);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, texture_u[3]);
    SetInt(h_program_handle[1], TEXTURE(2U), 4);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, texture_v[3]);
    SetInt(h_program_handle[1], TEXTURE(2V), 5);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_coords.glVertex_BR.size());

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(h_program_handle[2]);

    SetMat4(h_program_handle[2], "mvp", matrix_MVP);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.car_ver_tex_coord[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.car_ver_tex_coord[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_res[0]);
    SetInt(h_program_handle[1], "textureImg", 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void DrawUndistortBackCurve(ESMatrix ortho_matrix)
{
    int i;
    vec4 color[4] = {{1.0, 0.0, 0.0, 0.5}, {1.0, 1.0, 0.0, 0.5}, {0.0, 1.0, 0.0, 0.5}, {0.0, 0.0, 0.0, 0.5}};

    glUseProgram(h_program_handle[3]);

    SetMat4(h_program_handle[3], "mvp", ortho_matrix);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    for (i = 0; i < 6; ++i)
    {
        SetVec4(h_program_handle[3], "outColor", color[i / 2]);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices_rear_traj_line_point[i]);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, LENGTH * 2);
    }

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices_rear_traj_line_point[6]);
    for (i = 0; i < 3; ++i)
    {
        SetVec4(h_program_handle[3], "outColor", color[i]);
        glDrawArrays(GL_TRIANGLE_STRIP, i * 4, 4);
    }
}

vec2 Undistort(float i, float j, const float *A, const float *dist_coeffs, undistortParams params, double *inv_r)
{
    float u;
    float v;
    float x;
    float y;
    float x2;
    float y2;
    float r2;
    float r;
    float theta;
    float theta2;
    float theta4;
    float theta6;
    float theta8;
    float theta_d;
    float xd;
    float yd;
    vec2 imgPoints;
    float p3D[3];
    float oldP3D[3];
    float z;

    float fx = A[0];
    float fy = A[4];
    float cx = A[2];
    float cy = A[5];

    float k1 = dist_coeffs[0];
    float k2 = dist_coeffs[1];
    float p1 = dist_coeffs[2];
    float p2 = dist_coeffs[3];

    p3D[0] = (params.x_zoom * j + params.x - cx) / fx;
    p3D[1] = (params.y_zoom * i + params.y - cy) / fy;
    p3D[2] = 1.0;

    oldP3D[0] = inv_r[0] * p3D[0] + inv_r[1] * p3D[1] + inv_r[2] * p3D[2];
    oldP3D[1] = inv_r[3] * p3D[0] + inv_r[4] * p3D[1] + inv_r[5] * p3D[2];
    oldP3D[2] = inv_r[6] * p3D[0] + inv_r[7] * p3D[1] + inv_r[8] * p3D[2];

    if (oldP3D[2] > 0.001)
    {
        x = oldP3D[0] / oldP3D[2];
        y = oldP3D[1] / oldP3D[2];
        z = 1.0;

        /*perspective divide get image point*/
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

        if (r < 0.000001)
        {
            xd = 0;
            yd = 0;
        }
        else
        {
            /*get correct image point*/
            xd = x * theta_d / r;
            yd = y * theta_d / r;
        }

        /*get pixel point*/
        u = fx * xd + cx;
        v = fy * yd + cy;

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

    /*normalize 0~1*/
    imgPoints.x = u / IMAGE_WIDTH;
    imgPoints.y = v / IMAGE_HEIGHT;

    return imgPoints;
}

void ShowSingleView(int view_mode, float wheel_angle)
{
    int which_camera;
    int cnt;
    float vertices_view[] =
        {
            -1.0f, -1.0f, 0.0f, // left-buttom
            1.0f, -1.0f, 0.0f,  // right- buttom
            -1.0f, 1.0f, 0.0f,  // right-top
            1.0f, 1.0f, 0.0f,   // left-top
        };

    float tex_coord_view[] =
        {
            0.0f, 1.0f, // left-top
            1.0f, 1.0f, // right-top
            0.0f, 0.0f, // left-buttom
            1.0f, 0.0f, // right- buttom
        };

    float texCoordViewLR[] =
        {
            0.1f, 1.0f, // left-top
            0.9f, 1.0f, // right-top
            0.1f, 0.0f, // left-buttom
            0.9f, 0.0f, // right- buttom
        };

    float texCoordViewDMS[] =
        {
            0.0f, 0.0f, // left-top
            0.0f, 1.0f, // right-top
            1.0f, 0.0f, // left-buttom
            1.0f, 1.0f, // right- buttom
        };

    float *verAddr;
    float *texAddr;
    ESMatrix ortho_matrix;

    switch (view_mode)
    {
    case VIEW_FRONT:
        which_camera = 0;
        cnt = 4;
        verAddr = vertices_view;
        texAddr = tex_coord_view;
        break;
    case VIEW_BACK:
        which_camera = 1;
        cnt = 4;
        verAddr = vertices_view;
        texAddr = tex_coord_view;
        break;
    case VIEW_LEFT:
        which_camera = 2;
        cnt = 4;
        verAddr = vertices_view;
        texAddr = texCoordViewLR;
        break;
    case VIEW_RIGHT:
        which_camera = 3;
        cnt = 4;
        verAddr = vertices_view;
        texAddr = texCoordViewLR;
        break;
    case VIEW_UNDISTORT_FRONT:
        which_camera = 0;
        cnt = ver_count_front;
        verAddr = (float *)ver_coord_point_front;
        texAddr = (float *)tex_coord_point_front;
        break;
    case VIEW_UNDISTORT_BACK:
        which_camera = 1;
        cnt = ver_count_back;
        verAddr = (float *)ver_coord_point_back;
        texAddr = (float *)tex_coord_point_back;
        break;
    case VIEW_DMS:
        which_camera = 4;
        cnt = 4;
        verAddr = vertices_view;
        texAddr = texCoordViewDMS;
        break;
    case VIEW_CONTAINER:
        which_camera = 4;
        cnt = 4;
        verAddr = vertices_view;
        texAddr = tex_coord_view;
        break;
    default:
        which_camera = 4;
        cnt = 4;
        verAddr = vertices_view;
        texAddr = tex_coord_view;
        break;
    }

    esMatrixLoadIdentity(&ortho_matrix);

    glViewport(g_single_view.x, g_single_view.y, g_single_view.width, g_single_view.height);

    glUseProgram(h_program_handle[4]);

    /* Enable attributes for position, color and texture coordinates etc. */
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    SetMat4(h_program_handle[4], "mvp", ortho_matrix);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, verAddr);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, texAddr);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_y[which_camera]);
    SetInt(h_program_handle[0], TEXTURE(Y), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_u[which_camera]);
    SetInt(h_program_handle[0], TEXTURE(U), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture_v[which_camera]);
    SetInt(h_program_handle[0], TEXTURE(V), 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, cnt);

    if (view_mode == VIEW_UNDISTORT_BACK)
    {
        DrawUndistortBackCurve(ortho_matrix);
    }
}

void ShowSingleViewFullScreen(int view_mode)
{
    int which_camera;
    float vertices_view[] =
        {
            -1.0f, -1.0f, 0.0f, // left-buttom
            1.0f, -1.0f, 0.0f,  // right- buttom
            -1.0f, 1.0f, 0.0f,  // right-top
            1.0f, 1.0f, 0.0f,   // left-top
        };

    float tex_coord_view[] =
        {
            0.0f, 1.0f, // left-top
            1.0f, 1.0f, // right-top
            0.0f, 0.0f, // left-buttom
            1.0f, 0.0f, // right- buttom
        };

    ESMatrix ortho_matrix;

    switch (view_mode)
    {
    case VIEW_FRONT_FULL_SCREEN:
        which_camera = 0;
        break;
    case VIEW_BACK_FULL_SCREEN:
        which_camera = 1;
        break;
    case VIEW_LEFT_FULL_SCREEN:
        which_camera = 2;
        break;
    case VIEW_RIGHT_FULL_SCREEN:
        which_camera = 3;
        break;
    default:
        which_camera = 0;
        break;
    }

    esMatrixLoadIdentity(&ortho_matrix);

    glViewport(0, 0, WIDVPBE, HEIVPBE);

    glUseProgram(h_program_handle[4]);

    /* Enable attributes for position, color and texture coordinates etc. */
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    SetMat4(h_program_handle[4], "mvp", ortho_matrix);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices_view);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, tex_coord_view);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_y[which_camera]);
    SetInt(h_program_handle[0], TEXTURE(Y), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_u[which_camera]);
    SetInt(h_program_handle[0], TEXTURE(U), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture_v[which_camera]);
    SetInt(h_program_handle[0], TEXTURE(V), 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void ShowFourView()
{
    float vertices_view_front[] =
        {
            -1.0f, 0.0f, 0.0f, // left-buttom
            0.0f, 0.0f, 0.0f,  // right- buttom
            -1.0f, 1.0f, 0.0f, // right-top
            0.0f, 1.0f, 0.0f,  // left-top
        };

    float vertices_view_back[] =
        {
            0.0f, 0.0f, 0.0f, // left-buttom
            1.0f, 0.0f, 0.0f, // right- buttom
            0.0f, 1.0f, 0.0f, // right-top
            1.0f, 1.0f, 0.0f, // left-top
        };

    float vertices_view_left[] =
        {
            -1.0f, -1.0f, 0.0f, // left-buttom
            0.0f, -1.0f, 0.0f,  // rLeftight- buttom
            -1.0f, 0.0f, 0.0f,  // right-top
            0.0f, 0.0f, 0.0f,   // left-top
        };

    float vertices_view_right[] =
        {
            0.0f, -1.0f, 0.0f, // left-buttom
            1.0f, -1.0f, 0.0f, // right- buttom
            0.0f, 0.0f, 0.0f,  // right-top
            1.0f, 0.0f, 0.0f,  // left-top
        };

    float tex_coord_view[] =
        {
            0.0f, 1.0f, // left-top
            1.0f, 1.0f, // right-top
            0.0f, 0.0f, // left-buttom
            1.0f, 0.0f, // right- buttom
        };

    ESMatrix ortho_matrix;
    esMatrixLoadIdentity(&ortho_matrix);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glViewport(0, 0, WIDVPBE, HEIVPBE);

    glUseProgram(h_program_handle[4]);

    /* Enable attributes for position, color and texture coordinates etc. */
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    SetMat4(h_program_handle[4], "mvp", ortho_matrix);

    //Front
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices_view_front);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, tex_coord_view);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_y[0]);
    SetInt(h_program_handle[4], TEXTURE(Y), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_u[0]);
    SetInt(h_program_handle[4], TEXTURE(U), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture_v[0]);
    SetInt(h_program_handle[4], TEXTURE(V), 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    //Back
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices_view_back);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, tex_coord_view);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_y[1]);
    SetInt(h_program_handle[4], TEXTURE(Y), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_u[1]);
    SetInt(h_program_handle[4], TEXTURE(U), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture_v[1]);
    SetInt(h_program_handle[4], TEXTURE(V), 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    //Left
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices_view_left);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, tex_coord_view);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_y[2]);
    SetInt(h_program_handle[4], TEXTURE(Y), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_u[2]);
    SetInt(h_program_handle[4], TEXTURE(U), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture_v[2]);
    SetInt(h_program_handle[4], TEXTURE(V), 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    //Right
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices_view_right);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, tex_coord_view);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_y[3]);
    SetInt(h_program_handle[4], TEXTURE(Y), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_u[3]);
    SetInt(h_program_handle[4], TEXTURE(U), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture_v[3]);
    SetInt(h_program_handle[4], TEXTURE(V), 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

int GenerateTriangle(vec2 *triagnle, int width, int height)
{
    float p = 64.0;
    float i;
    float j;
    float fp;
    unsigned char directionFlag;
    vec2 point;
    int count;

    fp = 1.0 / p;
    directionFlag = 0;
    count = 0;

    for (i = 0; i < 1; i += fp)
    {
        switch (directionFlag)
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
            for (j = 1; j >= 0; j -= fp)
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

        if (fabs(j - fp - 1) < FLOAT_ZERO)
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

vec2 Function(float a1, float b1, float c1, float a2, float b2, float c2)
{
    float a3;
    float b3;
    float c3;
    float a4;
    float b4;
    float c4;
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

vec2 Function2(vec2 dist, const float *a, undistortParams params, double *inv_r)
{
    float fx;
    float fy;
    float cx;
    float cy;
    vec2 imgPoints;
    vec2 point;
    float a1;
    float b1;
    float c1;
    float a2;
    float b2;
    float c2;

    fx = a[0];
    fy = a[4];
    cx = a[2];
    cy = a[5];

    a1 = inv_r[0] - dist.x * inv_r[6];
    b1 = inv_r[1] - dist.x * inv_r[7];
    c1 = dist.x * inv_r[8] - inv_r[2];
    a2 = inv_r[3] - dist.y * inv_r[6];
    b2 = inv_r[4] - dist.y * inv_r[7];
    c2 = dist.y * inv_r[8] - inv_r[5];

    point = Function(a1, b1, c1, a2, b2, c2);

    /*get pixel point*/
    imgPoints.x = (cx + point.x * fx - params.x) / params.x_zoom;
    imgPoints.y = (cy + point.y * fy - params.y) / params.y_zoom;

    return imgPoints;
}

vec2 ProjectPoints3(const vec3 obj_points, const float *r_vec,
                    const float *t_vec)
{
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

vec3 Vec3(float x, float y, float z)
{
    vec3 ret;

    ret.x = x;
    ret.y = y;
    ret.z = z;

    return ret;
}

void FindRearCurve(float wheel_angle, undistortParams params, float *camera, float *r_vec, float *t_vec, double *inv_r, int width, int height)
{
    float wheel_base = 4675 / 20.0;
    float rear_wheel = 1375 / 20.0;
    float ex_r, in_r, ex_r1, in_r1;
    float tmpx, tmpy;
    float angle;
    int i, k;
    float j;
    vec3 pw;
    vec2 len, point;
    float line_width, half_line_width, other_half_line_width;
    double ex_r1_in, ex_r1_out, in_r1_in, in_r1_out;
    double base_angle[3][4], delta_angle[3][4], radius[4];
    float fp = 1.0 / 64;
    vec3 world_points[10][LENGTH * 2];
    float startx, length;
    float offset_y;
    int idx;
    int gap[3] = {3, 4, 5};
    float distance[4] = {0, 500 / 20.0, 1500 / 20.0, 4000 / 20.0};
    float car_width = 2556 / 20.0;
    float scale;

    line_width = 3;
    half_line_width = line_width / 2;
    other_half_line_width = line_width - half_line_width;
    offset_y = 0;

    startx = -car_width / 2 + 70;
    length = car_width;

    if (wheel_angle >= 0 && wheel_angle < 0.05)
    {
        wheel_angle = 0.05;
    }
    else if (wheel_angle < 0 && wheel_angle > -0.05)
    {
        wheel_angle = -0.05;
    }

    /*convert to rad*/
    angle = (90 - wheel_angle) * RADIAN;

    ex_r = fabs((float)wheel_base * tan(angle)) + car_width / 2;
    in_r = ex_r - car_width;

    in_r1 = sqrt(in_r * in_r + rear_wheel * rear_wheel);
    ex_r1 = sqrt(ex_r * ex_r + rear_wheel * rear_wheel);

    in_r1_in = in_r1 - half_line_width;
    in_r1_out = in_r1 + other_half_line_width;
    ex_r1_in = ex_r1 - half_line_width;
    ex_r1_out = ex_r1 + other_half_line_width;

    radius[0] = in_r1_in;
    radius[1] = in_r1_out;
    radius[2] = ex_r1_in;
    radius[3] = ex_r1_out;

    scale = (1.0 - 0.2 * fabs(wheel_angle) / MAX_WHEEL_ANGLE);

    for (idx = 0; idx < 3; ++idx)
    {
        for (k = 0; k < 4; ++k)
        {
            if (k < 2)
            {
                base_angle[idx][k] = asin((rear_wheel + distance[idx] * scale) / radius[k]);

                if (radius[k] > rear_wheel + distance[idx + 1] * scale)
                {
                    delta_angle[idx][k] = asin((rear_wheel + distance[idx + 1] * scale) / radius[k]) - base_angle[idx][k];
                }
                else
                {
                    delta_angle[idx][k] = PI / 2 - base_angle[idx][k];
                }
            }
            else
            {
                base_angle[idx][k] = asin((rear_wheel + distance[idx]) / radius[k]);

                if (radius[k] > rear_wheel + distance[idx + 1])
                {
                    delta_angle[idx][k] = asin((rear_wheel + distance[idx + 1]) / radius[k]) - base_angle[idx][k];
                }
                else
                {
                    delta_angle[idx][k] = PI / 2 - base_angle[idx][k];
                }
            }
        }
    }

    if (wheel_angle < 0)
    {
        for (idx = 0; idx < 3; ++idx)
        {
            for (j = 0, k = 0; j <= 1; j += fp, ++k)
            {
                for (i = 0; i < 2; ++i)
                {
                    tmpx = car_width - ex_r + radius[i * 2 + 0] * cos(base_angle[idx][i * 2 + 0] + delta_angle[idx][i * 2 + 0] * j);
                    tmpy = radius[i * 2 + 0] * sin(base_angle[idx][i * 2 + 0] + delta_angle[idx][i * 2 + 0] * j) - rear_wheel;

                    world_points[idx * 2 + i][k * 2 + 0] = Vec3(tmpx, tmpy + offset_y, 0.0);

                    tmpx = car_width - ex_r + radius[i * 2 + 1] * cos(base_angle[idx][i * 2 + 1] + delta_angle[idx][i * 2 + 1] * j);
                    tmpy = radius[i * 2 + 1] * sin(base_angle[idx][i * 2 + 1] + delta_angle[idx][i * 2 + 1] * j) - rear_wheel;

                    world_points[idx * 2 + i][k * 2 + 1] = Vec3(tmpx, tmpy + offset_y, 0.0);
                }
            }
        }

        for (idx = 0; idx < 3; ++idx)
        {
            tmpx = car_width - ex_r + radius[1] * cos(base_angle[idx][1] + delta_angle[idx][1] * 1.0);
            tmpy = radius[1] * sin(base_angle[idx][1] + delta_angle[idx][1] * 1.0) - rear_wheel;

            world_points[6][idx * 4 + 0] = Vec3(tmpx, tmpy + offset_y, 0.0);

            tmpx = car_width - ex_r + radius[1] * cos(base_angle[idx][1] + delta_angle[idx][1] * (1.0 - gap[idx] * fp));
            tmpy = radius[1] * sin(base_angle[idx][1] + delta_angle[idx][1] * (1.0 - gap[idx] * fp)) - rear_wheel;

            world_points[6][idx * 4 + 1] = Vec3(tmpx, tmpy + offset_y, 0.0);

            tmpx = car_width - ex_r + radius[2] * cos(base_angle[idx][2] + delta_angle[idx][2] * 1.0);
            tmpy = radius[2] * sin(base_angle[idx][2] + delta_angle[idx][2] * 1.0) - rear_wheel;

            world_points[6][idx * 4 + 2] = Vec3(tmpx, tmpy + offset_y, 0.0);

            tmpx = car_width - ex_r + radius[2] * cos(base_angle[idx][2] + delta_angle[idx][2] * (1.0 - gap[idx] * fp));
            tmpy = radius[2] * sin(base_angle[idx][2] + delta_angle[idx][2] * (1.0 - gap[idx] * fp)) - rear_wheel;

            world_points[6][idx * 4 + 3] = Vec3(tmpx, tmpy + offset_y, 0.0);
        }
    }
    else
    {
        for (idx = 0; idx < 3; ++idx)
        {
            for (j = 0, k = 0; j <= 1; j += fp, ++k)
            {
                for (i = 0; i < 2; ++i)
                {
                    tmpx = car_width + in_r - radius[i * 2 + 0] * cos(base_angle[idx][i * 2 + 0] + delta_angle[idx][i * 2 + 0] * j);
                    tmpy = radius[i * 2 + 0] * sin(base_angle[idx][i * 2 + 0] + delta_angle[idx][i * 2 + 0] * j) - rear_wheel;

                    world_points[idx * 2 + i][k * 2 + 0] = Vec3(tmpx, tmpy + offset_y, 0.0);

                    tmpx = car_width + in_r - radius[i * 2 + 1] * cos(base_angle[idx][i * 2 + 1] + delta_angle[idx][i * 2 + 1] * j);
                    tmpy = radius[i * 2 + 1] * sin(base_angle[idx][i * 2 + 1] + delta_angle[idx][i * 2 + 1] * j) - rear_wheel;

                    world_points[idx * 2 + i][k * 2 + 1] = Vec3(tmpx, tmpy + offset_y, 0.0);
                }
            }
        }

        for (idx = 0; idx < 3; ++idx)
        {
            tmpx = car_width + in_r - radius[1] * cos(base_angle[idx][1] + delta_angle[idx][1] * 1.0);
            tmpy = radius[1] * sin(base_angle[idx][1] + delta_angle[idx][1] * 1.0) - rear_wheel;

            world_points[6][idx * 4 + 0] = Vec3(tmpx, tmpy + offset_y, 0.0);

            tmpx = car_width + in_r - radius[1] * cos(base_angle[idx][1] + delta_angle[idx][1] * (1.0 - gap[idx] * fp));
            tmpy = radius[1] * sin(base_angle[idx][1] + delta_angle[idx][1] * (1.0 - gap[idx] * fp)) - rear_wheel;

            world_points[6][idx * 4 + 1] = Vec3(tmpx, tmpy + offset_y, 0.0);

            tmpx = car_width + in_r - radius[2] * cos(base_angle[idx][2] + delta_angle[idx][2] * 1.0);
            tmpy = radius[2] * sin(base_angle[idx][2] + delta_angle[idx][2] * 1.0) - rear_wheel;

            world_points[6][idx * 4 + 2] = Vec3(tmpx, tmpy + offset_y, 0.0);

            tmpx = car_width + in_r - radius[2] * cos(base_angle[idx][2] + delta_angle[idx][2] * (1.0 - gap[idx] * fp));
            tmpy = radius[2] * sin(base_angle[idx][2] + delta_angle[idx][2] * (1.0 - gap[idx] * fp)) - rear_wheel;

            world_points[6][idx * 4 + 3] = Vec3(tmpx, tmpy + offset_y, 0.0);
        }
    }

    for (j = 0, k = 0; j <= 1; j += fp)
    {
        tmpx = startx + (length)*j;
        tmpy = 0 - 5;

        world_points[7][k * 2 + 0] = Vec3(tmpx, tmpy, 0.0);

        tmpx = startx + (length)*j;
        tmpy = 0 + 5;

        world_points[7][k * 2 + 1] = Vec3(tmpx, tmpy, 0.0);
        k++;
    }

    for (k = 0; k < 8; ++k)
    {
        for (i = 0; i < LENGTH * 2; ++i)
        {
            pw = world_points[k][i]; /*get world point*/

            len = ProjectPoints3(pw, r_vec, t_vec); /*get img point*/

            point = Function2(len, camera, params, inv_r); /*get pixel point*/

            /*caculate vertex point*/
            vertices_rear_traj_line_point[k][i].x = -(1.0 - 2 * point.x / width);
            vertices_rear_traj_line_point[k][i].y = -(1.0 - 2 * point.y / height);
            vertices_rear_traj_line_point[k][i].z = 0.0;
        }
    }
}

void InitUndistort(int width, int height)
{
    float alpha, beta, gamma;
    double R[9], inv_r[9];
    undistortParams resizer;
    int k, p;
    p = 64;

    resizer.x = front_resizer.x;
    resizer.y = front_resizer.y;
    resizer.x_zoom = 1.0 * front_resizer.width / width;
    resizer.y_zoom = 1.0 * front_resizer.height / height;

    img_coord_point_front = (vec2 *)malloc(sizeof(vec2) * p * (p + 1) * 2);

    ver_count_front = GenerateTriangle(img_coord_point_front, width, height);

    tex_coord_point_front = (vec2 *)malloc(sizeof(vec2) * ver_count_front);
    ver_coord_point_front = (vec3 *)malloc(sizeof(vec3) * ver_count_front);

    RotationMatrixToEulerAngle(front_cam_params.mr, &alpha, &beta, &gamma);

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

    InvertMatrix(R, inv_r);

    for (k = 0; k < ver_count_front; ++k)
    {
        ver_coord_point_front[k].x = -(1.0 - 2 * img_coord_point_front[k].x / width);
        ver_coord_point_front[k].y = (1.0 - 2 * img_coord_point_front[k].y / height);
        ver_coord_point_front[k].z = 0;

        tex_coord_point_front[k] = Undistort(img_coord_point_front[k].y, img_coord_point_front[k].x,
                                             front_cam_params.mi, front_cam_params.md, resizer, inv_r);
    }

    resizer.x = rear_resizer.x;
    resizer.y = rear_resizer.y;
    resizer.x_zoom = 1.0 * rear_resizer.width / width;
    resizer.y_zoom = 1.0 * rear_resizer.height / height;

    img_coord_point_back = (vec2 *)malloc(sizeof(vec2) * p * (p + 1) * 2);

    ver_count_back = GenerateTriangle(img_coord_point_back, width, height);

    tex_coord_point_back = (vec2 *)malloc(sizeof(vec2) * ver_count_back);
    ver_coord_point_back = (vec3 *)malloc(sizeof(vec3) * ver_count_back);

    RotationMatrixToEulerAngle(rear_cam_params.mr, &alpha, &beta, &gamma);

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

    InvertMatrix(R, inv_r);

    for (k = 0; k < ver_count_back; ++k)
    {
        ver_coord_point_back[k].x = -(1.0 - 2 * img_coord_point_back[k].x / width);
        ver_coord_point_back[k].y = -(1.0 - 2 * img_coord_point_back[k].y / height);
        ver_coord_point_back[k].z = 0;

        tex_coord_point_back[k] = Undistort(img_coord_point_back[k].y, img_coord_point_back[k].x,
                                            rear_cam_params.mi, rear_cam_params.md, resizer, inv_r);
    }

    FindRearCurve(0.0, resizer, rear_cam_params.mi, rear_cam_params.mr, rear_cam_params.mt, inv_r, width, height);
}

int AvmInit(safImgRect allView, safImgRect singleView)
{
    g_all_view = allView;

    g_single_view = singleView;

    InitShader();

    InitCamParaData();

    Init2DModel();

    InitTextureCoords();

    GetLumiaCountPosition();

    LoadRes(IMAGE_WIDTH, IMAGE_HEIGHT);

    InitVBO();

    InitUndistort(g_single_view.width, g_single_view.height);

    return 0;
}

extern void UpdateTexture(unsigned char **src)
{
    int i;
    unsigned char *p_src_y, *p_src_u, *p_src_v;
    for (i = 0; i < 5; ++i)
    {
        if (NULL == src[i])
        {
            /*check image address*/
            continue;
        }

        p_src_y = src[i];
        p_src_u = p_src_y + IMAGE_WIDTH * IMAGE_HEIGHT;
        p_src_v = p_src_u + IMAGE_WIDTH * IMAGE_HEIGHT / 4;

        glBindTexture(GL_TEXTURE_2D, texture_y[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, IMAGE_WIDTH, IMAGE_HEIGHT, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, p_src_y);

        glBindTexture(GL_TEXTURE_2D, texture_u[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, p_src_u);

        glBindTexture(GL_TEXTURE_2D, texture_v[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, p_src_v);

        pu8_cam_vir[i] = p_src_y;
    }

    CaculateColorCoeff2D(src);
}
// int idx = 0;
// int count = 0;
void RunRender(int view_mode, float steeringWheelAngle)
{
    // Clear the colorbuffer and depth-buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    if (view_mode < VIEW_LEFT)
    {
        if (view_mode == VIEW_OVERALL)
        {
            ShowFourView();
        }
        else
        {
            ShowSingleViewFullScreen(view_mode);
        }
    }
    else
    {
        Show2DCar();
        ShowSingleView(view_mode, steeringWheelAngle);
    }
    // if (count % 100 == 0)
    // {
    //     printf("count=%d\n", count);
    //     idx++;
    //     UpdateCarImage(idx % 2);
    // }
    // count++;
}
