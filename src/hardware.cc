#include"hardware.h"

#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include "sy_log.h"
#include "mcuparse.h"

pthread_t gHardwareThr;
static void *HardwareRoutine(void *argc);

int32_t InitWarnLight()
{
  int res = 0;
  if((access("/sys/class/pwm/pwmchip6/pwm0",F_OK))==-1) 
  {
    res = system("echo 0 > /sys/class/pwm/pwmchip6/export");
  }  
  res = system("echo 10000000 > /sys/class/pwm/pwmchip6/pwm0/period");
  res = system("echo 10000000 > /sys/class/pwm/pwmchip6/pwm0/duty_cycle");
  WarnLightDown();
  return 0;
}
int32_t WarnLightUp()
{
  int res = 0;
  res = system("echo 1 > /sys/class/pwm/pwmchip6/pwm0/enable");
  return res;
}
int32_t WarnLightDown()
{
  int res = 0;
  res = system("echo 0 > /sys/class/pwm/pwmchip6/pwm0/enable");
  return res;
}
int32_t InitHardWare()
{
  int res = 0;
  res = InitWarnLight();
  if(res !=0 )
  {
    LOGOUT_WARN("Init Waring Light Error\n");
  }
  usleep(1000*10);
  if (pthread_create(&gHardwareThr, NULL, HardwareRoutine, NULL) ==-1) {
    LOGOUT_ERROR("Init Hardware thread failed\n");
    res = -1;
  } else {
    LOGOUT_INFO("Init Hardware thread success\n");
  }
  pthread_detach(gHardwareThr);
  return res;
}

static void *HardwareRoutine(void *argc)
{
  int bsd_alarm_pre = 0,bsd_alarm = 0;
  for(;;)
  {
    bsd_alarm = GetBSDAlarm();
    if(bsd_alarm != bsd_alarm_pre)
    {
      bsd_alarm_pre = bsd_alarm;
      if(bsd_alarm)
      {
        printf("BSD Alarm Activated\n");
        LOGOUT_INFO("BSD Alarm Activated\n");
        WarnLightUp();
      }
      else
      {
        printf("BSD Alarm Released\n");
        LOGOUT_INFO("BSD Alarm Released\n");
        WarnLightDown();
      }
    }
    usleep(10000);
  }
}