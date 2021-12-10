#ifndef _OPENGL_COMMON_H
#define _OPENGL_COMMON_H



#if 0
typedef struct _safImgRect
{
    int x;       
    int y;       
    int width;    
    int height;   
} safImgRect;
typedef enum _viewMode
{
	VIEW_FRONT = 0,
	VIEW_BACK_UP,
	VIEW_LEFT,
	VIEW_RIGHT,
	VIEW_BACK_DOWN,
	VIEW_FOUR_CAMERA
}viewMode;
#else
typedef enum _ViewState
{
    VIEW_OVERALL = 0,  
    VIEW_LEFT,  
    VIEW_RIGHT, 
    VIEW_BACK, 
    VIEW_FRONT, 
    VIEW_UNDISTORT_BACK,
    VIEW_UNDISTORT_FRONT,
    VIEW_DMS,
    VIEW_CONTAINER
}ViewState;
#endif


typedef enum _carState
{
    EMPTY = 0,   //$)Ag):h==
    HEAVY       //$)Af;!h==
}carState;

#endif
