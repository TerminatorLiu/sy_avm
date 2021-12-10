#ifndef _DMS_H
#define _DMS_H

int InitDms(int mode, char *model_path);
void UpdateDmsData(void *data);
void ExitDmsThread();

void DestroyDms();

#define DMS_WORK_MODE  1
#define DMS_MODEL_PATH "./test"
#endif
