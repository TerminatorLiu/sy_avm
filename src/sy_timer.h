#ifndef _SY_TIMER_H
#define _SY_TIMER_H

#define EPOOL_SIZE 1000

typedef void *(*TimerCallback)(int fd, void *);

typedef enum SY_FD_TYPE {
  FD_TIMER,
  FD_SOCKET,
  FD_FILE,
} ENFD_TYPE;

struct PollParam {
  int fd;            /*file descriptor*/
  ENFD_TYPE fdtype;  //文件描述符类型
  TimerCallback cb;  //定时器回调函数
  void *pv_param;    //回调函数参数
};
int InitTimer(const int ms);

#define TIMER_MS_COUNT(ms) (1000 * 1000 * ms)
#define LOG_PER_PERIOD     (30)
#endif
