  //  * Copyright 2017 The SANY Authors. All Rights Reserved.
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

#define SD_CFG_DVR_CONFIG_STR_LEN 320
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
#define cvStartWriteStruct(a1,node_name,a3,a4,a5) 	\
		  pnode=proot->FirstChildElement(nodename=node_name); \
		  if(pnode==0) {xml.SaveFile(); PRINTF("node not found:%s\n", node_name); }
	  
#define cvWriteRawData(a1, res, count, a2) write_param(res, count, pnode, proot, nodename)
	  
	  void write_param(int*res, int count, TiXmlElement*pnode, TiXmlElement*proot, const char*nodename)
	  {
		  do{ 
			  char strbuf[256]={0};   
			  int *pi = res;  
			  for(int i=0; i<count; i++){	  
				  char num[32]={0};   
				  if(i+1<count)   
					  sprintf(num, "%d ", *pi++);	  
				  else		  
					  sprintf(num, "%d", *pi++);  
				  strcat(strbuf, num);	  
			  }   
	  
			  if(pnode!=0)	
				  proot->RemoveChild(pnode);		  
	  
			  {
	  
				  TiXmlElement* pnode = new TiXmlElement(nodename);
				  TiXmlText *text =new TiXmlText(strbuf);
				  pnode->LinkEndChild(text);
				  proot->LinkEndChild(pnode);
			  }
		  }while(0);
	  }


#define cvWriteRawDataType(a1, res, count, a2, type) writeParamType(res, count, pnode, proot, nodename, type)

void writeParamType(void *res, int count, TiXmlElement*pnode, TiXmlElement*proot, const char*nodename, eSdXmlType type)
{
    do
    {
        char strbuf[SD_CFG_DVR_CONFIG_STR_LEN] = {0};
        switch(type)
        {
            case SD_XML_PP_TYPE_UINT8:
            {
                unsigned char *pi = (unsigned char *)res;
                for(int i = 0; i < count; i++)
                {
                    char num[32] = {0};
                    if(i + 1 < count)
                        sprintf(num, "%d ", *pi++);
                    else
                        sprintf(num, "%d", *pi++);
                    strcat(strbuf, num);
                }
                break;
            }
            case SD_XML_PP_TYPE_UINT32:
            {
                int *pi = (int *)res;
                for(int i = 0; i < count; i++)
                {
                    char num[32] = {0};
                    if(i + 1 < count)
                        sprintf(num, "%d ", *pi++);
                    else
                        sprintf(num, "%d", *pi++);
                    strcat(strbuf, num);
                }
                break;
            }
            case SD_XML_PP_TYPE_STRING:
            {
                unsigned char *pi = (unsigned char *)res;
                for(int i = 0; i < count; i++)
                {
                    char num[SD_CFG_DVR_CONFIG_STR_LEN] = {0};
                    if(i + 1 < count)
                        sprintf(num, "%s ", pi);
                    else
                        sprintf(num, "%s", pi);
                    strcat(strbuf, num);
                }
                break;
            }
            case SD_XML_PP_TYPE_FLOAT:
            {
                float *pi = (float *)res;
                for(int i = 0; i < count; i++)
                {
                    char num[32] = {0};
                    if(i + 1 < count)
                        sprintf(num, "%f ", *pi++);
                    else
                        sprintf(num, "%f", *pi++);
                    strcat(strbuf, num);
                }
                break;
            }
        defalut:
            {
                unsigned char *pi = (unsigned char *)res;
                for(int i = 0; i < count; i++)
                {
                    char num[32] = {0};
                    if(i + 1 < count)
                        sprintf(num, "%d ", *pi++);
                    else
                        sprintf(num, "%d", *pi++);
                    strcat(strbuf, num);
                }
                break;
            }
        }

        if(pnode != 0)
            proot->RemoveChild(pnode);

        {
            TiXmlElement* pnode = new TiXmlElement(nodename);
            TiXmlText *text = new TiXmlText(strbuf);
            pnode->LinkEndChild(text);
            proot->LinkEndChild(pnode);
        }
    }
    while(0);
}


#define cvGetFileNodeByName(a1, a2, node_name) \
  proot->FirstChildElement(nodename = node_name)

#define cvReadRawData(a1, a2, value_addr, c) \
  read_param(value_addr, mat_node, nodename)

void read_param(int *value_addr, TiXmlElement *mat_node, const char *nodename) {
  if (mat_node != 0) {
    int ints[128] = {0};
    const char *value = mat_node->GetText();
    //const char *valueend = value + strlen(value);
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


#define SD_READ_XML_TYPE(type, fmt) \
    type readBuf[128]={0}; \
    type *pi = (type *)value_addr; \
    while(value) \
    { \
        if(-1==sscanf(value, fmt, &readBuf[readCnt])) \
        { \
            break; \
        } \
        readCnt++; \
        value = strstr(value, " "); \
        if(value) \
        { \
            value++; \
        } \
        else \
        { \
            break; \
        } \
    } \
    for(int i=0; i<readCnt; i++) \
    { \
        *pi++=readBuf[i]; \
    }

void readParamType(void *value_addr, TiXmlElement *mat_node, const char *nodename, eSdXmlType type)
{
	if(mat_node != 0)
	{
		const char*value = mat_node->GetText();
		int readCnt = 0;

		switch(type)
		{
			case SD_XML_PP_TYPE_UINT8:
			{
				SD_READ_XML_TYPE(unsigned char, "%d");
				break;
			}
			case SD_XML_PP_TYPE_UINT32:
			{
				SD_READ_XML_TYPE(int, "%d");
				break;
			}
			case SD_XML_PP_TYPE_STRING:
			{
				memcpy((unsigned char *)value_addr, value, strlen(value));
				break;
			}
			case SD_XML_PP_TYPE_FLOAT:
			{
				SD_READ_XML_TYPE(float, "%f");
				break;
			}
		defalut:
			{
				SD_READ_XML_TYPE(unsigned char, "%d");
				break;
			}
		}
	}
	else
	{
		PRINTF("node not found:%s\n", nodename);
	}
}

#define SD_READ_RAW_DATA(a1,a2, value_addr, c, type) readParamType(value_addr, mat_node, nodename, type)


void ReadParamsXML() {

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
	SD_READ_RAW_DATA(fs_read_xml,mat_node,(void *)front_cam_params.mr_int,"i", SD_XML_PP_TYPE_UINT32);

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"A2");
	SD_READ_RAW_DATA(fs_read_xml,mat_node,(void *)front_cam_params.mt_int,"i", SD_XML_PP_TYPE_UINT32);

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"A3");
	SD_READ_RAW_DATA(fs_read_xml,mat_node,(void *)rear_cam_params.mr_int,"i", SD_XML_PP_TYPE_UINT32);

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"A4");
	SD_READ_RAW_DATA(fs_read_xml,mat_node,(void *)rear_cam_params.mt_int,"i", SD_XML_PP_TYPE_UINT32);

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"A5");
	SD_READ_RAW_DATA(fs_read_xml,mat_node,(void *)left_cam_params.mr_int,"i", SD_XML_PP_TYPE_UINT32);

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"A6");
	SD_READ_RAW_DATA(fs_read_xml,mat_node,(void *)left_cam_params.mt_int,"i", SD_XML_PP_TYPE_UINT32);

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"A7");
	SD_READ_RAW_DATA(fs_read_xml,mat_node,(void *)right_cam_params.mr_int,"i", SD_XML_PP_TYPE_UINT32);

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"A8");
	SD_READ_RAW_DATA(fs_read_xml,mat_node,(void *)right_cam_params.mt_int,"i", SD_XML_PP_TYPE_UINT32);

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"B1");
	SD_READ_RAW_DATA(fs_read_xml,mat_node,(void *)front_cam_params.mimd_int,"i", SD_XML_PP_TYPE_UINT32);

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"B2");
	SD_READ_RAW_DATA(fs_read_xml,mat_node,(void *)rear_cam_params.mimd_int,"i", SD_XML_PP_TYPE_UINT32);

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"B3");
	SD_READ_RAW_DATA(fs_read_xml,mat_node,(void *)left_cam_params.mimd_int,"i", SD_XML_PP_TYPE_UINT32);

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"B4");
	SD_READ_RAW_DATA(fs_read_xml,mat_node,(void *)right_cam_params.mimd_int,"i", SD_XML_PP_TYPE_UINT32);

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"C1");
	SD_READ_RAW_DATA(fs_read_xml,mat_node,(void *)&parking_assistant_params.car_world_x,"i", SD_XML_PP_TYPE_UINT32);

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"C2");
	SD_READ_RAW_DATA(fs_read_xml,mat_node,(void *)&parking_assistant_params.car_width,"i", SD_XML_PP_TYPE_UINT32);

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"C3");
	SD_READ_RAW_DATA(fs_read_xml,mat_node,(void *)&front_resizer.x,"i", SD_XML_PP_TYPE_UINT32);

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"C4");
	SD_READ_RAW_DATA(fs_read_xml,mat_node,(void *)&rear_resizer.x,"i", SD_XML_PP_TYPE_UINT32);

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"C5");
	SD_READ_RAW_DATA(fs_read_xml,mat_node,(void *)parking_assistant_params.car_name,"i", SD_XML_PP_TYPE_STRING);

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"D0");
	SD_READ_RAW_DATA(fs_read_xml,mat_node,(void *)&parking_assistant_params.coeff[0][0],"i", SD_XML_PP_TYPE_FLOAT);

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"D1");
	SD_READ_RAW_DATA(fs_read_xml,mat_node,(void *)&parking_assistant_params.coeff[1][0],"i", SD_XML_PP_TYPE_FLOAT);

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"D2");
	SD_READ_RAW_DATA(fs_read_xml,mat_node,(void *)&parking_assistant_params.coeff[2][0],"i", SD_XML_PP_TYPE_FLOAT);

	mat_node = cvGetFileNodeByName(fs_read_xml,NULL,"D3");
	SD_READ_RAW_DATA(fs_read_xml,mat_node,(void *)&parking_assistant_params.coeff[3][0],"i", SD_XML_PP_TYPE_FLOAT);
}


extern void WriteParamsXML()
{
	//CvFileStorage* fs_write_xml = cvOpenFileStorage(PARADIR,memstorage,CV_STORAGE_WRITE);
	TiXmlDocument xml(AVM_DIR);
	if(!xml.LoadFile()){
		
	}
	TiXmlElement*proot=xml.FirstChildElement("opencv_storage");
	TiXmlElement*pnode=0;
	if(proot==0)
	{
		TiXmlElement* node = new TiXmlElement("opencv_storage");
		xml.LinkEndChild(node);
		proot = node;
	}
	const char *nodename="AAAAA";
	
	cvStartWriteStruct(fs_write_xml,"A1",CV_NODE_SEQ,0,cvAttrList(NULL,NULL));
	cvWriteRawDataType(fs_write_xml,front_cam_params.mr_int,3,"i", SD_XML_PP_TYPE_UINT32);
	cvEndWriteStruct(fs_write_xml);

	cvStartWriteStruct(fs_write_xml,"A2",CV_NODE_SEQ,0,cvAttrList(NULL,NULL));
	cvWriteRawDataType(fs_write_xml,front_cam_params.mt_int,3,"i", SD_XML_PP_TYPE_UINT32);
	cvEndWriteStruct(fs_write_xml);

	cvStartWriteStruct(fs_write_xml,"A3",CV_NODE_SEQ,0,cvAttrList(NULL,NULL));
	cvWriteRawDataType(fs_write_xml,rear_cam_params.mr_int,3,"i", SD_XML_PP_TYPE_UINT32);
	cvEndWriteStruct(fs_write_xml);

	cvStartWriteStruct(fs_write_xml,"A4",CV_NODE_SEQ,0,cvAttrList(NULL,NULL));
	cvWriteRawDataType(fs_write_xml,rear_cam_params.mt_int,3,"i", SD_XML_PP_TYPE_UINT32);
	cvEndWriteStruct(fs_write_xml);

	cvStartWriteStruct(fs_write_xml,"A5",CV_NODE_SEQ,0,cvAttrList(NULL,NULL));
	cvWriteRawDataType(fs_write_xml,left_cam_params.mr_int,3,"i", SD_XML_PP_TYPE_UINT32);
	cvEndWriteStruct(fs_write_xml);

	cvStartWriteStruct(fs_write_xml,"A6",CV_NODE_SEQ,0,cvAttrList(NULL,NULL));
	cvWriteRawDataType(fs_write_xml,left_cam_params.mt_int,3,"i", SD_XML_PP_TYPE_UINT32);
	cvEndWriteStruct(fs_write_xml);

	cvStartWriteStruct(fs_write_xml,"A7",CV_NODE_SEQ,0,cvAttrList(NULL,NULL));
	cvWriteRawDataType(fs_write_xml,right_cam_params.mr_int,3,"i", SD_XML_PP_TYPE_UINT32);
	cvEndWriteStruct(fs_write_xml);

	cvStartWriteStruct(fs_write_xml,"A8",CV_NODE_SEQ,0,cvAttrList(NULL,NULL));
	cvWriteRawDataType(fs_write_xml,right_cam_params.mt_int,3,"i", SD_XML_PP_TYPE_UINT32);
	cvEndWriteStruct(fs_write_xml);

	cvStartWriteStruct(fs_write_xml,"B1",CV_NODE_SEQ,0,cvAttrList(NULL,NULL));
	cvWriteRawDataType(fs_write_xml,front_cam_params.mimd_int,8,"i", SD_XML_PP_TYPE_UINT32);
	cvEndWriteStruct(fs_write_xml);

	cvStartWriteStruct(fs_write_xml,"B2",CV_NODE_SEQ,0,cvAttrList(NULL,NULL));
	cvWriteRawDataType(fs_write_xml,rear_cam_params.mimd_int,8,"i", SD_XML_PP_TYPE_UINT32);
	cvEndWriteStruct(fs_write_xml);

	cvStartWriteStruct(fs_write_xml,"B3",CV_NODE_SEQ,0,cvAttrList(NULL,NULL));
	cvWriteRawDataType(fs_write_xml,left_cam_params.mimd_int,8,"i", SD_XML_PP_TYPE_UINT32);
	cvEndWriteStruct(fs_write_xml);

	cvStartWriteStruct(fs_write_xml,"B4",CV_NODE_SEQ,0,cvAttrList(NULL,NULL));
	cvWriteRawDataType(fs_write_xml,right_cam_params.mimd_int,8,"i", SD_XML_PP_TYPE_UINT32);
	cvEndWriteStruct(fs_write_xml);

	cvStartWriteStruct(fs_write_xml,"C1",CV_NODE_SEQ,0,cvAttrList(NULL,NULL));
	cvWriteRawDataType(fs_write_xml,&parking_assistant_params.car_world_x,4,"i", SD_XML_PP_TYPE_UINT32);
	cvEndWriteStruct(fs_write_xml);

	cvStartWriteStruct(fs_write_xml,"C2",CV_NODE_SEQ,0,cvAttrList(NULL,NULL));
	cvWriteRawDataType(fs_write_xml,&parking_assistant_params.car_width,3,"i", SD_XML_PP_TYPE_UINT32);
	cvEndWriteStruct(fs_write_xml);

	cvStartWriteStruct(fs_write_xml,"C3",CV_NODE_SEQ,0,cvAttrList(NULL,NULL));
	cvWriteRawDataType(fs_write_xml,&front_resizer.x,4,"i", SD_XML_PP_TYPE_UINT32);
	cvEndWriteStruct(fs_write_xml);

	cvStartWriteStruct(fs_write_xml,"C4",CV_NODE_SEQ,0,cvAttrList(NULL,NULL));
	cvWriteRawDataType(fs_write_xml,&rear_resizer.x,4,"i", SD_XML_PP_TYPE_UINT32);
	cvEndWriteStruct(fs_write_xml);

	cvStartWriteStruct(fs_write_xml,"C4",CV_NODE_SEQ,0,cvAttrList(NULL,NULL));
	cvWriteRawDataType(fs_write_xml,&rear_resizer.x,4,"i", SD_XML_PP_TYPE_UINT32);
	cvEndWriteStruct(fs_write_xml);

	cvStartWriteStruct(fs_write_xml,"C5",CV_NODE_SEQ,0,cvAttrList(NULL,NULL));
	cvWriteRawDataType(fs_write_xml,parking_assistant_params.car_name,1,"i", SD_XML_PP_TYPE_STRING);
	cvEndWriteStruct(fs_write_xml);

	cvStartWriteStruct(fs_write_xml,"D0",CV_NODE_SEQ,0,cvAttrList(NULL,NULL));
	cvWriteRawDataType(fs_write_xml,&parking_assistant_params.coeff[0][0],6,"i", SD_XML_PP_TYPE_FLOAT);
	cvEndWriteStruct(fs_write_xml);

	cvStartWriteStruct(fs_write_xml,"D1",CV_NODE_SEQ,0,cvAttrList(NULL,NULL));
	cvWriteRawDataType(fs_write_xml,&parking_assistant_params.coeff[1][0],6,"i", SD_XML_PP_TYPE_FLOAT);
	cvEndWriteStruct(fs_write_xml);

	cvStartWriteStruct(fs_write_xml,"D2",CV_NODE_SEQ,0,cvAttrList(NULL,NULL));
	cvWriteRawDataType(fs_write_xml,&parking_assistant_params.coeff[2][0],6,"i", SD_XML_PP_TYPE_FLOAT);
	cvEndWriteStruct(fs_write_xml);

	cvStartWriteStruct(fs_write_xml,"D3",CV_NODE_SEQ,0,cvAttrList(NULL,NULL));
	cvWriteRawDataType(fs_write_xml,&parking_assistant_params.coeff[3][0],6,"i", SD_XML_PP_TYPE_FLOAT);
	cvEndWriteStruct(fs_write_xml);

	xml.SaveFile();
}



