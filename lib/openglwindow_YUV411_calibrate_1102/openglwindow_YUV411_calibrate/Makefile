SRCS_CPP  := $(wildcard *.cpp)
LIB_PATH := ./lib
OBJ_PATH := ./obj
T5_SDK := /home/lt/Desktop/T507/T5_SDK

LIBNAME=libOpenGLWindow.a
ALL_OBJ_CPP = $(patsubst %.cpp,%.o,$(SRCS_CPP))
ALL_OBJ_CPP_POS = $(addprefix $(OBJ_PATH)/,$(ALL_OBJ_CPP))

CC = /home/lt/Desktop/T507/gcc-linaro-7.4.1-2019.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc
CXX = /home/lt/Desktop/T507/gcc-linaro-7.4.1-2019.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-g++
AR = /home/lt/Desktop/T507/gcc-linaro-7.4.1-2019.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-ar

INC =  -I.
INC += -I$(T5_SDK)/sdk_lib/include/opengl/include
INC += -I$(T5_SDK)/sdk_lib/include/opengl/include/khronos/original/
INC += -I/home/visem/project/linuxvideo/linuxvideo/lib/libOpenCV

CFLAGS := -g -Wall -Wfatal-errors -Wno-unused-result -Wno-unknown-pragmas -fPIC \
      -Wno-sign-compare -Wno-sign-compare  -Wno-unused-variable \
      -Wno-unused-but-set-variable 

#$(CC) -shared  -o libOpenGLWindow.so   $(OBJS)
#ar cr $(LIBNAME)  $(OBJS)

all:  $(ALL_OBJ_CPP) 
	$(CC) -shared  -o $(LIB_PATH)/libOpenGLWindow.so   $(ALL_OBJ_CPP_POS)
	$(AR) cr  $(LIB_PATH)/$(LIBNAME) $(ALL_OBJ_CPP_POS) 	
	cp $(LIB_PATH)/$(LIBNAME) ~/liutao_mnt


%.o:%.cpp
	@mkdir -p $(OBJ_PATH)
	@mkdir -p $(LIB_PATH)
	$(CXX) $(CFLAGS) $(INC) -c $< -o $(OBJ_PATH)/$@

clean:
	@rm -fr $(OBJ_PATH) $(LIB_PATH) ~/$(LIB_PATH)/$(LIBNAME)
