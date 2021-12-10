#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include "proto.h"
#include "sy_log.h"
#include "sy_timer.h"
#include "linux-serial-app.h"

void DoTimerReady(void *pvparam);

int CreateTimer(unsigned int ui_sec, unsigned int ui_nsec) {
  int ret = 0;
  int tfd = 0;
  struct itimerspec timevalue;

  /*
    When the file descriptor is no longer required it should be closed.
    When all file descriptors associated with the same timer object have been
    closed, the timer is disarmed and its resources are freed by the kernel.
  */

  tfd = timerfd_create(CLOCK_REALTIME, 0);
  if (tfd < 0) {
    perror("timerfd_create");
    return -1;
  }

  /*
    Setting either field of new_value.it_value to a nonzero value arms the
     timer. Setting both fields of new_value.it_value to zero disarms the timer.
  */
  timevalue.it_value.tv_sec = 1;
  timevalue.it_value.tv_nsec = 0;

  timevalue.it_interval.tv_sec = (time_t)ui_sec;
  timevalue.it_interval.tv_nsec = (long)ui_nsec;

  ret = timerfd_settime(tfd, 0, &timevalue, NULL);
  if (ret < 0) {
    perror("timerfd_settime");
    return -1;
  }
  return tfd;
}

int CreateEpoll(struct PollParam *pollparam) {
  int epfd = 0;
  struct epoll_event ev;
  int ret = -1;
  epfd = epoll_create(EPOOL_SIZE);
  if (epfd < 0) {
    return -1;
  }

  ev.data.ptr = pollparam;
  ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
  ret = epoll_ctl(epfd, EPOLL_CTL_ADD, pollparam->fd, &ev);
  if (ret < 0) {
    close(epfd);
    return -1;
  }
  return epfd;
}

int SetNonBlock(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  if (-1 == fcntl(fd, F_SETFL, flags)) {
    return 0;
  }
  return -1;
}

void EpollWait(int epfd) {
  const int EPOOL_EVENT = 2;
  struct epoll_event events[EPOOL_EVENT];
  int nfds = 0;
  memset(events, 0, sizeof(struct epoll_event) * EPOOL_EVENT);
  for (;;) {
    nfds = epoll_wait(epfd, events, EPOOL_EVENT, 10 * 1000 * 1000);
    if (nfds < 0) {
      break;
    }
    // printf("nfds:%d\r\n", nfds);
    for (int i = 0; i < nfds; i++) {
      if (events[i].events & EPOLLIN) {
        DoTimerReady(events[i].data.ptr);
      }
    }
  }
}

void DoTimerReady(void *pvparam) {
  struct PollParam *pstparam = (struct PollParam *)pvparam;
  if (pstparam != NULL) {
    switch (pstparam->fdtype) {
      case FD_TIMER:
        if (pstparam->cb != NULL) {
          pstparam->cb(pstparam->fd, pstparam->pv_param);
        }
        break;
      case FD_SOCKET:
        break;
      case FD_FILE:
        break;
      default:
        break;
    }
  }
}

void printCurrentTime() {
  struct timeval tv;
  struct tm *p;
  gettimeofday(&tv, NULL);
  p = localtime(&tv.tv_sec);
  printf("time_now:%d%d%d%d%d%d.%03ld\n", 1900 + p->tm_year, 1 + p->tm_mon,
         p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec / 1000);
}

void *TimerFunc(int fd, void *pvParam) {
  long data = 0;
  char buffer[41] = {0};
  int len = 0;
  uint8_t dtout[50] = {0};
  int32_t outlen = 0;

  read(fd, (void *)&data, sizeof(long));
  len = GetLogMessageFromCircleBuffer(buffer, sizeof(buffer)-1);
  //printCurrentTime();
  //printf("GetLogMessageFromCircleBuffer:%d\r\n", len);

  if (len > 0) {
    //printf("log:%s\r\n", buffer);
    /*{0xAA, 0x75, 0x14, 0x00, 0x00, 0x01, 0xCE, 0x04}*/
    outlen = EncodeJimuOutCommand(dtout, APP_LOG_CMD, (uint8_t *)buffer, len);
    AddTxBuffer(CCS_SER_IDX, dtout, outlen);
    //PrintArray(dtout, outlen);
  }
  // printCurrentTime();
  return NULL;
}

static void *TimerStartRoutine(void *argc) {
  int *epfd = (int *)argc;
  //printf("*epfd:%d\r\n", *epfd);
  while (1) {
    /* code */
    EpollWait(*epfd);
  }
  return (void *)"TimerStartRoutine";
}

int InitTimer(const int ms) {
  /*used by pthread ,so need to define static*/
  static struct PollParam pstparam;
  static pthread_t g_timer_pth;
  static int epfd;

  int tfd = -1;
  memset(&pstparam, 0, sizeof(pstparam));

  tfd = CreateTimer(0, TIMER_MS_COUNT(ms));
  if (tfd < 0) {
    printf("createTimer\r\n");
    return -1;
  }
  SetNonBlock(tfd);
  pstparam.fd = tfd;
  pstparam.fdtype = FD_TIMER;
  pstparam.cb = TimerFunc;
  pstparam.pv_param = &pstparam;

  epfd = CreateEpoll(&pstparam);
  if (epfd < 0) {
    printf("createEpoll\r\n");
    close(tfd);
    return -1;
  }
  printf("epfd:%d\r\n", epfd);
  int ret = pthread_create(&g_timer_pth, NULL, TimerStartRoutine, &epfd);
  if (-1 == ret) {
    printf("pthread_create TimerStartRoutine failed\r\n");
    return -1;
  }
  return 0;
}
