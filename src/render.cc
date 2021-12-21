
#include <stdio.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <EGL/eglext.h>
#include <fbdev_window.h>
#include <sys/time.h>

#include "render.h"
#include "displaymode.h"

#include "conf.h"
#include "dms.h"
#include "filecamera.h"
#include "linux-serial-app.h"
#include "mcuparse.h"

#include "stb_image_write.h"

#if OPENGL_WINDOW_VERSION > 1
#include "avmInit.hpp"
#include "cv.h"
#include "sy_log.h"

#define IMGWIDTH IMAGE_WIDTH
#define IMGHEIGHT IMAGE_HEIGHT

#else
#include "InitMosaic.hpp"
#include "common.h"
#endif

extern int AvmInit(safImgRect allView, safImgRect singleView);
extern void RunRender(int viewMode, float steeringWheelAngle);
extern void UpdateTexture(unsigned char **src);
extern void ShowFullScreeen();

extern int g_yuv_org_flag;
extern struct yuv_flip_buf g_yuv_flip_buf;
extern struct yuv_org_data g_yuv_org_data;

static EGLint const config_attribute_list[] = {EGL_RED_SIZE,
                                               8,
                                               EGL_GREEN_SIZE,
                                               8,
                                               EGL_BLUE_SIZE,
                                               8,
                                               EGL_ALPHA_SIZE,
                                               8,
                                               EGL_BUFFER_SIZE,
                                               32,

                                               EGL_STENCIL_SIZE,
                                               0,
                                               EGL_DEPTH_SIZE,
                                               0,

                                               EGL_SAMPLES,
                                               4,

                                               EGL_RENDERABLE_TYPE,
                                               EGL_OPENGL_ES3_BIT,

                                               EGL_SURFACE_TYPE,
                                               EGL_WINDOW_BIT | EGL_PIXMAP_BIT,

                                               EGL_NONE};

static EGLint window_attribute_list[] = {EGL_NONE};

static const EGLint context_attribute_list[] = {EGL_CONTEXT_CLIENT_VERSION, 2,
                                                EGL_NONE};

EGLDisplay egl_display;
EGLSurface egl_surface;
GLuint egl_program;
#if STATIC_DEBUG
unsigned char* gStaticDebugBuffer[4] = {0};
int InitStaticDebug()
{
  for(int i = 0;i<4;++i)
  {
    gStaticDebugBuffer[i] = (unsigned char*)malloc(1382400);
    memset(gStaticDebugBuffer[i],128,1382400);
    char filename[128] = {0};
    sprintf(filename,"test/%d.YUV",i);
    FILE *fp = fopen(filename,"rb");
    if(fp == NULL)
    {
      printf("can not load %s\n",filename);
    }
    
    fread(gStaticDebugBuffer[i],1,921600,fp);
    fclose(fp);
  }

}
#endif


/**
 * @brief  Initialize opengles running environment.
 **/
int InitOpenglesWindows()
{
  EGLint egl_major, egl_minor;
  EGLConfig config;
  EGLint num_config;
  EGLContext context;
  GLint width, height;

  egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (egl_display == EGL_NO_DISPLAY)
  {
    fprintf(stderr, "Error: No display found!\n");
    return -1;
  }

  if (!eglInitialize(egl_display, &egl_major, &egl_minor))
  {
    fprintf(stderr, "Error: eglInitialise failed!\n");
    return -1;
  }

  printf("egl major:%d, minor:%d\r\n", egl_major, egl_minor);

  eglChooseConfig(egl_display, config_attribute_list, &config, 1, &num_config);

  context = eglCreateContext(egl_display, config, EGL_NO_CONTEXT,
                             context_attribute_list);

  if (context == EGL_NO_CONTEXT)
  {
    printf("Error: eglCreateContext failed: 0x%08X\n", eglGetError());
    return -1;
  }

#if 1
  egl_surface =
      eglCreateWindowSurface(egl_display, config, 0, window_attribute_list);
#endif

  if (egl_surface == EGL_NO_SURFACE)
  {
    printf(
        "Error: eglCreateWindowSurface failed: "
        "0x%08X\n",
        eglGetError());
    return -1;
  }

  if (!eglQuerySurface(egl_display, egl_surface, EGL_WIDTH, &width) ||
      !eglQuerySurface(egl_display, egl_surface, EGL_HEIGHT, &height))
  {
    printf("Error: eglQuerySurface failed: 0x%08X\n", eglGetError());
    return -1;
  }

  printf("display width:%d, height:%d\r\n", width, height);

  if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, context))
  {
    printf("Error: eglMakeCurrent() failed: 0x%08X\n", eglGetError());
    return -1;
  }
  eglSwapInterval(egl_display, 3);
  return 0;
}

/**
 * @brief  configure 360surroud display parameter
 **/
int InitWindow()
{
  safImgRect allView, singleView;

  allView.width = 360;
  allView.height = 720;
  allView.x = 920;
  allView.y = 0;

  singleView.width = 920; // 288;
  singleView.height = 720;
  singleView.x = 0;
  singleView.y = 0;

#if OPENGL_WINDOW_VERSION > 1
  AvmInit(allView, singleView);
#else
  initMosaic(allView, singleView, EMPTY);
#endif
#if STATIC_DEBUG
  InitStaticDebug();
#endif
}

/**
 * @brief rotate yuv421 data 90 degree
 * @param src orgin yun421 data
 * @param dst rotate 90 degree yuv421 data
 **/

void ImageFilp(unsigned char *src, unsigned char *dst)
{
  unsigned char *pSrcY, *pSrcU, *pSrcV;
  unsigned char *pDstY, *pDstU, *pDstV;
  CvMat srcY, srcU, srcV;
  CvMat dstY, dstU, dstV;

  pSrcY = src;
  pSrcU = pSrcY + IMGWIDTH * IMGHEIGHT;
  pSrcV = pSrcU + IMGWIDTH * IMGHEIGHT / 4;

  pDstY = dst;
  pDstU = pDstY + IMGWIDTH * IMGHEIGHT;
  pDstV = pDstU + IMGWIDTH * IMGHEIGHT / 4;

  srcY = cvMat(IMGWIDTH, IMGHEIGHT, CV_8UC1, pSrcY);
  srcU = cvMat(IMGWIDTH / 2, IMGHEIGHT / 2, CV_8UC1, pSrcU);
  srcV = cvMat(IMGWIDTH / 2, IMGHEIGHT / 2, CV_8UC1, pSrcV);

  dstY = cvMat(IMGWIDTH, IMGHEIGHT, CV_8UC1, pDstY);
  dstU = cvMat(IMGWIDTH / 2, IMGHEIGHT / 2, CV_8UC1, pDstU);
  dstV = cvMat(IMGWIDTH / 2, IMGHEIGHT / 2, CV_8UC1, pDstV);

  cvFlip(&srcY, &dstY, -1);
  cvFlip(&srcU, &dstU, -1);
  cvFlip(&srcV, &dstV, -1);
}

/**
 * @brief save camera orgin data to file
 * @param buf   camera data address and length
 * @param nlen  number of cameras to be saved
 *
 */
void SaveFile(struct current_camera_buffer *buf, int32_t nlen)
{
  char filepath[255] = {0};
  printf("func:%s\r\n", __FUNCTION__);
  for (int i = 0; i < nlen; ++i)
  {
    snprintf(filepath, sizeof(filepath), "./test/jiaobanche/aa%d.YUV", i);
    FILE *fp = NULL;
    fp = fopen(filepath, "w");
    if (fp != NULL)
    {
      fwrite(buf[i].addr, buf[i].length, 1, fp);
      fclose(fp);
    }
  }
}

/**
 * @brief refresh camera data to screen.
 * @param param 6 channel camera data and render mode
 **/

void RenderWindow(struct render_parameter *param)
{
  unsigned char *psrc[5] = {0};
  float steer = GetSteerAngle();
  const char *switchmode = "unknown";

  for (int i = 0; i < 4; ++i)
  {
    psrc[i] = (unsigned char *)param->camerabuf[i].addr;
  }

  struct timeval tv, tvend;

  if (g_yuv_flip_buf.left_addr)
  {
#if ENABLE_ROTATE
    ImageFilp((unsigned char *)param->camerabuf[CAMERA_LEFT_DEV_ID].addr,
              (unsigned char *)g_yuv_flip_buf.left_addr);
    psrc[CAMERA_LEFT_DEV_ID] = (unsigned char *)g_yuv_flip_buf.left_addr;
    // memcpy((unsigned char *)param->camerabuf[CAMERA_LEFT_DEV_ID].addr,
    //&psrc[CAMERA_LEFT_DEV_ID], param->camerabuf[CAMERA_LEFT_DEV_ID].length);
#endif
  }

  if (g_yuv_flip_buf.right_addr)
  {
#if ENABLE_ROTATE
    ImageFilp((unsigned char *)param->camerabuf[CAMERA_RIGTH_DEV_ID].addr,
              (unsigned char *)g_yuv_flip_buf.right_addr);
    psrc[CAMERA_RIGTH_DEV_ID] = (unsigned char *)g_yuv_flip_buf.right_addr;
    // memcpy((unsigned char *)param->camerabuf[CAMERA_RIGTH_DEV_ID].addr,
    //&psrc[CAMERA_RIGTH_DEV_ID], param->camerabuf[CAMERA_RIGTH_DEV_ID].length);
#endif
  }

  // printf(" mode : %d, timestame:%ld\r\n",  renderparam->mode, timestamp);
  // renderwindow(renderparam);

  // gettimeofday(&tvend, NULL);
  // int64_t timestampend = tvend.tv_sec * 1000 + tvend.tv_usec / 1000;
  // printf("time spend:%d\r\n", timestampend - timestamp);

  // g_yuv_org_data
  if (g_yuv_org_flag)
  {
    psrc[CAMERA_FRONT_DEV_ID] = (unsigned char *)g_yuv_org_data.frontaddr;
    psrc[CAMERA_BACK_DEV_ID] = (unsigned char *)g_yuv_org_data.backaddr;
    psrc[CAMERA_LEFT_DEV_ID] = (unsigned char *)g_yuv_org_data.leftaddr;
    psrc[CAMERA_RIGTH_DEV_ID] = (unsigned char *)g_yuv_org_data.rightaddr;
  }
#if STATIC_DEBUG
    psrc[CAMERA_FRONT_DEV_ID] = gStaticDebugBuffer[0];
    psrc[CAMERA_BACK_DEV_ID] =  gStaticDebugBuffer[1];
    psrc[CAMERA_LEFT_DEV_ID] =  gStaticDebugBuffer[2];
    psrc[CAMERA_RIGTH_DEV_ID] = gStaticDebugBuffer[3];
#endif
  // printf("enter render\r\n");
  UpdateDmsData(psrc[CAMERA_DMS_DEV_ID]);
  gettimeofday(&tv, NULL);
  int64_t timestamp = tv.tv_sec * 1000 + tv.tv_usec / 1000;
#ifdef SCREEN_ROTATE
  switch (GetDisplayMode())
  {
  case FRONT_LEFT_MODE:
    switchmode = "VIEW_FRONT_LEFT";
    psrc[4] = (unsigned char *)param->camerabuf[CAMERA_FRONT_DEV_ID].addr;
    param->mode = VIEW_FRONT_LEFT;
    UpdateTexture(psrc); //
    RunRender(
        param->mode,
        steer / REVERSE_TRAJECTORY_COE); /*Reverse trajectory coefficient */

    break;

  case FRONT_RIGHT_MODE:
    switchmode = "VIEW_FRONT_RIGHT";
    psrc[4] = (unsigned char *)param->camerabuf[CAMERA_LEFT_DEV_ID].addr;
    param->mode = VIEW_FRONT_RIGHT;
    UpdateTexture(psrc);
    RunRender(param->mode, steer / REVERSE_TRAJECTORY_COE);
    break;

  case BACK_LEFT_MODE:
    switchmode = "VIEW_BACK_LEFT";
    param->mode = VIEW_BACK_LEFT;
    psrc[4] = (unsigned char *)param->camerabuf[CAMERA_BACK_DEV_ID].addr;
    UpdateTexture(psrc);
    RunRender(param->mode, steer / REVERSE_TRAJECTORY_COE);

    break;

  case BACK_RIGHT_MODE:
    switchmode = "VIEW_BACK_RIGHT";
    psrc[4] = (unsigned char *)param->camerabuf[CAMERA_RIGTH_DEV_ID].addr;
    UpdateTexture(psrc);
    param->mode = VIEW_BACK_RIGHT;
    RunRender(param->mode, steer / REVERSE_TRAJECTORY_COE);
    break;
  case ORIGIN_VIEW_AVM_MODE:
    switchmode = "ORIGIN_VIEW_AVM_MODE";
    UpdateTexture(psrc);
    break;
  case ORIGIN_VIEW_BACK_MODE:
    switchmode = "ORIGIN_VIEW_BACK_MODE";
    UpdateTexture(psrc);
    break;
  case ORIGIN_VIEW_FRONT_MODE:
    switchmode = "ORIGIN_VIEW_FRONT_MODE";
    UpdateTexture(psrc);
    break;
  case ORIGIN_VIEW_LEFT_MODE:
    switchmode = "ORIGIN_VIEW_LEFT_MODE";
    UpdateTexture(psrc);
    break;
  case ORIGIN_VIEW_RIGHT_MODE:
    switchmode = "ORIGIN_VIEW_RIGHT_MODE";
    UpdateTexture(psrc);
    break;
  case ORIGIN_VIEW_DMS_MODE:
    switchmode = "ORIGIN_VIEW_DMS_MODE";
    break;
  case ORIGIN_VIEW_CONTAINER_MODE:
    switchmode = "ORIGIN_VIEW_CONTAINER_MODE";
    break;

  default:
    switchmode = "VIEW_FRONT_RIGHT";
    psrc[4] = (unsigned char *)param->camerabuf[CAMERA_RIGTH_DEV_ID].addr;
    UpdateTexture(psrc);
    param->mode = VIEW_FRONT_RIGHT;
    RunRender(param->mode, steer / REVERSE_TRAJECTORY_COE);
    break;
  }
#else
  switch (GetDisplayMode())
  {
  case FRONT_DISPALY_MODE: /* 360surroud + front camera */
    switchmode = "FRONT_DISPALY_MODE";

    psrc[4] = (unsigned char *)param->camerabuf[CAMERA_FRONT_DEV_ID].addr;
    param->mode = VIEW_UNDISTORT_FRONT;
    UpdateTexture(psrc); //
    RunRender(
        param->mode,
        steer / REVERSE_TRAJECTORY_COE); /*Reverse trajectory coefficient */

    break;

  case LEFT_DISPLAY_MODE1: /* 360surroud + left camera */
  case LEFT_DISPLAY_MODE2:
    switchmode = "LEFT_DISPLAY_MODE";
    psrc[4] = (unsigned char *)param->camerabuf[CAMERA_LEFT_DEV_ID].addr;
    param->mode = VIEW_LEFT;
    UpdateTexture(psrc);
    RunRender(VIEW_LEFT, steer / REVERSE_TRAJECTORY_COE);
    break;

  case BACK_DISPLAY_MODE1: /* 360surroud + back camera */
  case BACK_DISPLAY_MODE2:
    switchmode = "BACK_DISPLAY_MODE";
    param->mode = VIEW_UNDISTORT_BACK;
    psrc[4] = (unsigned char *)param->camerabuf[CAMERA_BACK_DEV_ID].addr;
    UpdateTexture(psrc);
    RunRender(param->mode, steer / REVERSE_TRAJECTORY_COE);

    break;

  case RIGHT_DISPLAY_MODE1: /* 360surroud + right camera */
  case RIGHT_DISPLAY_MODE2:
    switchmode = "RIGHT_DISPLAY_MODE";
    psrc[4] = (unsigned char *)param->camerabuf[CAMERA_RIGTH_DEV_ID].addr;
    UpdateTexture(psrc);
    RunRender(VIEW_RIGHT, steer / REVERSE_TRAJECTORY_COE);
    param->mode = VIEW_RIGHT;
    break;

  case CONTRAINER_DISPLAY_MODE: /* 360surroud + container camera */
    // SaveFile(param->camerabuf, 4);
    switchmode = "CONTRAINER_DISPLAY_MODE";
    psrc[4] = (unsigned char *)param->camerabuf[CAMERA_CONTAINER_DEV_ID].addr;
    UpdateTexture(psrc);
    RunRender(VIEW_CONTAINER, steer / REVERSE_TRAJECTORY_COE);
    param->mode = VIEW_CONTAINER;
    break;

  case DMS_DISPLAY_MODE: /* 360surroud + DMS camera */
    switchmode = "DMS_DISPLAY_MODE";
    psrc[4] = (unsigned char *)param->camerabuf[CAMERA_DMS_DEV_ID].addr;
    UpdateTexture(psrc);
    RunRender(VIEW_DMS, steer / REVERSE_TRAJECTORY_COE);
    param->mode = VIEW_DMS;
    break;
  case ORIGIN_VIEW_AVM_MODE:
    switchmode = "ORIGIN_VIEW_AVM_MODE";
    UpdateTexture(psrc);
    RunRender(VIEW_OVERALL, steer / REVERSE_TRAJECTORY_COE);
    param->mode = VIEW_OVERALL;
    break;
  case ORIGIN_VIEW_BACK_MODE:
    switchmode = "ORIGIN_VIEW_BACK_MODE";
    UpdateTexture(psrc);
    RunRender(VIEW_BACK_FULL_SCREEN, steer / REVERSE_TRAJECTORY_COE);
    param->mode = VIEW_BACK_FULL_SCREEN;
    break;
  case ORIGIN_VIEW_FRONT_MODE:
    switchmode = "ORIGIN_VIEW_FRONT_MODE";
    UpdateTexture(psrc);
    RunRender(VIEW_FRONT_FULL_SCREEN, steer / REVERSE_TRAJECTORY_COE);
    param->mode = VIEW_FRONT_FULL_SCREEN;
    break;
  case ORIGIN_VIEW_LEFT_MODE:
    switchmode = "ORIGIN_VIEW_LEFT_MODE";
    UpdateTexture(psrc);
    RunRender(VIEW_LEFT_FULL_SCREEN, steer / REVERSE_TRAJECTORY_COE);
    param->mode = VIEW_LEFT_FULL_SCREEN;
    break;
  case ORIGIN_VIEW_RIGHT_MODE:
    switchmode = "ORIGIN_VIEW_RIGHT_MODE";
    UpdateTexture(psrc);
    RunRender(VIEW_RIGHT_FULL_SCREEN, steer / REVERSE_TRAJECTORY_COE);
    param->mode = VIEW_RIGHT_FULL_SCREEN;
    break;
  case ORIGIN_VIEW_DMS_MODE:
    switchmode = "ORIGIN_VIEW_DMS_MODE";
    break;
  case ORIGIN_VIEW_CONTAINER_MODE:
    switchmode = "ORIGIN_VIEW_CONTAINER_MODE";
    break;
  default:
    switchmode = "FRONT_DISPALY_MODE";

    psrc[4] = (unsigned char *)param->camerabuf[CAMERA_FRONT_DEV_ID].addr;
    param->mode = VIEW_UNDISTORT_FRONT;
    UpdateTexture(psrc); //
    RunRender(
        param->mode,
        steer / REVERSE_TRAJECTORY_COE); /*Reverse trajectory coefficient */
  }
#endif

  eglSwapBuffers(egl_display, egl_surface);

  gettimeofday(&tvend, NULL);
  int64_t timestampend = tvend.tv_sec * 1000 + tvend.tv_usec / 1000;
  // printf("time spend:%d\r\n", timestampend - timestamp);
  // printf("leave render\r\n");
  // LOGOUT_ERROR("switch mode:%s", switchmode);
}
