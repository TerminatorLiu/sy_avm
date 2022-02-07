
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>

#include "opengl_common.h"

#define SCALE1 100000
#define SCALE2 100
#define SCALE3 10000

#define IMAGE_WIDTH 1280
#define IMAGE_HEIGHT 720

#define LENGTH 65

#define FLOAT_ZERO 0.000001
#define PI 3.141592653589793238462643383279
#define RADIAN (PI / 180.0)

#define MAX_WHEEL_ANGLE (900 / 23.6)

#define AVERAGE_COUNT 10

#define SHADOW_X_OFFSET 0.0  
#define SHADOW_Y_OFFSET 0.0 

#define WIDVPBE 1280
#define HEIVPBE 720

#define COEFFS_NUM      4
#define COEFFS_LEN      6
#define CAR_FILE_PATH_LEN      128

typedef struct _vec2 {
  float x;
  float y;
} vec2;

typedef struct _vec3 {
  float x;
  float y;
  float z;
} vec3;

typedef struct _vec4 {
  float r;
  float g;
  float b;
  float a;
} vec4;

typedef struct _Parking_Assistant_Params {
  int car_width;
  int car_length;
  int LRchess2carFront_distance;
  int chessboard_width_corners;
  int chessboard_length_corners;
  int square_size;
  int car_world_x;
  int car_world_x2;
  int car_world_y;
  int car_world_y2;
  unsigned char car_name[CAR_FILE_PATH_LEN];
  float coeff[COEFFS_NUM][COEFFS_LEN];
} Parking_Assistant_Params;

typedef struct _textureCoords {
  std::vector<vec2> gl_tex_coord_f;
  std::vector<vec2> gl_tex_coord_b;
  std::vector<vec2> gl_tex_coord_l;
  std::vector<vec2> gl_tex_coord_r;
  std::vector<vec2> gl_tex_coord_fl_f;
  std::vector<vec2> gl_tex_coord_fl_l;
  std::vector<vec2> gl_tex_coord_fr_f;
  std::vector<vec2> gl_tex_coord_fr_r;
  std::vector<vec2> gl_tex_coord_bl_b;
  std::vector<vec2> gl_tex_coord_bl_l;
  std::vector<vec2> gl_tex_coord__br_b;
  std::vector<vec2> gl_tex_coord__br_r;
} TexCoords;

typedef struct _objectPoints {
  std::vector<vec3> gl_obj_points_f;
  std::vector<vec3> gl_obj_points_b;
  std::vector<vec3> gl_obj_points_l;
  std::vector<vec3> gl_obj_points_r;
  std::vector<vec3> gl_obj_points_fl_f;
  std::vector<vec3> glObjPoints_FL_L;
  std::vector<vec3> glObjPoints_FR_F;
  std::vector<vec3> glObjPoints_FR_R;
  std::vector<vec3> glObjPoints_BL_B;
  std::vector<vec3> glObjPoints_BL_L;
  std::vector<vec3> glObjPoints_BR_B;
  std::vector<vec3> glObjPoints_BR_R;
} ObjPoints;

typedef struct _vertexCoords {
  std::vector<vec3> glVertex_F;
  std::vector<vec3> glVertex_B;
  std::vector<vec3> glVertex_L;
  std::vector<vec3> glVertex_R;
  std::vector<vec3> glVertex_FL;
  std::vector<vec3> glVertex_FR;
  std::vector<vec3> glVertex_BL;
  std::vector<vec3> glVertex_BR;
} VertexCoords;

typedef struct _blendAlpha {
  std::vector<float> glAlpha_FL;
  std::vector<float> glAlpha_FR;
  std::vector<float> glAlpha_BL;
  std::vector<float> glAlpha_BR;
} BlendAlpha;

typedef struct _lumiaAdjust {
  std::vector<float> glLumiaAdjust_F;
  std::vector<float> glLumiaAdjust_B;
  std::vector<float> glLumiaAdjust_L;
  std::vector<float> glLumiaAdjust_R;
} LumiaAdjust;

typedef struct _objectPointsStatistics {
  std::vector<vec3> gl_obj_points_fl_f;
  std::vector<vec3> glObjPoints_FL_L;
  std::vector<vec3> glObjPoints_FR_F;
  std::vector<vec3> glObjPoints_FR_R;
  std::vector<vec3> glObjPoints_BL_B;
  std::vector<vec3> glObjPoints_BL_L;
  std::vector<vec3> glObjPoints_BR_B;
  std::vector<vec3> glObjPoints_BR_R;
} ObjPointsStatistics;

typedef struct _textureCoordsStatistics {
  std::vector<int> gl_tex_coord_fl_f;
  std::vector<int> gl_tex_coord_fl_l;
  std::vector<int> gl_tex_coord_fr_f;
  std::vector<int> gl_tex_coord_fr_r;
  std::vector<int> gl_tex_coord_bl_b;
  std::vector<int> gl_tex_coord_bl_l;
  std::vector<int> gl_tex_coord__br_b;
  std::vector<int> gl_tex_coord__br_r;
} TexCoordsStatistics;

typedef struct _VBOMosaicImage {
  GLuint cam_vertices_points[4];
  GLuint cam_texture_points[4];
  GLuint fuse_cam_vertices_points[4];
  GLuint fuse_fl_cam_texture_points[2];
  GLuint fuse_fr_cam_texture_points[2];
  GLuint fuse_bl_cam_texture_points[2];
  GLuint fuse_br_cam_texture_points[2];
  GLuint LumiaBalance[4];
  GLuint alpha[4];
  GLuint car_ver_tex_coord[2];
} VBOMosaicImage;

typedef struct _cam_params {
  int mr_int[3];
  int mt_int[3];
  int mimd_int[8];
  float mr[9];
  float mt[3];
  float mi[9];
  float md[4];
} camParams;

typedef struct undistortParams {
  float x;
  float y;
  float x_zoom;
  float y_zoom;
} undistortParams;

extern void ReadParamsXML();
extern void WriteParamsXML();

extern void Init2DModel();
extern void InitTextureCoords();
extern void InitVBO();
extern void InitCamParaData();
extern void GetLumiaCountPosition();

extern Parking_Assistant_Params parking_assistant_params;

extern VertexCoords vertex_coords;

extern VBOMosaicImage vbo_mosaic_image;

extern TexCoordsStatistics tex_coords_statistics;

extern camParams front_cam_params; 
extern camParams rear_cam_params;
extern camParams left_cam_params;
extern camParams right_cam_params;


extern safImgRect front_resizer;
extern safImgRect rear_resizer;

extern unsigned char* pu8_cam_vir[4];
extern int cameraCalib(int carLength, int carWidth, int chess2carFront);
