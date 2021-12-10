#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>  // struct timeval
#include <time.h>      //struct tm

#include "timeset.h"

int SetSystemDateTime(const int year, const int month, const int day,
                      const int hour, const int minite, const int second) {
  struct tm set_tm;
  struct timeval set_tv;
  time_t timep;
  int yearvalid = ((year >= 2020) && (year <= 2100));
  int mouthvalid = ((month >= 1) && (month <= 12));
  int dayvalid = ((day >= 1) && (day <= 31));
  int hourvalid = ((hour >= 0) && (hour <= 24));
  int minitevalid = ((minite >= 0) && (minite <= 59));
  int secondvalid = ((second >= 0) && (second <= 59));

  int datetimevalid = yearvalid && mouthvalid && dayvalid && hourvalid &&
                      minitevalid && secondvalid;

  if (!datetimevalid) {
    set_tm.tm_year = 2020 - 1900;
    set_tm.tm_mon = 10 - 1;
    set_tm.tm_mday = 1;
    set_tm.tm_hour = 12;
    set_tm.tm_min = 00;
    set_tm.tm_sec = 00;
  } else {
    set_tm.tm_year = year - 1900;
    set_tm.tm_mon = month - 1;
    set_tm.tm_mday = day;
    set_tm.tm_hour = hour;
    set_tm.tm_min = minite;
    set_tm.tm_sec = second;
  }
  timep = mktime(&set_tm);
  time_t machinetime = time(NULL);

  if (abs(machinetime - timep) > 2) {
    set_tv.tv_sec = timep;
    set_tv.tv_usec = 0;

    if (settimeofday(&set_tv, (struct timezone *)0) < 0) {
      perror("settimeofday");
      return -1;
    }
    system("hwclock -w");
    printf("time set to %d-%d-%d %d:%d:%d\r\n",
      year, month, day, hour, minite, second);
  }else{
    printf("time no need sync\r\n");
  }

  return 0;
}