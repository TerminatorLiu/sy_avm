#ifndef _SY_APP_CONF_H
#define _SY_APP_CONF_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <getopt.h>

#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <asm/types.h>

#include <linux/types.h>
#include <linux/videodev2.h>
#include <sys/time.h>
/*
t4qvY9DfBeE@jimu
mkdir /liutao_mnt
mount -t nfs -o nolock 192.168.1.101:/home/lt/liutao_mnt /liutao_mnt
*/
#include <semaphore.h>
#include <stdint.h>
#define STATIC_DEBUG (0)
#define CHASSIS_TYPE (0) /*0:Fuel 1:Electric 2:Heavy Truck*/ 
#define CHASSIS_TYPE_FUEL (0)
#define CHASSIS_TYPE_ELECTRIC (1)
#define CHASSIS_TYPE_CHANGE_BATTERY (2)
#define BSD_TYPE (1) /*0:jimu 1:Cheng Tech*/
#define BSD_TYPE_JIMU (0)
#define BSD_TYPE_CT (1)
#define REV_RADAR (0) 
#define REV_RADAR_NONE (0) 
#define REV_RADAR_SHUNHE (1) 
#define ENABLE_DISPLAYMODE_SWITCH (0)
#define ENABLE_ROTATE (0)
#define DISABLE_LEFT_FLIP (0)
#define ENABLE_LEFT_FLIP (1)
#define DISABLE_RIGHT_FLIP (0)
#define ENABLE_RIGHT_FLIP (1)
#define VECHICLE_TYPE_DUMP (0)
#define VECHICLE_TYPE_MIXER (1)
#define ENABLE_DMS (0)
#if CHASSIS_TYPE == CHASSIS_TYPE_FUEL
#define CAMERA_BACK_DEV "/dev/video0"
#define CAMERA_RIGTH_DEV "/dev/video1"
#define CAMERA_FRONT_DEV "/dev/video2"
#define CAMERA_LEFT_DEV "/dev/video3"
#define CAMERA_DMS_DEV "/dev/video4"
#define CAMERA_CONTAINER_DEV "/dev/video5"
#endif
#if CHASSIS_TYPE == CHASSIS_TYPE_CHANGE_BATTERY
#define CAMERA_BACK_DEV "/dev/video2"
#define CAMERA_RIGTH_DEV "/dev/video1"//0
#define CAMERA_FRONT_DEV "/dev/video0"//2
#define CAMERA_LEFT_DEV "/dev/video3"
#define CAMERA_DMS_DEV "/dev/video4"
#define CAMERA_CONTAINER_DEV "/dev/video5"
// #define CAMERA_BACK_DEV "/dev/video0"
// #define CAMERA_RIGTH_DEV "/dev/video1"//0
// #define CAMERA_FRONT_DEV "/dev/video2"//2
// #define CAMERA_LEFT_DEV "/dev/video3"
// #define CAMERA_DMS_DEV "/dev/video4"
// #define CAMERA_CONTAINER_DEV "/dev/video5"
#endif

#define CAMERA_FRONT_DEV_ID 0
#define CAMERA_BACK_DEV_ID 1
#define CAMERA_LEFT_DEV_ID 2
#define CAMERA_RIGTH_DEV_ID 3
#define CAMERA_DMS_DEV_ID 4
#define CAMERA_CONTAINER_DEV_ID 5

#define FRAME_NUM 4
#define SY_V4L2_BUF_TYPE (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)

#define CAMERA_WIDTH 1280
#define CAMERA_HEIGHT 720

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define OPENGL_WINDOW_VERSION 2

struct map_buffer {
  void *start;
  int32_t length;
  int64_t timestamp;
};

struct current_camera_buffer {
  void *addr;
  int32_t length;
  int64_t timestamp;
};

struct render_parameter {
  struct current_camera_buffer *camerabuf;
  int32_t cameralen;
  int32_t mode;
};

struct sync_flag {
  uint32_t flag;
  sem_t sem;
};

typedef struct vin_interface {
  int32_t fd;
  char *devname;
  /* data */
  struct map_buffer mapbuf[FRAME_NUM];
  struct timeval tv;
  int32_t nplanes;
} v4l2_op_interface;

struct yuv_flip_buf {
  void *left_addr;
  void *right_addr;
};

int GetFilpLeft();

int SetFlipLeft(int desired);

int GetFilpRight();

int SetFlipRight(int desired);

int GetVehicleType();

int SetVehicleTypet(int desired);

int InitConf();

#endif
