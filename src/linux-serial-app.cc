#include <errno.h>
#include <stdint.h>
#include <string.h>

#include "linux-serial-app.h"
#include "mcuparse.h"
#include "proto.h"

#define min(x, y)       \
  ({                    \
    typeof(x) _x = (x); \
    typeof(y) _y = (y); \
    (void)(&_x == &_y); \
    _x < _y ? _x : _y;  \
  })

static void *SerialParseTask(void *arg);
static void *SerialRxTask(void *arg);
static void *SerialTxTask(void *arg);

static struct sy_serial_info serial_info[SER_BUF_CNT] = {
#if 0
    {
        .fd = -1,
        // devname = "/dev/ttyUSB0",
        .devname = "/dev/ttyS3",
        .name = "Central Control screen",
        .baudrate = 115200,

        .tx_buf = {
          .rx = 0,
          .wd = 0,
        },

        .rx_buf = {
          .rx = 0,
          .wd = 0,
        },
        .idx = CCS_SER_IDX,
        .parse_ser_task = serial_parse_task,
        .rx_ser_task = serial_rx_task,
        .tx_ser_task = serial_tx_task,
        .state = JIMU_SYNC_STATE1,
    },

    {
        .fd = -1,
        // devname = "/dev/ttyUSB1",
        .devname = "/dev/ttyS1",
        .name = "MCU",
        .baudrate = 115200,

        .tx_buf = {
          .rx = 0,
          .wd = 0,
        },

        .rx_buf = {
          .rx = 0,
          .wd = 0,
        },

        .idx = MCU_SER_IDX,
        .parse_ser_task = serial_parse_task,
        .rx_ser_task = serial_rx_task,
        .tx_ser_task = serial_tx_task,
        .state = MCU_SYNC_STATE1,
    }
#endif
};

/**
 * @brief Initialize serial info struct and device name, baudrate.
 **/
void InitSerialInfo() {
  serial_info[CCS_SER_IDX].fd = -1;
  serial_info[CCS_SER_IDX].devname = CCS_SER_DEV_NAME;
  serial_info[CCS_SER_IDX].name = "Central Control screen";
  serial_info[CCS_SER_IDX].baudrate = CCS_SER_DEV_BAUDRATE;
  serial_info[CCS_SER_IDX].tx_buf.rx = 0;
  serial_info[CCS_SER_IDX].tx_buf.wd = 0;
  serial_info[CCS_SER_IDX].rx_buf.rx = 0;
  serial_info[CCS_SER_IDX].rx_buf.wd = 0;
  serial_info[CCS_SER_IDX].idx = CCS_SER_IDX;
  serial_info[CCS_SER_IDX].parse_ser_task = SerialParseTask;
  serial_info[CCS_SER_IDX].rx_ser_task = SerialRxTask;
  serial_info[CCS_SER_IDX].tx_ser_task = SerialTxTask;
  serial_info[CCS_SER_IDX].state = JIMU_SYNC_STATE1;

  serial_info[MCU_SER_IDX].fd = -1;
  serial_info[MCU_SER_IDX].devname = MCU_SER_DEV_NAME;
  serial_info[MCU_SER_IDX].name = "MCU";
  serial_info[MCU_SER_IDX].baudrate = MCU_SER_DEV_BAUDRATE;
  serial_info[MCU_SER_IDX].tx_buf.rx = 0;
  serial_info[MCU_SER_IDX].tx_buf.wd = 0;
  serial_info[MCU_SER_IDX].rx_buf.rx = 0;
  serial_info[MCU_SER_IDX].rx_buf.wd = 0;
  serial_info[MCU_SER_IDX].idx = MCU_SER_IDX;
  serial_info[MCU_SER_IDX].parse_ser_task = SerialParseTask;
  serial_info[MCU_SER_IDX].rx_ser_task = SerialRxTask;
  serial_info[MCU_SER_IDX].tx_ser_task = SerialTxTask;
  serial_info[MCU_SER_IDX].state = MCU_SYNC_STATE1;
}

/**
 * @brief converts integer baud to Linux define.
 **/
static int32_t GetConvertBaudrate(int32_t baud) {
  switch (baud) {
    case 9600:
      return B9600;

    case 19200:
      return B19200;

    case 38400:
      return B38400;

    case 57600:
      return B57600;

    case 115200:
      return B115200;

    case 230400:
      return B230400;

    case 460800:
      return B460800;

    case 500000:
      return B500000;

    case 576000:
      return B576000;

    case 921600:
      return B921600;

#ifdef B1000000
    case 1000000:
      return B1000000;
#endif

#ifdef B1152000
    case 1152000:
      return B1152000;
#endif

#ifdef B1500000
    case 1500000:
      return B1500000;
#endif

#ifdef B2000000
    case 2000000:
      return B2000000;
#endif

#ifdef B2500000
    case 2500000:
      return B2500000;
#endif

#ifdef B3000000
    case 3000000:
      return B3000000;
#endif

#ifdef B3500000
    case 3500000:
      return B3500000;
#endif

#ifdef B4000000
    case 4000000:
      return B4000000;
#endif

    default:
      return -1;
  }
}

/**
 * @brief open and configure serial port.
 * @param devname serial port name
 * @param baudrate serial baudrate
 **/

int32_t OpenSerialPort(const char *devname, int32_t baudrate) {
  /* Open the serial port. Change device path as needed (currently set to an
   * standard FTDI USB-UART cable type device) */
  int32_t fd = open(devname, O_RDWR | O_NONBLOCK);

  if (fd == -1) {
    printf("Error %s \n", devname);
    printf("errno:%d, errmsg:%s\r\n", errno, strerror(errno));
    return -1;
  } else {
    printf("open %s success\r\n", devname);
  }

  /* Create new termios struc, we call it 'tty' for convention */
  struct termios tty;

  /* Read in existing settings, and handle any error */
  if (tcgetattr(fd, &tty) != 0) {
    printf("Error %d from tcgetattr: %s\n", errno, strerror(errno));
    close(fd);
    return -1;
  }

  /* Clear parity bit, disabling parity (most common) */
  tty.c_cflag &= ~PARENB;

  /* Clear stop field, only one stop bit used in communication (most common) */
  tty.c_cflag &= ~CSTOPB;

  /* Clear all bits that set the data size  */
  tty.c_cflag &= ~CSIZE;

  /* 8 bits per byte (most common) */
  tty.c_cflag |= CS8;

  /* Disable RTS/CTS hardware flow control (most common) */
  tty.c_cflag &= ~CRTSCTS;

  /* Turn on READ & ignore ctrl lines (CLOCAL = 1) */
  tty.c_cflag |= CREAD | CLOCAL;

  tty.c_lflag &= ~ICANON;

  /* Disable echo */
  tty.c_lflag &= ~ECHO;

  /* Disable erasure */
  tty.c_lflag &= ~ECHOE;

  /*  Disable new-line echo */
  tty.c_lflag &= ~ECHONL;

  /* Disable interpretation of INTR, QUIT and SUSP */
  tty.c_lflag &= ~ISIG;

  /* Turn off s/w flow ctrl */
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);

  /* Disable any special handling of received bytes */
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

  /* Prevent special interpretation of output bytes (e.g. newline chars) */
  tty.c_oflag &= ~OPOST;

  /* Prevent conversion of newline to carriage return/line feed */
  tty.c_oflag &= ~ONLCR;

#if 0
    tty.c_oflag &= ~OXTABS;  /* Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX) */
    tty.c_oflag &= ~ONOEOT;  /* Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX) */
#endif

  /* Wait for up to 1s (10 deci seconds), returning as soon as any data is
   * received. */
  tty.c_cc[VTIME] = 10;
  tty.c_cc[VMIN] = 0;

  /*  Set in/out baud rate to be 9600 */
  cfsetispeed(&tty, GetConvertBaudrate(baudrate));
  cfsetospeed(&tty, GetConvertBaudrate(baudrate));

  /* Save tty settings, also checking for error */
  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    close(fd);
    return -1;
  }
  return fd;
}

/**
 * @brief open and configure serial port.
 * @param devname serial port name
 * @param baudrate serial baudrate
 **/
static int32_t AddRxBuffer(struct sy_serial_info *ser_info, unsigned char buf[],
                           int32_t len) {
  //PrintArray(buf, len);
  pthread_mutex_lock(&(ser_info->rx_buf.mutex));
  for (int32_t i = 0; i < len; ++i) {
    ser_info->rx_buf.buf[ser_info->rx_buf.wd] = buf[i];
    ser_info->rx_buf.wd++;
    ser_info->rx_buf.wd %= SERIA_BUF_SIZE;
  }
  pthread_mutex_unlock(&(ser_info->rx_buf.mutex));
  sem_post(&(ser_info->rx_buf.sem));
  return 0;
}

/**
 * @brief add data to serial tx buffer, and wake up tx thread send data.
 * @param idx buffer id, can be MCU_SER_IDX or CCS_SER_IDX
 * @param buf data buffer
 * @param len data buffer length
 **/
int32_t AddTxBuffer(const int32_t idx, const uint8_t buf[],const int32_t len) {
  struct sy_serial_info *ser_info = NULL;
  if ((idx == MCU_SER_IDX) || (idx == CCS_SER_IDX)) {
    ser_info = &serial_info[idx];
    pthread_mutex_lock(&(ser_info->tx_buf.mutex));
    for (int32_t i = 0; i < len; ++i) {
      ser_info->tx_buf.buf[ser_info->tx_buf.wd] = buf[i];
      ser_info->tx_buf.wd++;
      ser_info->tx_buf.wd %= SERIA_BUF_SIZE;
    }
    pthread_mutex_unlock(&(ser_info->tx_buf.mutex));
    sem_post(&(ser_info->tx_buf.sem));
  }
  return 0;
}

/**
 * @brief serial receive task, read data from serial port, save data to circle
 *buffer, and wake up parse task.
 * @param arg serial parameter, file descript and rx buffer.
 **/
static void *SerialRxTask(void *arg) {
  struct pollfd pollfd[1] = {0};
  int32_t ret = 0;
  int32_t num;
  unsigned char buf[64];
  int32_t nread;
  struct sy_serial_info *p_ser_info = (struct sy_serial_info *)arg;
  int32_t fd = p_ser_info->fd;

  pollfd[0].fd = fd;
  pollfd[0].events = POLLIN;

  for (;;) {
    ret = poll(pollfd, 1, 100);
    if ((ret == -1) && (errno == EINTR)) {
      /*system call interupt by signal, wait again*/
      continue;
    }

    if ((ret > 0) && (pollfd[0].revents & POLLIN)) {
      num = 0;
      if (ioctl(pollfd[0].fd, FIONREAD, &num) == 0) {
        nread = min(num, (int)(sizeof(buf) / sizeof(buf[0])));
        ret = read(pollfd[0].fd, buf, nread);

        if (ret > 0) {
          AddRxBuffer(p_ser_info, buf, ret);
        }
      }
    }
  }
  return (void *)"serial_rx_task";
}

/**
 * @brief serial transimit task, get data from circle buffer, and send data to
 *serial port.
 * @param arg serial parameter, file descript and tx buffer.
 **/
static void *SerialTxTask(void *arg) {
  struct sy_serial_info *ser_info = (struct sy_serial_info *)arg;
  uint8_t *p = NULL;
  int32_t nwrite = 0;
  int32_t nlen = 0;
  for (;;) {
    int32_t ret = sem_wait(&(ser_info->tx_buf.sem));
    if ((ret != 0) && (errno == EINTR)) {
      /*system call interupt by signal, wait again*/
      continue;
    }

    if ((ret == 0) && (ser_info->fd > 0)) {
      pthread_mutex_lock(&(ser_info->tx_buf.mutex));
      while (ser_info->tx_buf.rx != ser_info->tx_buf.wd) {
        if (ser_info->tx_buf.wd > ser_info->tx_buf.rx) {
          p = &ser_info->tx_buf.buf[ser_info->tx_buf.rx];
          nwrite = ser_info->tx_buf.wd - ser_info->tx_buf.rx;
        } else {
          p = &ser_info->tx_buf.buf[ser_info->tx_buf.rx];
          nwrite = SERIA_BUF_SIZE - ser_info->tx_buf.rx;
        }

        nlen = write(ser_info->fd, p, nwrite);
        if (nlen == -1) {
          if (errno == EINTR) {
            continue;
          } else {
            break;
          }
        } else if (nlen > 0) {
          ser_info->tx_buf.rx += nlen;
          ser_info->tx_buf.rx %= SERIA_BUF_SIZE;
        }
      }
      pthread_mutex_unlock(&(ser_info->tx_buf.mutex));
    }
  }
  return (void *)"serial_tx_task";
}

/**
 * @brief serial parse task, parse recevie circle buffer data
 * @param arg serial parameter, file descript and rx buffer.
 **/
static void *SerialParseTask(void *arg) {
  struct sy_serial_info *ser_info = (struct sy_serial_info *)arg;
  for (;;) {
    int ret = sem_wait(&(ser_info->rx_buf.sem));
    if ((ret != 0) && (errno == EINTR)) {
      /*system call interupt by signal, wait again*/
      continue;
    }

    if (ret == 0) {
      switch (ser_info->idx) {
        case MCU_SER_IDX:
          /* code */
          McuParseCmd(ser_info);
          break;

        case CCS_SER_IDX:
          /* code */
          ScreenParseCmd(ser_info);
          break;

        default:
          break;
      }
    }
  }
  return (void *)"serial_parse_task";
}

/**
 * @brief initialize serial and parameter, create rx pthread, tx pthread, parse
 *pthread.
 **/
void InitSerialPthread(void) {
  int32_t cnt = SER_BUF_CNT;
  InitSerialInfo();
  for (int32_t i = 0; i < cnt; ++i) {
    serial_info[i].rx_buf.rx = 0;
    serial_info[i].rx_buf.wd = 0;
    sem_init(&(serial_info[i].rx_buf.sem), 0, 0);
    pthread_mutex_init(&(serial_info[i].rx_buf.mutex), NULL);

    serial_info[i].tx_buf.rx = 0;
    serial_info[i].tx_buf.wd = 0;
    sem_init(&(serial_info[i].tx_buf.sem), 0, 0);
    pthread_mutex_init(&(serial_info[i].tx_buf.mutex), NULL);

    serial_info[i].fd =
        OpenSerialPort(serial_info[i].devname, serial_info[i].baudrate);
    serial_info[i].p = serial_info[i].cmdinfbuf;
    // serial_buf[i].state = JIMU_SYNC_STATE1;

    if (-1 != serial_info[i].fd) {
      if ((pthread_create(&serial_info[i].rx_ser_th, NULL,
                          serial_info[i].rx_ser_task,
                          (void *)&serial_info[i])) == -1) {
        printf("serial_rx_pthread create failed\r\n");
      } else {
        printf("serial_rx_pthread create success\r\n");
      }
      pthread_detach(serial_info[i].rx_ser_th);

      if ((pthread_create(&serial_info[i].tx_ser_th, NULL,
                          serial_info[i].tx_ser_task,
                          (void *)&serial_info[i])) == -1) {
        printf("serial_tx_pthread create failed\r\n");
      } else {
        printf("serial_tx_pthread create success\r\n");
      }
      pthread_detach(serial_info[i].tx_ser_th);

      if ((pthread_create(&serial_info[i].parse_ser_th, NULL,
                          serial_info[i].parse_ser_task,
                          (void *)&serial_info[i])) == -1) {
        printf("serial_parse_pthread create failed\r\n");
      } else {
        printf("serial_parse_pthread create success\r\n");
      }
      pthread_detach(serial_info[i].parse_ser_th);
    }
  }
  usleep(10 * 1000);
}

/**
 * @brief Calculate XOR checksum
 **/
uint8_t CalcCheckSum(uint8_t *buf, int32_t len) {
  uint8_t a = buf[0];
  for (int i = 1; i < len; ++i) {
    a ^= buf[i];
  }
  return a;
}

#if 0
struct can_msg_t {
  uint32_t canid;
  uint32_t cs;
  uint8_t msg[8];
};

int encode_test_command(uint8_t *buf) {
  struct can_msg_t canmsg;
  canmsg.canid = 0x35;
  canmsg.cs = 0;
  for (int i = 0; i < 8; ++i) {
    canmsg.msg[i] = i + 3;
  }

  buf[0] = 0xAA;
  buf[1] = 0x55;
  buf[2] = 0xCC;
  buf[3] = 5 + sizeof(struct can_msg_t);
  memcpy(&buf[4], &canmsg, sizeof(canmsg));
  buf[buf[3] - 1] = calc_check_sum(&buf[2], buf[3] - 3);
  return buf[3];
}

void test_ser() {
  // static struct sy_serial_buf serial_buf[SER_BUF_CNT] = {
  uint8_t bufp[100] = {0};
  int nlen = 0;
  while (1) {
#if 1
    const unsigned char dt[] = {0xAA, 0x75, 0x00, 0x00, 0x00,
                                0x01, 0x00, 0x12, 0xCC};
    int nwr = write(serial_buf[0].fd, dt, sizeof(dt));
    usleep(10 * 1000);
    nwr = nwr;
#endif
    nlen = encode_test_command(bufp);
    add_tx_buffer(MCU_SER_IDX, bufp, nlen);
    // nlen = write(serial_info[1].fd , bufp, nlen);

    // print_array(bufp, nlen);
    usleep(1000 * 1000);
    /* code */
  }
}

#endif
