﻿  //  * Copyright 2017 The SANY Authors. All Rights Reserved.
// *
// * Licensed under the Apache License, Version 2.0 (the "License");
// * you may not use this file except in compliance with the License.
// * You may obtain a copy of the License at
// *
// * Unless required by applicable law or agreed to in writing, software
// * distributed under the License is distributed on an "AS IS" BASIS,
// * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// * See the License for the specific language governing permissions and
// * limitations under the License.

#ifdef TIXML_USE_STL
#include <iostream>
#include <sstream>
#else
#include <stdio.h>
#endif

#if defined(WIN32) && defined(TUNE)
#include <crtdbg.h>
_CrtMemState startMemState;
_CrtMemState endMemState;
#endif

#include "tinyxml.h"
#include "avmInit.hpp"



bool XmlTest(const char *testString, const char *expected, const char *found,
             bool noEcho = false);
bool XmlTest(const char *testString, int expected, int found,
             bool noEcho = false);

static int gPass = 0;
static int gFail = 0;
#define AVM_DIR "./test/avm.xml"

#define PRINTF printf

bool XmlTest(const char *testString, const char *expected, const char *found,
             bool noEcho) {
  bool pass = !strcmp(expected, found);
  if (pass)
    printf("[pass]");
  else
    printf("[fail]");

  if (noEcho)
    printf(" %s\n", testString);
  else
    printf(" %s [%s][%s]\n", testString, expected, found);

  if (pass)
    ++gPass;
  else
    ++gFail;
  return pass;
}

bool XmlTest(const char *testString, int expected, int found, bool noEcho) {
  bool pass = (expected == found);
  if (pass)
    printf("[pass]");
  else
    printf("[fail]");

  if (noEcho)
    printf(" %s\n", testString);
  else
    printf(" %s [%d][%d]\n", testString, expected, found);

  if (pass)
    ++gPass;
  else
    ++gFail;
  return pass;
}

void NullLineEndings(char *p) {
  while (p && *p) {
    if (*p == '\n' || *p == '\r') {
      *p = 0;
      return;
    }
    ++p;
  }
}

#define cvEndWriteStruct(...)
#define cvAttrList(...) 0
#define cvStartWriteStruct(a1, node_name, a3, a4, a5)     \
  pnode = proot->FirstChildElement(nodename = node_name); \
  if (pnode == 0) {                                       \
    xml.SaveFile();                                       \
    PRINTF("node not found:%s\n", node_name);             \
  }

#define cvWriteRawData(a1, res, count, a2) \
  write_param(res, count, pnode, proot, nodename)

void write_param(int *res, int count, TiXmlElement *pnode, TiXmlElement *proot,
                 const char *nodename) {
  do {
    char strbuf[256] = {0};
    int *pi = res;
    for (int i = 0; i < count; i++) {
      char num[32] = {0};
      if (i + 1 < count) snprintf(num, sizeof(num), "%d", *pi++);
      // sprintf(num, "%d ", *pi++);
      else
        snprintf(num, sizeof(num), "%d", *pi++);
      // sprintf(num, "%d", *pi++);
      snprintf(strbuf, sizeof(strbuf), "%s", num);
      // strcat(strbuf, num);
    }

    if (pnode != 0) proot->RemoveChild(pnode);
    {
      TiXmlElement *pnode = new TiXmlElement(nodename);
      TiXmlText *text = new TiXmlText(strbuf);
      pnode->LinkEndChild(text);
      proot->LinkEndChild(pnode);
    }
  } while (0);
}

#define cvGetFileNodeByName(a1, a2, node_name) \
  proot->FirstChildElement(nodename = node_name)

#define cvReadRawData(a1, a2, value_addr, c) \
  read_param(value_addr, mat_node, nodename)

void read_param(int *value_addr, TiXmlElement *mat_node, const char *nodename) {
  if (mat_node != 0) {
    int ints[128] = {0};
    const char *value = mat_node->GetText();
    const char *valueend = value + strlen(value);
    int readint = 0;
    while (value) {
      if (-1 == sscanf(value, "%d", &ints[readint])) {
        break;
      }
      readint++;
      value = strstr(value, " ");
      if (value)
        value++;
      else
        break;
    }
    int *pi = value_addr;
    for (int i = 0; i < readint; i++) {
      *pi++ = ints[i];
    }
  } else {
    PRINTF("node not found:%s\n", nodename);
  }
}

void readParamsXML() {

  // CvFileStorage* fs_read_xml =
  // cvOpenFileStorage(PARADIR,memstorage,CV_STORAGE_READ);
  TiXmlDocument xml(AVM_DIR);
  if (!xml.LoadFile()) {
    printf("file error\n");
  }
  TiXmlElement *proot = xml.FirstChildElement("opencv_storage");
  TiXmlElement *mat_node = 0;
  if (proot == 0) {
    return;
  }
  const char *nodename = "aaaaaa";

  	mat_node = cvGetFileNodeByName(fs_read_xml, NULL, "A1");
	cvReadRawData(fs_read_xml,mat_node,frontCamParams.mrInt,"i");

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"A2");
	cvReadRawData(fs_read_xml,mat_node,frontCamParams.mtInt,"i");

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"A3");
	cvReadRawData(fs_read_xml,mat_node,rearCamParams.mrInt,"i");

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"A4");
	cvReadRawData(fs_read_xml,mat_node,rearCamParams.mtInt,"i");

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"A5");
	cvReadRawData(fs_read_xml,mat_node,leftCamParams.mrInt,"i");

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"A6");
	cvReadRawData(fs_read_xml,mat_node,leftCamParams.mtInt,"i");

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"A7");
	cvReadRawData(fs_read_xml,mat_node,rightCamParams.mrInt,"i");

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"A8");
	cvReadRawData(fs_read_xml,mat_node,rightCamParams.mtInt,"i");

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"B1");
	cvReadRawData(fs_read_xml,mat_node,frontCamParams.mimdInt,"i");

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"B2");
	cvReadRawData(fs_read_xml,mat_node,rearCamParams.mimdInt,"i");

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"B3");
	cvReadRawData(fs_read_xml,mat_node,leftCamParams.mimdInt,"i");

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"B4");
	cvReadRawData(fs_read_xml,mat_node,rightCamParams.mimdInt,"i");

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"C1");
	cvReadRawData(fs_read_xml,mat_node,&para_field.carWorldX,"i");

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"C2");
	cvReadRawData(fs_read_xml,mat_node,&para_field.car_width,"i");

}

