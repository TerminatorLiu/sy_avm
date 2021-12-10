#ifndef _SY_LOG_H
#define _SY_LOG_H

#include <pthread.h>
#include <semaphore.h>

struct linux_circ_buf {
  char *buf;
  int head; // wr
  int tail; // rd
};


enum SY_APP_LOG_LEVEL {
  DEBUG_LEVEL = 0,
  INFO_LEVEL = 1,
  WARN_LEVEL = 2,
  ERROR_LEVEL = 3,
};

#define LOG_BUFFER_SIZE (32*1024)

#define LOG_TMP_BUFFER_SIZE 1024

#define LOG_PER_COUNT     (20)


void InitLogBuffer(int level);
void DestroyLogBuffer();
int GetLogMessageFromCircleBuffer(char* buffer, int count);
int LogOut(int level, const char* file, const int line, char* format, ...);

#define LOGOUT_DEBUG(...) LogOut(DEBUG_LEVEL, __FILE__, __LINE__, __VA_ARGS__)
#define LOGOUT_INFO(...)  LogOut(INFO_LEVEL,  __FILE__, __LINE__, __VA_ARGS__)
#define LOGOUT_WARN(...)  LogOut(WARN_LEVEL,  __FILE__, __LINE__, __VA_ARGS__)
#define LOGOUT_ERROR(...) LogOut(ERROR_LEVEL, __FILE__, __LINE__, __VA_ARGS__)

struct sy_log_category {
  int level;
  char *tmpbuf;
  struct linux_circ_buf logbuf;
  pthread_mutex_t mutex;
  sem_t sem;
};

#endif
