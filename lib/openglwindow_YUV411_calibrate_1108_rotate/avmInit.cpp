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

TexCoords tex_coords;
ObjPoints obj_points;
VertexCoords vertex_coords;
BlendAlpha blend_alpha;
LumiaAdjust lumia_adjust;

ObjPointsStatistics obj_points_statistics;
TexCoordsStatistics tex_coords_statistics;

VBOMosaicImage vbo_mosaic_image;

camParams front_cam_params;
camParams rear_cam_params;
camParams left_cam_params;
camParams right_cam_params;

Parking_Assistant_Params parking_assistant_params;


GLfloat gl_vertices_car[12] =
{
    -1.0f, -1.0f, 0.0f,  // left-buttom
    1.0f,  -1.0f, 0.0f,  // right- buttom
    -1.0f, 1.0f,  0.0f,  // right-top
    1.0f,  1.0f,  0.0f,  // left-top
};

GLfloat gl_tex_coord_car[] =
{
    0.0f, 1.0f,  // left-top
    1.0f, 1.0f,  // right-top
    0.0f, 0.0f,  // left-buttom
    1.0f, 0.0f,  // right- buttom
};



static float GetDistance(float x1, float y1, float x2, float y2, float x,
                         float y)
{
    float temp1;
    float temp2;
    temp1 = (y2 - y1) * x + (x1 - x2) * y + (y1 - y2) * x1 - (x1 - x2) * y1;
    temp2 =
        static_cast<float>(sqrt((y2 - y1) * (y2 - y1) + (x1 - x2) * (x1 - x2)));
    return static_cast<float>(temp1 / temp2);
}

static float GetPixelDistance(int x1, int y1, int x2, int y2)
{
    float temp1;
    temp1 =
        static_cast<float>(sqrt((y2 - y1) * (y2 - y1) + (x1 - x2) * (x1 - x2)));
    return temp1;
}



// flag: mode  status, directionFlag: direction, world_width: world data, fp:
// increase step
void Init2DModelF(int half_world_width,
                  int half_world_height, int p)
{
    float pxt;
    float pyt;
    vec3 pt3d_w;
    vec3 pt3d_v;
    float adjust_coeff;
	int i;
	int j;
	int k;
	int direct;

    for (i = 0; i < p; ++i)
    {
    	direct = i % 2;
    	
        for (j = 0; j <= p; ++j)
        {
        	for(k=0; k<2; ++k)
        	{
                pxt = (static_cast<float>(j) / p * (1 - direct) + static_cast<float>(p - j) / p * direct) * parking_assistant_params.car_width + parking_assistant_params.car_world_x;
                pyt = static_cast<float>(i+k) / p * parking_assistant_params.car_world_y;

                pt3d_w.x = pxt - half_world_width +
                      parking_assistant_params.chessboard_length_corners *
                      parking_assistant_params.square_size / 2;
                pt3d_w.y = parking_assistant_params.chessboard_width_corners *
                      parking_assistant_params.square_size +
                      (pyt - parking_assistant_params.car_world_y);
                pt3d_w.z = 0;

                pt3d_v.x = (pxt - half_world_width) / half_world_width;
                pt3d_v.y = (half_world_height - pyt) / half_world_height;
                pt3d_v.z = 0;

                obj_points.gl_obj_points_f.push_back(pt3d_w);
                vertex_coords.glVertex_F.push_back(pt3d_v);

                adjust_coeff = (pxt - parking_assistant_params.car_world_x) /
                              (parking_assistant_params.car_width);
                lumia_adjust.glLumiaAdjust_F.push_back(adjust_coeff);
			}
        }
    }
    
    printf("VTX_NUM_F: %lu\n", obj_points.gl_obj_points_f.size());
    tex_coords.gl_tex_coord_f.resize(obj_points.gl_obj_points_f.size());
}



void Init2DModelB(int half_world_width,
                  int half_world_height, int p)
{
    float pxt;
    float pyt;
    vec3 pt3d_w;
    vec3 pt3d_v;
    float adjust_coeff;
	int i;
	int j;
	int k;
	int direct;

    for (i = 0; i < p; ++i)
    {
        direct = i % 2;
    	
        for (j = 0; j <= p; ++j)
        {
        	for(k=0; k<2; ++k)
        	{
                pxt = (static_cast<float>(j) / p * (1 - direct) + static_cast<float>(p - j) / p * direct) * parking_assistant_params.car_width + parking_assistant_params.car_world_x;
                pyt = static_cast<float>(i+k) / p * parking_assistant_params.car_world_y2;

                pt3d_w.x = pxt - half_world_width +
                      parking_assistant_params.chessboard_length_corners *
                      parking_assistant_params.square_size / 2;
                pt3d_w.y = pyt;
                pt3d_w.z = 0;

                pt3d_v.x = (pxt - half_world_width) / half_world_width;
                pt3d_v.y = (half_world_height - (pyt + parking_assistant_params.car_world_y +
                                          parking_assistant_params.car_length)) /
                      half_world_height;
                pt3d_v.z = 0;

                obj_points.gl_obj_points_b.push_back(pt3d_w);
                vertex_coords.glVertex_B.push_back(pt3d_v);

                adjust_coeff = (pxt - parking_assistant_params.car_world_x) /
                              (parking_assistant_params.car_width);
                lumia_adjust.glLumiaAdjust_B.push_back(adjust_coeff);
        	}
        }
    }
    printf("VTX_NUM_B: %lu\n", obj_points.gl_obj_points_b.size());
    tex_coords.gl_tex_coord_b.resize(obj_points.gl_obj_points_b.size());
}



void Init2DModelL(int half_world_width,
                  int half_world_height, int p)
{
    float pxt;
    float pyt;
    vec3 pt3d_w;
    vec3 pt3d_v;
    float adjust_coeff;
	int i;
	int j;
	int k;
	int direct;

    for (i = 0; i < p; ++i)
    {
        direct = i % 2;
    	
        for (j = 0; j <= p; ++j)
        {
        	for(k=0; k<2; ++k)
            {
                pxt = (static_cast<float>(j) / p * (1 - direct) + static_cast<float>(p - j) / p * direct) * parking_assistant_params.car_world_x;
                pyt = static_cast<float>(i+k) / p * parking_assistant_params.car_length;

                pt3d_w.x = pxt - parking_assistant_params.car_world_x +
                      parking_assistant_params.chessboard_width_corners *
                      parking_assistant_params.square_size;
                pt3d_w.y = pyt - parking_assistant_params.LRchess2carFront_distance;
                pt3d_w.z = 0;

                pt3d_v.x = (pxt - half_world_width) / half_world_width;
                pt3d_v.y = (half_world_height - (pyt + parking_assistant_params.car_world_y)) /
                      half_world_height;
                pt3d_v.z = 0;

                obj_points.gl_obj_points_l.push_back(pt3d_w);
                vertex_coords.glVertex_L.push_back(pt3d_v);

                adjust_coeff = pyt / parking_assistant_params.car_length;
                lumia_adjust.glLumiaAdjust_L.push_back(adjust_coeff);
            }
        }
    }
    printf("VTX_NUM_L: %lu\n", obj_points.gl_obj_points_l.size());
    tex_coords.gl_tex_coord_l.resize(obj_points.gl_obj_points_l.size());
}


void Init2DModelR(int half_world_width,
                  int half_world_height, int p)
{
    float pxt;
    float pyt;
    vec3 pt3d_w;
    vec3 pt3d_v;
    float adjust_coeff;
	int i;
	int j;
	int k;
	int direct;
    
   	for (i = 0; i < p; ++i)
    {
        direct = i % 2;
    	
        for (j = 0; j <= p; ++j)
        {
        	for(k=0; k<2; ++k)
            {
                pxt = (static_cast<float>(j) / p * (1 - direct) + static_cast<float>(p - j) / p * direct) * parking_assistant_params.car_world_x2;
                pyt = static_cast<float>(i+k) / p * parking_assistant_params.car_length;

                pt3d_w.x = pxt;
                pt3d_w.y = pyt - parking_assistant_params.LRchess2carFront_distance;
                pt3d_w.z = 0;

                pt3d_v.x = (parking_assistant_params.car_world_x + parking_assistant_params.car_width + pxt -
                       half_world_width) /
                      half_world_width;
                pt3d_v.y = (half_world_height - (pyt + parking_assistant_params.car_world_y)) /
                      half_world_height;
                pt3d_v.z = 0;

                obj_points.gl_obj_points_r.push_back(pt3d_w);
                vertex_coords.glVertex_R.push_back(pt3d_v);

                adjust_coeff = pyt / parking_assistant_params.car_length;
                lumia_adjust.glLumiaAdjust_R.push_back(adjust_coeff);
            }
    	}
    }
    printf("VTX_NUM_R: %lu\n", obj_points.gl_obj_points_r.size());
    tex_coords.gl_tex_coord_r.resize(obj_points.gl_obj_points_r.size());
}



void Init2DModelFL(int half_world_width,
                   int half_world_height, int p)
{
    float pxt;
    float pyt;
    vec3 pt3d_w0;
    vec3 pt3d_w1;
    vec3 pt3d_v;
    int i;
    int j;
    int k;
    int direct;
    float dist1;
    float dist2;
    float theta;
    float theta1;
    float frontXfuse;
    float frontYfuse;
    vec2 f1;
    vec2 f2;
    vec2 f3;

    frontXfuse = parking_assistant_params.car_world_x * 0.5;
    frontYfuse = parking_assistant_params.car_world_y * 0.5;

    f1.x = parking_assistant_params.car_world_x;
    f1.y = parking_assistant_params.car_world_y;

    f2.x = 0;
    f2.y = parking_assistant_params.car_world_y - frontYfuse;

    f3.x = parking_assistant_params.car_world_x - frontXfuse;
    f3.y = 0;

    theta = atan(static_cast<float>(f1.y - f3.y) / (f1.x - f3.x)) -
            atan(static_cast<float>(f1.y - f2.y) / (f1.x - f2.x));

    for (i = 0; i < p; ++i)
    {
        direct = i % 2;
    	
        for (j = 0; j <= p; ++j)
        {
        	for(k=0; k<2; ++k)
            {
                pxt = (static_cast<float>(j) / p * (1 - direct) + static_cast<float>(p - j) / p * direct) * parking_assistant_params.car_world_x;
                pyt = static_cast<float>(i+k) / p * parking_assistant_params.car_world_y;

                pt3d_w0.x = pxt - half_world_width +
                      parking_assistant_params.chessboard_length_corners *
                      parking_assistant_params.square_size / 2;
                pt3d_w0.y = parking_assistant_params.chessboard_width_corners *
                      parking_assistant_params.square_size +
                      (pyt - parking_assistant_params.car_world_y);
                pt3d_w0.z = 0;

                pt3d_v.x = (pxt - half_world_width) / half_world_width;
                pt3d_v.y = (half_world_height - pyt) / half_world_height;
                pt3d_v.z = 0;

                obj_points.gl_obj_points_fl_f.push_back(pt3d_w0);
                vertex_coords.glVertex_FL.push_back(pt3d_v);

                pt3d_w1.x = pxt - parking_assistant_params.car_world_x +
                      parking_assistant_params.chessboard_width_corners *
                      parking_assistant_params.square_size;
                pt3d_w1.y = pyt - parking_assistant_params.car_world_y -
                      parking_assistant_params.LRchess2carFront_distance;
                pt3d_w1.z = 0;

                obj_points.glObjPoints_FL_L.push_back(pt3d_w1);

                if (GetDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) < 0)
                {
                    blend_alpha.glAlpha_FL.push_back(0.0);
                }
                else if (GetDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) > 0)
                {
                    blend_alpha.glAlpha_FL.push_back(1.0);
                }
                else
                {
                    dist1 = fabs(GetDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
                    dist2 = GetPixelDistance(f1.x, f1.y, pxt, pyt);

                    theta1 = asin(dist1 / dist2);

                    blend_alpha.glAlpha_FL.push_back(1.0 - theta1 / theta);

                    if ((pxt > parking_assistant_params.car_world_x / 2) &&
                            (pyt > parking_assistant_params.car_world_y / 2))
                    {
                        obj_points_statistics.gl_obj_points_fl_f.push_back(pt3d_w0);
                        obj_points_statistics.glObjPoints_FL_L.push_back(pt3d_w1);
                    }
                }
            }
        }
    }
    printf("VTX_NUM_FL: %lu %lu\n", obj_points.gl_obj_points_fl_f.size(),
           obj_points.glObjPoints_FL_L.size());
    tex_coords.gl_tex_coord_fl_f.resize(
        obj_points.gl_obj_points_fl_f.size());
    tex_coords.gl_tex_coord_fl_l.resize(
        obj_points.glObjPoints_FL_L.size());
    tex_coords_statistics.gl_tex_coord_fl_f.resize(
        obj_points_statistics.gl_obj_points_fl_f.size());
    tex_coords_statistics.gl_tex_coord_fl_l.resize(
        obj_points_statistics.glObjPoints_FL_L.size());
}


void Init2DModelFR(int half_world_width,
                   int half_world_height, int p)
{
    float pxt;
    float pyt;
    vec3 pt3d_w0;
    vec3 pt3d_w1;
    vec3 pt3d_v;
    int i;
    int j;
    int k;
    int direct;
    float dist1;
    float dist2;
    float theta;
    float theta1;
    float frontXfuse;
    float frontYfuse;
    vec2 f1;
    vec2 f2;
    vec2 f3;

   
    f1.x = parking_assistant_params.car_world_x + parking_assistant_params.car_width;
    f1.y = parking_assistant_params.car_world_y;

    f2.x = half_world_width * 2;
    f2.y = parking_assistant_params.car_world_y - frontYfuse;

    f3.x = parking_assistant_params.car_world_x + parking_assistant_params.car_width + frontXfuse;
    f3.y = 0;

    theta = atan(static_cast<float>(f1.y - f3.y) / (f3.x - f1.x)) -
            atan(static_cast<float>(f1.y - f2.y) / (f2.x - f1.x));

    for (i = 0; i < p; ++i)
    {
        direct = i % 2;
    	
        for (j = 0; j <= p; ++j)
        {
        	for(k=0; k<2; ++k)
            {
                pxt = parking_assistant_params.car_world_x + parking_assistant_params.car_width +
                      (static_cast<float>(j) / p * (1 - direct) + static_cast<float>(p - j) / p * direct) * parking_assistant_params.car_world_x2;
                pyt = static_cast<float>(i+k) / p * parking_assistant_params.car_world_y;

                pt3d_w0.x = pxt - half_world_width +
                      parking_assistant_params.chessboard_length_corners *
                      parking_assistant_params.square_size / 2;
                pt3d_w0.y = parking_assistant_params.chessboard_width_corners *
                      parking_assistant_params.square_size +
                      (pyt - parking_assistant_params.car_world_y);
                pt3d_w0.z = 0;

                pt3d_v.x = (pxt - half_world_width) / half_world_width;
                pt3d_v.y = (half_world_height - pyt) / half_world_height;
                pt3d_v.z = 0;

                obj_points.glObjPoints_FR_F.push_back(pt3d_w0);
                vertex_coords.glVertex_FR.push_back(pt3d_v);

                pt3d_w1.x = pxt - parking_assistant_params.car_world_x - parking_assistant_params.car_width;
                pt3d_w1.y = pyt - parking_assistant_params.car_world_y - parking_assistant_params.LRchess2carFront_distance;
                pt3d_w1.z = 0;

                obj_points.glObjPoints_FR_R.push_back(pt3d_w1);

                if (GetDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) > 0)
                {
                    blend_alpha.glAlpha_FR.push_back(0.0);
                }
                else if (GetDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) < 0)
                {
                    blend_alpha.glAlpha_FR.push_back(1.0);
                }
                else
                {
                    dist1 = fabs(GetDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
                    dist2 = GetPixelDistance(f1.x, f1.y, pxt, pyt);

                    theta1 = asin(dist1 / dist2);

                    blend_alpha.glAlpha_FR.push_back(1.0 - theta1 / theta);

                    if ((pxt < parking_assistant_params.car_world_x + parking_assistant_params.car_width +
                            parking_assistant_params.car_world_x2 / 2) &&
                            (pyt > parking_assistant_params.car_world_y / 2))
                    {
                        obj_points_statistics.glObjPoints_FR_F.push_back(pt3d_w0);
                        obj_points_statistics.glObjPoints_FR_R.push_back(pt3d_w1);
                    }
                }
            }
        }
    }
    
    printf("VTX_NUM_FR: %lu %lu\n", obj_points.glObjPoints_FR_F.size(),
           obj_points.glObjPoints_FR_R.size());
    tex_coords.gl_tex_coord_fr_f.resize(
        obj_points.glObjPoints_FR_F.size());
    tex_coords.gl_tex_coord_fr_r.resize(
        obj_points.glObjPoints_FR_R.size());
    tex_coords_statistics.gl_tex_coord_fr_f.resize(
        obj_points_statistics.glObjPoints_FR_F.size());
    tex_coords_statistics.gl_tex_coord_fr_r.resize(
        obj_points_statistics.glObjPoints_FR_R.size());
}


void Init2DModelBL(int half_world_width,
                   int half_world_height, int p)
{
    float pxt;
    float pyt;
    vec3 pt3d_w0;
    vec3 pt3d_w1;
    vec3 pt3d_v;
    int i;
    int j;
    int k;
    int direct;
    float dist1;
    float dist2;
    float theta;
    float theta1;
    float rearXfuse;
    float rearYfuse;
    vec2 f1;
    vec2 f2;
    vec2 f3;

    rearXfuse = parking_assistant_params.car_world_x * 0.8;
    rearYfuse =
        (half_world_height * 2 - parking_assistant_params.car_world_y - parking_assistant_params.car_length) *
        0.2;

    f1.x = parking_assistant_params.car_world_x;
    f1.y = parking_assistant_params.car_world_y + parking_assistant_params.car_length;

    f2.x = 0;
    f2.y = parking_assistant_params.car_world_y + parking_assistant_params.car_length + rearYfuse;

    f3.x = parking_assistant_params.car_world_x - rearXfuse;
    f3.y = half_world_height * 2;

    theta = atan(static_cast<float>((f3.y - f1.y)) / (f1.x - f3.x)) -
            atan(static_cast<float>((f2.y - f1.y)) / (f1.x - f2.x));


    for (i = 0; i < p; ++i)
    {
        direct = i % 2;
    	
        for (j = 0; j <= p; ++j)
        {
        	for(k=0; k<2; ++k)
            {
                pxt = (static_cast<float>(j) / p * (1 - direct) + static_cast<float>(p - j) / p * direct) * parking_assistant_params.car_world_x;
                pyt = parking_assistant_params.car_world_y + parking_assistant_params.car_length +
                      static_cast<float>(i+k) / p * parking_assistant_params.car_world_y2;

                pt3d_w0.x = pxt - half_world_width +
                      parking_assistant_params.chessboard_length_corners *
                      parking_assistant_params.square_size / 2;
                pt3d_w0.y = pyt - parking_assistant_params.car_world_y - parking_assistant_params.car_length;
                pt3d_w0.z = 0;

                pt3d_v.x = (pxt - half_world_width) / half_world_width;
                pt3d_v.y = (half_world_height - pyt) / half_world_height;
                pt3d_v.z = 0;

                obj_points.glObjPoints_BL_B.push_back(pt3d_w0);
                vertex_coords.glVertex_BL.push_back(pt3d_v);

                pt3d_w1.x = pxt - parking_assistant_params.car_world_x +
                      parking_assistant_params.chessboard_width_corners *
                      parking_assistant_params.square_size;
                pt3d_w1.y = pyt - parking_assistant_params.car_world_y -
                      parking_assistant_params.LRchess2carFront_distance;
                pt3d_w1.z = 0;

                obj_points.glObjPoints_BL_L.push_back(pt3d_w1);

                if (GetDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) > 0)
                {
                    blend_alpha.glAlpha_BL.push_back(0.0);
                }
                else if (GetDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) < 0)
                {
                    blend_alpha.glAlpha_BL.push_back(1.0);
                }
                else
                {
                    dist1 = fabs(GetDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
                    dist2 = GetPixelDistance(f1.x, f1.y, pxt, pyt);

                    theta1 = asin(dist1 / dist2);

                    blend_alpha.glAlpha_BL.push_back(1.0 - theta1 / theta);

                    if ((pxt > parking_assistant_params.car_world_x / 2) &&
                            (pyt < parking_assistant_params.car_world_y +
                             parking_assistant_params.car_length +
                             parking_assistant_params.car_world_y2 / 2))
                    {
                        obj_points_statistics.glObjPoints_BL_B.push_back(pt3d_w0);
                        obj_points_statistics.glObjPoints_BL_L.push_back(pt3d_w1);
                    }
                }
            }
        }
    }
    printf("VTX_NUM_BL: %lu %lu\n", obj_points.glObjPoints_BL_B.size(),
           obj_points.glObjPoints_BL_L.size());
    tex_coords.gl_tex_coord_bl_b.resize(
        obj_points.glObjPoints_BL_B.size());
    tex_coords.gl_tex_coord_bl_l.resize(
        obj_points.glObjPoints_BL_L.size());
    tex_coords_statistics.gl_tex_coord_bl_b.resize(
        obj_points_statistics.glObjPoints_BL_B.size());
    tex_coords_statistics.gl_tex_coord_bl_l.resize(
        obj_points_statistics.glObjPoints_BL_L.size());
}


void Init2DModelBR(int half_world_width,
                   int half_world_height, int p)
{
    float pxt;
    float pyt;
    vec3 pt3d_w0;
    vec3 pt3d_w1;
    vec3 pt3d_v;
    int i;
    int j;
    int k;
    int direct;
    float dist1;
    float dist2;
    float theta;
    float theta1;
    float rearXfuse;
    float rearYfuse;
    vec2 f1;
    vec2 f2;
    vec2 f3;
    
    rearXfuse = parking_assistant_params.car_world_x * 0.8;
    rearYfuse =
        (half_world_height * 2 - parking_assistant_params.car_world_y - parking_assistant_params.car_length) *
        0.2;

    f1.x = parking_assistant_params.car_world_x + parking_assistant_params.car_width;
    f1.y = parking_assistant_params.car_world_y + parking_assistant_params.car_length;

    f2.x = half_world_height * 2;
    f2.y = parking_assistant_params.car_world_y + parking_assistant_params.car_length + rearYfuse;

    f3.x = parking_assistant_params.car_world_x + parking_assistant_params.car_width + rearXfuse;
    f3.y = half_world_height * 2;

    theta = atan(static_cast<float>((f3.y - f1.y)) / (f3.x - f1.x)) -
            atan(static_cast<float>((f2.y - f1.y)) / (f2.x - f1.x));

    for (i = 0; i < p; ++i)
    {
        direct = i % 2;
    	
        for (j = 0; j <= p; ++j)
        {
        	for(k=0; k<2; ++k)
            {
                pxt = parking_assistant_params.car_world_x + parking_assistant_params.car_width +
                      (static_cast<float>(j) / p * (1 - direct) + static_cast<float>(p - j) / p * direct) * parking_assistant_params.car_world_x2;
                pyt = parking_assistant_params.car_world_y + parking_assistant_params.car_length +
                      static_cast<float>(i+k) / p * parking_assistant_params.car_world_y2;

                pt3d_w0.x = pxt - half_world_width +
                      parking_assistant_params.chessboard_length_corners *
                      parking_assistant_params.square_size / 2;
                pt3d_w0.y = pyt - parking_assistant_params.car_world_y - parking_assistant_params.car_length;
                pt3d_w0.z = 0;

                pt3d_v.x = (pxt - half_world_width) / half_world_width;
                pt3d_v.y = (half_world_height - pyt) / half_world_height;
                pt3d_v.z = 0;

                obj_points.glObjPoints_BR_B.push_back(pt3d_w0);
                vertex_coords.glVertex_BR.push_back(pt3d_v);

                pt3d_w1.x = pxt - parking_assistant_params.car_world_x - parking_assistant_params.car_width;
                pt3d_w1.y = pyt - parking_assistant_params.car_world_y -
                      parking_assistant_params.LRchess2carFront_distance;
                pt3d_w1.z = 0;

                obj_points.glObjPoints_BR_R.push_back(pt3d_w1);

                if (GetDistance(f1.x, f1.y, f3.x, f3.y, pxt, pyt) < 0)
                {
                    blend_alpha.glAlpha_BR.push_back(0.0);
                }
                else if (GetDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt) > 0)
                {
                    blend_alpha.glAlpha_BR.push_back(1.0);
                }
                else
                {
                    dist1 = fabs(GetDistance(f1.x, f1.y, f2.x, f2.y, pxt, pyt));
                    dist2 = GetPixelDistance(f1.x, f1.y, pxt, pyt);

                    theta1 = asin(dist1 / dist2);

                    blend_alpha.glAlpha_BR.push_back(1.0 - theta1 / theta);

                    if ((pxt < parking_assistant_params.car_world_x + parking_assistant_params.car_width +
                            parking_assistant_params.car_world_x2 / 2) &&
                            (pyt < parking_assistant_params.car_world_y +
                             parking_assistant_params.car_length +
                             parking_assistant_params.car_world_y2 / 2))
                    {
                        obj_points_statistics.glObjPoints_BR_B.push_back(pt3d_w0);
                        obj_points_statistics.glObjPoints_BR_R.push_back(pt3d_w1);
                    }
                }
            }
        }
    }
    
    printf("VTX_NUM_BR: %lu %lu\n", obj_points.glObjPoints_BR_B.size(),
           obj_points.glObjPoints_BR_R.size());
    tex_coords.gl_tex_coord__br_b.resize(
        obj_points.glObjPoints_BR_B.size());
    tex_coords.gl_tex_coord__br_r.resize(
        obj_points.glObjPoints_BR_R.size());
    tex_coords_statistics.gl_tex_coord__br_b.resize(
        obj_points_statistics.glObjPoints_BR_B.size());
    tex_coords_statistics.gl_tex_coord__br_r.resize(
        obj_points_statistics.glObjPoints_BR_R.size());
}

void Init2DModel()
{
    int half_world_width;
    int half_world_height;
    int world_width;
    int world_height;
    int p = 64;

    world_width = parking_assistant_params.car_world_x + parking_assistant_params.car_width +
                 parking_assistant_params.car_world_x2;
    world_height = parking_assistant_params.car_world_y + parking_assistant_params.car_length +
                  parking_assistant_params.car_world_y2;

    half_world_width = world_width / 2;
    half_world_height = world_height / 2;


    Init2DModelF(half_world_width, half_world_height, p);

    Init2DModelB(half_world_width, half_world_height, p);

    Init2DModelL(half_world_width, half_world_height, p);

    Init2DModelR(half_world_width, half_world_height, p);

    Init2DModelFL(half_world_width, half_world_height, p);

    Init2DModelFR(half_world_width, half_world_height, p);

    Init2DModelBL(half_world_width, half_world_height, p);

    Init2DModelBR(half_world_width, half_world_height, p);



    gl_vertices_car[0] =
        1.0 * (parking_assistant_params.car_world_x - half_world_width) / half_world_width -
        SHADOW_X_OFFSET;
    gl_vertices_car[1] = 1.0 *
                         (half_world_height - parking_assistant_params.car_world_y -
                          parking_assistant_params.car_length) /
                         half_world_height -
                         SHADOW_Y_OFFSET;

    gl_vertices_car[3] = 1.0 *
                         (parking_assistant_params.car_world_x +
                          parking_assistant_params.car_width - half_world_width) /
                         half_world_width +
                         SHADOW_X_OFFSET;
    gl_vertices_car[4] = 1.0 *
                         (half_world_height - parking_assistant_params.car_world_y -
                          parking_assistant_params.car_length) /
                         half_world_height -
                         SHADOW_Y_OFFSET;

    gl_vertices_car[6] =
        1.0 * (parking_assistant_params.car_world_x - half_world_width) / half_world_width -
        SHADOW_X_OFFSET;
    gl_vertices_car[7] =
        1.0 * (half_world_height - parking_assistant_params.car_world_y) / half_world_height +
        SHADOW_Y_OFFSET;

    gl_vertices_car[9] = 1.0 *
                         (parking_assistant_params.car_world_x +
                          parking_assistant_params.car_width - half_world_width) /
                         half_world_width +
                         SHADOW_X_OFFSET;
    gl_vertices_car[10] =
        1.0 * (half_world_height - parking_assistant_params.car_world_y) / half_world_height +
        SHADOW_Y_OFFSET;

}

void RotateVectorToRotateMatrix(float *vector, float *matrix)
{
    int k;
    float rx;
    float ry;
    float rz;
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
                   ry * rz, rx * rz, ry * rz, rz *rz
                  };
    float _r_x_[] = {0, -rz, ry, rz, 0, -rx, -ry, rx, 0};

    // R = cos(theta)*I + (1 - cos(theta))*r*rT + sin(theta)*[r_x]
    // where [r_x] is [0 -rz ry; rz 0 -rx; -ry rx 0]
    for (k = 0; k < 9; k++)
    {
        matrix[k] = c * I[k] + c1 * rrt[k] + s * _r_x_[k];
    }
}


void ProjectPoints(int count, vec3 *obj_points, float *r_mat, float *t_vec,
                   float *A, float *k, void *img_points, int flag)
{
    float fx;
    float fy;
    float cx;
    float cy;
    int i;
    vec2 imgPoints;
	int *imgIdx;
	vec2 *imgTex;

	imgTex = (vec2 *)img_points;
	imgIdx = (int *)img_points;
	

    fx = A[0];
    fy = A[4];
    cx = A[2];
    cy = A[5];

    for (i = 0; i < count; i++)
    {
        double X = obj_points[i].x;
        double Y = obj_points[i].y;
        double Z = obj_points[i].z;
        double x = r_mat[0] * X + r_mat[1] * Y + r_mat[2] * Z + t_vec[0];
        double y = r_mat[3] * X + r_mat[4] * Y + r_mat[5] * Z + t_vec[1];
        double z = r_mat[6] * X + r_mat[7] * Y + r_mat[8] * Z + t_vec[2];
        double r, r2, xd, yd, theta, theta2, theta4, theta6, theta8, theta_d;

        if (z < 0)
        {
            imgPoints.x = 0.0001;
            imgPoints.y = 0.0001;
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

            theta_d = theta * (1 + k[0] * theta2 + k[1] * theta4 + k[2] * theta6 +
                               k[3] * theta8);

            if (r < 0.00001f)
            {
                xd = 0;
                yd = 0;
            }
            else
            {
                xd = (x * theta_d) / r;
                yd = (y * theta_d) / r;
            }
        
            imgPoints.x = fx * xd + cx;
            imgPoints.y = fy * yd + cy;
            
            if (imgPoints.x < 0 || imgPoints.x > IMAGE_WIDTH || imgPoints.y < 0 ||
                    imgPoints.y > IMAGE_HEIGHT)
            {
                imgPoints.x = 0.0001;
                imgPoints.y = 0.0001;
            }
        }
        
        if(flag)
        {
        	imgTex[i].x = imgPoints.x / IMAGE_WIDTH;
            imgTex[i].y = imgPoints.y / IMAGE_HEIGHT;
        }
        else
        {
        	imgIdx[i] =
            	int(imgPoints.y + 0.5) * IMAGE_WIDTH + (imgPoints.x + 0.5);
        }
    }
}


void InitTextureCoords()
{
    int count;
    vec3 *obj_point;
    vec2 *img_point;

    obj_point = &obj_points.gl_obj_points_f[0];
    img_point = &tex_coords.gl_tex_coord_f[0];
    count = obj_points.gl_obj_points_f.size();

    ProjectPoints(count, obj_point, front_cam_params.mr,
                  front_cam_params.mt, front_cam_params.mi,
                  front_cam_params.md, (void *)img_point, 1);

    obj_points.gl_obj_points_f.clear();

    // front left blend front part
    obj_point = &obj_points.gl_obj_points_fl_f[0];
    img_point = &tex_coords.gl_tex_coord_fl_f[0];
    count = obj_points.gl_obj_points_fl_f.size();

    ProjectPoints(count, obj_point, front_cam_params.mr,
                  front_cam_params.mt, front_cam_params.mi,
                  front_cam_params.md, (void *)img_point, 1);
    obj_points.gl_obj_points_fl_f.clear();

    // front right blend front part
    obj_point = &obj_points.glObjPoints_FR_F[0];
    img_point = &tex_coords.gl_tex_coord_fr_f[0];
    count = obj_points.glObjPoints_FR_F.size();

    ProjectPoints(count, obj_point, front_cam_params.mr,
                  front_cam_params.mt, front_cam_params.mi,
                  front_cam_params.md, (void *)img_point, 1);
    obj_points.glObjPoints_FR_F.clear();

    // rear part
    obj_point = &obj_points.gl_obj_points_b[0];
    img_point = &tex_coords.gl_tex_coord_b[0];
    count = obj_points.gl_obj_points_b.size();

    ProjectPoints(count, obj_point, rear_cam_params.mr,
                  rear_cam_params.mt, rear_cam_params.mi,
                  rear_cam_params.md, (void *)img_point, 1);

    obj_points.gl_obj_points_b.clear();

    // rear left blend rear part
    obj_point = &obj_points.glObjPoints_BL_B[0];
    img_point = &tex_coords.gl_tex_coord_bl_b[0];
    count = obj_points.glObjPoints_BL_B.size();

    ProjectPoints(count, obj_point, rear_cam_params.mr,
                  rear_cam_params.mt, rear_cam_params.mi,
                  rear_cam_params.md, (void *)img_point, 1);

    obj_points.glObjPoints_BL_B.clear();

    // rear right blend rear part
    obj_point = &obj_points.glObjPoints_BR_B[0];
    img_point = &tex_coords.gl_tex_coord__br_b[0];
    count = obj_points.glObjPoints_BR_B.size();

    ProjectPoints(count, obj_point, rear_cam_params.mr,
                  rear_cam_params.mt, rear_cam_params.mi,
                  rear_cam_params.md, (void *)img_point, 1);

    obj_points.glObjPoints_BR_B.clear();

    // left cam
    obj_point = &obj_points.gl_obj_points_l[0];
    img_point = &tex_coords.gl_tex_coord_l[0];
    count = obj_points.gl_obj_points_l.size();

    ProjectPoints(count, obj_point, left_cam_params.mr,
                  left_cam_params.mt, left_cam_params.mi,
                  left_cam_params.md, (void *)img_point, 1);

    obj_points.gl_obj_points_l.clear();

    // left front blend left part
    obj_point = &obj_points.glObjPoints_FL_L[0];
    img_point = &tex_coords.gl_tex_coord_fl_l[0];
    count = obj_points.glObjPoints_FL_L.size();

    ProjectPoints(count, obj_point, left_cam_params.mr,
                  left_cam_params.mt, left_cam_params.mi,
                  left_cam_params.md, (void *)img_point, 1);

    obj_points.glObjPoints_FL_L.clear();

    // left rear blend left part
    obj_point = &obj_points.glObjPoints_BL_L[0];
    img_point = &tex_coords.gl_tex_coord_bl_l[0];
    count = obj_points.glObjPoints_BL_L.size();

    ProjectPoints(count, obj_point, left_cam_params.mr,
                  left_cam_params.mt, left_cam_params.mi,
                  left_cam_params.md, (void *)img_point, 1);

    obj_points.glObjPoints_BL_L.clear();

    // right cam
    obj_point = &obj_points.gl_obj_points_r[0];
    img_point = &tex_coords.gl_tex_coord_r[0];
    count = obj_points.gl_obj_points_r.size();

    ProjectPoints(count, obj_point, right_cam_params.mr,
                  right_cam_params.mt, right_cam_params.mi,
                  right_cam_params.md, (void *)img_point, 1);

    obj_points.gl_obj_points_r.clear();

    // right front blend right part
    obj_point = &obj_points.glObjPoints_FR_R[0];
    img_point = &tex_coords.gl_tex_coord_fr_r[0];
    count = obj_points.glObjPoints_FR_R.size();

    ProjectPoints(count, obj_point, right_cam_params.mr,
                  right_cam_params.mt, right_cam_params.mi,
                  right_cam_params.md, (void *)img_point, 1);

    obj_points.glObjPoints_FR_R.clear();

    // right rear blend right part
    obj_point = &obj_points.glObjPoints_BR_R[0];
    img_point = &tex_coords.gl_tex_coord__br_r[0];
    count = obj_points.glObjPoints_BR_R.size();

    ProjectPoints(count, obj_point, right_cam_params.mr,
                  right_cam_params.mt, right_cam_params.mi,
                  right_cam_params.md, (void *)img_point, 1);

    obj_points.glObjPoints_BR_R.clear();
}

void GetLumiaCountPosition()
{
    int count;
    vec3 *obj_points;
    int *img_points;

    // front left blend front part
    obj_points = &obj_points_statistics.gl_obj_points_fl_f[0];
    img_points = &tex_coords_statistics.gl_tex_coord_fl_f[0];
    count = obj_points_statistics.gl_obj_points_fl_f.size();

    ProjectPoints(count, obj_points, front_cam_params.mr,
                   front_cam_params.mt, front_cam_params.mi,
                   front_cam_params.md, (void *)img_points, 0);

    obj_points_statistics.gl_obj_points_fl_f.clear();

    // front right blend front part
    obj_points = &obj_points_statistics.glObjPoints_FR_F[0];
    img_points = &tex_coords_statistics.gl_tex_coord_fr_f[0];
    count = obj_points_statistics.glObjPoints_FR_F.size();

    ProjectPoints(count, obj_points, front_cam_params.mr,
                   front_cam_params.mt, front_cam_params.mi,
                   front_cam_params.md, (void *)img_points, 0);

    obj_points_statistics.glObjPoints_FR_F.clear();

    // front left blend front part
    obj_points = &obj_points_statistics.glObjPoints_BL_B[0];
    img_points = &tex_coords_statistics.gl_tex_coord_bl_b[0];
    count = obj_points_statistics.glObjPoints_BL_B.size();

    ProjectPoints(count, obj_points, rear_cam_params.mr,
                   rear_cam_params.mt, rear_cam_params.mi,
                   rear_cam_params.md, (void *)img_points, 0);

    obj_points_statistics.glObjPoints_BL_B.clear();

    // front right blend front part
    obj_points = &obj_points_statistics.glObjPoints_BR_B[0];
    img_points = &tex_coords_statistics.gl_tex_coord__br_b[0];
    count = obj_points_statistics.glObjPoints_BR_B.size();

    ProjectPoints(count, obj_points, rear_cam_params.mr,
                   rear_cam_params.mt, rear_cam_params.mi,
                   rear_cam_params.md, (void *)img_points, 0);

    obj_points_statistics.glObjPoints_BR_B.clear();

    // front left blend front part
    obj_points = &obj_points_statistics.glObjPoints_FL_L[0];
    img_points = &tex_coords_statistics.gl_tex_coord_fl_l[0];
    count = obj_points_statistics.glObjPoints_FL_L.size();

    ProjectPoints(count, obj_points, left_cam_params.mr,
                   left_cam_params.mt, left_cam_params.mi,
                   left_cam_params.md, (void *)img_points, 0);

    obj_points_statistics.glObjPoints_FL_L.clear();

    // front right blend front part
    obj_points = &obj_points_statistics.glObjPoints_BL_L[0];
    img_points = &tex_coords_statistics.gl_tex_coord_bl_l[0];
    count = obj_points_statistics.glObjPoints_BL_L.size();

    ProjectPoints(count, obj_points, left_cam_params.mr,
                   left_cam_params.mt, left_cam_params.mi,
                   left_cam_params.md, (void *)img_points, 0);

    obj_points_statistics.glObjPoints_BL_L.clear();

    // front left blend front part
    obj_points = &obj_points_statistics.glObjPoints_FR_R[0];
    img_points = &tex_coords_statistics.gl_tex_coord_fr_r[0];
    count = obj_points_statistics.glObjPoints_FR_R.size();

    ProjectPoints(count, obj_points, right_cam_params.mr,
                   right_cam_params.mt, right_cam_params.mi,
                   right_cam_params.md, (void *)img_points, 0);

    obj_points_statistics.glObjPoints_FR_R.clear();

    // front right blend front part
    obj_points = &obj_points_statistics.glObjPoints_BR_R[0];
    img_points = &tex_coords_statistics.gl_tex_coord__br_r[0];
    count = obj_points_statistics.glObjPoints_BR_R.size();

    ProjectPoints(count, obj_points, right_cam_params.mr,
                   right_cam_params.mt, right_cam_params.mi,
                   right_cam_params.md, (void *)img_points, 0);

    obj_points_statistics.glObjPoints_BR_R.clear();
}

void InitVBO()
{

    glGenBuffers(4, vbo_mosaic_image.cam_vertices_points);
    glGenBuffers(4, vbo_mosaic_image.cam_texture_points);

    glGenBuffers(4, vbo_mosaic_image.fuse_cam_vertices_points);
    glGenBuffers(2, vbo_mosaic_image.fuse_fl_cam_texture_points);
    glGenBuffers(2, vbo_mosaic_image.fuse_fr_cam_texture_points);
    glGenBuffers(2, vbo_mosaic_image.fuse_bl_cam_texture_points);
    glGenBuffers(2, vbo_mosaic_image.fuse_br_cam_texture_points);

    glGenBuffers(4, vbo_mosaic_image.lumia_balance);
    glGenBuffers(4, vbo_mosaic_image.alpha);
    glGenBuffers(2, vbo_mosaic_image.car_ver_tex_coord);




    glBindBuffer(GL_ARRAY_BUFFER,
                 vbo_mosaic_image.cam_vertices_points[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 tex_coords.gl_tex_coord_f.size() * sizeof(vec3),
                 &vertex_coords.glVertex_F[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 vbo_mosaic_image.cam_vertices_points[1]);
    glBufferData(GL_ARRAY_BUFFER,
                 tex_coords.gl_tex_coord_b.size() * sizeof(vec3),
                 &vertex_coords.glVertex_B[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 vbo_mosaic_image.cam_vertices_points[2]);
    glBufferData(GL_ARRAY_BUFFER,
                 tex_coords.gl_tex_coord_l.size() * sizeof(vec3),
                 &vertex_coords.glVertex_L[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 vbo_mosaic_image.cam_vertices_points[3]);
    glBufferData(GL_ARRAY_BUFFER,
                 tex_coords.gl_tex_coord_r.size() * sizeof(vec3),
                 &vertex_coords.glVertex_R[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.cam_texture_points[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 tex_coords.gl_tex_coord_f.size() * sizeof(vec2),
                 &tex_coords.gl_tex_coord_f[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.cam_texture_points[1]);
    glBufferData(GL_ARRAY_BUFFER,
                 tex_coords.gl_tex_coord_b.size() * sizeof(vec2),
                 &tex_coords.gl_tex_coord_b[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.cam_texture_points[2]);
    glBufferData(GL_ARRAY_BUFFER,
                 tex_coords.gl_tex_coord_l.size() * sizeof(vec2),
                 &tex_coords.gl_tex_coord_l[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.cam_texture_points[3]);
    glBufferData(GL_ARRAY_BUFFER,
                 tex_coords.gl_tex_coord_r.size() * sizeof(vec2),
                 &tex_coords.gl_tex_coord_r[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 vbo_mosaic_image.fuse_cam_vertices_points[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertex_coords.glVertex_FL.size() * sizeof(vec3),
                 &vertex_coords.glVertex_FL[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 vbo_mosaic_image.fuse_cam_vertices_points[1]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertex_coords.glVertex_FR.size() * sizeof(vec3),
                 &vertex_coords.glVertex_FR[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 vbo_mosaic_image.fuse_cam_vertices_points[2]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertex_coords.glVertex_BL.size() * sizeof(vec3),
                 &vertex_coords.glVertex_BL[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 vbo_mosaic_image.fuse_cam_vertices_points[3]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertex_coords.glVertex_BR.size() * sizeof(vec3),
                 &vertex_coords.glVertex_BR[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 vbo_mosaic_image.fuse_fl_cam_texture_points[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertex_coords.glVertex_FL.size() * sizeof(vec2),
                 &tex_coords.gl_tex_coord_fl_f[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 vbo_mosaic_image.fuse_fl_cam_texture_points[1]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertex_coords.glVertex_FL.size() * sizeof(vec2),
                 &tex_coords.gl_tex_coord_fl_l[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 vbo_mosaic_image.fuse_fr_cam_texture_points[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertex_coords.glVertex_FR.size() * sizeof(vec2),
                 &tex_coords.gl_tex_coord_fr_f[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 vbo_mosaic_image.fuse_fr_cam_texture_points[1]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertex_coords.glVertex_FR.size() * sizeof(vec2),
                 &tex_coords.gl_tex_coord_fr_r[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 vbo_mosaic_image.fuse_bl_cam_texture_points[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertex_coords.glVertex_BL.size() * sizeof(vec2),
                 &tex_coords.gl_tex_coord_bl_b[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 vbo_mosaic_image.fuse_bl_cam_texture_points[1]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertex_coords.glVertex_BL.size() * sizeof(vec2),
                 &tex_coords.gl_tex_coord_bl_l[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 vbo_mosaic_image.fuse_br_cam_texture_points[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertex_coords.glVertex_BR.size() * sizeof(vec2),
                 &tex_coords.gl_tex_coord__br_b[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,
                 vbo_mosaic_image.fuse_br_cam_texture_points[1]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertex_coords.glVertex_BR.size() * sizeof(vec2),
                 &tex_coords.gl_tex_coord__br_r[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.lumia_balance[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 lumia_adjust.glLumiaAdjust_F.size() * sizeof(float),
                 &lumia_adjust.glLumiaAdjust_F[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.lumia_balance[1]);
    glBufferData(GL_ARRAY_BUFFER,
                 lumia_adjust.glLumiaAdjust_B.size() * sizeof(float),
                 &lumia_adjust.glLumiaAdjust_B[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.lumia_balance[2]);
    glBufferData(GL_ARRAY_BUFFER,
                 lumia_adjust.glLumiaAdjust_L.size() * sizeof(float),
                 &lumia_adjust.glLumiaAdjust_L[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.lumia_balance[3]);
    glBufferData(GL_ARRAY_BUFFER,
                 lumia_adjust.glLumiaAdjust_R.size() * sizeof(float),
                 &lumia_adjust.glLumiaAdjust_R[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.alpha[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertex_coords.glVertex_FL.size() * sizeof(float),
                 &blend_alpha.glAlpha_FL[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.alpha[1]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertex_coords.glVertex_FR.size() * sizeof(float),
                 &blend_alpha.glAlpha_FR[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.alpha[2]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertex_coords.glVertex_BL.size() * sizeof(float),
                 &blend_alpha.glAlpha_BL[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.alpha[3]);
    glBufferData(GL_ARRAY_BUFFER,
                 vertex_coords.glVertex_BR.size() * sizeof(float),
                 &blend_alpha.glAlpha_BR[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.car_ver_tex_coord[0]);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), gl_vertices_car,
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_mosaic_image.car_ver_tex_coord[1]);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), gl_tex_coord_car,
                 GL_STATIC_DRAW);
}

void SetCameraParams(camParams *scm)
{
    int j;
    float r_vec[3];
    for (j = 0; j < 3; j++)
    {
        r_vec[j] = static_cast<float>(scm->mr_int[j]) / SCALE3;
        scm->mt[j] = static_cast<float>(scm->mt_int[j]) / SCALE2;
    }

    RotateVectorToRotateMatrix(r_vec, scm->mr);

    for (j = 0; j < 4; j++)
    {
        scm->md[j] = static_cast<float>(scm->mimd_int[4 + j]) / SCALE1;
    }

    scm->mi[0] = static_cast<float>(scm->mimd_int[0]) / SCALE2;
    scm->mi[1] = 0.0;
    scm->mi[2] = static_cast<float>(scm->mimd_int[2]) / SCALE2;
    scm->mi[3] = 0.0;
    scm->mi[4] = static_cast<float>(scm->mimd_int[1]) / SCALE2;
    scm->mi[5] = static_cast<float>(scm->mimd_int[3]) / SCALE2;
    scm->mi[6] = 0.0;
    scm->mi[7] = 0.0;
    scm->mi[8] = 1.0;
}

void InitCamParaData()
{
    parking_assistant_params.chessboard_width_corners = 5;
    parking_assistant_params.chessboard_length_corners = 7;
    parking_assistant_params.square_size = 20;

    ReadParamsXML();

    SetCameraParams(&front_cam_params);
    SetCameraParams(&rear_cam_params);
    SetCameraParams(&left_cam_params);
    SetCameraParams(&right_cam_params);
}
