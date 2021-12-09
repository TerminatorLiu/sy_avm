#ifndef FATIGUE_DRIVING_STRUCT_H_
#define FATIGUE_DRIVING_STRUCT_H_

//人脸关键点数量
#define NUM_LANDMARK 72

enum detect_state
{
	AbNormal_DX = -2,	  //异常
	Initializing_DS = -1, //初始化中
	NORMAL_DS = 0,		  //正常
	CLOSED_EYE_DS = 201,  //闭眼
	YAWNING_DS, 		  //打哈欠
	SLEEP_OPEN_EYE_DS, 	  //睁眼睡
    LOOK_DOWN_DS,		  //向下看
	LOOK_AROUND_DS=205,	  //左顾右盼
	NO_FACE_DS,			  //无人脸
	PHONING_DS,			  //打电话
	SMOKING_DS,			  //抽烟
	OCCLUSION_DS,		  //遮挡
	UP_HEAD_DS=210,		  //仰头
	Unfasten_Seatbelt_DS, //未系安全带
	HAND_OFF_DS,		  //双手脱离方向盘
	HAND_SINGLE_OFF_DS,	  //单手脱离方向盘
	INFRARED_BLOCKING_DS, //红外阻断眼镜
	EYE_GAZE_DS,		  //视线检测
	ADAS_OCCLUSION_DS,
	WITHOUT_MASK_DS=217,  //未戴口罩
    Down_Head_DS=220,	  //低头
	REAL_FATIGUE_DS,      //真疲劳
};

enum alarm_sensitivity
{
	TEST_MODE_AS = 0,		  //测试模式
	//正常模式
	NORMAL_1_AS,
	NORMAL_2_AS,
	NORMAL_3_AS,
	NORMAL_4_AS,
	NORMAL_5_AS,
};

typedef struct TagAlarmParameter
{
	//max time for return event
	int max_time_event;	//秒 返回事件的时间上限，默认设为40
	int min_time_event;	//秒 返回事件的时间下限，针对无人脸和遮挡，默认设为2

	//speed
	float min_speed; //报警的最小速度，默认为20
	float min_update_speed;//特征的最小更新速度，默认为50

	//calibration for pitch and yaw
	float avg_yaw;
	float avg_pitch;

	//down
	int down_bin;//300    量化间隔	
	float down_detect_L;//检测时长
	int down_detect_F;//检测帧数
	float down_angle_T ;
	float down_curve_up_T;
	//around
	int around_bin;  // 200
	float around_detect_L;
	float yaw_vary_T;
	float yaw_T;

	//no face
	float no_face_detect_L;

	//occlusion
	float occlusion_detect_L;

	//eye
	float eye_detect_L;
	int eye_detect_F;
	int eye_bin; //200
	float eye_dist_T;
	float curve_down_T;

	//yawn
	int mouth_bin; //200
	float mouth_detect_L;

	//phoning
	int phoning_bin;//500
	float phoning_detect_L;
	int mobile_phone_n_T; //2
	int use_phone; //use_and

	//smoking
	int smoking_bin;//500
	float smoking_detect_L;
	int smoking_blight;//1
	int cigarette_n_T;//1
	int use_cigarette;//use_and

	//up
	int up_bin; //300
	float up_detect_L;
	float up_angle_T ; //25;

	//seat belt
	float seatbelt_detect_L;

	//infrare block
	float block_detect_L;

	//breath mask
	int mask_bin;
	float mask_detect_L;
}TAlarmPara;

//设定标定模式和参数值：俯仰角和偏航角
typedef struct TagCalibrationParameter{
	int flag;
	float avg_pitch;
	float avg_yaw;
}TCaliPara;

//报警事件的中间结果，例如眨眼事件(眨眼开始，眨眼结束，眨眼类型)
typedef struct TagFatigueEvent{
	double start_time;
	double end_time;
	int type;
}TFatigueEvent;

struct FrameDetectResult{
	int smoking;        // 0--smoking,1--no_smoking,-1--failed
	int phoning;        // 0--phoning,1--no_phoning,-1--failed
	float face_ld[NUM_LANDMARK*2]; //landmarks
	float headpose[3];  // 0-pitch,1-yaw,2-roll

	int eye;            //INVALID_EYE=-1,OPEN_EYE=0,CLOSED_EYE=1,HALF_EYE=2
	int mask;           //INVALID_MASK=-1,WITH_MASK=0,NO_MASK=1
	float mouth[2];     //0-ratio,1-angle
	double frame_mark;  //frame mark
};

struct ObjectDetectInfo
{
	int id;//1-phoning,2-smoking,3-seat_belt
	float confidence;
	int x,y,w,h;//cv::Rect:x,y,w,h
	double time;
};

#endif
