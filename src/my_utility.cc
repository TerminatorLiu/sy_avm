#include "my_utility.h"

#include "time.h"
void GetTimeStmpStr(char str[], const char *format)
{
	struct tm now_time;
	time_t time_seconds = time(0);
	localtime_r(&time_seconds, &now_time);
	strftime(str, 100, format, &now_time);
}