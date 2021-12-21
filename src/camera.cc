#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

#include "dms.h"
#include "filecamera.h"
#include "linux-serial-app.h"
#include "render.h"
#include "sy_log.h"
#include "sy_timer.h"
#include "v4l2_op.h"
#include "hardware.h"
#include "mcuparse.h"
#include "displaymode.h"
#include "my_utility.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

struct vin_interface g_vin_conf[] = {
#ifdef SCREEN_ROTATE
    {.fd = -1, .devname = CAMERA_FRONT_DEV},
    {.fd = -1, .devname = CAMERA_RIGTH_DEV},
    {.fd = -1, .devname = CAMERA_LEFT_DEV},
    {.fd = -1, .devname = CAMERA_BACK_DEV},
    {.fd = -1, .devname = CAMERA_DMS_DEV},
    {.fd = -1, .devname = CAMERA_CONTAINER_DEV}};
#else
    {.fd = -1, .devname = CAMERA_FRONT_DEV},
    {.fd = -1, .devname = CAMERA_BACK_DEV},
    {.fd = -1, .devname = CAMERA_LEFT_DEV},
    {.fd = -1, .devname = CAMERA_RIGTH_DEV},
    {.fd = -1, .devname = CAMERA_DMS_DEV},
    {.fd = -1, .devname = CAMERA_CONTAINER_DEV}};
#endif

struct yuv_flip_buf g_yuv_flip_buf = {
    .left_addr = NULL,
    .right_addr = NULL,
};

struct current_camera_buffer
    g_current_camera_buffer[sizeof(g_vin_conf) / sizeof(g_vin_conf[0])] = {0};
struct render_parameter g_render_parameter;
struct sync_flag g_sync_flag;
pthread_t g_render_th;
struct yuv_org_data g_yuv_org_data;
int32_t g_yuv_org_flag = 0;


int ResetSaveImage();
int SaveImage();
/**
 * @brief Initialize camera related configuration parameters
 */

void InitCameraParam() {
  const int32_t narraysize = sizeof(g_vin_conf) / sizeof(g_vin_conf[0]);

  g_sync_flag.flag = 0;

  /*Initialize semaphore, for camera capature thread and render thread
   * synchronization*/
  sem_init(&g_sync_flag.sem, 0, 0);

  g_render_parameter.camerabuf = g_current_camera_buffer;
  g_render_parameter.cameralen = narraysize;

  /*Render mode setting ,can be change by control screen send a command*/
  g_render_parameter.mode = 0;
}

/**
 * @brief Memory allocation for frame rotatrion
 */
void InitYuvFlipBuffer() {
  /*buffer size is 1.5 * video size*/
  const int32_t nsize =
      CAMERA_WIDTH * CAMERA_HEIGHT + CAMERA_WIDTH * CAMERA_HEIGHT / 2;

  g_yuv_flip_buf.left_addr = malloc(nsize);
  g_yuv_flip_buf.right_addr = malloc(nsize);
  if ((NULL == g_yuv_flip_buf.left_addr) ||
      (NULL == g_yuv_flip_buf.right_addr)) {
    printf("malloc yuv flip buf failed\r\n");
  } else {
    memset(g_yuv_flip_buf.left_addr, 0, nsize);
    memset(g_yuv_flip_buf.right_addr, 0, nsize);
  }
}

/**
 * @brief release memrory
 **/

void ExitYuvFlipBuffer() {
  if (g_yuv_flip_buf.right_addr) {
    free(g_yuv_flip_buf.right_addr);
  }
  if (g_yuv_flip_buf.left_addr) {
    free(g_yuv_flip_buf.left_addr);
  }
}

/**
 * @brief wake up render thread task, called by camera capture task.
 **/
int RenderPost() { return sem_post(&g_sync_flag.sem); }

/**
 * @brief wait camera capture thread data ready, called by render thread task.
 **/
int RenderWait() { return sem_wait(&g_sync_flag.sem); }

/**
 * @brief signal handler function,release resource before application exit.
 * @param sig_no  signal number
 **/
static void Terminate(int32_t sig_no) {
  int32_t narray = sizeof(g_vin_conf) / sizeof(g_vin_conf[0]);
  int32_t ret;

  printf("Got signal %d, exiting ...\n", sig_no);
  for (int32_t i = 0; i < narray; ++i) {
    if (g_vin_conf[i].fd != -1) {
      ret = V4l2OpVidiocStreamOff(&g_vin_conf[i]);
      V4L2_OP_CHECK(ret == -1);
      close(g_vin_conf[i].fd);
    }
  }
  /*wait some time for kernel release resource*/
  usleep(50 * 1000);
  ExitYuvFlipBuffer();
  if (g_yuv_org_flag) {
    DeinitYuvFromFile(&g_yuv_org_data);
  }
  ExitDmsThread();
  DestroyDms();
  exit(0);
}

/**
 * @brief install signal hander function
 **/
static void InstallSignalHandler(void) {
  signal(SIGBUS, Terminate);
  signal(SIGFPE, Terminate);
  signal(SIGHUP, Terminate);
  signal(SIGILL, Terminate);
  signal(SIGKILL, Terminate);
  signal(SIGINT, Terminate);
  signal(SIGIOT, Terminate);
  signal(SIGPIPE, Terminate);
  signal(SIGQUIT, Terminate);
  signal(SIGSEGV, Terminate);
  signal(SIGSYS, Terminate);
  signal(SIGTERM, Terminate);
  signal(SIGTRAP, Terminate);
  signal(SIGUSR1, Terminate);
  signal(SIGUSR2, Terminate);
}

/**
 * @brief open camera and configure parameter.
 **/
void InitCamera() {
  int32_t narray = sizeof(g_vin_conf) / sizeof(g_vin_conf[0]);
  int32_t ret;
  for (int32_t i = 0; i < narray; ++i) {
    g_vin_conf[i].fd = V4l2OpOpen(&g_vin_conf[i]);
    V4L2_OP_CHECK(g_vin_conf[i].fd == -1);
  }

  for (int32_t i = 0; i < narray; ++i) {
    ret = V4l2OpVidiocsInput(&g_vin_conf[i]);
    V4L2_OP_CHECK(ret == -1);
  }

  for (int32_t i = 0; i < narray; ++i) {
    ret = V4l2OpVidiocsParm(&g_vin_conf[i]);
    V4L2_OP_CHECK(ret == -1);
  }

  for (int32_t i = 0; i < narray; ++i) {
    ret = V4l2OpVidiocsParm(&g_vin_conf[i]);
    V4L2_OP_CHECK(ret == -1);
  }

  for (int32_t i = 0; i < narray; ++i) {
    ret = V4l2OpVidiocsFmt(&g_vin_conf[i]);
    V4L2_OP_CHECK(ret == -1);
  }

  for (int32_t i = 0; i < narray; ++i) {
    ret = V4l2OpVidiocgParm(&g_vin_conf[i]);
    V4L2_OP_CHECK(ret == -1);
  }

  for (int32_t i = 0; i < narray; ++i) {
    ret = V4l2OpVidiocReqbufs(&g_vin_conf[i]);
    V4L2_OP_CHECK(ret == -1);
  }

  for (int32_t i = 0; i < narray; ++i) {
    ret = V4l2OpVidiocQuerybuf(&g_vin_conf[i]);
    V4L2_OP_CHECK(ret == -1);
  }
}

/**
 * @brief start camera data capature.
 **/
void StartCamera() {
  int32_t narray = sizeof(g_vin_conf) / sizeof(g_vin_conf[0]);
  int32_t ret = -1;

  for (int32_t i = 0; i < narray; ++i) {
    ret = V4l2OpVidiocStreamOn(&g_vin_conf[i]);
    V4L2_OP_CHECK(ret == -1);
    gettimeofday(&g_vin_conf[i].tv, NULL);
  }
}

/**
 * @brief process all camera frame and wait up render thread to refresh display.
 *
 **/

void ProcessFrame() {
  fd_set fds;
  struct timeval tv;
  int32_t r = -1;
  int32_t camera_fd = 0;

  int32_t narray = sizeof(g_vin_conf) / sizeof(g_vin_conf[0]);
  static int save_image = 0;
  static unsigned int save_flag = 0;
  const unsigned int save_mask = (1<<narray) - 1;

  while (1) {
    FD_ZERO(&fds);
    /*find max camera file description (fd)*/
    for (int i = 0; i < narray; ++i) {
      if (g_vin_conf[i].fd != -1) {
        FD_SET(g_vin_conf[i].fd, &fds);
        if (g_vin_conf[i].fd > camera_fd) {
          camera_fd = g_vin_conf[i].fd;
        }
      }
    }
    tv.tv_sec = 2; /* Timeout. */
    tv.tv_usec = 0;

    /*wait camera data ready*/
    r = select(camera_fd + 1, &fds, NULL, NULL, &tv);
    
    if (-1 == r) {
      if (errno == EINTR) {
        
        continue;
      }
    } else if (r == 0) {
      
      continue;
    }
    if(save_image == 0)
    {
      save_image = SaveImage();
    }
    
    for (int32_t i = 0; i < narray; ++i) {
      int32_t fd = g_vin_conf[i].fd;
      if ((fd != -1) && (FD_ISSET(fd, &fds))) {
        struct v4l2_buffer buf = {0};
        struct v4l2_plane plane = {0};
        struct timeval tv;

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.length = 1;
        buf.m.planes = &plane;
        if (-1 == ioctl(fd, VIDIOC_DQBUF, &buf)) {
          printf("%d:%s, VIDIOC_DQBUF failed\n", i, g_vin_conf[i].devname);

          V4L2_OP_CHECK(1);
          // terminate(SIGQUIT);
          exit(-1);
          break;
        }
        gettimeofday(&tv, NULL);

        g_current_camera_buffer[i].addr = g_vin_conf[i].mapbuf[buf.index].start;
        g_current_camera_buffer[i].length =
            g_vin_conf[i].mapbuf[buf.index].length;
        g_current_camera_buffer[i].timestamp =
            tv.tv_sec * 1000 + tv.tv_usec / 1000;
        g_sync_flag.flag |= 1 << i;
        if(save_image && !(save_flag & (1<<i)))
        {
          char filename[256];
			    char time_stmp[100];
			    GetTimeStmpStr(time_stmp, "%Y-%m-%d_%H_%M_%S");
			    sprintf(filename, "/home/bin/%d_%s.png", i, time_stmp);
			    stbi_write_png(filename, 1280, 720, 1, g_current_camera_buffer[i].addr, 0);
          printf("Save Image %s\n",filename);
          sprintf(filename, "/home/bin/%d_%s.YUV", i, time_stmp);
          FILE *fp = fopen(filename,"wr");
          fwrite(g_current_camera_buffer[i].addr,1280*1080,1,fp);
          fclose(fp);
          save_flag = save_flag | 1<<i;
        }
        const uint32_t mask = 0xF;
        
        if ((g_sync_flag.flag & mask) == mask) {
          /* four channel ready, front back left right */
          g_sync_flag.flag = 0;

          RenderPost();
        }

        if (-1 == ioctl(fd, VIDIOC_QBUF, &buf)) {
          printf("VIDIOC_QBUF buf.index %d failed\n", buf.index);
          continue;
        }
      }
    }
    if(save_image == 1 && save_flag == save_mask)
    {
      ResetSaveImage();
      save_image = 0;
      save_flag = 0;
    }   
  }
}

/**
 * @brief render thread task
 *
 */
static void *RenderPthread(void *arg) {
  struct render_parameter *renderparam = (struct render_parameter *)arg;
  struct timeval tv;
  struct timeval tvend;
  InitOpenglesWindows();
  InitWindow();
  for (;;) {
    RenderWait();
    // usleep(1000*1000);
    gettimeofday(&tv, NULL);
    int64_t timestamp = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    /*printf(" mode : %d, timestame:%ld\r\n",  renderparam->mode, timestamp);*/
    /*called 360 surroud library function*/
    RenderWindow(renderparam);

    gettimeofday(&tvend, NULL);
    int64_t timestampend = tvend.tv_sec * 1000 + tvend.tv_usec / 1000;
    /* printf("frame internal:%d, mode:%d\r\n", timestampend -  timestamp,
     * renderparam->mode);*/
  }
  return NULL;
}

/**
 * @brief create render pthread
 *
 */
void InitRenderPthread() {
  if (pthread_create(&g_render_th, NULL, RenderPthread, &g_render_parameter) ==
      -1) {
    printf("pthread create failed\r\n");
  } else {
    printf("render thread success\r\n");
  }
  pthread_detach(g_render_th);
}

int main(int argc, char **argv) {
  InitLogBuffer(DEBUG_LEVEL);
  InitTimer(LOG_PER_PERIOD);
  InitYuvFlipBuffer();
  if ((argc > 1) && (strncmp(argv[1], "-t", 2) == 0)) {
    g_yuv_org_flag = 1;
    g_yuv_org_data = InitYuvFromFile(YUV_FRONT_FILE_NAME, YUV_BACK_FILE_NAME,
                                     YUV_LEFT_FILE_NAME, YUV_RIGHT_FILE_NAME);
    printf("load calirate test file\r\n");
  }

  InstallSignalHandler();
  InitCANFrameInfo();
  InitDisplayMode();
  InitCameraParam();
  InitCamera();
  InitRenderPthread();
  // init_ser_pth("/dev/ttyS3");
  InitSerialPthread();
  InitDms(DMS_WORK_MODE, DMS_MODEL_PATH);
#if VEHICLE_TYPE == VEHICLE_TYPE_CHANGE_BATTERY 
  InitHardWare();
#endif
#if ENABLE_DISPLAYMODE_SWITCH
  InitDisplayModeSwitch();
#endif
  StartCamera();
  ProcessFrame();
  while (1) {
    usleep(100 * 1000);
  }
}
