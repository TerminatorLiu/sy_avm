#ifndef _DISPLAY_MODE_H
#define _DISPLAY_MODE_H
#include <stdint.h>

#define FRONT_DISPALY_MODE 0x10

#define LEFT_DISPLAY_MODE1 0x11
#define LEFT_DISPLAY_MODE2 0x19

#define BACK_DISPLAY_MODE1 0x12
#define BACK_DISPLAY_MODE2 0x07

#define RIGHT_DISPLAY_MODE1 0x13
#define RIGHT_DISPLAY_MODE2 0x1B

#define CONTRAINER_DISPLAY_MODE 0x15

#define DMS_DISPLAY_MODE 0x14

#define FRONT_LEFT_MODE 0x30
#define FRONT_RIGHT_MODE 0x38
#define BACK_LEFT_MODE 0x32
#define BACK_RIGHT_MODE 0x3A

#define ORIGIN_VIEW_AVM_MODE 0x60
#define ORIGIN_VIEW_FRONT_MODE 0x61
#define ORIGIN_VIEW_LEFT_MODE 0x62
#define ORIGIN_VIEW_BACK_MODE 0x63
#define ORIGIN_VIEW_RIGHT_MODE 0x64
#define ORIGIN_VIEW_DMS_MODE 0x65
#define ORIGIN_VIEW_CONTAINER_MODE 0x66

/**
 * @brief get camera render mode.
 **/
int GetDisplayMode() ;

/**
 * @brief update 360 surroud mode.
 * @param display mode.
 **/
int SetDisplayMode(uint8_t mode) ;

int InitDisplayMode();
int DestroyDisplayMode();

int32_t InitDisplayModeSwitch();
#endif