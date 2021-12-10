#include <stdio.h>
#include <stdlib.h>

#include "conf.h"
#include "filecamera.h"

/**
 * @brief Read the local yuv file and test the automatic calibration
 * @param frontfile front camera yuv file
 * @param backfile  back camera yuv file
 * @param leftfile  left camera yuv file
 * @param rightfile right camera yuv file
 **/
struct yuv_org_data InitYuvFromFile(const char *frontfile, const char *backfile,
                                    const char *leftfile,
                                    const char *rightfile) {
  int32_t length =
      CAMERA_WIDTH * CAMERA_HEIGHT + CAMERA_WIDTH * CAMERA_HEIGHT / 2;
  struct yuv_org_data yuvaddr;
  FILE *fp = NULL;

  yuvaddr.frontaddr = malloc(length);
  yuvaddr.backaddr = malloc(length);
  yuvaddr.leftaddr = malloc(length);
  yuvaddr.rightaddr = malloc(length);
  if ((NULL == yuvaddr.frontaddr) || (NULL == yuvaddr.backaddr) ||
      (NULL == yuvaddr.leftaddr) || (NULL == yuvaddr.rightaddr)) {
    DeinitYuvFromFile(&yuvaddr);
    return yuvaddr;
  } else {
    memset(yuvaddr.frontaddr, 0, length);
    memset(yuvaddr.backaddr, 0, length);
    memset(yuvaddr.leftaddr, 0, length);
    memset(yuvaddr.rightaddr, 0, length);
  }

  fp = fopen(YUV_FRONT_FILE_NAME, "r");
  if (NULL != fp) {
    fread(yuvaddr.frontaddr, 1, length, fp);
    fclose(fp);
  }

  fp = fopen(YUV_BACK_FILE_NAME, "r");
  if (NULL != fp) {
    fread(yuvaddr.backaddr, 1, length, fp);
    fclose(fp);
  }

  fp = fopen(YUV_LEFT_FILE_NAME, "r");
  if (NULL != fp) {
    fread(yuvaddr.leftaddr, 1, length, fp);
    fclose(fp);
  }

  fp = fopen(YUV_RIGHT_FILE_NAME, "r");
  if (NULL != fp) {
    fread(yuvaddr.rightaddr, 1, length, fp);
    fclose(fp);
  }
  return yuvaddr;
}

/**
 * @brief release  yuv file allocate merory
 * @param yuvaddr front, back, right, left camera data address.
 **/
void DeinitYuvFromFile(struct yuv_org_data *yuvaddr) {
  if (yuvaddr->frontaddr) {
    free(yuvaddr->frontaddr);
    yuvaddr->frontaddr = NULL;
  }

  if (yuvaddr->backaddr) {
    free(yuvaddr->backaddr);
    yuvaddr->backaddr = NULL;
  }

  if (yuvaddr->leftaddr) {
    free(yuvaddr->leftaddr);
    yuvaddr->leftaddr = NULL;
  }

  if (yuvaddr->rightaddr) {
    free(yuvaddr->rightaddr);
    yuvaddr->rightaddr = NULL;
  }
}
