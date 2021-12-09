
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "fatigue_driving_c.h"

int main(int argc, const char *argv[]) 
{
	int tm=0;

	int width = 1280;
	int height = 720;
    int speed = 60;     //no need

    int mode = 1;
    char model_path[64] = "."; // spb.bin path
	
    printf("DMS Version:%s\n", get_version_fatigue_driving());
	void *handle = init_fatigue_driving(model_path,mode,NULL,false,false);
    printf("init_fatigue_driving mode %d\n", mode);

	while(1) {
        void *video = NULL;
        // get video buffer

        // dms detect
		int result = detect_fatigue_driving(handle, (char *)video,width,height,speed,2046,tm++);
        if (result) {
            // enum detect_state
            printf("detect_fatigue_driving %d\n", result);
        }
#if 0
        // no need
        int rects[16] = {0};
        result = get_face_fatigue_driving(handle, rects);
        if (result > 0) {
            // printf("face_rect %d, %d, %d, %d\n", rects[0], rects[1], rects[2], rects[3]);
        }

        float angles[8] = {0};
        result = get_face_angle_fatigue_driving(handle, angles);
        if (result >= 0) {
            // printf("face_angle %f, %f, %f\n", angles[0], angles[1], angles[2]);
        }
#endif
		usleep(60 * 1000);
	}

    return 0;
}
