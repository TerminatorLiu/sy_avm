#ifndef OPENGL_COMMON_H
#define OPENGL_COMMON_H
#include "common.h"
//显示窗口尺寸大小
typedef struct _safImgRect
{
    int x;       
    int y;       
    int width;    
    int height;   
}safImgRect;

#if 0


//视图模式
typedef enum _viewMode
{
	VIEW_OVERALL = 0,   //全景图（全景图+四个田字格）
	VIEW_LEFT,                   //左视图（点击左边按钮，左转向灯，左转方向盘）
	VIEW_RIGHT,                //右视图 （点击右边按钮，右转向灯，右转方向盘）
	VIEW_BACK,                  //后视图  （点击后面按钮）
	VIEW_FRONT,               //前视图 （点击前面视图按钮）
	VIEW_BACKWARD       //倒车
}viewMode;

#endif

#endif