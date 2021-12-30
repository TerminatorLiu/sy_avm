
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "opencv/cv.h"
#include "dms.h"
#include "fatigue_driving_c.h"
#include "linux-serial-app.h"
#include "proto.h"
#include "sy_log.h"
#include "stbi_image_write.h"
#include "conf.h"
#if ENABLE_DMS
void *g_dmshandle = NULL;
static pthread_t g_dms_pth;
static char *g_dms_data_buffer_t = NULL;
static char *g_dms_data_buffer = NULL;
static volatile int g_dms_busy_flag = 1;
static pthread_rwlock_t g_dms_flag_lock;
static sem_t g_dms_sem;
static int g_dms_running_flag = 1;

const uint8_t DMS_NORMAL_ALRAM_MSG[] = {0xAA, 0x75, 0x14, 0x00,
                                        0x00, 0x01, 0x00, 0xCA};
const uint8_t DMS_NORMAL_ALRAM = 0x00;

const uint8_t DMS_NOFACE_ALRAM_MSG[] = {0xAA, 0x75, 0x14, 0x00,
                                        0x00, 0x01, 0xCE, 0x04};
const uint8_t DMS_NOFACE_ALRAM = 0xCE;

const uint8_t DMS_SMOKE_ALRAM_MSG[] = {0xAA, 0x75, 0x14, 0x00,
                                       0x00, 0x01, 0xD0, 0x1A};
const uint8_t DMS_SMOKE_ALRAM = 0xD0;

const uint8_t DMS_PHOING_ALRAM_MSG[] = {0xAA, 0x75, 0x14, 0x00,
                                        0x00, 0x01, 0xCF, 0x05};
const uint8_t DMS_PHOING_ALRAM = 0xCF;

const uint8_t DMS_CLOSEEYE_ALRAM_MSG[] = {0xAA, 0x75, 0x14, 0x00,
                                          0x00, 0x01, 0xC9, 0x03};
const uint8_t DMS_CLOSEEYE_ALRAM = 0xC9;

const uint8_t DMS_YAWNG_ALRAM_MSG[] = {0xAA, 0x75, 0x14, 0x00,
                                       0x00, 0x01, 0xCA, 0x00};
const uint8_t DMS_YAWNG_ALRAM = 0xCA;

const uint8_t DMS_LOOKAROUND_ALRAM_MSG[] = {0xAA, 0x75, 0x14, 0x00,
                                            0x00, 0x01, 0xCD, 0x07};
const uint8_t DMS_LOOKAROUND_ALRAM = 0xCD;

const uint8_t DMS_UPHEAD_ALRAM_MSG[] = {0xAA, 0x75, 0x14, 0x00,
                                        0x00, 0x01, 0xDC, 0x16};
const uint8_t DMS_UPHEAD_ALRAM = 0xDC;

const uint8_t DMS_OCCLUSIOM_ALRAM_MSG[] = {0xAA, 0x75, 0x14, 0x00,
                                           0x00, 0x01, 0xD1, 0x1B};
const uint8_t DMS_OCCLUSIOM_ALRAM = 0xD1;

static void UploadDMSDetectResult(const int32_t dms_res);
static void SetDmsBusyFlag(int flag);
static int GetDmsBusyFlag();
static void *DmsStartRoutine(void *argc);

static void SetDmsBusyFlag(int flag)
{
  pthread_rwlock_wrlock(&g_dms_flag_lock);
  g_dms_busy_flag = flag;
  pthread_rwlock_unlock(&g_dms_flag_lock);
}

int GetDmsBusyFlag()
{
  int flag = 0;
  pthread_rwlock_rdlock(&g_dms_flag_lock);
  flag = g_dms_busy_flag;
  pthread_rwlock_unlock(&g_dms_flag_lock);
  return flag;
}

void UpdateDmsData(void *data)
{
  if (0 == GetDmsBusyFlag())
  {
    g_dms_data_buffer = (char *)data;
    CvMat m = cvMat(720, 1280, CV_8UC1, g_dms_data_buffer);
    CvMat m2 = cvMat(1280, 720, CV_8UC1, g_dms_data_buffer_t);
    cvTranspose(&m, &m2);
    cvFlip(&m2, &m2, 0);
    SetDmsBusyFlag(1);
    sem_post(&g_dms_sem);
  }
}

static void UploadDMSDetectResult(const int32_t dms_res)
{
  // printf("dms res:%d\r\n", dms_res);
  if (1)
  {
    LOGOUT_INFO("dms result:%d", dms_res);
    printf("dms result:%d\n", dms_res);
  }

  uint8_t dtout[16] = {0};
  int32_t outlen = 0;
  uint8_t ret = 0xFF;
  switch (dms_res)
  {
  case NO_FACE_DS:

    /*AddTxBuffer(CCS_SER_IDX, DMS_NOFACE_ALRAM_MSG,
                  sizeof(DMS_NOFACE_ALRAM_MSG));*/
    ret = DMS_NOFACE_ALRAM;
    break;

  case SMOKING_DS:
    /*AddTxBuffer(CCS_SER_IDX, DMS_SMOKE_ALRAM_MSG,
                  sizeof(DMS_SMOKE_ALRAM_MSG));*/
    ret = DMS_SMOKE_ALRAM;
    break;

  case PHONING_DS:
    /*AddTxBuffer(CCS_SER_IDX, DMS_PHOING_ALRAM_MSG,
                  sizeof(DMS_PHOING_ALRAM_MSG));*/
    ret = DMS_PHOING_ALRAM;
    break;

  case CLOSED_EYE_DS:
    /*AddTxBuffer(CCS_SER_IDX, DMS_CLOSEEYE_ALRAM_MSG,
                  sizeof(DMS_CLOSEEYE_ALRAM_MSG));*/
    ret = DMS_CLOSEEYE_ALRAM;
    break;

  case YAWNING_DS:
    /*AddTxBuffer(CCS_SER_IDX, DMS_YAWNG_ALRAM_MSG,
                  sizeof(DMS_YAWNG_ALRAM_MSG));*/
    ret = DMS_YAWNG_ALRAM;
    break;
  case LOOK_AROUND_DS:
    /*AddTxBuffer(CCS_SER_IDX, DMS_LOOKAROUND_ALRAM_MSG,
                  sizeof(DMS_LOOKAROUND_ALRAM_MSG));*/
    ret = DMS_LOOKAROUND_ALRAM;
    break;
  case UP_HEAD_DS:
    /*AddTxBuffer(CCS_SER_IDX, DMS_UPHEAD_ALRAM_MSG,
                  sizeof(DMS_UPHEAD_ALRAM_MSG));*/
    ret = DMS_UPHEAD_ALRAM;
    break;
  case OCCLUSION_DS:
    /*AddTxBuffer(CCS_SER_IDX, DMS_OCCLUSIOM_ALRAM_MSG,
                  sizeof(DMS_OCCLUSIOM_ALRAM_MSG));*/
    ret = DMS_OCCLUSIOM_ALRAM;
    break;
  default:
    /*AddTxBuffer(CCS_SER_IDX, DMS_NORMAL_ALRAM_MSG,
                  sizeof(DMS_NORMAL_ALRAM_MSG));*/
    ret = DMS_NORMAL_ALRAM;
    break;
  }
  outlen = EncodeJimuOutCommand(dtout, APP_DMS_DECTECT_CMD, (uint8_t *)&ret, 1);
  AddTxBuffer(CCS_SER_IDX, dtout, outlen);
}

static void *DmsStartRoutine(void *argc)
{
  int tm = 0;
  const int DMS_MASK = 2046;
  int speed = 60;
  int dms_res;
  SetDmsBusyFlag(1);
  g_dmshandle = NULL;
  g_dmshandle =
      init_fatigue_driving(DMS_MODEL_PATH, DMS_WORK_MODE, NULL, false, false);
  g_dms_data_buffer_t = (char *)malloc(921600);
  LOGOUT_INFO("dms init finished");
  printf("DMS Version:%s\n", get_version_fatigue_driving());
  while (g_dms_running_flag)
  {
    if ((g_dms_data_buffer_t != NULL) && (g_dmshandle != NULL))
    {
      SetDmsBusyFlag(0);
      int ret = sem_wait(&g_dms_sem);
      if ((ret != 0) && (errno == EINTR))
      {
        /*system call interupt by signal, wait again*/
        SetDmsBusyFlag(1);
        usleep(5 * 1000);
        continue;
      }

      SetDmsBusyFlag(1);
      // {
      //   char filename[256];
      //   sprintf(filename, "/home/bin/dms%d.png", tm);
      //   stbi_write_png(filename,720, 1280, 1, g_dms_data_buffer_t, 0);
      //   printf("Save Image %s\n", filename);
      // }
      //dms_res = 0;
      dms_res = detect_fatigue_driving(g_dmshandle, g_dms_data_buffer_t, 720,
                                       1280, speed, DMS_MASK, ++tm);
      //UploadDMSDetectResult(206);
      //usleep(500 * 1000);
      UploadDMSDetectResult(dms_res);
    }
    SetDmsBusyFlag(1);
    usleep(50 * 1000);
  }
  return (void *)"DmsStartRoutine";
}

int InitDms(int mode, char *model_path)
{
  int ret = -1;
  printf("dms version:%s\r\n", get_version_fatigue_driving());
  ret = pthread_rwlock_init(&g_dms_flag_lock, NULL);
  ret = sem_init(&g_dms_sem, 0, 0);

  ret = pthread_create(&g_dms_pth, NULL, DmsStartRoutine, g_dmshandle);
}

void ExitDmsThread()
{
  void *status;
  g_dms_running_flag = 0;
  pthread_join(g_dms_pth, &status);
}

void DestroyDms()
{
  if (g_dmshandle != NULL)
  {
    release_fatigue_driving(g_dmshandle);
  }
  pthread_rwlock_destroy(&g_dms_flag_lock);
  if (g_dms_data_buffer_t != NULL)
  {
    free(g_dms_data_buffer_t);
  }
}
#endif