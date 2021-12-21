#include "sharedmem.h"

#include <semaphore.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

sem_t *gsem_dms_res = NULL;
sem_t *gsem_dms_slot = NULL;
const char g_sem_dms_res_str[] = "sem dms res";
const char g_sem_dms_slot_str[] = "sem dms slot";



sem_t *gsem_resbuffer_res = NULL;
sem_t *gsem_resbuffer_slot = NULL;
const char g_sem_resbuffer_res_str[] = "sem resbuffer res";
const char g_sem_resbuffer_slot_str[] = "sem resbuffer slot";


int InitDMSSM()
{
  gsem_dms_res = sem_open(g_sem_dms_res_str, O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
  gsem_dms_slot = sem_open(g_sem_dms_slot_str, O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 1);
  return 0;
}

int UpdataDMSSM(unsigned char *pdata,int width,int height,int datatype)
{
  
  return 0;
}

unsigned char* GetDMSSM(int width,int height,int datatype)
{
  return NULL;
}

int DMSSMResP()
{
  return 0;
}
int DMSSMResV()
{
  return 0;
}
int DMSSMSlotP()
{
  return 0;
}
int DMSSMSlotV()
{
  return 0;
}

int InitResSM()
{
  return 0;
}

int UpdataResSM(unsigned char *res_msg,unsigned int length)
{
  
  return 0;
}

unsigned char* GetResSM(int width,int height,int datatype)
{
  return NULL;
}

int ResSMResP()
{
  return 0;
}
int ResSMResV()
{
  return 0;
}
int ResSMSlotP()
{
  return 0;
}
int ResSMSlotV()
{
  return 0;
}

int ProduceRes(unsigned char*resmsg,unsigned int length)
{
  ResSMSlotP();
  UpdataResSM(resmsg,length);
  ResSMResV();
}

static void *DMSSMRoutine(void *argc)
{
  for (;;)
  {
    DMSSMSlotP();
    UpdataDMSSM(NULL,1280,720,1);
    DMSSMResV();
  }
}

static void *ParseResSMRoutine(void *argc)
{
  for(;;)
  {
    ResSMResP();
    ResSMSlotV();
  }
}
