SRCS_C  := $(wildcard *.c)
SRCS_CC  := $(wildcard *.cc)

BIN_PATH := ../bin
OBJ_PATH := ../obj
OBJ_SUB_PATH := $(OBJ_PATH)
include MakeConf
#/home/visem/project/tools/sdk_lib/include/opengl/include/khronos/original/EGL
T5_SDK := /home/lt/Desktop/T507/T5_SDK

MOD := camera
BIN := $(BIN_PATH)/$(MOD)

ALL_OBJ_C = $(patsubst %.c,%.o,$(SRCS_C))
ALL_OBJ_C_POS = $(addprefix $(OBJ_SUB_PATH)/,$(ALL_OBJ_C))

ALL_OBJ_CC = $(patsubst %.cc,%.o,$(SRCS_CC))
ALL_OBJ_CC_POS = $(addprefix $(OBJ_SUB_PATH)/,$(ALL_OBJ_CC))

INC =  -I.
INC += -I$(T5_SDK)/sdk_lib/include/opengl/include
#INC += -Ilib/openglwindow/openglwindow/
INC += -I../lib/openglwindow_YUV411_calibrate_1102/openglwindow_YUV411_calibrate
#INC += -I/home/visem/project/linuxvideo/linuxvideo/lib/openglwindow_YUV411_calibrate
INC += -I ../include/opencv3410/

# /home/visem/tftpboot/libOpenCV.a
#sdk_lib/include/opengl/include/khronos/original sdk_openCV
INC += -I$(T5_SDK)/sdk_lib/include/opengl/include/khronos/original/
ifeq ($(DEBUG),1)
CFLAGS := -g $(INC) -Wno-write-strings -Wall -Wl,-rpath=/home/lib
else
CFLAGS := -O2 $(INC) -Wno-write-strings -Wall -Wl,-rpath=/home/lib
endif
ifeq ($(ENABLE_DMS),1)
LIBS += -L../lib/ -lMNN -lncnn  -ltensorflow-lite -lopencv_world -ldms_T5
endif
LIBS += -L../lib/  -lsdk_openCV -Llib -lGLESv2 -lmali  -lpthread 
LIBS += ../lib/openglwindow_YUV411_calibrate_1102/openglwindow_YUV411_calibrate/lib/libOpenGLWindow.a 

CC = /home/lt/Desktop/T507/gcc-linaro-7.4.1-2019.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc
CXX = /home/lt/Desktop/T507/gcc-linaro-7.4.1-2019.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-g++

.PHONY : clean all
 
all: $(ALL_OBJ_C) $(ALL_OBJ_CC)
	@mkdir -p $(BIN_PATH)
	$(CXX) $(CFLAGS) -o $(BIN) $(ALL_OBJ_C_POS) $(ALL_OBJ_CC_POS) $(LIBS)
	cp $(BIN) ~/liutao_mnt
%.o:%.cc
	@mkdir -p $(OBJ_SUB_PATH)
	$(CXX) $(CFLAGS) -c $< -o $(OBJ_SUB_PATH)/$@
 
%.o:%.c
	@mkdir -p $(OBJ_SUB_PATH)
	$(CXX) $(CFLAGS) -c $< -o $(OBJ_SUB_PATH)/$@	

clean:
	@rm -fr $(ALL_OBJ_C) $(ALL_OBJ_CC) $(BIN_PATH) $(OBJ_SUB_PATH)

