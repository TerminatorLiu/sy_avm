#include <stdint.h>
#include <string.h>

#include "linux-serial-app.h"
#include "mcuparse.h"
#include "proto.h"
#include "conf.h"

//#define DEBUG
#ifdef DEBUG
#define MCUPARSEDBGprint(...) printf(__VA_ARGS__)
#else
#define MCUPARSEDBGprint(...)
#endif

#define CANFRAME_UNUSED (0)
#define CANFRAME_BSD (1)
#define CANFRAME_STEER (2)
#define CANFRAME_GEAR (3)
#define CANFRAME_TURN (4)



struct CANFrameInfo{
  int32_t BSD_alarm;
  pthread_rwlock_t BSD_alarm_lock;
  float steer_angle;
  pthread_rwlock_t steer_angle_lock;
  int gear;//
  pthread_rwlock_t gear_lock;
  int turn;
  pthread_rwlock_t turn_lock;
};

struct CANFrameInfo gCANFrameInfo;

int32_t InitCANFrameInfo()
{
  int ret = 0;
  gCANFrameInfo.BSD_alarm = 0;
  gCANFrameInfo.steer_angle = 0.f;
  ret = pthread_rwlock_init(&(gCANFrameInfo.BSD_alarm_lock),NULL);
  ret = pthread_rwlock_init(&(gCANFrameInfo.steer_angle_lock),NULL);
  ret = pthread_rwlock_init(&(gCANFrameInfo.gear_lock),NULL);
  ret = pthread_rwlock_init(&(gCANFrameInfo.turn_lock),NULL);
  return ret;
}
int32_t DestroyCANFrameInfo()
{
  int ret = 0;
  ret = pthread_rwlock_destroy(&(gCANFrameInfo.BSD_alarm_lock));
  ret = pthread_rwlock_destroy(&(gCANFrameInfo.steer_angle_lock));
  ret = pthread_rwlock_destroy(&(gCANFrameInfo.gear_lock));
  ret = pthread_rwlock_destroy(&(gCANFrameInfo.turn_lock));
  return ret;
}

int32_t UpdateBSDAlarm(uint8_t canmsg[])
{
  int ret = 0;
  ret = pthread_rwlock_wrlock(&(gCANFrameInfo.BSD_alarm_lock));
  if(canmsg[0] & 0x80)
  {
    gCANFrameInfo.BSD_alarm = 1;
  }
  else
  {
    gCANFrameInfo.BSD_alarm = 0;
  }
  ret = pthread_rwlock_unlock(&(gCANFrameInfo.BSD_alarm_lock));
  return ret;
}

int32_t UpdateSteerAngle(uint8_t canmsg[])
{
  int ret = 0;
  ret = pthread_rwlock_wrlock(&(gCANFrameInfo.steer_angle_lock));
  printf("%x %x %x %x %x %x %x %x\n",canmsg[0],canmsg[1],canmsg[2],canmsg[3],canmsg[4],canmsg[5],canmsg[6],canmsg[7]);
  uint16_t low = canmsg[0];
  uint16_t high = canmsg[1]<<8;
  uint16_t raw_steer = high | low;

  gCANFrameInfo.steer_angle = ((float)raw_steer)/1024 - 31.374;
  ret = pthread_rwlock_unlock(&(gCANFrameInfo.steer_angle_lock));
  return ret;
}

int32_t UpdateGear(uint8_t canmsg[])
{
  int ret = 0;
  int gear = GEAR_OTHERS;
  //printf("%x %x %x %x %x %x %x %x\n",canmsg[0],canmsg[1],canmsg[2],canmsg[3],canmsg[4],canmsg[5],canmsg[6],canmsg[7]);
#if VEHICLE_TYPE == VEHICLE_TYPE_CHANGE_BATTERY
  if(canmsg[3] >= 0 && canmsg[3]<=0x7C)
  {
    gear = GEAR_REVERSE;
  }
  if(canmsg[3] == 0x7D)
  {
    gear = GEAR_NEURAL;
  }
  if(canmsg[3] >=0x7E && canmsg[3] <=0xFA)
  {
    gear = GEAR_FORWARDS;
  }
  if(canmsg[3] == 0xFB)
  {
    gear = GEAR_PARK;
  }
#endif
#if VEHICLE_TYPE == VEHICLE_TYPE_FUEL
  if(0x1u & canmsg[2])
  {
    gear = GEAR_NEURAL;
  }
  if((!(0x1u & canmsg[2])) && (0x2u & canmsg[0]))
  {
    gear = GEAR_REVERSE;
  }
  if((!(0x1u & canmsg[2])) && (!(0x2u & canmsg[0])))
  {
    gear = GEAR_FORWARDS;
  }
#endif
  ret = pthread_rwlock_wrlock(&(gCANFrameInfo.gear_lock));
  gCANFrameInfo.gear = gear;
  ret = pthread_rwlock_unlock(&(gCANFrameInfo.gear_lock));
  return ret;
}

int32_t UpdateTurn(uint8_t canmsg[])
{
  int ret = 0;
  int turn = TURN_NORMAL;
  //printf("%x %x %x %x %x %x %x %x\n",canmsg[0],canmsg[1],canmsg[2],canmsg[3],canmsg[4],canmsg[5],canmsg[6],canmsg[7]);
#if VEHICLE_TYPE == VEHICLE_TYPE_CHANGE_BATTERY
  if(canmsg[1] & 0x10)
  {
    turn = TURN_RIGHT;
  }
  if(canmsg[1] & 0x40)
  {
    turn = TURN_LEFT;
  }
#endif
#if VEHICLE_TYPE == VEHICLE_TYPE_FUEL
  if(canmsg[1] & 0x02u)
  {
    turn = TURN_RIGHT;
  }
  if(canmsg[1] & 0x01u)
  {
    turn = TURN_LEFT;
  }
#endif
  ret = pthread_rwlock_wrlock(&(gCANFrameInfo.turn_lock));
  gCANFrameInfo.turn = turn;
  ret = pthread_rwlock_unlock(&(gCANFrameInfo.turn_lock));
}

int32_t ParseCANID(uint8_t *canid)
{
  //printf("%x %x %x %x\n",canid[0],canid[1],canid[2],canid[3]);
#if VEHICLE_TYPE == VEHICLE_TYPE_CHANGE_BATTERY
#if BSD_TYPE == BSD_TYPE_CT
  if(canid[0] == 0x00 && canid[1] == 0x00 && canid[2] == 0x07 && canid[3] == 0xB5)
  {
    return CANFRAME_BSD;
  }
#endif
  if(canid[0] == 0x18 && canid[1] == 0xF0 && canid[2] == 0x09 && canid[3] == 0x0B)
  {
    return CANFRAME_STEER;
  }
  if(canid[0] == 0x18 && canid[1] == 0xF0 && canid[2] == 0x05 && canid[3] == 0x03)
  {
    return CANFRAME_GEAR;
  }
  if(canid[0] == 0x18 && canid[1] == 0xFE && canid[2] == 0x41 && canid[3] == 0x21)
  {
    return CANFRAME_TURN;
  }
#endif
#if VEHICLE_TYPE == VEHICLE_TYPE_FUEL
  if(canid[0] == 0x00 && canid[1] == 0x00 && canid[2] == 0x07 && canid[3] == 0xF1)
  {
    return CANFRAME_GEAR;
  }
  if(canid[0] == 0x00 && canid[1] == 0x00 && canid[2] == 0x07 && canid[3] == 0xF2)
  {
    return CANFRAME_TURN;
  }
#endif
  return CANFRAME_UNUSED;
}

#if VEHICLE_TYPE == VEHICLE_TYPE_FUEL

#endif

int32_t UpdateCANFrameInfo(uint8_t *dtin)
{
  int32_t count = (dtin[1] - 5)/16;
  uint8_t *p = dtin+2;
  for(int i = 0;i<count;++i)
  {
    u_int32_t canmsg_type = ParseCANID(p);
    p += 8;
    switch (canmsg_type)
    {
    case CANFRAME_BSD:
      UpdateBSDAlarm(p);
      break;
    case CANFRAME_STEER:
      UpdateSteerAngle(p);
      break;
    case CANFRAME_GEAR:
      UpdateGear(p);
      break;
    case CANFRAME_TURN:
      UpdateTurn(p);
      break;
    default:
      break;
    }
    p += 8;
  }
  return 0;
}

int32_t GetBSDAlarm()
{
  int32_t res = 0;
  pthread_rwlock_rdlock(&(gCANFrameInfo.BSD_alarm_lock));
  res = gCANFrameInfo.BSD_alarm;
  pthread_rwlock_unlock(&(gCANFrameInfo.BSD_alarm_lock));
  return res;
}
float GetSteerAngle()
{
  float res = 0.f;
  pthread_rwlock_rdlock(&(gCANFrameInfo.steer_angle_lock));
  res = gCANFrameInfo.steer_angle;
  //printf("steer angle = %f\n",res);
  pthread_rwlock_unlock(&(gCANFrameInfo.steer_angle_lock));
  return res;
}

int32_t GetGear()
{
  int32_t res ;
  pthread_rwlock_rdlock(&(gCANFrameInfo.gear_lock));
  res = gCANFrameInfo.gear;
  pthread_rwlock_unlock(&(gCANFrameInfo.gear_lock));
  return res;
}

int32_t GetTurn()
{
  int32_t res ;
  pthread_rwlock_rdlock(&(gCANFrameInfo.turn_lock));
  res = gCANFrameInfo.turn;
  pthread_rwlock_unlock(&(gCANFrameInfo.turn_lock));
  return res;
}



/**
 * @brief Convert the array into hexadecimal string and output it to the
 *terminal.
 **/
void PrintArray(const uint8_t *dt, int32_t len) {
  printf("data:");
  for (int32_t i = 0; i < len; ++i) {
    printf("%02X", dt[i]);
  }
  printf("\r\n");
}

/**
 * @brief Encode the data according to the jimu protocol.
 * @param cmd command field
 * @param data data field
 * @param dtlen data length
 **/
int32_t EncodeJimuOutCommand(uint8_t *dtout, uint8_t cmd, uint8_t *data,
                             uint16_t dtlen) {
  
  struct jimu_proto_header *outheader = (struct jimu_proto_header *)&dtout[2];
  dtout[0] = MCU_HEADER_SYNC1; /*0xAA*/
  dtout[1] = MCU_HEADER_SYNC2; /*0x75*/
  outheader->cmd = cmd;
  outheader->len = dtlen;

  outheader->errcode = 0;
  memcpy(outheader->dt, data, dtlen);
  outheader->dt[dtlen] = CalcCheckSum(dtout, outheader->len + 7);
  return outheader->len + 8;
}

/**
 * @brief  parse the data from mcu.
 * @param dtin input data
 * @param dtout output data, for cached the encode proto data
 **/
int32_t ParseMcuSerialMsg(uint8_t *dtin, uint8_t *dtout) {
  struct mcu_proto_header *header = (struct mcu_proto_header *)dtin;
  int32_t ret = 0;
  uint8_t crcval = CalcCheckSum(&dtin[0], header->len - 2);
  MCUPARSEDBGprint("crcval:0x%02X, len:%d\r\n", 0, header->len);
  if (0 == crcval) {
    UpdateCANFrameInfo(dtin);
    switch (header->cmd) {
      case MCU_FORWARD_CMD:
        /* tow byte sync char, one byte command char , one byte length , one
         */
#if 1//ENABLE_DISPLAYMODE_SWITCH == 0
        ret = EncodeJimuOutCommand(dtout, MCU_FORWARD_REPLY_CMD, header->dt,
                                   header->len - 5);
        AddTxBuffer(CCS_SER_IDX, dtout, ret);
#endif
        break;

      default:
        break;
    }
  }

  return ret;
}

/**
 * @brief  parse proto and call command excution function.
 * @param ser_info serial parameter and data buffer
 **/
int32_t McuParseCmd(struct sy_serial_info *ser_info) {
  uint8_t ch = 0;
  struct mcu_proto_header *header =
      (struct mcu_proto_header *)ser_info->cmdinfbuf;

  pthread_mutex_lock(&(ser_info->rx_buf.mutex));
  uint8_t *p = (uint8_t *)ser_info->p;
  enum mcu_proto_state_t state = (enum mcu_proto_state_t)ser_info->state;

  while (ser_info->rx_buf.rx != ser_info->rx_buf.wd) {
    /* code */
    ch = ser_info->rx_buf.buf[ser_info->rx_buf.rx];
    switch (state) {
      case MCU_SYNC_STATE1:
        if (MCU_HOST_SYNC1 == ch) {
          state = MCU_SYNC_STATE2;
        }
        break;

      case MCU_SYNC_STATE2:
        if (MCU_HOST_SYNC2 == ch) {
          state = MCU_HEADER_STATE;
          p = (uint8_t *)ser_info->cmdinfbuf;
        }
        ser_info->count = 0;
        break;

      case MCU_HEADER_STATE:
        *p = ch;
        ser_info->count++;
        if (ser_info->count >= sizeof(*header)) {
          state = MCU_DATA;
          ser_info->count = 0;
        }
        p++;
        break;

      case MCU_DATA:
        *p = ch;
        ser_info->count++;
        if (ser_info->count >= header->len - 5) {
          state = MCU_CHECK_SUM;
          ser_info->count = 0;
        }
        p++;
        break;

      case MCU_CHECK_SUM:
        *p = ch;
        state = MCU_SYNC_STATE1;
        ParseMcuSerialMsg(ser_info->cmdinfbuf, ser_info->cmdoutbuf);
        break;

      default:
        break;
    }
    ser_info->rx_buf.rx++;
    ser_info->rx_buf.rx %= SERIA_BUF_SIZE;
  }
  MCUPARSEDBGprint("leave while\r\n");
  ser_info->p = p;
  ser_info->state = state;
  pthread_mutex_unlock(&(ser_info->rx_buf.mutex));

  return 0;
}
