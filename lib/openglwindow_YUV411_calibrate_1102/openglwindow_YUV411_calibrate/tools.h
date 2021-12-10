/******************************************************************************
 * Copyright 2017 The SANY Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/
#ifndef TOOLS_H
#define TOOLS_H
#include <iostream>
#include <string>

#include "modules/surroundview/opencv/std_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "modules/surroundview/opencv/std_image/stb_image_write.h"
void loadImage(unsigned char **outbuf) {
  int x, y, n;
  stbi_set_flip_vertically_on_load(false);
  outbuf[0] = stbi_load("../data/front.png", &x, &y, &n, 4);
  outbuf[1] = stbi_load("../data/back.png", &x, &y, &n, 4);
  outbuf[2] = stbi_load("../data/left.png", &x, &y, &n, 4);
  outbuf[3] = stbi_load("../data/right.png", &x, &y, &n, 4);
  outbuf[4] = stbi_load("../data/daoche.png", &x, &y, &n, 4);
}

void saveImage(int err_num, unsigned char **outbuf) {
  if (err_num < 10000) {
    std::string filename = "../Image/front/" + std::to_string(err_num) + ".png";
    stbi_write_png(filename.c_str(), 1280, 720, 4, outbuf[0], 0);

    filename = "../Image/back/" + std::to_string(err_num) + ".png";
    stbi_write_png(filename.c_str(), 1280, 720, 4, outbuf[1], 0);

    filename = "../Image/left/" + std::to_string(err_num) + ".png";
    stbi_write_png(filename.c_str(), 1280, 720, 4, outbuf[2], 0);

    filename = "../Image/right/" + std::to_string(err_num) + ".png";
    stbi_write_png(filename.c_str(), 1280, 720, 4, outbuf[3], 0);

    filename = "../Image/dc/" + std::to_string(err_num) + ".png";
    stbi_write_png(filename.c_str(), 1280, 720, 4, outbuf[4], 0);
  }
}

void saveYUYV(int err_num, unsigned char **outbuf, int size) {
  if (err_num < 1000) {
    FILE *fp;
    std::string filename = "../Image/front" + std::to_string(err_num) + ".dat";
    fp = fopen(filename.c_str(), "w");
    fwrite(outbuf[0], sizeof(unsigned char), size, fp);
    fclose(fp);

    filename = "../Image/back" + std::to_string(err_num) + ".dat";
    fp = fopen(filename.c_str(), "w");
    fwrite(outbuf[1], sizeof(unsigned char), size, fp);
    fclose(fp);

    filename = "../Image/left" + std::to_string(err_num) + ".dat";
    fp = fopen(filename.c_str(), "w");
    fwrite(outbuf[2], sizeof(unsigned char), size, fp);
    fclose(fp);

    filename = "../Image/right" + std::to_string(err_num) + ".dat";
    fp = fopen(filename.c_str(), "w");
    fwrite(outbuf[3], sizeof(unsigned char), size, fp);
    fclose(fp);

    filename = "../Image/dc" + std::to_string(err_num) + ".dat";
    fp = fopen(filename.c_str(), "w");
    fwrite(outbuf[4], sizeof(unsigned char), size, fp);
    fclose(fp);
  }
}

void loadYUYV(unsigned char **outbuf, int size) {
  int err_num = 5;
  for (int i = 0; i < 5; i++)
    outbuf[i] = (unsigned char *)calloc(size, sizeof(unsigned char));

  FILE *fp;
  std::string filename = "../data/front" + std::to_string(err_num) + ".yuyv";
  fp = fopen(filename.c_str(), "r");
  fread(outbuf[0], 1, size, fp);  //读取文件的字符串
  fclose(fp);

  filename = "../data/back" + std::to_string(err_num) + ".yuyv";
  fp = fopen(filename.c_str(), "r");
  fread(outbuf[1], 1, size, fp);  //读取文件的字符串
  fclose(fp);

  filename = "../data/left" + std::to_string(err_num) + ".yuyv";
  fp = fopen(filename.c_str(), "r");
  fread(outbuf[2], 1, size, fp);  //读取文件的字符串
  fclose(fp);

  filename = "../data/right" + std::to_string(err_num) + ".yuyv";
  fp = fopen(filename.c_str(), "r");
  fread(outbuf[3], 1, size, fp);  //读取文件的字符串
  fclose(fp);

  filename = "../data/dc" + std::to_string(err_num) + ".yuyv";
  fp = fopen(filename.c_str(), "r");
  fread(outbuf[4], 1, size, fp);  //读取文件的字符串
  fclose(fp);
}

//使用opencv进行显示YUYV数据
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

static inline float clamp(float val, float mn, float mx) {
  return (val >= mn) ? ((val <= mx) ? val : mx) : mn;
}

static void YUVToRGB(const unsigned char y, const unsigned char u,
                     const unsigned char v, unsigned char *r, unsigned char *g,
                     unsigned char *b) {
  const int y1 = (int)y - 16;
  const int u1 = (int)u - 128;
  const int v1 = (int)v - 128;

  *r = clamp(1.164f * y1 + 2.018f * v1, 0.0f, 255.0f);
  *g = clamp(1.164f * y1 - 0.813f * u1 - 0.391f * v1, 0.0f, 255.0f);
  *b = clamp(1.164f * y1 + 1.596f * u1, 0.0f, 255.0f);
}

static void YUYVToRGB(unsigned char *yuv, int w, int h) {
  int num_pixels = w * h;
  unsigned char *bgr =(unsigned char *)calloc(num_pixels * 3, sizeof(unsigned char));
  memset(bgr, 0, num_pixels * 3 * sizeof(char));

  int i, j;
  unsigned char y0, y1, u, v;
  unsigned char r, g, b;
  for (i = 0, j = 0; i < (num_pixels << 1); i += 4, j += 6) {
    y0 = yuv[i + 0];
    u = yuv[i + 1];
    y1 = yuv[i + 2];
    v = yuv[i + 3];

    YUVToRGB(y0, u, v, &r, &g, &b);
    bgr[j + 0] = b;
    bgr[j + 1] = g;
    bgr[j + 2] = r;

    YUVToRGB(y1, u, v, &r, &g, &b);
    bgr[j + 3] = b;
    bgr[j + 4] = g;
    bgr[j + 5] = r;
  }
  cv::Mat img(h, w, CV_8UC3, bgr);
  cv::imshow("img", img);
  cv::waitKey();
  delete[] bgr;
}

void showYUYV(unsigned char **outbuf)
{
    for (size_t i = 0; i < 5; i++)
    {
        YUYVToRGB(outbuf[i],1280,720);
    }
}

#endif
