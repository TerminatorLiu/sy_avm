#include "displaymode.h"

#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

#include "mcuparse.h"
#include "sy_log.h"

volatile int gCameraDisplayMode = 0x10;
pthread_rwlock_t gCameraDisplayModeLock;
pthread_t gDisplayModeSwitch;
/**
 * @brief get camera render mode.
 **/

int InitDisplayMode()
{
  int res = 0;
  res = pthread_rwlock_init(&gCameraDisplayModeLock, NULL);
  return res;
}

int GetDisplayMode()
{
  int res = 0x00;
  pthread_rwlock_rdlock(&gCameraDisplayModeLock);
  res = gCameraDisplayMode;
  pthread_rwlock_unlock(&gCameraDisplayModeLock);
  return res;
}

/**
 * @brief update 360 surroud mode.
 * @param display mode.
 **/
int SetDisplayMode(uint8_t mode)
{
  /*TODO add check code*/
  pthread_rwlock_wrlock(&gCameraDisplayModeLock);
  gCameraDisplayMode = mode & 0xFF;
  pthread_rwlock_unlock(&gCameraDisplayModeLock);
  return 0;
}

int DestroyDisplayMode()
{
  int res = 0;
  res = pthread_rwlock_destroy(&gCameraDisplayModeLock);
  return res;
}

static void *DisplayModeSwitchRoutine(void *argc)
{
  int is_turn = 0;
  int delay_count = 0;
#ifdef SCREEN_ROTATE
  int pre_mode = FRONT_RIGHT_MODE;
  int cur_mode = FRONT_RIGHT_MODE;
#else
  int pre_mode = FRONT_DISPALY_MODE;
  int cur_mode = FRONT_DISPALY_MODE;
#endif
  for (;;)
  {
    if (GetGear() == GEAR_REVERSE)
    {
#ifdef SCREEN_ROTATE
      cur_mode = BACK_RIGHT_MODE;
#else
      cur_mode = BACK_DISPLAY_MODE1;
#endif
      is_turn = 0;
      goto DELAY;
    }
    else if (GetTurn() == TURN_LEFT)
    {
#ifdef SCREEN_ROTATE
      cur_mode = FRONT_LEFT_MODE; //SetDisplayMode(FRONT_LEFT_MODE);
#else
      cur_mode = LEFT_DISPLAY_MODE1;  //SetDisplayMode(LEFT_DISPLAY_MODE1);
#endif
      is_turn = 1;
      delay_count = 0;
    }
    else if (GetTurn() == TURN_RIGHT)
    {
#ifdef SCREEN_ROTATE
      cur_mode = FRONT_RIGHT_MODE; //SetDisplayMode(FRONT_RIGHT_MODE);
#else
      cur_mode = RIGHT_DISPLAY_MODE1; //SetDisplayMode(RIGHT_DISPLAY_MODE1);
#endif
      is_turn = 1;
      delay_count = 0;
    }
    else
    {
      if (is_turn)
      {
        if (delay_count < 200)
        {
          delay_count++;
          goto DELAY;
        }
        else
        {
          is_turn = 0;
        }
      }
#ifdef SCREEN_ROTATE
      cur_mode = FRONT_RIGHT_MODE; //SetDisplayMode(FRONT_RIGHT_MODE);
#else
      cur_mode = FRONT_DISPALY_MODE;  //SetDisplayMode(FRONT_DISPALY_MODE);
#endif
      is_turn = 0;
    }
DELAY: if (cur_mode != pre_mode)
     {
      SetDisplayMode(cur_mode);
      printf("cur mode = 0x%x\n", cur_mode);
      pre_mode = cur_mode;
    }
    usleep(5e3);
  }
  return NULL;
}

int32_t InitDisplayModeSwitch()
{
  int res = 0;
  if (pthread_create(&gDisplayModeSwitch, NULL, DisplayModeSwitchRoutine, NULL) == -1)
  {
    LOGOUT_ERROR("Init InitDisplayModeSwitch thread failed\n");
    res = -1;
  }
  else
  {
    LOGOUT_INFO("Init InitDisplayModeSwitch thread success\n");
  }
  pthread_detach(gDisplayModeSwitch);
  return res;
}

