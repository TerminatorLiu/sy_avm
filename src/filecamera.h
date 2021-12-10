#ifndef _SY_APP_FILECAMERA_H
#define _SY_APP_FILECAMERA_H

struct yuv_org_data {
  void *frontaddr;
  void *backaddr;
  void *leftaddr;
  void *rightaddr;
};

#define YUV_FRONT_FILE_NAME "test/jiaobanche/aa0.YUV"
#define YUV_BACK_FILE_NAME "test/jiaobanche/aa1.YUV"

#define YUV_LEFT_FILE_NAME "test/jiaobanche/aa2.YUV"
#define YUV_RIGHT_FILE_NAME "test/jiaobanche/aa3.YUV"

struct yuv_org_data InitYuvFromFile(const char *frontfile, const char *backfile,
                                    const char *leftfile,
                                    const char *rightfile);
void DeinitYuvFromFile(struct yuv_org_data *yuvaddr);

#endif
