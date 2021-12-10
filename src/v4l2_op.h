#ifndef _SY_APP_V4L2_OP_H
#define _SY_APP_V4L2_OP_H
#include <stdbool.h>
#include "conf.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void V4l2OpCheckErrno(bool flag, char *file, int line);
int V4l2OpOpen(v4l2_op_interface *v4l2_op);
int V4l2OpVidiocsInput(v4l2_op_interface *v4l2_op);
int V4l2OpVidiocsParm(v4l2_op_interface *v4l2_op);
int V4l2OpVidiocsFmt(v4l2_op_interface *v4l2_op);
int V4l2OpVidiocgParm(v4l2_op_interface *v4l2_op);
int V4l2OpVidiocReqbufs(v4l2_op_interface *v4l2_op);
int V4l2OpVidiocQuerybuf(v4l2_op_interface *v4l2_op);
int V4l2OpVidiocStreamOn(v4l2_op_interface *v4l2_op);
int V4l2OpVidiocStreamOff(v4l2_op_interface *v4l2_op);

#define V4L2_OP_CHECK(cond) V4l2OpCheckErrno(cond, __FILE__, __LINE__)

#ifdef __cplusplus
}
#endif
#endif