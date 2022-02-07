#ifndef _SY_APP_RENDER_H
#define _SY_APP_RENDER_H
#include "conf.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


int InitOpenglesWindows();
int InitWindow();
void RenderWindow(struct render_parameter *param);



#define REVERSE_TRAJECTORY_COE (1.f)//23.6


#ifdef __cplusplus
}
#endif

#endif