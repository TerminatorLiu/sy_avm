#ifndef _SY_APP_LINUX_SERIAL_APP_H
#define _SY_APP_LINUX_SERIAL_APP_H

#include <fcntl.h>
#include <linux/serial.h>
#include <poll.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include <conf.h>

#define SERIA_BUF_SIZE 512

extern int parse_ui_ser_msg(uint8_t *dtin, uint8_t *dtout);

extern int EncodeJimuOutCommand(uint8_t *dtout, uint8_t cmd, uint8_t *data,
                                uint16_t dtlen);

struct sy_serial_buf {
  /* data */
  unsigned short rx;
  unsigned short wd;
  unsigned char buf[SERIA_BUF_SIZE];
  pthread_mutex_t mutex;
  sem_t sem;
};

struct sy_serial_info {
  /* data */
  int fd;
  char *devname;
  char *name;
  int baudrate;
  int idx;

  // for command parse thread
  struct sy_serial_buf rx_buf;
  struct sy_serial_buf tx_buf;

  unsigned char *p;
  int state;
  ssize_t count;
  unsigned char cmdinfbuf[65 * 1024];
  unsigned char cmdoutbuf[65 * 1024];

  pthread_t rx_ser_th;
  void *(*rx_ser_task)(void *);

  pthread_t tx_ser_th;
  void *(*tx_ser_task)(void *);

  pthread_t parse_ser_th;
  void *(*parse_ser_task)(void *);
};

struct AMS_Parameter {
  int carLength;
  int carWidth;
  int chess2carFront;
};

#define CCS_SER_IDX 0
#define MCU_SER_IDX 1


#define CCS_SER_DEV_NAME "/dev/ttyS3"
#define CCS_SER_DEV_BAUDRATE 115200

#define MCU_SER_DEV_NAME "/dev/ttyS1"
#define MCU_SER_DEV_BAUDRATE 115200

#if REV_RADAR == REV_RADAR_NONE
#define SER_BUF_CNT 2
#endif

#if REV_RADAR == REV_RADAR_SHUNHE
#define SER_BUF_CNT 4
#define BACK_RIGHT_RADAR_SER_IDX 2
#define BACK_LEFT_RADAR_SER_IDX 3
#define BACK_RIGHT_RADAR_SER_DEV_NAME "/dev/ttyS4"
#define BACK_RIGHT_RADAR_SER_DEV_BAUDRATE 19200

#define BACK_LEFT_RADAR_SER_DEV_NAME "/dev/ttyS5"
#define BACK_LEFT_RADAR_SER_DEV_BAUDRATE 19200
#endif





void InitSerialPthread(void);
int32_t AddTxBuffer(const int32_t idx, const uint8_t buf[],const int32_t len);
uint8_t CalcCheckSum(uint8_t *buf, int32_t len);

int ScreenParseCmd(struct sy_serial_info *rx_buf);
int McuParseCmd(struct sy_serial_info *rx_buf);
void PrintArray(const uint8_t *dt, int len);

#define SWAP32(A)                                                           \
  ((((uint32_t)(A)&0xff000000) >> 24) | (((uint32_t)(A)&0x00ff0000) >> 8) | \
   (((uint32_t)(A)&0x0000ff00) << 8) | (((uint32_t)(A)&0x000000ff) << 24))

#endif
