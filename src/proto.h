#ifndef _SY_APP_PROTO_H
#define _SY_APP_PROTO_H

struct jimu_proto_header {
  unsigned char cmd;
  unsigned short errcode;
  unsigned short len;
  unsigned char dt[0];
} __attribute__((packed));

enum jimu_proto_state_t {
  JIMU_SYNC_STATE1 = 0,
  JIMU_SYNC_STATE2,
  JIMU_HEADER_STATE,
  JIMU_DATA,
  JIMU_CHECK_SUM,
};

#define JIMU_HOST_SYNC1 0xAA
#define JIMU_HOST_SYNC2 0x75

struct mcu_proto_header {
  uint8_t cmd;
  uint8_t len;
  uint8_t dt[0];
};

enum mcu_proto_state_t {
  MCU_SYNC_STATE1 = 0,
  MCU_SYNC_STATE2,
  MCU_HEADER_STATE,
  MCU_DATA,
  MCU_CHECK_SUM,
};

#define MCU_HOST_SYNC1      (0xAA)
#define MCU_HOST_SYNC2      (0x55)

#define MCU_FORWARD_CMD     (0xCC)
#define APP_LOG_CMD         (0xA0)
#define APP_CALIB_CMD       (0xBB)
#define APP_DMS_DECTECT_CMD (0x14)

int EncodeJimuOutCommand(uint8_t *dtout, uint8_t cmd, uint8_t *data,
                                uint16_t dtlen);
/*
横屏二分屏 （左2D）
0x10 2D+0通道
0x11 2D+1通道
0x12 2D+2通道
0x13 2D+3通道
0x14 2D+4通道 dms 旋转90度
0x15 2D+5通道
0x16 2D+6通道
0x17 2D+7通道
0x1F 2D+3D
*/

#endif