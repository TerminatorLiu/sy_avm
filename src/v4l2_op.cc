#include "sunxi_camera_v2.h"
#include "sunxi_display2.h"

#include "v4l2_op.h"

/**
 * @brief check v4l2 system call weather success, output error message.
 * @param flag weather the system call failed, if true, output error message.
 * @param file __FILE__, Indicate source filename
 * @param line __LINE__, Indicate lines
 **/
void V4l2OpCheckErrno(bool flag, char *file, int line) {
  if (flag) {
    printf("file:%s,line:%d, error message:%s\r\n", file, line,
           strerror(errno));
  }
}

/**
 * @brief open camera device.
 * @param v4l2_op camera device name.
 * @return camera file descriptor.
 **/
int V4l2OpOpen(v4l2_op_interface *v4l2_op) {
  return open(v4l2_op->devname, O_RDWR | O_NONBLOCK, 0);
}

/**
 * @brief select a video input applications store the number of the desired
 *input in an intege.
 * @param v4l2_op camera file descriptor.
 * @return On success 0 is returned, on error -1.
 **/
int V4l2OpVidiocsInput(v4l2_op_interface *v4l2_op) {
  struct v4l2_input inp = {0};
  int fd = v4l2_op->fd;
  inp.index = 0;
  return ioctl(fd, VIDIOC_S_INPUT, &inp);
}

/**
 * @brief  set the streaming parameters.
 * @param v4l2_op camera file descriptor.
 * @return On success 0 is returned, on error -1.
 **/
int V4l2OpVidiocsParm(v4l2_op_interface *v4l2_op) {  
#if 0  
  struct v4l2_streamparm parms;
  memset(&parms, 0, sizeof(parms));
  int fd = v4l2_op->fd;
  parms.type = SY_V4L2_BUF_TYPE;
  parms.parm.capture.timeperframe.numerator = 1;
  parms.parm.capture.timeperframe.denominator = 25; /*25 fps*/

  parms.parm.capture.capturemode = V4L2_MODE_VIDEO; /* V4L2_MODE_VIDEO */
  return ioctl(fd, VIDIOC_S_PARM, &parms);
#endif
  return 0;  
}

/**
 * @brief  set the streaming parameters.
 * @param v4l2_op camera file descriptor.
 * @return On success 0 is returned, on error -1.
 **/
int V4l2OpVidiocsFmt(v4l2_op_interface *v4l2_op) {
  struct v4l2_format fmt = {0};
  int fd = v4l2_op->fd;

  fmt.type = SY_V4L2_BUF_TYPE;
  fmt.fmt.pix_mp.width = CAMERA_WIDTH;
  fmt.fmt.pix_mp.height = CAMERA_HEIGHT;
  fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_YUV420;
  fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;
  return ioctl(fd, VIDIOC_S_FMT, &fmt);
}

/**
 * @brief   get the data format.
 * @param v4l2_op camera file descriptor.
 * @return On success 0 is returned, on error -1.
 **/
int V4l2OpVidiocgParm(v4l2_op_interface *v4l2_op) {
  struct v4l2_format fmt;
  int fd = v4l2_op->fd;
  int ret;

  fmt.type = SY_V4L2_BUF_TYPE;
  printf("get fmt...\n");
  if ((ret = ioctl(fd, VIDIOC_G_FMT, &fmt)) == -1) {
    perror(" Unable to get format ");
    return ret;
  }

  {
    printf("fmt.type:\t\t%d\n", fmt.type);
    printf("pix.pixelformat:\t%c%c%c%c\n", fmt.fmt.pix.pixelformat & 0xFF,
           (fmt.fmt.pix.pixelformat >> 8) & 0xFF,
           (fmt.fmt.pix.pixelformat >> 16) & 0xFF,
           (fmt.fmt.pix.pixelformat >> 24) & 0xFF);
    printf("pix.height:\t\t%d\n", fmt.fmt.pix.height);
    printf("pix.width:\t\t%d\n", fmt.fmt.pix.width);
    printf("pix.field:\t\t%d\n", fmt.fmt.pix.field);
    v4l2_op->nplanes = fmt.fmt.pix_mp.num_planes;
    printf("resolution got from sensor = %d*%d num_planes = %d\n",
           fmt.fmt.pix_mp.width, fmt.fmt.pix_mp.height,
           fmt.fmt.pix_mp.num_planes);
  }
  return ret;
}

/**
 * @brief   Initiate Memory Mapping (User Pointer I/O or DMA buffer I/O) .
 * @param v4l2_op camera file descriptor.
 * @return On success 0 is returned, on error -1.
 **/
int V4l2OpVidiocReqbufs(v4l2_op_interface *v4l2_op) {
  struct v4l2_requestbuffers req = {0};
  int fd = v4l2_op->fd;
  req.count = FRAME_NUM;
  req.type = SY_V4L2_BUF_TYPE;
  req.memory = V4L2_MEMORY_MMAP; /* V4L2_MEMORY_USERPTR */
  return ioctl(fd, VIDIOC_REQBUFS, &req);
}

/**
 * @brief   Query the status of a buffer and exchange a buffer with the driver.
 * @param v4l2_op camera file descriptor.
 * @return On success 0 is returned, on error -1.
 **/
int V4l2OpVidiocQuerybuf(v4l2_op_interface *v4l2_op) {
  int fd = v4l2_op->fd;
  int ret = -1;
  for (int i = 0; i < FRAME_NUM; ++i) {
    struct v4l2_buffer buf = {0};
    struct v4l2_plane plane = {0};

    const int size = CAMERA_WIDTH * CAMERA_HEIGHT * 3 / 2;

    buf.type = SY_V4L2_BUF_TYPE;
    buf.memory = V4L2_MEMORY_MMAP;  // V4L2_MEMORY_USERPTR;
    buf.index = i;
    buf.length = 1;
    buf.m.planes = &plane;
    plane.length = size;

    if (ret == ioctl(fd, VIDIOC_QUERYBUF, &buf)) {
      printf("VIDIOC_QUERYBUF error\n");
      V4L2_OP_CHECK(ret == -1);
      return -1;
    }

    int length = plane.length;
    int offset = plane.m.mem_offset;

    v4l2_op->mapbuf[i].length = length;
    v4l2_op->mapbuf[i].start =
        mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);

    if (v4l2_op->mapbuf[i].start == MAP_FAILED) {
      printf("mmap failed\n");
      V4L2_OP_CHECK(v4l2_op->mapbuf[i].start == MAP_FAILED);
      // close(fd);
      return -1;
    }

    if (ret == ioctl(fd, VIDIOC_QBUF, &buf)) {
      printf("[%s]VIDIOC_QBUF failed\n", v4l2_op->devname);
      // close(fd);
      V4L2_OP_CHECK(ret == -1);
      return -1;
    }
  }
  return 0;
}

/**
 * @brief start streaming I/O.
 * @param v4l2_op camera file descriptor.
 * @return On success 0 is returned, on error -1.
 **/
int V4l2OpVidiocStreamOn(v4l2_op_interface *v4l2_op) {
  int fd = v4l2_op->fd;
  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  return ioctl(fd, VIDIOC_STREAMON, &type);
}

/**
 * @brief stop streaming I/O.
 * @param v4l2_op camera file descriptor.
 * @return On success 0 is returned, on error -1.
 **/
int V4l2OpVidiocStreamOff(v4l2_op_interface *v4l2_op) {
  int fd = v4l2_op->fd;
  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  return ioctl(fd, VIDIOC_STREAMOFF, &type);
}
