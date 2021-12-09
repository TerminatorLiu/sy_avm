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
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*==================================   常量定义
 * =================================*/
#define PI 3.1415926535897932384626433832795f

/*===============================    全局数据类型
 * ================================*/
typedef struct {
  GLfloat m[4][4];
} ESMatrix;

#if 1
/*===============================  函数声明  ================================*/
extern void esScale(ESMatrix *result, GLfloat sx, GLfloat sy, GLfloat sz);

extern void esTranslate(ESMatrix *result, GLfloat tx, GLfloat ty, GLfloat tz);

extern void esRotate(ESMatrix *result, GLfloat angle, GLfloat x, GLfloat y,
                     GLfloat z);

extern void esFrustum(ESMatrix *result, float left, float right, float bottom,
                      float top, float nearZ, float farZ);

extern void esPerspective(ESMatrix *result, float fovy, float aspect,
                          float nearZ, float farZ);

extern void esOrtho(ESMatrix *result, float left, float right, float bottom,
                    float top, float nearZ, float farZ);

extern void esMatrixMultiply(ESMatrix *result, ESMatrix *srcA, ESMatrix *srcB);

extern void esMatrixLoadIdentity(ESMatrix *result);

extern void esMatrixLookAt(ESMatrix *result, float posX, float posY, float posZ,
                           float lookAtX, float lookAtY, float lookAtZ,
                           float upX, float upY, float upZ);

extern void esMatrixTranspose(ESMatrix *matrix);

extern ESMatrix esMatrixScale(ESMatrix *matrix, float scale);

extern ESMatrix esMatrixInvert(ESMatrix *matrix);
#endif
