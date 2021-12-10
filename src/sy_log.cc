#include "sy_log.h"
#include <stdarg.h>
#include <stdio.h> /* puts */
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h> /* time_t, struct tm, time, localtime, strftime */

static char s_log_buffer[LOG_BUFFER_SIZE] = {0};
static struct sy_log_category g_sy_log_category;
static char g_log_tmp_buffer[1024] = {0};

static int GetDateTimeString(char* buffer, int nsize) {
  time_t rawtime;
  struct tm* timeinfo;

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  return strftime(buffer, nsize, "%F %T", timeinfo);
}

static const char* GetLogLevelName(int level) {
  switch (level) {
    case DEBUG_LEVEL:
      return "DEBUG";

    case INFO_LEVEL:
      return "INFO";

    case WARN_LEVEL:
      return "WARN";

    case ERROR_LEVEL:
      return "ERROR";

    default:
      break;
  }
  return "DEBUG";
}

void InitLogBuffer(int level) {
  memset((void*)&g_sy_log_category, 0, sizeof(g_sy_log_category));
  g_sy_log_category.level = level;
  g_sy_log_category.tmpbuf = g_log_tmp_buffer;
  sem_init(&(g_sy_log_category.sem), 0, 0);
  pthread_mutex_init(&g_sy_log_category.mutex, NULL);
  g_sy_log_category.logbuf.buf = s_log_buffer;
}

void DestroyLogBuffer() {
  pthread_mutex_destroy(&(g_sy_log_category.mutex));
  sem_destroy(&(g_sy_log_category.sem));
}

void AddToLogCircleBuffer(char* logmsg, int length) {
  struct linux_circ_buf* circlebuf = &g_sy_log_category.logbuf;

  if (circlebuf->buf == NULL) {
    return;
  }

  if ((circlebuf->head + length) <= LOG_BUFFER_SIZE) {
    memcpy(&circlebuf->buf[circlebuf->head], logmsg, length);
    circlebuf->head += length;
    circlebuf->head %= LOG_BUFFER_SIZE;
  } else {
    int leavecount = length - (LOG_BUFFER_SIZE - circlebuf->head);
    memcpy(&circlebuf->buf[circlebuf->head], logmsg,
           LOG_BUFFER_SIZE - circlebuf->head);
    memcpy(&circlebuf->buf[0], &logmsg[LOG_BUFFER_SIZE - circlebuf->head],
           leavecount);
    circlebuf->head = leavecount;
  }

  /*printf("after circle buffer:head:%d,tail:%d\r\n", circlebuf->head,
         circlebuf->tail);*/
}

int LogOut(int level, const char* file, const int line, char* format, ...) {
  char timebuf[30] = {0};
  int i = 0;
  int n = 0;
  int count = 0;

  pthread_mutex_lock(&g_sy_log_category.mutex);
  if (g_sy_log_category.tmpbuf) {
    GetDateTimeString(timebuf, sizeof(timebuf));
    n = snprintf(g_sy_log_category.tmpbuf, 1024, "%s %s (%s:%d) ", timebuf,
                 GetLogLevelName(level), file, line);
    va_list va;
    va_start(va, format);
    if ((n > 0) && (n < (LOG_TMP_BUFFER_SIZE - 2))) {
      i = vsnprintf(&(g_sy_log_category.tmpbuf[n]), LOG_TMP_BUFFER_SIZE - n,
                    format, va);
      if (i > 0) {
        strcat(&(g_sy_log_category.tmpbuf[i + n]), "\r\n");
        i = i + 2;
      }
    }
    va_end(va);
  }
  count = i + n;
  AddToLogCircleBuffer(g_sy_log_category.tmpbuf, count);
  pthread_mutex_unlock(&g_sy_log_category.mutex);
  return count;
}

int GetLogMessageFromCircleBuffer(char* buffer, int count) {
  int n = 0;
  struct linux_circ_buf* circlebuf = &g_sy_log_category.logbuf;
  pthread_mutex_lock(&g_sy_log_category.mutex);
  while ((g_sy_log_category.logbuf.tail != g_sy_log_category.logbuf.head) &&
         (n < count)) {
    buffer[n] = g_sy_log_category.logbuf.buf[g_sy_log_category.logbuf.tail];

    g_sy_log_category.logbuf.tail++;
    g_sy_log_category.logbuf.tail %= LOG_BUFFER_SIZE;
    n++;
  }
  if (0) {
    printf("aa after circle buffer:head:%d,tail:%d\r\n", circlebuf->head,
           circlebuf->tail);
  }

  pthread_mutex_unlock(&g_sy_log_category.mutex);

  return n;
}
