#ifndef _SY_APP_MCU_PARSE_H
#define _SY_APP_MCU_PARSE_H

#define MCU_HEADER_SYNC1 0xAA
#define MCU_HEADER_SYNC2 0x75

#define MCU_FORWARD_REPLY_CMD 0xAA

#define GEAR_REVERSE (2)
#define GEAR_FORWARDS (1)
#define GEAR_NEURAL (0)
#define GEAR_PARK (4)
#define GEAR_OTHERS (5)

#define TURN_NORMAL (0)
#define TURN_LEFT (1)
#define TURN_RIGHT (2)

int32_t InitCANFrameInfo();
int32_t DestroyCANFrameInfo();

int32_t UpdateBSDAlarm(uint8_t canmsg[]);

int32_t UpdateSteerAngle(uint8_t canmsg[]);

int32_t UpdateTurn(uint8_t canmsg[]);

int32_t UpdateGear(uint8_t canmsg[]);

int32_t UpdateCANFrameInfo(uint8_t *dtin);

int32_t GetBSDAlarm();
float GetSteerAngle();
int32_t GetGear();
int32_t GetTurn();


#endif
