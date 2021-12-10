
#include "cv.h"
#include "cxcore.h"
#include "opengl_common.h"
#include "objloader.hpp"

/*$)Ag<)f?g3;f?*/
#define SCALE1          100000                
#define SCALE2          100                   
#define SCALE3          10000                 

#define IMGWIDTH 1280
#define IMGHEIGHT 720

#define RULE_LEN		4
#define LENGTH 65
#define RADARLEN 66
#define RADARRADIUS 120

#define PI 3.141592653589793238462643383279
#define RADIAN (PI/180.0)

#define MAX_WHEEL_ANGLE (900 / 23.6)

#define AVERAGE_COUNT				10	

#define SHADOW_X_OFFSET 0.0//0.28//0.2//0.1
#define SHADOW_Y_OFFSET 0.05//0.19//0.15//0.05

#define CAR_PATH "../test/MAGOTAN/"  //$)Ah=?(!h7??


typedef struct PARA_FIELD
{
	int car_width;
	int car_length;
	int LRchess2carFront_distance;
    int chessboard_width_corners;
    int chessboard_length_corners;
    int square_size;
    int carWorldX;
    int carWorldX2;
    int carWorldY;
    int carWorldY2;
} PARA_FIELD ;

typedef struct _textureCoords
{
    std::vector<CvPoint2D32f> glTexCoord_F;
    std::vector<CvPoint2D32f> glTexCoord_B;
    std::vector<CvPoint2D32f> glTexCoord_L;
    std::vector<CvPoint2D32f> glTexCoord_R;
    std::vector<CvPoint2D32f> glTexCoord_FL_F;
    std::vector<CvPoint2D32f> glTexCoord_FL_L;
    std::vector<CvPoint2D32f> glTexCoord_FR_F;
    std::vector<CvPoint2D32f> glTexCoord_FR_R;
    std::vector<CvPoint2D32f> glTexCoord_BL_B;
    std::vector<CvPoint2D32f> glTexCoord_BL_L;
    std::vector<CvPoint2D32f> glTexCoord_BR_B;
    std::vector<CvPoint2D32f> glTexCoord_BR_R;
} TexCoords;

typedef struct _objectPoints
{
    std::vector<CvPoint3D32f> glObjPoints_F;
    std::vector<CvPoint3D32f> glObjPoints_B;
    std::vector<CvPoint3D32f> glObjPoints_L;
    std::vector<CvPoint3D32f> glObjPoints_R;
    std::vector<CvPoint3D32f> glObjPoints_FL_F;
    std::vector<CvPoint3D32f> glObjPoints_FL_L;
    std::vector<CvPoint3D32f> glObjPoints_FR_F;
    std::vector<CvPoint3D32f> glObjPoints_FR_R;
    std::vector<CvPoint3D32f> glObjPoints_BL_B;
    std::vector<CvPoint3D32f> glObjPoints_BL_L;
    std::vector<CvPoint3D32f> glObjPoints_BR_B;
    std::vector<CvPoint3D32f> glObjPoints_BR_R;
} ObjPoints;

typedef struct _vertexCoords
{
    std::vector<CvPoint3D32f> glVertex_F;
    std::vector<CvPoint3D32f> glVertex_B;
    std::vector<CvPoint3D32f> glVertex_L;
    std::vector<CvPoint3D32f> glVertex_R;
    std::vector<CvPoint3D32f> glVertex_FL;
    std::vector<CvPoint3D32f> glVertex_FR;
    std::vector<CvPoint3D32f> glVertex_BL;
    std::vector<CvPoint3D32f> glVertex_BR;
} VertexCoords;

typedef struct _blendAlpha
{
    std::vector<float> glAlpha_FL;
    std::vector<float> glAlpha_FR;
    std::vector<float> glAlpha_BL;
    std::vector<float> glAlpha_BR;
} BlendAlpha;

typedef struct _lumiaAdjust
{
    std::vector<float> glLumiaAdjust_F;
    std::vector<float> glLumiaAdjust_B;
    std::vector<float> glLumiaAdjust_L;
    std::vector<float> glLumiaAdjust_R;
} LumiaAdjust;

typedef struct _objectPointsStatistics
{
    std::vector<CvPoint3D32f> glObjPoints_FL_F;
    std::vector<CvPoint3D32f> glObjPoints_FL_L;
    std::vector<CvPoint3D32f> glObjPoints_FR_F;
    std::vector<CvPoint3D32f> glObjPoints_FR_R;
    std::vector<CvPoint3D32f> glObjPoints_BL_B;
    std::vector<CvPoint3D32f> glObjPoints_BL_L;
    std::vector<CvPoint3D32f> glObjPoints_BR_B;
    std::vector<CvPoint3D32f> glObjPoints_BR_R;
} ObjPointsStatistics;


typedef struct _textureCoordsStatistics
{
    std::vector<int> glTexCoord_FL_F;
    std::vector<int> glTexCoord_FL_L;
    std::vector<int> glTexCoord_FR_F;
    std::vector<int> glTexCoord_FR_R;
    std::vector<int> glTexCoord_BL_B;
    std::vector<int> glTexCoord_BL_L;
    std::vector<int> glTexCoord_BR_B;
    std::vector<int> glTexCoord_BR_R;
} TexCoordsStatistics;


typedef struct _VBO3DMosaicImage
{
    GLuint CamVerticesPoints[4];
    GLuint CamImagePoints[4];
    GLuint MosaicCamVerticesPoints[4];
    GLuint MosaicFLCamImagePoints[2];
    GLuint MosaicFRCamImagePoints[2];
    GLuint MosaicBLCamImagePoints[2];
    GLuint MosaicBRCamImagePoints[2];
    GLuint LumiaBalance[4];
    GLuint Alpha[4];
    GLuint CarVerTexCoord[2];
} VBO3DMosaicImage;


typedef struct _VBO3DCarModel
{
    GLuint vertices[100];
    GLuint textures[100];
    GLuint normals[100];
	GLuint tangents[100];
    GLuint bitTangents[100];
} VBO3DCarModel;

typedef struct _cam_params
{
    CvMat *camera;
    CvMat *dist_coeffs;
    CvMat *r_vec;
    CvMat *t_vec;
    int mrInt[3];
    int mtInt[3];
    int mimdInt[8];
} SimplifyCamParams;









typedef struct undistortParams
{
    float x;
    float y;
    float xZoom;
    float yZoom;
}
undistortParams;

unsigned int esLoadProgram ( const char *vertShaderSrc, const char *fragShaderSrc );

//extern void init3DCar(void);
extern void init3DModel(void);
extern void init2DModel(void);
extern void initTextureCoords(void);
extern void initVBO();
extern void initCamParaData(int flag);
extern void getCamPixelPosition();
extern void rotateView(float angle);
extern void findRearCurve(float wheelAngle, undistortParams params, float *camera, float *distortTable, float *rVec, float *tVec, double *invR, int width, int height);
extern void findRearCurve4(float wheelAngle, undistortParams params, float *camera, float *distortTable, float *rVec, float *tVec, double *invR, int width, int height);

extern void getInvertMatrix(double *src, double *dst);

extern PARA_FIELD para_field;

extern VertexCoords vertexCoords;
extern TexCoords texCoords;

extern VertexCoords vertexCoords2D;
extern TexCoords texCoords2D;


extern std::vector<MATERIAL> carMaterialList;
extern std::vector<OBJECTNAME> carObjectList;
extern std::vector<GLuint> carProgramIDList;
extern std::vector<GLuint> carFragmentShaderIDList;
extern std::vector<GLuint> carVertexShaderIDList;
extern std::vector<GLuint> carVertexPositionIDList;
extern std::vector<GLuint> carNormalPositionIDList;
extern std::vector<GLuint> carTangentPositionIDList;  //$)A6%5cJ}>]
extern std::vector<GLuint> carBitTangentPositionIDList; //$)ANF@mJ}>]
extern std::vector<GLuint> carTexturePositionIDList;
extern std::vector<GLuint> carMatrixModelIDList;
extern std::vector<GLuint> carMatrixMVPIDList;
extern std::vector<GLuint> carMatrixNormalIDList;
extern std::vector<GLuint> carFragLightPosIDList;
extern std::vector<GLuint> carFragViewPosIDList;
extern std::vector<GLuint> carFragKdColorIDList;
extern std::vector<GLuint> carFragAlphaIDList;
extern std::vector<GLuint> carTextureIDList;
extern std::vector<GLuint> carLocTextureIDList;
extern std::vector<GLuint> carLocCubeTextureIDList;
extern std::vector<GLuint> carFragLightStrengthIDList;

extern std::vector< std::vector< vec3 > > carVertices;
extern std::vector< unsigned int > carMaterialIdx;

extern VBO3DMosaicImage VBO3DMosaicImageParams;
extern VBO3DMosaicImage VBO2DMosaicImageParams;
extern VBO3DCarModel VBO3DCarModelParams;

extern GLuint curveVerticesPoints[2];
extern GLuint cameraVerTexCoord[2];

extern float carScaleX, carScaleY, carScaleZ;

extern TexCoordsStatistics texCoordsStatistics;
extern TexCoordsStatistics texCoordsStatistics2D;

extern SimplifyCamParams SimplifyrearCamParams;

extern CvPoint3D32f verticesRearTrajLinePoint[11][LENGTH * 2];

extern int initMosaic(safImgRect allView, safImgRect singleView, int flag);
