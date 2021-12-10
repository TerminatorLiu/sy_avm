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
#ifndef OPENGL_COMMON_H
#define OPENGL_COMMON_H


typedef struct _safImgRect {
  int x;
  int y;
  int width;
  int height;
} safImgRect;


typedef enum _viewMode {
  	VIEW_OVERALL = 0,  
  	VIEW_LEFT_FULL_SCREEN,  
    VIEW_RIGHT_FULL_SCREEN, 
    VIEW_BACK_FULL_SCREEN, 
    VIEW_FRONT_FULL_SCREEN,
    VIEW_LEFT,  
    VIEW_RIGHT, 
    VIEW_BACK, 
    VIEW_FRONT, 
    VIEW_UNDISTORT_BACK,
    VIEW_UNDISTORT_FRONT,
    VIEW_DMS,
    VIEW_CONTAINER
} viewMode;

typedef enum _vehicleStatus {
  VEHICLE_STATUS_EMPTY = 0,  
  VEHICLE_STATUS_HEAVY    
} vehicleStatus;
#endif
