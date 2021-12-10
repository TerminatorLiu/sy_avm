/*
 * fatigue_driving.h
 *
 *  Created on: 2017年8月30日
 *      Author: qkj
 */

#ifndef FATIGUE_DRIVING_C_H_
#define FATIGUE_DRIVING_C_H_

#include "fatigue_driving_struct.h"


/*
 * 功能：			初始化
 * fatigue_T: 	疲劳灵敏度（1——5，0为测试模式, 1为正常模式, 2为深圳出租车）
 * para: 		当fatigue_T为-1时生效
 * event: 		是否返回事件
 * show: 		显示
*/
void *init_fatigue_driving(char *model_path, int fatigue_T, TAlarmPara *para, bool event, bool show);

/*
 * 功能：			设定模式和标定值
 * para: 		标定设置模式和俯仰角、偏航角标定值，模式为0或１：0为初始化标定一次，1为每一帧都标定
*/
int set_calibration_param_fatigue_driving(void *handle, TCaliPara *para);

/*
 * 功能：			检测
 * alarm_type: 	报警类型开关，报警类别(detect_state)对应二进制位标示，1--打开 0--关闭, 位数从0开始计数
 * 返回值： 		-1——初始化  0——正常   >=1——告警 (detect_state)
*/
int detect_fatigue_driving(void *handle, char *gray, int width, int height, float car_speed, int alarm_type, double time_stamp);

/*
 * 功能:		初始化是否全部完成
 * 返回值:   0——未全部完成　１——全部完成
*/
int get_init_state_fatigue_driving(void *handle);

/*
 * 功能：		获得中间检测事件结果
 * 返回值：	事件数量
 * 输出值：	fatigue_event——事件,　内存由算法内部分配
*/
int get_event_fatigue_driving(void *handle, TFatigueEvent **fatigue_event);

/*
 * 功能：		获得人脸检测框
 * 返回值：	检测框数量(小于等于０表示失败)
 * 输出值：	检测框(x,y,w,h),顺序存放，内存由调用者分配，目前只返回一个检测框
*/
int get_face_fatigue_driving(void *handle, int *rect);

/*
 * 功能：		获得人脸特征点
 * 返回值：	特征点个数(小于等于０表示失败)
 * 输出值：	landmark——特征点，内存由调用者分配，长度为10
*/
//int get_face_landmark_fatigue_driving(void *handle, int *landmark);

/*
 *功能：	获得人脸角度Pitch(上下俯仰角) Yaw(左右偏转角) Roll(左右翻滚角),偏转角自标定正方向角度,俯仰角标定值
 *返回值：　0——成功　-1——失败
*/
int get_face_angle_fatigue_driving(void *handle, float angle[5]);

/*
 *功能：	获得视线方向
 *返回值：　0——成功　-1——失败
*/
int get_eye_gaze_fatigue_driving(void *handle, float gaze[3]);

/*
 *功能：	是否正面人脸
 *返回值：　0——否 1——是　-1——失败
*/
int get_frontface_state_fatigue_driving(void *handle);

/*
 *功能：	获得眼睛状态
 *返回值:		0——闭合，１——睁开, -1——失败
*/
int get_eye_state_fatigue_driving(void *handle);

/*
 *功能：	获取眼睛开度：人脸高度和眼睛开度
 *返回值：　0——成功　-1——失败
*/
int get_eye_open_size_fatigue_driving(void *handle, float eye_open[2]);

/*
 * 功能：获得口罩佩戴状态
 * 返回值：  0--戴有口罩，1--未戴口罩，-1--失败
 */
int get_mask_state_fatigue_driving(void *handle);

/*
 * 功能：获得每一帧图像检测结果：人脸特征点，头部姿态，有无闭眼、抽烟、打电话、戴口罩
 * 返回值：  0--成功,-1--失败
 * 输出值：FrameDetectResult,人脸特征点坐标，人脸角度：pitch偏转角，yaw航向角，roll翻滚角，闭眼、抽烟、打电话、戴口罩状态,嘴巴张开度：横向张角、纵向开度
 */
int get_frame_detect_state_fatigue_driving(void *handle,struct FrameDetectResult **frame_detect_result);

/*
 * 功能：		获得目标检测结果：香烟，电话，安全带
 * 返回值：	目标数量
 * 输出值：	objects——目标信息,　内存由算法内部分配，
 * id:　1--电话，2--香烟，3--安全带
 * confidence:　对应目标置信度
 * x,y,w,h: 分别对应目标检测框左顶点横坐标、纵坐标、宽度、高度
 * time: 检测到目标的系统实时时间
*/
int get_object_detect_fatigue_driving(void *handle,struct ObjectDetectInfo **objects);

/*
 * 功能：		获得版本号
 * 返回值：	版本号
 * 输出值：
*/
const char* get_version_fatigue_driving();

/*
 * 功能：		释放
*/
int release_fatigue_driving(void *handle);

#endif /* FATIGUE_DRIVING_H_ */
