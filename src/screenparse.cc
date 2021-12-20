#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* memcpy */

#include "linux-serial-app.h"
#include "proto.h"
#include "screenparse.h"
#include "timeset.h"
#include "displaymode.h"

#define SCREEN_PARSE_DEBUG
#ifdef SCREEN_PARSE_DEBUG
#define SCREENPARSEDBGprint(...) printf(__VA_ARGS__)
#else
#define SCREENPARSEDBGprint(...)
#endif

extern int cameraCalib(int carLength, int carWidth, int chess2carFront);

volatile int cameraCalib_busy_flag = 0;

int gSaveImageFlag = 0;
pthread_rwlock_t gSaveImageLock = PTHREAD_RWLOCK_INITIALIZER;
int SetSaveImage()
{
  pthread_rwlock_wrlock(&gSaveImageLock);
  gSaveImageFlag = 1;
  pthread_rwlock_unlock(&gSaveImageLock);
  return 0;
}
int ResetSaveImage()
{
  pthread_rwlock_wrlock(&gSaveImageLock);
  gSaveImageFlag = 0;
  pthread_rwlock_unlock(&gSaveImageLock);
  return 0;
}
int SaveImage()
{
  int res = -1;
  pthread_rwlock_rdlock(&gSaveImageLock);
  res = gSaveImageFlag;
  pthread_rwlock_unlock(&gSaveImageLock);
  return res;
}
/**
 * @brief caculate xor checksum.
 * @param buf caculate protol checksum.
 **/
uint8_t SerialRxCheckSum(uint8_t *buf) {
  struct jimu_proto_header *header = (struct jimu_proto_header *)buf;
  uint8_t crc = 0xAA ^ 0x75;
  uint16_t len = 5 + header->len;
  for (size_t i = 0; i < len; ++i) {
    crc ^= buf[i];
  }
  return crc;
}


/**
 * @brief parse 360surroud display mode message.
 * @param dtin input proto data.
 * @param dtout encode command.
 **/
static int ParseDisplayModeMsg(uint8_t *dtin, uint8_t *dtout) {
  struct jimu_proto_header *header = (struct jimu_proto_header *)dtin;
  struct jimu_proto_header *outheader = (struct jimu_proto_header *)&dtout[2];
  uint8_t mode = header->dt[0];
  dtout[0] = 0x55;
  dtout[1] = 0x7A;
  outheader->cmd = header->cmd;
  outheader->len = 1;
  printf("mode = 0x%x in %s:%d\n",mode,__FILE__,__LINE__);
  if (SetDisplayMode(mode) == 0) {
    /* support command */
    outheader->errcode = 0;
  } else {
    /* unsupport command */
    outheader->errcode = 1;
  }
  outheader->dt[0] = header->dt[0];
  outheader->dt[1] = CalcCheckSum(dtout, outheader->len + 7);
  return outheader->len + 8;
}

/**
 * @brief calibration task processing.
 * @param argc calibration parameter.
 **/
void *CameraCalibRoutine(void *argc) {
  struct AMS_Parameter *ams_parameter = (struct AMS_Parameter *)argc;
  int carLength = ams_parameter->carLength;
  int carWidth = ams_parameter->carWidth;
  int chess2carFront = ams_parameter->chess2carFront;
  int ret = -1;
  unsigned char dtout[30] = {0};
  unsigned char data = 0x1;

  SCREENPARSEDBGprint("enter cameraCalib, %d,%d,%d\r\n", carLength, carWidth,
                      chess2carFront);
  cameraCalib_busy_flag = 1;
  ret = cameraCalib(carLength, carWidth, chess2carFront);
  //ret = 0;

  if (ret == 0) {
    data = 0;
    /* success calibrate finished */
    SCREENPARSEDBGprint("--------success calibrate---------\r\n");
    SCREENPARSEDBGprint("system reboot now \r\n");
  } else {
    data = 1;
    SCREENPARSEDBGprint("--------failed calibrate---------\r\n");
  }
  int len = EncodeJimuOutCommand(dtout, APP_CALIB_CMD, &data, 1);
  AddTxBuffer(CCS_SER_IDX, dtout, len);
  usleep(1e5);
  AddTxBuffer(CCS_SER_IDX, dtout, len);
  usleep(1e5);
  AddTxBuffer(CCS_SER_IDX, dtout, len);
  PrintArray(dtout, len);
  cameraCalib_busy_flag = 0;
  if (ret == 0) {
    sleep(1);
    system("reboot");
  }
  return (void *)"cameraCalib_routine";
}

/**
 * @brief create calibration pthread, it take a long time, so need create a
 *thread to do calibrate task. if failed to calibrated, it will take 60 second.
 * @param ams_parameter calibration  input parameter.
 **/
void InitCameraCalibPthread(struct AMS_Parameter *ams_parameter) {
  pthread_t calib_th;
  pthread_create(&calib_th, NULL, CameraCalibRoutine, ams_parameter);
}

/**
 * @brief parse the command from central control screen, and encode the command
 *reply to central control screen.
 * @param dtin command from central control screen.
 * @param dtout the command reply to central control screen.
 **/
int ParseUiSerialMsg(uint8_t *dtin, uint8_t *dtout) {
  struct jimu_proto_header *header = (struct jimu_proto_header *)dtin;
  uint16_t len = 5 + header->len;
  int ret = 0;
  int p[3] = {0};
  uint8_t tmpbuf[20] = {0};
  static FILE *s_firmwarefile_fp = NULL;
  struct AMS_Parameter ams_parameter = {0};
  if (dtin[len] == SerialRxCheckSum(dtin)) {
    switch (header->cmd) {
      case CMD_SET_DISPLAY_MODE: /* 0x00 */
        ret = ParseDisplayModeMsg(dtin, dtout);
        break;
      case SAVE_IMAGE:
        SetSaveImage();
        break;

      case AMS_CONFIGURE_PARAMETER: /*0xBB AMS configure parameter */
        memcpy(p, header->dt, 12);
        p[0] = SWAP32(p[0]);
        p[1] = SWAP32(p[1]);
        p[2] = SWAP32(p[2]);

        ams_parameter.carLength = p[0];
        ams_parameter.carWidth = p[1];
        ams_parameter.chess2carFront = p[2];

        if (cameraCalib_busy_flag == 0) {
          SCREENPARSEDBGprint(
              "AMS parameter carLength:%d, carLength:%d, chess2carFront:%d\r\n",
              p[0], p[1], p[2]);
          InitCameraCalibPthread(&ams_parameter);
          usleep(100 * 1000);
        }

        break;

      case AEB_CONFIGURE_PARAMETER: /* 0xCC AEB configure parameter */
        SCREENPARSEDBGprint("AEB check success, cmd:0x%X, len:%d, dtlen:%d\r\n",
                            header->cmd, len, header->len);
        if (len < 15) {
          tmpbuf[0] = SCREEN_PARSE_SYNC1;
          tmpbuf[1] = SCREEN_PARSE_SYNC2;
          memcpy(&tmpbuf[2], dtin, len + 1);
          PrintArray(tmpbuf, len + 3);
          AddTxBuffer(MCU_SER_IDX, tmpbuf, len + 3);
        }
        break;

      case FILE_TRANSFORM_CMD:
        if (s_firmwarefile_fp == NULL) {
          s_firmwarefile_fp = fopen("/tmp/firmware.bin", "w");
        }
        if (s_firmwarefile_fp) {
          fwrite(header->dt, header->len, 1, s_firmwarefile_fp);
        }
        break;

      case FILE_MD5CHECKSUM_CMD:
        SCREENPARSEDBGprint("md5:");
        PrintArray(header->dt, header->len);
        if (s_firmwarefile_fp != NULL) {
          fclose(s_firmwarefile_fp);
        }
        break;

      case RTC_SYNC_CMD:
        //PrintArray(header->dt, header->len);
        //printf("year:%d,%d\r\n", header->dt[0], header->dt[0] + 2000);
        SetSystemDateTime(header->dt[0] + 2000, header->dt[1], header->dt[2],
                          header->dt[3], header->dt[4], header->dt[5]);
        break;

      default:
        break;
    }

    SCREENPARSEDBGprint("header, cmd:%d, errcode:%d, len:%d, dt0:0x%02X\r\n",
                        header->cmd, header->errcode, header->len,
                        header->dt[0]);

  } else {
    SCREENPARSEDBGprint(
        "check sum failed header, cmd:%d, errcode:%d, len:%d, dt0:0x%02X\r\n",
        header->cmd, header->errcode, header->len, header->dt[0]);

    /*SCREENPARSEDBGprint("--------check sum failed,  0x%02X, 0x%02X\r\n",
                        dtin[len], ser_rx_check_sum(dtin));*/
  }
  return ret;
}

/**
 * @brief protol parse task, called by parse thread.
 * @param ser_info serial buffer and state
 **/
int ScreenParseCmd(struct sy_serial_info *ser_info) {
  unsigned char ch = 0;
  struct jimu_proto_header *header =
      (struct jimu_proto_header *)ser_info->cmdinfbuf;

  pthread_mutex_lock(&(ser_info->rx_buf.mutex));
  unsigned char *p = (unsigned char *)ser_info->p;
  enum jimu_proto_state_t state = (enum jimu_proto_state_t)ser_info->state;
  while (ser_info->rx_buf.rx != ser_info->rx_buf.wd) {
    /* code */
    ch = ser_info->rx_buf.buf[ser_info->rx_buf.rx];
    switch (state) {
      case JIMU_SYNC_STATE1:
        if (JIMU_HOST_SYNC1 == ch) {
          state = JIMU_SYNC_STATE2;
        }
        break;

      case JIMU_SYNC_STATE2:
        if (JIMU_HOST_SYNC2 == ch) {
          state = JIMU_HEADER_STATE;
          p = (unsigned char *)ser_info->cmdinfbuf;
        }
        ser_info->count = 0;
        break;

      case JIMU_HEADER_STATE:
        *p = ch;
        ser_info->count++;
        if (ser_info->count >= sizeof(struct jimu_proto_header)) {
          state = JIMU_DATA;
          ser_info->count = 0;
          // SCREENPARSEDBGprint("data len:%d, 0x%X \r\n", header.len,
          // header.len);
        }
        p++;
        // SCREENPARSEDBGprint("count:%d,0x %02x\r\n", count, ch & 0xFF);
        break;

      case JIMU_DATA:
        *p = ch;
        ser_info->count++;
        // SCREENPARSEDBGprint("data :%d, %d, ch:0x%X\r\n", count, header.len,
        // ch);
        if (ser_info->count >= header->len) {
          state = JIMU_CHECK_SUM;
          ser_info->count = 0;
        }
        p++;
        break;

      case JIMU_CHECK_SUM:
        *p = ch;
        state = JIMU_SYNC_STATE1;
        ParseUiSerialMsg(ser_info->cmdinfbuf, ser_info->cmdoutbuf);
        SCREENPARSEDBGprint(
            "screenparse_cmd headinfo, cmd:0x%X, errcode:%d, len:%d\r\n",
            header->cmd, header->errcode, header->len);
        break;

      default:
        break;
    }
    ser_info->rx_buf.rx++;
    ser_info->rx_buf.rx %= SERIA_BUF_SIZE;
  }
  ser_info->p = p;
  ser_info->state = state;
  pthread_mutex_unlock(&(ser_info->rx_buf.mutex));

  return 0;
}
