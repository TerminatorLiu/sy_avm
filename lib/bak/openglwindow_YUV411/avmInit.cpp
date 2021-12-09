/******************************************************************************
 * Copyright 2017 The SANY Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/
#include "avmInit.hpp"

#include <vector>

TexCoords texCoords2D;
ObjPoints objPoints2D;
VertexCoords vertexCoords2D;
BlendAlpha blendAlpha2D;
LumiaAdjust lumiaAdjust2D;

ObjPointsStatistics objPointsStatistics2D;
TexCoordsStatistics texCoordsStatistics2D;

VBOMosaicImage VBO2DMosaicImageParams;

camParams frontCamParams;
camParams rearCamParams;
camParams leftCamParams;
camParams rightCamParams;

PARA_FIELD para_field;


GLfloat glVertices2DCar[12] = {
    -1.0f, -1.0f, 0.0f,  // left-buttom
    1.0f,  -1.0f, 0.0f,  // right- buttom
    -1.0f, 1.0f,  0.0f,  // right-top
    1.0f,  1.0f,  0.0f,  // left-top
};

GLfloat glTexCoordCar[] = {
    0.0f, 1.0f,  // left-top
    1.0f, 1.0f,  // right-top
    0.0f, 0.0f,  // left-buttom
    1.0f, 0.0f,  // right- buttom
};

vec3 Vec3(float x, float y, float z) {
  vec3 ret;

  ret.x = x;
  ret.y = y;
  ret.z = z;

  return ret;
}

static float getDistance(float x1, float y1, float x2, float y2, float x,
                         float y) {
  float temp1, temp2;
  temp1 = (y2 - y1) * x + (x1 - x2) * y + (y1 - y2) * x1 - (x1 - x2) * y1;
  temp2 =
      static_cast<float>(sqrt((y2 - y1) * (y2 - y1) + (x1 - x2) * (x1 - x2)));
  return static_cast<float>(temp1 / temp2);
}

static float getPixelDistance(int x1, int y1, int x2, int y2) {
  float temp1;
  temp1 =
      static_cast<float>(sqrt((y2 - y1) * (y2 - y1) + (x1 - x2) * (x1 - x2)));
  return temp1;
}
// flag: mode  status, directionFlag: direction, worldWidth: world data, fp:
// increase step
void init2DModelF(int worldWidth,
                  int worldHeight, float fp) {
  float pxv, pyv, pzv;
  float pxw, pyw, pzw;
  float pxt, pyt;
  vec3 pt3d_w;
  vec3 pt3d_v;

  int halfWorldWidth, halfWorldHeight;
  float i, j;
  float adjustCoeff;
  unsigned char directionFlag;

  halfWorldWidth = worldWidth / 2;
  halfWorldHeight = worldHeight / 2;

  directionFlag = 0;

  for (i = 0; i < 1; i += fp) {
    switch (directionFlag) {
      case 0:
        for (j = 0; j <= 1; j += fp) {
          pxt = j * para_field.car_width + para_field.carWorldX;
          pyt = i * para_field.carWorldY;

          pxw = pxt - worldWidth / 2 +
                para_field.chessboard_length_corners *
                    para_field.square_size / 2;
          pyw = para_field.chessboard_width_corners *
                    para_field.square_size +
                (pyt - para_field.carWorldY);
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - pyt) / halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_F.push_back(pt3d_w);
          vertexCoords2D.glVertex_F.push_back(pt3d_v);

          adjustCoeff = 1.0 * (pxt - para_field.carWorldX) /
                        (para_field.car_width);
          lumiaAdjust2D.glLumiaAdjust_F.push_back(adjustCoeff);

          pxt = j * para_field.car_width + para_field.carWorldX;
          pyt = (i + fp) * para_field.carWorldY;

          pxw = pxt - worldWidth / 2 +
                para_field.chessboard_length_corners *
                    para_field.square_size / 2;
          pyw = para_field.chessboard_width_corners *
                    para_field.square_size +
                (pyt - para_field.carWorldY);
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - pyt) / halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_F.push_back(pt3d_w);
          vertexCoords2D.glVertex_F.push_back(pt3d_v);

          adjustCoeff = 1.0 * (pxt - para_field.carWorldX) /
                        (para_field.car_width);
          lumiaAdjust2D.glLumiaAdjust_F.push_back(adjustCoeff);
        }

        break;
      case 1:
        for (j = 1; j >= 0; j -= fp) {
          pxt = j * para_field.car_width + para_field.carWorldX;
          pyt = i * para_field.carWorldY;

          pxw = pxt - worldWidth / 2 +
                para_field.chessboard_length_corners *
                    para_field.square_size / 2;
          pyw = para_field.chessboard_width_corners *
                    para_field.square_size +
                (pyt - para_field.carWorldY);
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - pyt) / halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_F.push_back(pt3d_w);
          vertexCoords2D.glVertex_F.push_back(pt3d_v);

          adjustCoeff = 1.0 * (pxt - para_field.carWorldX) /
                        (para_field.car_width);
          lumiaAdjust2D.glLumiaAdjust_F.push_back(adjustCoeff);

          pxt = j * para_field.car_width + para_field.carWorldX;
          pyt = (i + fp) * para_field.carWorldY;

          pxw = pxt - worldWidth / 2 +
                para_field.chessboard_length_corners *
                    para_field.square_size / 2;
          pyw = para_field.chessboard_width_corners *
                    para_field.square_size +
                (pyt - para_field.carWorldY);
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - pyt) / halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_F.push_back(pt3d_w);
          vertexCoords2D.glVertex_F.push_back(pt3d_v);

          adjustCoeff = 1.0 * (pxt - para_field.carWorldX) /
                        (para_field.car_width);
          lumiaAdjust2D.glLumiaAdjust_F.push_back(adjustCoeff);
        }
 
        break;
    }
    // change direction!
    if ( fabs(j - fp - 1) < FLOAT_ZERO ) {
      directionFlag = 1;
    } else {
      directionFlag = 0;
    }
  }
  printf("VTX_NUM_F: %lu\n", objPoints2D.glObjPoints_F.size());
  texCoords2D.glTexCoord_F.resize(objPoints2D.glObjPoints_F.size());
}

void init2DModelB(int worldWidth,
                  int worldHeight, float fp) {
  float pxv, pyv, pzv;
  float pxw, pyw, pzw;
  float pxt, pyt;
  vec3 pt3d_w;
  vec3 pt3d_v;

  int halfWorldWidth, halfWorldHeight;
  float i, j;
  float adjustCoeff;
  unsigned char directionFlag = 0;

  halfWorldWidth = worldWidth / 2;
  halfWorldHeight = worldHeight / 2;
  for (i = 0; i < 1; i += fp) {
    switch (directionFlag) {
      case 0:
        for (j = 0; j <= 1; j += fp) {
          pxt = j * para_field.car_width + para_field.carWorldX;
          pyt = i * para_field.carWorldY2;

          pxw = pxt - halfWorldWidth +
                para_field.chessboard_length_corners *
                    para_field.square_size / 2;
          pyw = pyt;
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - (pyt + para_field.carWorldY +
                                    para_field.car_length)) /
                halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_B.push_back(pt3d_w);
          vertexCoords2D.glVertex_B.push_back(pt3d_v);

          adjustCoeff = 1.0 * (pxt - para_field.carWorldX) /
                        (para_field.car_width);
          lumiaAdjust2D.glLumiaAdjust_B.push_back(adjustCoeff);

          pxt = j * para_field.car_width + para_field.carWorldX;
          pyt = (i + fp) * para_field.carWorldY2;

          pxw = pxt - halfWorldWidth +
                para_field.chessboard_length_corners *
                    para_field.square_size / 2;
          pyw = pyt;
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - (pyt + para_field.carWorldY +
                                    para_field.car_length)) /
                halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_B.push_back(pt3d_w);
          vertexCoords2D.glVertex_B.push_back(pt3d_v);

          adjustCoeff = 1.0 * (pxt - para_field.carWorldX) /
                        (para_field.car_width);
          lumiaAdjust2D.glLumiaAdjust_B.push_back(adjustCoeff);
        }

        break;
      case 1:
        for (j = 1; j >= 0; j -= fp) {
          pxt = j * para_field.car_width + para_field.carWorldX;
          pyt = i * para_field.carWorldY2;

          pxw = pxt - halfWorldWidth +
                para_field.chessboard_length_corners *
                    para_field.square_size / 2;
          pyw = pyt;
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - (pyt + para_field.carWorldY +
                                    para_field.car_length)) /
                halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_B.push_back(pt3d_w);
          vertexCoords2D.glVertex_B.push_back(pt3d_v);

          adjustCoeff = 1.0 * (pxt - para_field.carWorldX) /
                        (para_field.car_width);
          lumiaAdjust2D.glLumiaAdjust_B.push_back(adjustCoeff);

          pxt = j * para_field.car_width + para_field.carWorldX;
          pyt = (i + fp) * para_field.carWorldY2;

          pxw = pxt - halfWorldWidth +
                para_field.chessboard_length_corners *
                    para_field.square_size / 2;
          pyw = pyt;
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - (pyt + para_field.carWorldY +
                                    para_field.car_length)) /
                halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_B.push_back(pt3d_w);
          vertexCoords2D.glVertex_B.push_back(pt3d_v);

          adjustCoeff = 1.0 * (pxt - para_field.carWorldX) /
                        (para_field.car_width);
          lumiaAdjust2D.glLumiaAdjust_B.push_back(adjustCoeff);
        }

        break;
    }
    // change direction!
    if ( fabs(j - fp - 1) < FLOAT_ZERO ) {
      directionFlag = 1;
    } else {
      directionFlag = 0;
    }
  }
  printf("VTX_NUM_B: %lu\n", objPoints2D.glObjPoints_B.size());
  texCoords2D.glTexCoord_B.resize(objPoints2D.glObjPoints_B.size());
}

void init2DModelL(int worldWidth,
                  int worldHeight, float fp) {
  float pxv, pyv, pzv;
  float pxw, pyw, pzw;
  float pxt, pyt;
  vec3 pt3d_w;
  vec3 pt3d_v;

  int halfWorldWidth, halfWorldHeight;
  float i, j;
  float adjustCoeff;
  unsigned char directionFlag = 0;

  halfWorldWidth = worldWidth / 2;
  halfWorldHeight = worldHeight / 2;
  for (i = 0; i < 1; i += fp) {
    switch (directionFlag) {
      case 0:
        for (j = 0; j <= 1; j += fp) {
          pxt = j * para_field.carWorldX;
          pyt = i * para_field.car_length;

          pxw = pxt - para_field.carWorldX +
                para_field.chessboard_width_corners *
                    para_field.square_size;
          pyw = pyt - para_field.LRchess2carFront_distance;
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - (pyt + para_field.carWorldY)) /
                halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_L.push_back(pt3d_w);
          vertexCoords2D.glVertex_L.push_back(pt3d_v);

          adjustCoeff = 1.0 * pyt / para_field.car_length;
          lumiaAdjust2D.glLumiaAdjust_L.push_back(adjustCoeff);

          pxt = j * para_field.carWorldX;
          pyt = (i + fp) * para_field.car_length;

          pxw = pxt - para_field.carWorldX +
                para_field.chessboard_width_corners *
                    para_field.square_size;
          pyw = pyt - para_field.LRchess2carFront_distance;
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - (pyt + para_field.carWorldY)) /
                halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_L.push_back(pt3d_w);
          vertexCoords2D.glVertex_L.push_back(pt3d_v);

          adjustCoeff = 1.0 * pyt / para_field.car_length;

          lumiaAdjust2D.glLumiaAdjust_L.push_back(adjustCoeff);
        }
   
        break;
      case 1:
        for (j = 1; j >= 0; j -= fp) {
          pxt = j * para_field.carWorldX;
          pyt = i * para_field.car_length;

          pxw = pxt - para_field.carWorldX +
                para_field.chessboard_width_corners *
                    para_field.square_size;
          pyw = pyt - para_field.LRchess2carFront_distance;
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - (pyt + para_field.carWorldY)) /
                halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_L.push_back(pt3d_w);
          vertexCoords2D.glVertex_L.push_back(pt3d_v);

          adjustCoeff = 1.0 * pyt / para_field.car_length;

          lumiaAdjust2D.glLumiaAdjust_L.push_back(adjustCoeff);

          pxt = j * para_field.carWorldX;
          pyt = (i + fp) * para_field.car_length;

          pxw = pxt - para_field.carWorldX +
                para_field.chessboard_width_corners *
                    para_field.square_size;
          pyw = pyt - para_field.LRchess2carFront_distance;
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - (pyt + para_field.carWorldY)) /
                halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_L.push_back(pt3d_w);
          vertexCoords2D.glVertex_L.push_back(pt3d_v);

          adjustCoeff = 1.0 * pyt / para_field.car_length;

          lumiaAdjust2D.glLumiaAdjust_L.push_back(adjustCoeff);
        }
 
        break;
    }
    // change direction!
    if ( fabs(j - fp - 1) < FLOAT_ZERO ) {
      directionFlag = 1;
    } else {
      directionFlag = 0;
    }
  }
  printf("VTX_NUM_L: %lu\n", objPoints2D.glObjPoints_L.size());
  texCoords2D.glTexCoord_L.resize(objPoints2D.glObjPoints_L.size());
}

void init2DModelR(int worldWidth,
                  int worldHeight, float fp) {
  float pxv, pyv, pzv;
  float pxw, pyw, pzw;
  float pxt, pyt;
  vec3 pt3d_w;
  vec3 pt3d_v;

  int halfWorldWidth, halfWorldHeight;
  float i, j;
  float adjustCoeff;
  unsigned char directionFlag = 0;

  halfWorldWidth = worldWidth / 2;
  halfWorldHeight = worldHeight / 2;
  for (i = 0; i < 1; i += fp) {
    switch (directionFlag) {
      case 0:
        for (j = 0; j <= 1; j += fp) {
          pxt = j * para_field.carWorldX2;
          pyt = i * para_field.car_length;

          pxw = pxt;
          pyw = pyt - para_field.LRchess2carFront_distance;
          pzw = 0;

          pxv = (para_field.carWorldX + para_field.car_width + pxt -
                 halfWorldWidth) /
                halfWorldWidth;
          pyv = (halfWorldHeight - (pyt + para_field.carWorldY)) /
                halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_R.push_back(pt3d_w);
          vertexCoords2D.glVertex_R.push_back(pt3d_v);

          adjustCoeff = 1.0 * pyt / para_field.car_length;

          lumiaAdjust2D.glLumiaAdjust_R.push_back(adjustCoeff);

          pxt = j * para_field.carWorldX2;
          pyt = (i + fp) * para_field.car_length;

          pxw = pxt;
          pyw = pyt - para_field.LRchess2carFront_distance;
          pzw = 0;

          pxv = (para_field.carWorldX + para_field.car_width + pxt -
                 halfWorldWidth) /
                halfWorldWidth;
          pyv = (halfWorldHeight - (pyt + para_field.carWorldY)) /
                halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_R.push_back(pt3d_w);
          vertexCoords2D.glVertex_R.push_back(pt3d_v);

          adjustCoeff = 1.0 * pyt / para_field.car_length;

          lumiaAdjust2D.glLumiaAdjust_R.push_back(adjustCoeff);
        }

        break;
      case 1:
        for (j = 1; j >= 0; j -= fp) {
          pxt = j * para_field.carWorldX2;
          pyt = i * para_field.car_length;

          pxw = pxt;
          pyw = pyt - para_field.LRchess2carFront_distance;
          pzw = 0;

          pxv = (para_field.carWorldX + para_field.car_width + pxt -
                 halfWorldWidth) /
                halfWorldWidth;
          pyv = (halfWorldHeight - (pyt + para_field.carWorldY)) /
                halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_R.push_back(pt3d_w);
          vertexCoords2D.glVertex_R.push_back(pt3d_v);

          adjustCoeff = 1.0 * pyt / para_field.car_length;

          lumiaAdjust2D.glLumiaAdjust_R.push_back(adjustCoeff);

          pxt = j * para_field.carWorldX2;
          pyt = (i + fp) * para_field.car_length;

          pxw = pxt;
          pyw = pyt - para_field.LRchess2carFront_distance;
          pzw = 0;

          pxv = (para_field.carWorldX + para_field.car_width + pxt -
                 halfWorldWidth) /
                halfWorldWidth;
          pyv = (halfWorldHeight - (pyt + para_field.carWorldY)) /
                halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_R.push_back(pt3d_w);
          vertexCoords2D.glVertex_R.push_back(pt3d_v);

          adjustCoeff = 1.0 * pyt / para_field.car_length;

          lumiaAdjust2D.glLumiaAdjust_R.push_back(adjustCoeff);
        }
 
        break;
    }
    // change direction!
    if ( fabs(j - fp - 1) < FLOAT_ZERO ) {
      directionFlag = 1;
    } else {
      directionFlag = 0;
    }
  }
  printf("VTX_NUM_R: %lu\n", objPoints2D.glObjPoints_R.size());
  texCoords2D.glTexCoord_R.resize(objPoints2D.glObjPoints_R.size());
}
void init2DModelFL(int worldWidth,
                   int worldHeight, float fp) {
  float pxv, pyv, pzv;
  float pxw, pyw, pzw;
  float pxt, pyt;
  vec3 pt3d_w, pt3d_w0, pt3d_w1;
  vec3 pt3d_v;
  float i, j;

  int halfWorldWidth, halfWorldHeight;
  float dist1, dist2;
  float theta, theta1;
  float frontXfuse, frontYfuse;
  vec2 f1, f2, f3;
  unsigned char directionFlag = 0;

  halfWorldWidth = worldWidth / 2;
  halfWorldHeight = worldHeight / 2;

  frontXfuse = para_field.carWorldX * 0.5;
  frontYfuse = para_field.carWorldY * 0.5;

  f1.x = para_field.carWorldX;
  f1.y = para_field.carWorldY;

  f2.x = 0;
  f2.y = para_field.carWorldY - frontYfuse;

  f3.x = para_field.carWorldX - frontXfuse;
  f3.y = 0;

  theta = atan(static_cast<float>(f1.y - f3.y) / (f1.x - f3.x)) -
          atan(static_cast<float>(f1.y - f2.y) / (f1.x - f2.x));

  for (i = 0; i <= 1; i += fp) {
    switch (directionFlag) {
      case 0:
        for (j = 0; j <= 1; j += fp) {
          pxt = j * para_field.carWorldX;
          pyt = i * para_field.carWorldY;

          pxw = pxt - worldWidth / 2 +
                para_field.chessboard_length_corners *
                    para_field.square_size / 2;
          pyw = para_field.chessboard_width_corners *
                    para_field.square_size +
                (pyt - para_field.carWorldY);
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - pyt) / halfWorldHeight;
          pzv = 0;

          pt3d_w0 = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_FL_F.push_back(pt3d_w0);
          vertexCoords2D.glVertex_FL.push_back(pt3d_v);

          pxw = pxt - para_field.carWorldX +
                para_field.chessboard_width_corners *
                    para_field.square_size;
          pyw = pyt - para_field.carWorldY -
                para_field.LRchess2carFront_distance;
          pzw = 0;

          pt3d_w1 = Vec3(pxw, pyw, pzw);

          objPoints2D.glObjPoints_FL_L.push_back(pt3d_w1);

          if (getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) < 0) {
            blendAlpha2D.glAlpha_FL.push_back(0.0);
          } else if (getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) > 0) {
            blendAlpha2D.glAlpha_FL.push_back(1.0);
          } else {
            dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
            dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

            theta1 = asin(dist1 / dist2);

            blendAlpha2D.glAlpha_FL.push_back(1.0 - theta1 / theta);

            if ((pxt > para_field.carWorldX / 2) &&
                (pyt > para_field.carWorldY / 2)) {
              objPointsStatistics2D.glObjPoints_FL_F.push_back(pt3d_w0);
              objPointsStatistics2D.glObjPoints_FL_L.push_back(pt3d_w1);
            }
          }

          pxt = j * para_field.carWorldX;
          pyt = (i + fp) * para_field.carWorldY;

          pxw = pxt - worldWidth / 2 +
                para_field.chessboard_length_corners *
                    para_field.square_size / 2;
          pyw = para_field.chessboard_width_corners *
                    para_field.square_size +
                (pyt - para_field.carWorldY);
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - pyt) / halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_FL_F.push_back(pt3d_w);
          vertexCoords2D.glVertex_FL.push_back(pt3d_v);

          pxw = pxt - para_field.carWorldX +
                para_field.chessboard_width_corners *
                    para_field.square_size;
          pyw = pyt - para_field.carWorldY -
                para_field.LRchess2carFront_distance;
          pzw = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);

          objPoints2D.glObjPoints_FL_L.push_back(pt3d_w);

          if (getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) < 0) {
            blendAlpha2D.glAlpha_FL.push_back(0.0);
          } else if (getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) > 0) {
            blendAlpha2D.glAlpha_FL.push_back(1.0);
          } else {
            dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
            dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

            theta1 = asin(dist1 / dist2);

            blendAlpha2D.glAlpha_FL.push_back(1.0 - theta1 / theta);
          }
        }
  
        break;
      case 1:
        for (j = 1; j >= 0; j -= fp) {
          pxt = j * para_field.carWorldX;
          pyt = i * para_field.carWorldY;

          pxw = pxt - worldWidth / 2 +
                para_field.chessboard_length_corners *
                    para_field.square_size / 2;
          pyw = para_field.chessboard_width_corners *
                    para_field.square_size +
                (pyt - para_field.carWorldY);
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - pyt) / halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_FL_F.push_back(pt3d_w);
          vertexCoords2D.glVertex_FL.push_back(pt3d_v);

          pxw = pxt - para_field.carWorldX +
                para_field.chessboard_width_corners *
                    para_field.square_size;
          pyw = pyt - para_field.carWorldY -
                para_field.LRchess2carFront_distance;
          pzw = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);

          objPoints2D.glObjPoints_FL_L.push_back(pt3d_w);

          if (getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) < 0) {
            blendAlpha2D.glAlpha_FL.push_back(0.0);
          } else if (getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) > 0) {
            blendAlpha2D.glAlpha_FL.push_back(1.0);
          } else {
            dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
            dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

            theta1 = asin(dist1 / dist2);

            blendAlpha2D.glAlpha_FL.push_back(1.0 - theta1 / theta);
          }

          pxt = j * para_field.carWorldX;
          pyt = (i + fp) * para_field.carWorldY;

          pxw = pxt - worldWidth / 2 +
                para_field.chessboard_length_corners *
                    para_field.square_size / 2;
          pyw = para_field.chessboard_width_corners *
                    para_field.square_size +
                (pyt - para_field.carWorldY);
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - pyt) / halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_FL_F.push_back(pt3d_w);
          vertexCoords2D.glVertex_FL.push_back(pt3d_v);

          pxw = pxt - para_field.carWorldX +
                para_field.chessboard_width_corners *
                    para_field.square_size;
          pyw = pyt - para_field.carWorldY -
                para_field.LRchess2carFront_distance;
          pzw = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);

          objPoints2D.glObjPoints_FL_L.push_back(pt3d_w);

          if (getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) < 0) {
            blendAlpha2D.glAlpha_FL.push_back(0.0);
          } else if (getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) > 0) {
            blendAlpha2D.glAlpha_FL.push_back(1.0);
          } else {
            dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
            dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

            theta1 = asin(dist1 / dist2);

            blendAlpha2D.glAlpha_FL.push_back(1.0 - theta1 / theta);
          }
        }

        break;
    }
    // change direction!
    if ( fabs(j - fp - 1) < FLOAT_ZERO ) {
      directionFlag = 1;
    } else {
      directionFlag = 0;
    }
  }
  printf("VTX_NUM_FL: %lu %lu\n", objPoints2D.glObjPoints_FL_F.size(),
         objPoints2D.glObjPoints_FL_L.size());
  texCoords2D.glTexCoord_FL_F.resize(
      objPoints2D.glObjPoints_FL_F.size());
  texCoords2D.glTexCoord_FL_L.resize(
      objPoints2D.glObjPoints_FL_L.size());
  texCoordsStatistics2D.glTexCoord_FL_F.resize(
      objPointsStatistics2D.glObjPoints_FL_F.size());
  texCoordsStatistics2D.glTexCoord_FL_L.resize(
      objPointsStatistics2D.glObjPoints_FL_L.size());
}

void init2DModelFR(int worldWidth,
                   int worldHeight, float fp) {
  float pxv, pyv, pzv;
  float pxw, pyw, pzw;
  float pxt, pyt;
  vec3 pt3d_w, pt3d_w0, pt3d_w1;
  vec3 pt3d_v;

  int halfWorldWidth, halfWorldHeight;
  float i, j;
  float dist1, dist2;
  float theta, theta1;
  float frontXfuse, frontYfuse;
  vec2 f1, f2, f3;
  unsigned char directionFlag = 0;

  halfWorldWidth = worldWidth / 2;
  halfWorldHeight = worldHeight / 2;
  f1.x = para_field.carWorldX + para_field.car_width;
  f1.y = para_field.carWorldY;

  f2.x = worldWidth;
  f2.y = para_field.carWorldY - frontYfuse;

  f3.x = para_field.carWorldX + para_field.car_width + frontXfuse;
  f3.y = 0;

  theta = atan(static_cast<float>(f1.y - f3.y) / (f3.x - f1.x)) -
          atan(static_cast<float>(f1.y - f2.y) / (f2.x - f1.x));


  for (i = 0; i <= 1; i += fp) {
    switch (directionFlag) {
      case 0:
        for (j = 0; j <= 1; j += fp) {
          pxt = para_field.carWorldX + para_field.car_width +
                j * para_field.carWorldX2;
          pyt = i * para_field.carWorldY;

          pxw = pxt - worldWidth / 2 +
                para_field.chessboard_length_corners *
                    para_field.square_size / 2;
          pyw = para_field.chessboard_width_corners *
                    para_field.square_size +
                (pyt - para_field.carWorldY);
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - pyt) / halfWorldHeight;
          pzv = 0;

          pt3d_w0 = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_FR_F.push_back(pt3d_w0);
          vertexCoords2D.glVertex_FR.push_back(pt3d_v);

          pxw = pxt - para_field.carWorldX - para_field.car_width;
          pyw = pyt - para_field.carWorldY -
                para_field.LRchess2carFront_distance;
          pzw = 0;

          pt3d_w1 = Vec3(pxw, pyw, pzw);

          objPoints2D.glObjPoints_FR_R.push_back(pt3d_w1);

          if (getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) > 0) {
            blendAlpha2D.glAlpha_FR.push_back(0.0);
          } else if (getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) < 0) {
            blendAlpha2D.glAlpha_FR.push_back(1.0);
          } else {
            dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
            dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

            theta1 = asin(dist1 / dist2);

            blendAlpha2D.glAlpha_FR.push_back(1.0 - theta1 / theta);

            if ((pxt < para_field.carWorldX + para_field.car_width +
                           para_field.carWorldX2 / 2) &&
                (pyt > para_field.carWorldY / 2)) {
              objPointsStatistics2D.glObjPoints_FR_F.push_back(pt3d_w0);
              objPointsStatistics2D.glObjPoints_FR_R.push_back(pt3d_w1);
            }
          }

          pxt = para_field.carWorldX + para_field.car_width +
                j * para_field.carWorldX2;
          pyt = (i + fp) * para_field.carWorldY;

          pxw = pxt - worldWidth / 2 +
                para_field.chessboard_length_corners *
                    para_field.square_size / 2;
          pyw = para_field.chessboard_width_corners *
                    para_field.square_size +
                (pyt - para_field.carWorldY);
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - pyt) / halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_FR_F.push_back(pt3d_w);
          vertexCoords2D.glVertex_FR.push_back(pt3d_v);

          pxw = pxt - para_field.carWorldX - para_field.car_width;
          pyw = pyt - para_field.carWorldY -
                para_field.LRchess2carFront_distance;
          pzw = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);

          objPoints2D.glObjPoints_FR_R.push_back(pt3d_w);

          if (getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) > 0) {
            blendAlpha2D.glAlpha_FR.push_back(0.0);
          } else if (getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) < 0) {
            blendAlpha2D.glAlpha_FR.push_back(1.0);
          } else {
            dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
            dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

            theta1 = asin(dist1 / dist2);

            blendAlpha2D.glAlpha_FR.push_back(1.0 - theta1 / theta);
          }
        }

        break;
      case 1:
        for (j = 1; j >= 0; j -= fp) {
          pxt = para_field.carWorldX + para_field.car_width +
                j * para_field.carWorldX2;
          pyt = i * para_field.carWorldY;

          pxw = pxt - worldWidth / 2 +
                para_field.chessboard_length_corners *
                    para_field.square_size / 2;
          pyw = para_field.chessboard_width_corners *
                    para_field.square_size +
                (pyt - para_field.carWorldY);
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - pyt) / halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_FR_F.push_back(pt3d_w);
          vertexCoords2D.glVertex_FR.push_back(pt3d_v);

          pxw = pxt - para_field.carWorldX - para_field.car_width;
          pyw = pyt - para_field.carWorldY -
                para_field.LRchess2carFront_distance;
          pzw = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);

          objPoints2D.glObjPoints_FR_R.push_back(pt3d_w);

          if (getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) > 0) {
            blendAlpha2D.glAlpha_FR.push_back(0.0);
          } else if (getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) < 0) {
            blendAlpha2D.glAlpha_FR.push_back(1.0);
          } else {
            dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
            dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

            theta1 = asin(dist1 / dist2);

            blendAlpha2D.glAlpha_FR.push_back(1.0 - theta1 / theta);
          }
      
          pxt = para_field.carWorldX + para_field.car_width +
                j * para_field.carWorldX2;
          pyt = (i + fp) * para_field.carWorldY;

          pxw = pxt - worldWidth / 2 +
                para_field.chessboard_length_corners *
                    para_field.square_size / 2;
          pyw = para_field.chessboard_width_corners *
                    para_field.square_size +
                (pyt - para_field.carWorldY);
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - pyt) / halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_FR_F.push_back(pt3d_w);
          vertexCoords2D.glVertex_FR.push_back(pt3d_v);

          pxw = pxt - para_field.carWorldX - para_field.car_width;
          pyw = pyt - para_field.carWorldY -
                para_field.LRchess2carFront_distance;
          pzw = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);

          objPoints2D.glObjPoints_FR_R.push_back(pt3d_w);

          if (getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) > 0) {
            blendAlpha2D.glAlpha_FR.push_back(0.0);
          } else if (getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) < 0) {
            blendAlpha2D.glAlpha_FR.push_back(1.0);
          } else {
            dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
            dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

            theta1 = asin(dist1 / dist2);

            blendAlpha2D.glAlpha_FR.push_back(1.0 - theta1 / theta);
          }
        }
  
        break;
    }
    // change direction!
    if ( fabs(j - fp - 1) < FLOAT_ZERO ) {
      directionFlag = 1;
    } else {
      directionFlag = 0;
    }
  }
  printf("VTX_NUM_FR: %lu %lu\n", objPoints2D.glObjPoints_FR_F.size(),
         objPoints2D.glObjPoints_FR_R.size());
  texCoords2D.glTexCoord_FR_F.resize(
      objPoints2D.glObjPoints_FR_F.size());
  texCoords2D.glTexCoord_FR_R.resize(
      objPoints2D.glObjPoints_FR_R.size());
  texCoordsStatistics2D.glTexCoord_FR_F.resize(
      objPointsStatistics2D.glObjPoints_FR_F.size());
  texCoordsStatistics2D.glTexCoord_FR_R.resize(
      objPointsStatistics2D.glObjPoints_FR_R.size());
}

void init2DModelBL(int worldWidth,
                   int worldHeight, float fp) {
  float pxv, pyv, pzv;
  float pxw, pyw, pzw;
  float pxt, pyt;
  vec3 pt3d_w, pt3d_w0, pt3d_w1;
  vec3 pt3d_v;

  int halfWorldWidth, halfWorldHeight;
  float i, j;
  float dist1, dist2;
  float theta, theta1;
  float weight;
  float rearXfuse, rearYfuse;
  vec2 f1, f2, f3;
  unsigned char directionFlag = 0;

  halfWorldWidth = worldWidth / 2;
  halfWorldHeight = worldHeight / 2;

  rearXfuse = para_field.carWorldX * 0.8;
  rearYfuse =
      (worldHeight - para_field.carWorldY - para_field.car_length) *
      0.2;

  f1.x = para_field.carWorldX;
  f1.y = para_field.carWorldY + para_field.car_length;

  f2.x = 0;
  f2.y = para_field.carWorldY + para_field.car_length + rearYfuse;

  f3.x = para_field.carWorldX - rearXfuse;
  f3.y = worldHeight;

  theta = atan(static_cast<float>((f3.y - f1.y)) / (f1.x - f3.x)) -
          atan(static_cast<float>((f2.y - f1.y)) / (f1.x - f2.x));


  for (i = 0; i <= 1; i += fp) {
    switch (directionFlag) {
      case 0:
        for (j = 0; j <= 1; j += fp) {
          pxt = j * para_field.carWorldX;
          pyt = para_field.carWorldY + para_field.car_length +
                i * para_field.carWorldY2;

          pxw = pxt - halfWorldWidth +
                para_field.chessboard_length_corners *
                    para_field.square_size / 2;
          pyw = pyt - para_field.carWorldY - para_field.car_length;
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - pyt) / halfWorldHeight;
          pzv = 0;

          pt3d_w0 = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_BL_B.push_back(pt3d_w0);
          vertexCoords2D.glVertex_BL.push_back(pt3d_v);

          pxw = pxt - para_field.carWorldX +
                para_field.chessboard_width_corners *
                    para_field.square_size;
          pyw = pyt - para_field.carWorldY -
                para_field.LRchess2carFront_distance;
          pzw = 0;

          pt3d_w1 = Vec3(pxw, pyw, pzw);

          objPoints2D.glObjPoints_BL_L.push_back(pt3d_w1);

          if (getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) > 0) {
            blendAlpha2D.glAlpha_BL.push_back(0.0);
          } else if (getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) < 0) {
            blendAlpha2D.glAlpha_BL.push_back(1.0);
          } else {
            dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
            dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

            theta1 = asin(dist1 / dist2);

            weight = theta1 / theta;
            blendAlpha2D.glAlpha_BL.push_back(1.0 - theta1 / theta);

            if ((pxt > para_field.carWorldX / 2) &&
                (pyt < para_field.carWorldY +
                           para_field.car_length +
                           para_field.carWorldY2 / 2)) {
              objPointsStatistics2D.glObjPoints_BL_B.push_back(pt3d_w0);
              objPointsStatistics2D.glObjPoints_BL_L.push_back(pt3d_w1);
            }
          }

          pxt = j * para_field.carWorldX;
          pyt = para_field.carWorldY + para_field.car_length +
                (i + fp) * para_field.carWorldY2;

          pxw = pxt - halfWorldWidth +
                para_field.chessboard_length_corners *
                    para_field.square_size / 2;
          pyw = pyt - para_field.carWorldY - para_field.car_length;
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - pyt) / halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_BL_B.push_back(pt3d_w);
          vertexCoords2D.glVertex_BL.push_back(pt3d_v);

          pxw = pxt - para_field.carWorldX +
                para_field.chessboard_width_corners *
                    para_field.square_size;
          pyw = pyt - para_field.carWorldY -
                para_field.LRchess2carFront_distance;
          pzw = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);

          objPoints2D.glObjPoints_BL_L.push_back(pt3d_w);

          if (getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) > 0) {
            blendAlpha2D.glAlpha_BL.push_back(0.0);
          } else if (getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) < 0) {
            blendAlpha2D.glAlpha_BL.push_back(1.0);
          } else {
            dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
            dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

            theta1 = asin(dist1 / dist2);

            weight = theta1 / theta;
            blendAlpha2D.glAlpha_BL.push_back(1.0 - theta1 / theta);
          }
        }

        break;
      case 1:
        for (j = 1; j >= 0; j -= fp) {
          pxt = j * para_field.carWorldX;
          pyt = para_field.carWorldY + para_field.car_length +
                i * para_field.carWorldY2;

          pxw = pxt - halfWorldWidth +
                para_field.chessboard_length_corners *
                    para_field.square_size / 2;
          pyw = pyt - para_field.carWorldY - para_field.car_length;
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - pyt) / halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_BL_B.push_back(pt3d_w);
          vertexCoords2D.glVertex_BL.push_back(pt3d_v);

          pxw = pxt - para_field.carWorldX +
                para_field.chessboard_width_corners *
                    para_field.square_size;
          pyw = pyt - para_field.carWorldY -
                para_field.LRchess2carFront_distance;
          pzw = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);

          objPoints2D.glObjPoints_BL_L.push_back(pt3d_w);

          if (getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) > 0) {
            blendAlpha2D.glAlpha_BL.push_back(0.0);
          } else if (getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) < 0) {
            blendAlpha2D.glAlpha_BL.push_back(1.0);
          } else {
            dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
            dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

            theta1 = asin(dist1 / dist2);

            weight = theta1 / theta;
            blendAlpha2D.glAlpha_BL.push_back(1.0 - theta1 / theta);
          }

          pxt = j * para_field.carWorldX;
          pyt = para_field.carWorldY + para_field.car_length +
                (i + fp) * para_field.carWorldY2;

          pxw = pxt - halfWorldWidth +
                para_field.chessboard_length_corners *
                    para_field.square_size / 2;
          pyw = pyt - para_field.carWorldY - para_field.car_length;
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - pyt) / halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_BL_B.push_back(pt3d_w);
          vertexCoords2D.glVertex_BL.push_back(pt3d_v);

          pxw = pxt - para_field.carWorldX +
                para_field.chessboard_width_corners *
                    para_field.square_size;
          pyw = pyt - para_field.carWorldY -
                para_field.LRchess2carFront_distance;
          pzw = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);

          objPoints2D.glObjPoints_BL_L.push_back(pt3d_w);

          if (getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) > 0) {
            blendAlpha2D.glAlpha_BL.push_back(0.0);
          } else if (getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) < 0) {
            blendAlpha2D.glAlpha_BL.push_back(1.0);
          } else {
            dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
            dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

            theta1 = asin(dist1 / dist2);

            weight = theta1 / theta;
            blendAlpha2D.glAlpha_BL.push_back(1.0 - theta1 / theta);
          }
        }
       
        break;
    }
    // change direction!
    if ( fabs(j - fp - 1) < FLOAT_ZERO ) {
      directionFlag = 1;
    } else {
      directionFlag = 0;
    }
  }
  printf("VTX_NUM_BL: %lu %lu\n", objPoints2D.glObjPoints_BL_B.size(),
         objPoints2D.glObjPoints_BL_L.size());
  texCoords2D.glTexCoord_BL_B.resize(
      objPoints2D.glObjPoints_BL_B.size());
  texCoords2D.glTexCoord_BL_L.resize(
      objPoints2D.glObjPoints_BL_L.size());
  texCoordsStatistics2D.glTexCoord_BL_B.resize(
      objPointsStatistics2D.glObjPoints_BL_B.size());
  texCoordsStatistics2D.glTexCoord_BL_L.resize(
      objPointsStatistics2D.glObjPoints_BL_L.size());
}

void init2DModelBR(int worldWidth,
                   int worldHeight, float fp) {
  float pxv, pyv, pzv;
  float pxw, pyw, pzw;
  float pxt, pyt;
  vec3 pt3d_w, pt3d_w0, pt3d_w1;
  vec3 pt3d_v;

  int halfWorldWidth, halfWorldHeight;
  float i, j;
  float dist1, dist2;
  float theta, theta1;
  float weight;
  float rearXfuse, rearYfuse;
  vec2 f1, f2, f3;
  unsigned char directionFlag = 0;

  halfWorldWidth = worldWidth / 2;
  halfWorldHeight = worldHeight / 2;

  rearXfuse = para_field.carWorldX * 0.8;
  rearYfuse =
      (worldHeight - para_field.carWorldY - para_field.car_length) *
      0.2;

  f1.x = para_field.carWorldX + para_field.car_width;
  f1.y = para_field.carWorldY + para_field.car_length;

  f2.x = worldHeight;
  f2.y = para_field.carWorldY + para_field.car_length + rearYfuse;

  f3.x = para_field.carWorldX + para_field.car_width + rearXfuse;
  f3.y = worldHeight;

  theta = atan(static_cast<float>((f3.y - f1.y)) / (f3.x - f1.x)) -
          atan(static_cast<float>((f2.y - f1.y)) / (f2.x - f1.x));

  for (i = 0; i <= 1; i += fp) {
    switch (directionFlag) {
      case 0:
        for (j = 0; j <= 1; j += fp) {
          pxt = para_field.carWorldX + para_field.car_width +
                j * para_field.carWorldX2;
          pyt = para_field.carWorldY + para_field.car_length +
                i * para_field.carWorldY2;

          pxw = pxt - halfWorldWidth +
                para_field.chessboard_length_corners *
                    para_field.square_size / 2;
          pyw = pyt - para_field.carWorldY - para_field.car_length;
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - pyt) / halfWorldHeight;
          pzv = 0;

          pt3d_w0 = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_BR_B.push_back(pt3d_w0);
          vertexCoords2D.glVertex_BR.push_back(pt3d_v);

          pxw = pxt - para_field.carWorldX - para_field.car_width;
          pyw = pyt - para_field.carWorldY -
                para_field.LRchess2carFront_distance;
          pzw = 0;

          pt3d_w1 = Vec3(pxw, pyw, pzw);

          objPoints2D.glObjPoints_BR_R.push_back(pt3d_w1);

          if (getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) < 0) {
            blendAlpha2D.glAlpha_BR.push_back(0.0);
          } else if (getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) > 0) {
            blendAlpha2D.glAlpha_BR.push_back(1.0);
          } else {
            dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
            dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

            theta1 = asin(dist1 / dist2);

            blendAlpha2D.glAlpha_BR.push_back(1.0 - theta1 / theta);

            if ((pxt < para_field.carWorldX + para_field.car_width +
                           para_field.carWorldX2 / 2) &&
                (pyt < para_field.carWorldY +
                           para_field.car_length +
                           para_field.carWorldY2 / 2)) {
              objPointsStatistics2D.glObjPoints_BR_B.push_back(pt3d_w0);
              objPointsStatistics2D.glObjPoints_BR_R.push_back(pt3d_w1);
            }
          }

          pxt = para_field.carWorldX + para_field.car_width +
                j * para_field.carWorldX2;
          pyt = para_field.carWorldY + para_field.car_length +
                (i + fp) * para_field.carWorldY2;

          pxw = pxt - halfWorldWidth +
                para_field.chessboard_length_corners *
                    para_field.square_size / 2;
          pyw = pyt - para_field.carWorldY - para_field.car_length;
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - pyt) / halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_BR_B.push_back(pt3d_w);
          vertexCoords2D.glVertex_BR.push_back(pt3d_v);

          pxw = pxt - para_field.carWorldX - para_field.car_width;
          pyw = pyt - para_field.carWorldY -
                para_field.LRchess2carFront_distance;
          pzw = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);

          objPoints2D.glObjPoints_BR_R.push_back(pt3d_w);

          if (getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) < 0) {
            blendAlpha2D.glAlpha_BR.push_back(0.0);
          } else if (getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) > 0) {
            blendAlpha2D.glAlpha_BR.push_back(1.0);
          } else {
            dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
            dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

            theta1 = asin(dist1 / dist2);

            blendAlpha2D.glAlpha_BR.push_back(1.0 - theta1 / theta);
          }
        }

        break;
      case 1:
        for (j = 1; j >= 0; j -= fp) {
          pxt = para_field.carWorldX + para_field.car_width +
                j * para_field.carWorldX2;
          pyt = para_field.carWorldY + para_field.car_length +
                i * para_field.carWorldY2;

          pxw = pxt - halfWorldWidth +
                para_field.chessboard_length_corners *
                    para_field.square_size / 2;
          pyw = pyt - para_field.carWorldY - para_field.car_length;
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - pyt) / halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_BR_B.push_back(pt3d_w);
          vertexCoords2D.glVertex_BR.push_back(pt3d_v);

          pxw = pxt - para_field.carWorldX - para_field.car_width;
          pyw = pyt - para_field.carWorldY -
                para_field.LRchess2carFront_distance;
          pzw = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);

          objPoints2D.glObjPoints_BR_R.push_back(pt3d_w);

          if (getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) < 0) {
            blendAlpha2D.glAlpha_BR.push_back(0.0);
          } else if (getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) > 0) {
            blendAlpha2D.glAlpha_BR.push_back(1.0);
          } else {
            dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
            dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

            theta1 = asin(dist1 / dist2);

            blendAlpha2D.glAlpha_BR.push_back(1.0 - theta1 / theta);
          }

          pxt = para_field.carWorldX + para_field.car_width +
                j * para_field.carWorldX2;
          pyt = para_field.carWorldY + para_field.car_length +
                (i + fp) * para_field.carWorldY2;

          pxw = pxt - halfWorldWidth +
                para_field.chessboard_length_corners *
                    para_field.square_size / 2;
          pyw = pyt - para_field.carWorldY - para_field.car_length;
          pzw = 0;

          pxv = (pxt - halfWorldWidth) / halfWorldWidth;
          pyv = (halfWorldHeight - pyt) / halfWorldHeight;
          pzv = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);
          pt3d_v = Vec3(pxv, pyv, pzv);

          objPoints2D.glObjPoints_BR_B.push_back(pt3d_w);
          vertexCoords2D.glVertex_BR.push_back(pt3d_v);

          pxw = pxt - para_field.carWorldX - para_field.car_width;
          pyw = pyt - para_field.carWorldY -
                para_field.LRchess2carFront_distance;
          pzw = 0;

          pt3d_w = Vec3(pxw, pyw, pzw);

          objPoints2D.glObjPoints_BR_R.push_back(pt3d_w);

          if (getDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) < 0) {
            blendAlpha2D.glAlpha_BR.push_back(0.0);
          } else if (getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) > 0) {
            blendAlpha2D.glAlpha_BR.push_back(1.0);
          } else {
            dist1 = fabs(getDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
            dist2 = getPixelDistance(f1.x, f1.y, pxt, pyt);

            theta1 = asin(dist1 / dist2);

            blendAlpha2D.glAlpha_BR.push_back(1.0 - theta1 / theta);
          }
        }
       
        break;
    }
    // change direction!
    if ( fabs(j - fp - 1) < FLOAT_ZERO ) {
      directionFlag = 1;
    } else {
      directionFlag = 0;
    }
  }
  printf("VTX_NUM_BR: %lu %lu\n", objPoints2D.glObjPoints_BR_B.size(),
         objPoints2D.glObjPoints_BR_R.size());
  texCoords2D.glTexCoord_BR_B.resize(
      objPoints2D.glObjPoints_BR_B.size());
  texCoords2D.glTexCoord_BR_R.resize(
      objPoints2D.glObjPoints_BR_R.size());
  texCoordsStatistics2D.glTexCoord_BR_B.resize(
      objPointsStatistics2D.glObjPoints_BR_B.size());
  texCoordsStatistics2D.glTexCoord_BR_R.resize(
      objPointsStatistics2D.glObjPoints_BR_R.size());
}

void init2DModel() {
  int halfWorldWidth, halfWorldHeight, worldWidth, worldHeight;
  float fp;
  float p = 64.0;

  worldWidth = para_field.carWorldX + para_field.car_width +
               para_field.carWorldX2;
  worldHeight = para_field.carWorldY + para_field.car_length +
                para_field.carWorldY2;

  halfWorldWidth = worldWidth / 2;
  halfWorldHeight = worldHeight / 2;

  fp = 1.0 / p;

#if 1
  init2DModelF(worldWidth, worldHeight, fp);
#endif
 
#if 1
  init2DModelB(worldWidth, worldHeight, fp);
#endif

#if 1
  init2DModelL(worldWidth, worldHeight, fp);
#endif

#if 1
  init2DModelR(worldWidth, worldHeight, fp);
#endif

#if 1
  init2DModelFL(worldWidth, worldHeight, fp);
#endif

#if 1
  init2DModelFR(worldWidth, worldHeight, fp);
#endif

#if 1
  init2DModelBL(worldWidth, worldHeight, fp);
#endif

#if 1
  init2DModelBR(worldWidth, worldHeight, fp);
#endif

#if 1
  glVertices2DCar[0] =
      1.0 * (para_field.carWorldX - halfWorldWidth) / halfWorldWidth -
      SHADOW_X_OFFSET;
  glVertices2DCar[1] = 1.0 *
                                 (halfWorldHeight - para_field.carWorldY -
                                  para_field.car_length) /
                                 halfWorldHeight -
                             SHADOW_Y_OFFSET;

  glVertices2DCar[3] = 1.0 *
                                 (para_field.carWorldX +
                                  para_field.car_width - halfWorldWidth) /
                                 halfWorldWidth +
                             SHADOW_X_OFFSET;
  glVertices2DCar[4] = 1.0 *
                                 (halfWorldHeight - para_field.carWorldY -
                                  para_field.car_length) /
                                 halfWorldHeight -
                             SHADOW_Y_OFFSET;

  glVertices2DCar[6] =
      1.0 * (para_field.carWorldX - halfWorldWidth) / halfWorldWidth -
      SHADOW_X_OFFSET;
  glVertices2DCar[7] =
      1.0 * (halfWorldHeight - para_field.carWorldY) / halfWorldHeight +
      SHADOW_Y_OFFSET;

  glVertices2DCar[9] = 1.0 *
                                 (para_field.carWorldX +
                                  para_field.car_width - halfWorldWidth) /
                                 halfWorldWidth +
                             SHADOW_X_OFFSET;
  glVertices2DCar[10] =
      1.0 * (halfWorldHeight - para_field.carWorldY) / halfWorldHeight +
      SHADOW_Y_OFFSET;
#endif
}

void rotateVectorToRotateMatrix(float *vector, float *matrix) {
  int k;
  float rx, ry, rz;
  float theta;

  rx = vector[0];
  ry = vector[1];
  rz = vector[2];

  theta = sqrt(rx * rx + ry * ry + rz * rz);

  const float I[] = {1, 0, 0, 0, 1, 0, 0, 0, 1};

  float c = cos(theta);
  float s = sin(theta);
  float c1 = 1. - c;
  float itheta = theta ? 1. / theta : 0.;

  rx *= itheta;
  ry *= itheta;
  rz *= itheta;

  float rrt[] = {rx * rx, rx * ry, rx * rz, rx * ry, ry * ry,
                 ry * rz, rx * rz, ry * rz, rz * rz};
  float _r_x_[] = {0, -rz, ry, rz, 0, -rx, -ry, rx, 0};

  // R = cos(theta)*I + (1 - cos(theta))*r*rT + sin(theta)*[r_x]
  // where [r_x] is [0 -rz ry; rz 0 -rx; -ry rx 0]
  for (k = 0; k < 9; k++) {
    matrix[k] = c * I[k] + c1 * rrt[k] + s * _r_x_[k];
  }
}

void projectPoints1(int count, vec3 *obj_points, float *r_mat, float *t_vec,
                    float *A, float *k, int *img_points) {
  float fx, fy, cx, cy;
  int i;
  vec2 imgPoints;

  fx = A[0];
  fy = A[4];
  cx = A[2];
  cy = A[5];

  for (i = 0; i < count; i++) {
    double X = obj_points[i].x;
    double Y = obj_points[i].y;
    double Z = obj_points[i].z;
    double x = r_mat[0] * X + r_mat[1] * Y + r_mat[2] * Z + t_vec[0];
    double y = r_mat[3] * X + r_mat[4] * Y + r_mat[5] * Z + t_vec[1];
    double z = r_mat[6] * X + r_mat[7] * Y + r_mat[8] * Z + t_vec[2];
    double r, r2, xd, yd, theta, theta2, theta4, theta6, theta8, theta_d;

    if (z < 0) {
      imgPoints.x = 0.0001;
      imgPoints.y = 0.0001;
    } else {
      if (!k) {
        xd = x;
        yd = y;
      } else {
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

        theta_d = theta * (1 + k[0] * theta2 + k[1] * theta4 + k[2] * theta6 +
                           k[3] * theta8);

        if (r < 0.00001f) {
          xd = 0;
          yd = 0;
        } else {
          xd = (x * theta_d) / r;
          yd = (y * theta_d) / r;
        }
      }
      imgPoints.x = fx * xd + cx;
      imgPoints.y = fy * yd + cy;

      img_points[i] =
          int(imgPoints.y + 0.5) * IMAGE_WIDTH + (imgPoints.x + 0.5);
    }
  }
}

void projectPoints(int count, vec3 *obj_points, float *r_mat, float *t_vec,
                   float *A, float *k, vec2 *img_points) {
  float fx, fy, cx, cy;
  int i;
  vec2 imgPoints;

  fx = A[0];
  fy = A[4];
  cx = A[2];
  cy = A[5];

  for (i = 0; i < count; i++) {
    double X = obj_points[i].x;
    double Y = obj_points[i].y;
    double Z = obj_points[i].z;
    double x = r_mat[0] * X + r_mat[1] * Y + r_mat[2] * Z + t_vec[0];
    double y = r_mat[3] * X + r_mat[4] * Y + r_mat[5] * Z + t_vec[1];
    double z = r_mat[6] * X + r_mat[7] * Y + r_mat[8] * Z + t_vec[2];
    double r, r2, xd, yd, theta, theta2, theta4, theta6, theta8, theta_d;

    if (z < 0) {
      imgPoints.x = 0.0001;
      imgPoints.y = 0.0001;
    } else {
      if (!k) {
        xd = x;
        yd = y;
      } else {
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

        theta_d = theta * (1 + k[0] * theta2 + k[1] * theta4 + k[2] * theta6 +
                           k[3] * theta8);

        if (r < 0.00001f) {
          xd = 0;
          yd = 0;
        } else {
          xd = (x * theta_d) / r;
          yd = (y * theta_d) / r;
        }
      }
      imgPoints.x = fx * xd + cx;
      imgPoints.y = fy * yd + cy;
      if (imgPoints.x < 0 || imgPoints.x > IMAGE_WIDTH || imgPoints.y < 0 ||
          imgPoints.y > IMAGE_HEIGHT) {
        img_points[i].x = 0.0001;
        img_points[i].y = 0.0001;
      } else {
        img_points[i].x = imgPoints.x / IMAGE_WIDTH;
        img_points[i].y = imgPoints.y / IMAGE_HEIGHT;
      }
    }
  }
}



void initTextureCoords() {
  int count;
  vec3 *obj_points;
  vec2 *img_points;

  obj_points = &objPoints2D.glObjPoints_F[0];
  img_points = &texCoords2D.glTexCoord_F[0];
  count = objPoints2D.glObjPoints_F.size();

  projectPoints(count, obj_points, frontCamParams.mr,
                frontCamParams.mt, frontCamParams.mi,
                frontCamParams.md, img_points);

  objPoints2D.glObjPoints_F.clear();

  // front left blend front part
  obj_points = &objPoints2D.glObjPoints_FL_F[0];
  img_points = &texCoords2D.glTexCoord_FL_F[0];
  count = objPoints2D.glObjPoints_FL_F.size();

  projectPoints(count, obj_points, frontCamParams.mr,
                frontCamParams.mt, frontCamParams.mi,
                frontCamParams.md, img_points);
  objPoints2D.glObjPoints_FL_F.clear();

  // front right blend front part
  obj_points = &objPoints2D.glObjPoints_FR_F[0];
  img_points = &texCoords2D.glTexCoord_FR_F[0];
  count = objPoints2D.glObjPoints_FR_F.size();

  projectPoints(count, obj_points, frontCamParams.mr,
                frontCamParams.mt, frontCamParams.mi,
                frontCamParams.md, img_points);
  objPoints2D.glObjPoints_FR_F.clear();

  // rear part
  obj_points = &objPoints2D.glObjPoints_B[0];
  img_points = &texCoords2D.glTexCoord_B[0];
  count = objPoints2D.glObjPoints_B.size();

  projectPoints(count, obj_points, rearCamParams.mr,
                rearCamParams.mt, rearCamParams.mi,
                rearCamParams.md, img_points);

  objPoints2D.glObjPoints_B.clear();

  // rear left blend rear part
  obj_points = &objPoints2D.glObjPoints_BL_B[0];
  img_points = &texCoords2D.glTexCoord_BL_B[0];
  count = objPoints2D.glObjPoints_BL_B.size();

  projectPoints(count, obj_points, rearCamParams.mr,
                rearCamParams.mt, rearCamParams.mi,
                rearCamParams.md, img_points);

  objPoints2D.glObjPoints_BL_B.clear();

  // rear right blend rear part
  obj_points = &objPoints2D.glObjPoints_BR_B[0];
  img_points = &texCoords2D.glTexCoord_BR_B[0];
  count = objPoints2D.glObjPoints_BR_B.size();

  projectPoints(count, obj_points, rearCamParams.mr,
                rearCamParams.mt, rearCamParams.mi,
                rearCamParams.md, img_points);

  objPoints2D.glObjPoints_BR_B.clear();

  // left cam
  obj_points = &objPoints2D.glObjPoints_L[0];
  img_points = &texCoords2D.glTexCoord_L[0];
  count = objPoints2D.glObjPoints_L.size();

  projectPoints(count, obj_points, leftCamParams.mr,
                leftCamParams.mt, leftCamParams.mi,
                leftCamParams.md, img_points);

  objPoints2D.glObjPoints_L.clear();

  // left front blend left part
  obj_points = &objPoints2D.glObjPoints_FL_L[0];
  img_points = &texCoords2D.glTexCoord_FL_L[0];
  count = objPoints2D.glObjPoints_FL_L.size();

  projectPoints(count, obj_points, leftCamParams.mr,
                leftCamParams.mt, leftCamParams.mi,
                leftCamParams.md, img_points);

  objPoints2D.glObjPoints_FL_L.clear();

  // left rear blend left part
  obj_points = &objPoints2D.glObjPoints_BL_L[0];
  img_points = &texCoords2D.glTexCoord_BL_L[0];
  count = objPoints2D.glObjPoints_BL_L.size();

  projectPoints(count, obj_points, leftCamParams.mr,
                leftCamParams.mt, leftCamParams.mi,
                leftCamParams.md, img_points);

  objPoints2D.glObjPoints_BL_L.clear();

  // right cam
  obj_points = &objPoints2D.glObjPoints_R[0];
  img_points = &texCoords2D.glTexCoord_R[0];
  count = objPoints2D.glObjPoints_R.size();

  projectPoints(count, obj_points, rightCamParams.mr,
                rightCamParams.mt, rightCamParams.mi,
                rightCamParams.md, img_points);

  objPoints2D.glObjPoints_R.clear();

  // right front blend right part
  obj_points = &objPoints2D.glObjPoints_FR_R[0];
  img_points = &texCoords2D.glTexCoord_FR_R[0];
  count = objPoints2D.glObjPoints_FR_R.size();

  projectPoints(count, obj_points, rightCamParams.mr,
                rightCamParams.mt, rightCamParams.mi,
                rightCamParams.md, img_points);

  objPoints2D.glObjPoints_FR_R.clear();

  // right rear blend right part
  obj_points = &objPoints2D.glObjPoints_BR_R[0];
  img_points = &texCoords2D.glTexCoord_BR_R[0];
  count = objPoints2D.glObjPoints_BR_R.size();

  projectPoints(count, obj_points, rightCamParams.mr,
                rightCamParams.mt, rightCamParams.mi,
                rightCamParams.md, img_points);

  objPoints2D.glObjPoints_BR_R.clear();
}

void getCamPixelPosition() {
  int count;
  vec3 *obj_points;
  int *img_points;

  // front left blend front part
  obj_points = &objPointsStatistics2D.glObjPoints_FL_F[0];
  img_points = &texCoordsStatistics2D.glTexCoord_FL_F[0];
  count = objPointsStatistics2D.glObjPoints_FL_F.size();

  projectPoints1(count, obj_points, frontCamParams.mr,
                 frontCamParams.mt, frontCamParams.mi,
                 frontCamParams.md, img_points);

  objPointsStatistics2D.glObjPoints_FL_F.clear();

  // front right blend front part
  obj_points = &objPointsStatistics2D.glObjPoints_FR_F[0];
  img_points = &texCoordsStatistics2D.glTexCoord_FR_F[0];
  count = objPointsStatistics2D.glObjPoints_FR_F.size();

  projectPoints1(count, obj_points, frontCamParams.mr,
                 frontCamParams.mt, frontCamParams.mi,
                 frontCamParams.md, img_points);

  objPointsStatistics2D.glObjPoints_FR_F.clear();

  // front left blend front part
  obj_points = &objPointsStatistics2D.glObjPoints_BL_B[0];
  img_points = &texCoordsStatistics2D.glTexCoord_BL_B[0];
  count = objPointsStatistics2D.glObjPoints_BL_B.size();

  projectPoints1(count, obj_points, rearCamParams.mr,
                 rearCamParams.mt, rearCamParams.mi,
                 rearCamParams.md, img_points);

  objPointsStatistics2D.glObjPoints_BL_B.clear();

  // front right blend front part
  obj_points = &objPointsStatistics2D.glObjPoints_BR_B[0];
  img_points = &texCoordsStatistics2D.glTexCoord_BR_B[0];
  count = objPointsStatistics2D.glObjPoints_BR_B.size();

  projectPoints1(count, obj_points, rearCamParams.mr,
                 rearCamParams.mt, rearCamParams.mi,
                 rearCamParams.md, img_points);

  objPointsStatistics2D.glObjPoints_BR_B.clear();

  // front left blend front part
  obj_points = &objPointsStatistics2D.glObjPoints_FL_L[0];
  img_points = &texCoordsStatistics2D.glTexCoord_FL_L[0];
  count = objPointsStatistics2D.glObjPoints_FL_L.size();

  projectPoints1(count, obj_points, leftCamParams.mr,
                 leftCamParams.mt, leftCamParams.mi,
                 leftCamParams.md, img_points);

  objPointsStatistics2D.glObjPoints_FL_L.clear();

  // front right blend front part
  obj_points = &objPointsStatistics2D.glObjPoints_BL_L[0];
  img_points = &texCoordsStatistics2D.glTexCoord_BL_L[0];
  count = objPointsStatistics2D.glObjPoints_BL_L.size();

  projectPoints1(count, obj_points, leftCamParams.mr,
                 leftCamParams.mt, leftCamParams.mi,
                 leftCamParams.md, img_points);

  objPointsStatistics2D.glObjPoints_BL_L.clear();

  // front left blend front part
  obj_points = &objPointsStatistics2D.glObjPoints_FR_R[0];
  img_points = &texCoordsStatistics2D.glTexCoord_FR_R[0];
  count = objPointsStatistics2D.glObjPoints_FR_R.size();

  projectPoints1(count, obj_points, rightCamParams.mr,
                 rightCamParams.mt, rightCamParams.mi,
                 rightCamParams.md, img_points);

  objPointsStatistics2D.glObjPoints_FR_R.clear();

  // front right blend front part
  obj_points = &objPointsStatistics2D.glObjPoints_BR_R[0];
  img_points = &texCoordsStatistics2D.glTexCoord_BR_R[0];
  count = objPointsStatistics2D.glObjPoints_BR_R.size();

  projectPoints1(count, obj_points, rightCamParams.mr,
                 rightCamParams.mt, rightCamParams.mi,
                 rightCamParams.md, img_points);

  objPointsStatistics2D.glObjPoints_BR_R.clear();
}

void initVBO() {

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
  


#if 1
    glBindBuffer(GL_ARRAY_BUFFER,
                 VBO2DMosaicImageParams.CamVerticesPoints[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 texCoords2D.glTexCoord_F.size() * sizeof(vec3),
                 &vertexCoords2D.glVertex_F[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 VBO2DMosaicImageParams.CamVerticesPoints[1]);
    glBufferData(GL_ARRAY_BUFFER,
                 texCoords2D.glTexCoord_B.size() * sizeof(vec3),
                 &vertexCoords2D.glVertex_B[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 VBO2DMosaicImageParams.CamVerticesPoints[2]);
    glBufferData(GL_ARRAY_BUFFER,
                 texCoords2D.glTexCoord_L.size() * sizeof(vec3),
                 &vertexCoords2D.glVertex_L[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 VBO2DMosaicImageParams.CamVerticesPoints[3]);
    glBufferData(GL_ARRAY_BUFFER,
                 texCoords2D.glTexCoord_R.size() * sizeof(vec3),
                 &vertexCoords2D.glVertex_R[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CamImagePoints[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 texCoords2D.glTexCoord_F.size() * sizeof(vec2),
                 &texCoords2D.glTexCoord_F[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CamImagePoints[1]);
    glBufferData(GL_ARRAY_BUFFER,
                 texCoords2D.glTexCoord_B.size() * sizeof(vec2),
                 &texCoords2D.glTexCoord_B[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CamImagePoints[2]);
    glBufferData(GL_ARRAY_BUFFER,
                 texCoords2D.glTexCoord_L.size() * sizeof(vec2),
                 &texCoords2D.glTexCoord_L[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CamImagePoints[3]);
    glBufferData(GL_ARRAY_BUFFER,
                 texCoords2D.glTexCoord_R.size() * sizeof(vec2),
                 &texCoords2D.glTexCoord_R[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 VBO2DMosaicImageParams.MosaicCamVerticesPoints[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertexCoords2D.glVertex_FL.size() * sizeof(vec3),
                 &vertexCoords2D.glVertex_FL[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 VBO2DMosaicImageParams.MosaicCamVerticesPoints[1]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertexCoords2D.glVertex_FR.size() * sizeof(vec3),
                 &vertexCoords2D.glVertex_FR[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 VBO2DMosaicImageParams.MosaicCamVerticesPoints[2]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertexCoords2D.glVertex_BL.size() * sizeof(vec3),
                 &vertexCoords2D.glVertex_BL[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 VBO2DMosaicImageParams.MosaicCamVerticesPoints[3]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertexCoords2D.glVertex_BR.size() * sizeof(vec3),
                 &vertexCoords2D.glVertex_BR[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 VBO2DMosaicImageParams.MosaicFLCamImagePoints[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertexCoords2D.glVertex_FL.size() * sizeof(vec2),
                 &texCoords2D.glTexCoord_FL_F[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 VBO2DMosaicImageParams.MosaicFLCamImagePoints[1]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertexCoords2D.glVertex_FL.size() * sizeof(vec2),
                 &texCoords2D.glTexCoord_FL_L[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 VBO2DMosaicImageParams.MosaicFRCamImagePoints[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertexCoords2D.glVertex_FR.size() * sizeof(vec2),
                 &texCoords2D.glTexCoord_FR_F[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 VBO2DMosaicImageParams.MosaicFRCamImagePoints[1]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertexCoords2D.glVertex_FR.size() * sizeof(vec2),
                 &texCoords2D.glTexCoord_FR_R[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 VBO2DMosaicImageParams.MosaicBLCamImagePoints[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertexCoords2D.glVertex_BL.size() * sizeof(vec2),
                 &texCoords2D.glTexCoord_BL_B[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 VBO2DMosaicImageParams.MosaicBLCamImagePoints[1]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertexCoords2D.glVertex_BL.size() * sizeof(vec2),
                 &texCoords2D.glTexCoord_BL_L[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 VBO2DMosaicImageParams.MosaicBRCamImagePoints[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertexCoords2D.glVertex_BR.size() * sizeof(vec2),
                 &texCoords2D.glTexCoord_BR_B[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 VBO2DMosaicImageParams.MosaicBRCamImagePoints[1]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertexCoords2D.glVertex_BR.size() * sizeof(vec2),
                 &texCoords2D.glTexCoord_BR_R[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.LumiaBalance[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 lumiaAdjust2D.glLumiaAdjust_F.size() * sizeof(float),
                 &lumiaAdjust2D.glLumiaAdjust_F[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.LumiaBalance[1]);
    glBufferData(GL_ARRAY_BUFFER,
                 lumiaAdjust2D.glLumiaAdjust_B.size() * sizeof(float),
                 &lumiaAdjust2D.glLumiaAdjust_B[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.LumiaBalance[2]);
    glBufferData(GL_ARRAY_BUFFER,
                 lumiaAdjust2D.glLumiaAdjust_L.size() * sizeof(float),
                 &lumiaAdjust2D.glLumiaAdjust_L[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.LumiaBalance[3]);
    glBufferData(GL_ARRAY_BUFFER,
                 lumiaAdjust2D.glLumiaAdjust_R.size() * sizeof(float),
                 &lumiaAdjust2D.glLumiaAdjust_R[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.Alpha[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertexCoords2D.glVertex_FL.size() * sizeof(float),
                 &blendAlpha2D.glAlpha_FL[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.Alpha[1]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertexCoords2D.glVertex_FR.size() * sizeof(float),
                 &blendAlpha2D.glAlpha_FR[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.Alpha[2]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertexCoords2D.glVertex_BL.size() * sizeof(float),
                 &blendAlpha2D.glAlpha_BL[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.Alpha[3]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertexCoords2D.glVertex_BR.size() * sizeof(float),
                 &blendAlpha2D.glAlpha_BR[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CarVerTexCoord[0]);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), glVertices2DCar,
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO2DMosaicImageParams.CarVerTexCoord[1]);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), glTexCoordCar,
                 GL_STATIC_DRAW);
  
#endif

}

void setCameraParams(camParams *scm) {
  int j;
  float rVec[3];
  for (j = 0; j < 3; j++) {
    rVec[j] = static_cast<float>(scm->mrInt[j]) / SCALE3;
    scm->mt[j] = static_cast<float>(scm->mtInt[j]) / SCALE2;
  }

  rotateVectorToRotateMatrix(rVec, scm->mr);

  for (j = 0; j < 4; j++) {
    scm->md[j] = static_cast<float>(scm->mimdInt[4 + j]) / SCALE1;
  }

  scm->mi[0] = static_cast<float>(scm->mimdInt[0]) / SCALE2;
  scm->mi[1] = 0.0;
  scm->mi[2] = static_cast<float>(scm->mimdInt[2]) / SCALE2;
  scm->mi[3] = 0.0;
  scm->mi[4] = static_cast<float>(scm->mimdInt[1]) / SCALE2;
  scm->mi[5] = static_cast<float>(scm->mimdInt[3]) / SCALE2;
  scm->mi[6] = 0.0;
  scm->mi[7] = 0.0;
  scm->mi[8] = 1.0;
}

void initCamParaData() {

	para_field.chessboard_width_corners = 5;
	para_field.chessboard_length_corners = 7;
	para_field.square_size = 20;

	readParamsXML();

	setCameraParams(&frontCamParams);
	setCameraParams(&rearCamParams);
	setCameraParams(&leftCamParams);
	setCameraParams(&rightCamParams);
}
