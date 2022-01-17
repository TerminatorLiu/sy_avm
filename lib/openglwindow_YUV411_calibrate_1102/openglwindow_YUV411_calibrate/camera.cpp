#include <stdio.h>
#include <stdlib.h>
#include "cv.h"
#include "cvrectify.h"

/*$)ADZ2N1j6(J12I</M<Oq;:3eGx*/
unsigned char *pu8_cam_vir[4] = {NULL, NULL, NULL, NULL};

CvPoint2D32f debug_out_corners[24]; //$)AMb2NWT6/5wJT9}3LVP#,SCSZOTJ>RQ>-UR5=5D=G5c#,1cSZH7HODD8v=G5cC;UR5=!#
int debug_corner_count = 0;

int findChessBoardPoints(CvMat *image_D1, CvSeq *image_points_seq,
												 CvPoint2D32f *image_points_buf, int cam_id, CvSize board_size, int corners_count)
{
	int i;
	int count = board_size.width * board_size.height;
	//CvMat* grayImage = cvCreateMat(HEIGHT,WIDTH,CV_8UC1);
	unsigned char *tmpPtr = NULL;
	int find = 0;
	//IplImage* chess = cvCreateImage(cvSize(WIDTH*2,HEIGHT*2), IPL_DEPTH_8U, 3);

	find = 0;

	while (find != 1) // gbb 20150115, in case of no corner leads to no end loop
	{
		int temp_find = 0;
		//int transfer_flag;
		int offset_x = 0, offset_y = 0;
		int statusOP = 0;

		//frames +=1;
		printf("file:%s, line:%d, addr:%p, cam_id:%d, %p\r\n", __FILE__, __LINE__, image_D1->data.ptr, cam_id, pu8_cam_vir[cam_id]);
		printf("HEIVPFE:%d, WIDVPFE:%d\r\n", HEIVPFE, WIDVPFE);
		memcpy(image_D1->data.ptr, pu8_cam_vir[cam_id], HEIVPFE * WIDVPFE);
		//cvResize(image_D1,grayImage,CV_INTER_LINEAR);
		printf("file:%s, line:%d\r\n", __FILE__, __LINE__);
		find = cvFindChessboardCorners_ext(image_D1, board_size, image_points_buf,
																			 &corners_count, cam_id, 1);
		printf("file:%s, line:%d\r\n", __FILE__, __LINE__);
#if 0
		cvCvtColor(image_D1, chess, CV_GRAY2BGR);

		printf("debug_corner_count=%d\n",debug_corner_count);
		for(i=0; i<debug_corner_count; i++)
		{
			printf("%f %f\n",debug_out_corners[i].x, debug_out_corners[i].y);
			//cvCircle(chess, cvPoint(debug_out_corners[i].x,debug_out_corners[i].y), 5, CV_RGB(255, 0, 0), 0);  //$)A;-T2
			cvCircle(chess, cvPoint(debug_out_corners[i].x, debug_out_corners[i].y), 2, CV_RGB(255, 0, 0), -1, 8, 0);  //$)A;-T2
		}

		cvNamedWindow("chessBoard",1);
		cvShowImage("chessBoard",chess);

		cvWaitKey(0);
#endif
		if (find) // found with D1 image
		{
			//resort image points direction,in case of turning 180 degree of image.
			if (image_points_buf[0].x > image_points_buf[corners_count - 1].x)
			{
				CvPoint2D32f pts = {0, 0};
				printf("corners reorder!");
				for (i = 0; i < corners_count * 0.5; i++)
				{
					pts = image_points_buf[i];
					image_points_buf[i] = image_points_buf[corners_count - 1 - i];
					image_points_buf[corners_count - 1 - i] = pts;
				}
			}
			cvSeqPush(image_points_seq, image_points_buf);
			printf("get all chessPoint\n");
			//cvReleaseMat( &grayImage);

			return 1;
		}
	}

	//cvReleaseMat( &grayImage);
	return find;
}

/********************************
* inExtFlag:
*	0 : instrisic para calib flag
*   1 : extrinsic para calib flag
*   2 : in & Ext  para calib flag
********************************/
int cameraCalib(int carLength, int carWidth, int chess2carFront)
{
	CvMat *frontImage = NULL, *rearImage = NULL, *leftImage = NULL, *rightImage = NULL;
	CvPoint2D32f *image_points_buf = 0;
	CvMemStorage *frontStorage = NULL, *rearStorage = NULL,
							 *leftStorage = NULL, *rightStorage = NULL;
	CvSeq *front_image_points_seq = 0, *rear_image_points_seq = 0,
				*left_image_points_seq = 0, *right_image_points_seq = 0;
	CvSize board_size = {FIELD_CHESSBOARD_LENGTH_CORNERS - 1, FIELD_CHESSBOARD_WIDTH_CORNERS - 1}; // $)A8y>]5wJT2<6x6(!#
	int elem_size;
	int ret = 0;
	printf("******************************************%d %d %d\n", carLength, carWidth, chess2carFront);
	//parking_assistant_params parking_assistant_params;

	double _camera[9], _dist_coeffs[4], _extr_params[6];
	CvMat camera = cvMat(3, 3, CV_64F, _camera);
	CvMat dist_coeffs = cvMat(1, 4, CV_64F, _dist_coeffs);
	CvMat extr_params = cvMat(1, 6, CV_64F, _extr_params);
	int corners_count = 0;
	int extCalibError = 0;

	extCalibError = 0;

	parking_assistant_params.car_length = carLength / 2 - CAR_F_BLIND - CAR_B_BLIND; //$)A5wJTSC5D353$#,6TFk5=5wJT2<:ZI+8qWS1_
	parking_assistant_params.car_width = carWidth / 2 - 2 * CAR_LR_BLIND;						 //$)A5wJTSC5D35?m#,6TFk5=5wT2<:ZI?qWS1?
	parking_assistant_params.LRchess2carFront_distance = chess2carFront / 2 - CAR_F_BLIND;
	parking_assistant_params.square_size = FIELD_CHESSBOARD_SQUARE_SIZE;									//$)A5wJT<8qWS4sP? 8qWS1_3$20cm 1XPkSkJ5<J5wJT2<O`7{
	parking_assistant_params.chessboard_length_corners = FIELD_CHESSBOARD_LENGTH_CORNERS; //$)A5wJT2<8qWSJ}
	parking_assistant_params.chessboard_width_corners = FIELD_CHESSBOARD_WIDTH_CORNERS;		//$)A5wJT2<8qWSJ}
	parking_assistant_params.car_world_x = 250;
	parking_assistant_params.car_world_x2 = 250;
	parking_assistant_params.car_world_y = 250;
	parking_assistant_params.car_world_y2 = 250;

	elem_size = board_size.width * board_size.height * sizeof(CvPoint2D32f);

	printf("three value %d %d %d\n", carLength, carWidth, chess2carFront);
	

	frontImage = cvCreateMat(HEIVPFE, WIDVPFE, CV_8UC1);
	
	rearImage = cvCreateMat(HEIVPFE, WIDVPFE, CV_8UC1);
	
	leftImage = cvCreateMat(HEIVPFE, WIDVPFE, CV_8UC1);
	
	rightImage = cvCreateMat(HEIVPFE, WIDVPFE, CV_8UC1);
	
	frontStorage = cvCreateMemStorage(MAX(elem_size, 1 << 16));
	
	rearStorage = cvCreateMemStorage(MAX(elem_size, 1 << 16));
	
	leftStorage = cvCreateMemStorage(MAX(elem_size, 1 << 16));
	
	rightStorage = cvCreateMemStorage(MAX(elem_size, 1 << 16));
	
	image_points_buf = (CvPoint2D32f *)cvAlloc(elem_size);
	
	front_image_points_seq = cvCreateSeq(0, sizeof(CvSeq), elem_size, frontStorage);
	
	rear_image_points_seq = cvCreateSeq(0, sizeof(CvSeq), elem_size, rearStorage);
	
	left_image_points_seq = cvCreateSeq(0, sizeof(CvSeq), elem_size, leftStorage);
	
	right_image_points_seq = cvCreateSeq(0, sizeof(CvSeq), elem_size, rightStorage);
	
	findChessBoardPoints(frontImage, front_image_points_seq, image_points_buf, 0,
											 board_size, corners_count);
	
	findChessBoardPoints(rearImage, rear_image_points_seq, image_points_buf, 1,
											 board_size, corners_count);
	
	findChessBoardPoints(leftImage, left_image_points_seq, image_points_buf, 2,
											 board_size, corners_count);
	
	findChessBoardPoints(rightImage, right_image_points_seq, image_points_buf, 3,
											 board_size, corners_count);
#if 1
	parking_assistant_params.car_length = carLength / 2 - CAR_F_BLIND - CAR_B_BLIND; //$)A5wJTSC5D353$#,6TFk5=5wJT2<:ZI+8qWS1_
	parking_assistant_params.car_width = carWidth / 2 - 2 * CAR_LR_BLIND;						 //$)A5wJTSC5D35?m#,6TFk5=5wT2<:ZI?qWS1?
	parking_assistant_params.LRchess2carFront_distance = chess2carFront / 2 - CAR_F_BLIND;
	parking_assistant_params.square_size = FIELD_CHESSBOARD_SQUARE_SIZE;									//$)A5wJT<8qWS4sP? 8qWS1_3$20cm 1XPkSkJ5<J5wJT2<O`7{
	parking_assistant_params.chessboard_length_corners = FIELD_CHESSBOARD_LENGTH_CORNERS; //$)A5wJT2<8qWSJ}
	parking_assistant_params.chessboard_width_corners = FIELD_CHESSBOARD_WIDTH_CORNERS;		//$)A5wJT2<8qWSJ}
	parking_assistant_params.car_world_x = 250;
	parking_assistant_params.car_world_x2 = 250;
	parking_assistant_params.car_world_y = 250;
	parking_assistant_params.car_world_y2 = 250;
	printf("calculate front\n");
	
	if (ret = run_calib_InstExt(frontImage, front_image_points_seq, &camera, &dist_coeffs,
															&extr_params, 0, parking_assistant_params))
	{
		printf("frontCam calib success!\n");
		front_cam_params.mimd_int[0] = cvRound(_camera[0] * SCALE2 * 2);
		front_cam_params.mimd_int[2] = cvRound(_camera[2] * SCALE2 * 2);
		front_cam_params.mimd_int[1] = cvRound(_camera[4] * SCALE2 * 2);
		front_cam_params.mimd_int[3] = cvRound(_camera[5] * SCALE2 * 2);
		front_cam_params.mimd_int[4] = cvRound(_dist_coeffs[0] * SCALE1);
		front_cam_params.mimd_int[5] = cvRound(_dist_coeffs[1] * SCALE1);
		front_cam_params.mimd_int[6] = cvRound(_dist_coeffs[2] * SCALE1);
		front_cam_params.mimd_int[7] = cvRound(_dist_coeffs[3] * SCALE1);

		front_cam_params.mr_int[0] = cvRound(_extr_params[0] * SCALE3);
		front_cam_params.mr_int[1] = cvRound(_extr_params[1] * SCALE3);
		front_cam_params.mr_int[2] = cvRound(_extr_params[2] * SCALE3);
		front_cam_params.mt_int[0] = cvRound(_extr_params[3] * SCALE2);
		front_cam_params.mt_int[1] = cvRound(_extr_params[4] * SCALE2);
		front_cam_params.mt_int[2] = cvRound(_extr_params[5] * SCALE2);
	}
	else
	{
		extCalibError = extCalibError | (0x1 << (0));
		printf("frontCam calib error !!!\n");
	}
	parking_assistant_params.car_length = carLength / 2 - CAR_F_BLIND - CAR_B_BLIND; //$)A5wJTSC5D353$#,6TFk5=5wJT2<:ZI+8qWS1_
	parking_assistant_params.car_width = carWidth / 2 - 2 * CAR_LR_BLIND;						 //$)A5wJTSC5D35?m#,6TFk5=5wT2<:ZI?qWS1?
	parking_assistant_params.LRchess2carFront_distance = chess2carFront / 2 - CAR_F_BLIND;
	parking_assistant_params.square_size = FIELD_CHESSBOARD_SQUARE_SIZE;									//$)A5wJT<8qWS4sP? 8qWS1_3$20cm 1XPkSkJ5<J5wJT2<O`7{
	parking_assistant_params.chessboard_length_corners = FIELD_CHESSBOARD_LENGTH_CORNERS; //$)A5wJT2<8qWSJ}
	parking_assistant_params.chessboard_width_corners = FIELD_CHESSBOARD_WIDTH_CORNERS;		//$)A5wJT2<8qWSJ}
	parking_assistant_params.car_world_x = 250;
	parking_assistant_params.car_world_x2 = 250;
	parking_assistant_params.car_world_y = 250;
	parking_assistant_params.car_world_y2 = 250;
	printf("calculate back\n");
	
	if (ret = run_calib_InstExt(rearImage, rear_image_points_seq, &camera, &dist_coeffs,
															&extr_params, 1, parking_assistant_params))
	{
		printf("rearCam calib success!\n");

		rear_cam_params.mimd_int[0] = cvRound(_camera[0] * SCALE2 * 2);
		rear_cam_params.mimd_int[2] = cvRound(_camera[2] * SCALE2 * 2);
		rear_cam_params.mimd_int[1] = cvRound(_camera[4] * SCALE2 * 2);
		rear_cam_params.mimd_int[3] = cvRound(_camera[5] * SCALE2 * 2);
		rear_cam_params.mimd_int[4] = cvRound(_dist_coeffs[0] * SCALE1);
		rear_cam_params.mimd_int[5] = cvRound(_dist_coeffs[1] * SCALE1);
		rear_cam_params.mimd_int[6] = cvRound(_dist_coeffs[2] * SCALE1);
		rear_cam_params.mimd_int[7] = cvRound(_dist_coeffs[3] * SCALE1);

		rear_cam_params.mr_int[0] = cvRound(_extr_params[0] * SCALE3);
		rear_cam_params.mr_int[1] = cvRound(_extr_params[1] * SCALE3);
		rear_cam_params.mr_int[2] = cvRound(_extr_params[2] * SCALE3);
		rear_cam_params.mt_int[0] = cvRound(_extr_params[3] * SCALE2);
		rear_cam_params.mt_int[1] = cvRound(_extr_params[4] * SCALE2);
		rear_cam_params.mt_int[2] = cvRound(_extr_params[5] * SCALE2);
	}
	else
	{
		extCalibError = extCalibError | (0x1 << (1));
		printf("rearCam calib error!\n");
	}
	parking_assistant_params.car_length = carLength / 2 - CAR_F_BLIND - CAR_B_BLIND; //$)A5wJTSC5D353$#,6TFk5=5wJT2<:ZI+8qWS1_
	parking_assistant_params.car_width = carWidth / 2 - 2 * CAR_LR_BLIND;						 //$)A5wJTSC5D35?m#,6TFk5=5wT2<:ZI?qWS1?
	parking_assistant_params.LRchess2carFront_distance = chess2carFront / 2 - CAR_F_BLIND;
	parking_assistant_params.square_size = FIELD_CHESSBOARD_SQUARE_SIZE;									//$)A5wJT<8qWS4sP? 8qWS1_3$20cm 1XPkSkJ5<J5wJT2<O`7{
	parking_assistant_params.chessboard_length_corners = FIELD_CHESSBOARD_LENGTH_CORNERS; //$)A5wJT2<8qWSJ}
	parking_assistant_params.chessboard_width_corners = FIELD_CHESSBOARD_WIDTH_CORNERS;		//$)A5wJT2<8qWSJ}
	parking_assistant_params.car_world_x = 250;
	parking_assistant_params.car_world_x2 = 250;
	parking_assistant_params.car_world_y = 250;
	parking_assistant_params.car_world_y2 = 250;
	printf("calculate left\n");
	
	if (ret = run_calib_InstExt(leftImage, left_image_points_seq, &camera, &dist_coeffs,
															&extr_params, 2, parking_assistant_params))
	{
		printf("leftCam calib success!\n");
		left_cam_params.mimd_int[0] = cvRound(_camera[0] * SCALE2 * 2);
		left_cam_params.mimd_int[2] = cvRound(_camera[2] * SCALE2 * 2);
		left_cam_params.mimd_int[1] = cvRound(_camera[4] * SCALE2 * 2);
		left_cam_params.mimd_int[3] = cvRound(_camera[5] * SCALE2 * 2);
		left_cam_params.mimd_int[4] = cvRound(_dist_coeffs[0] * SCALE1);
		left_cam_params.mimd_int[5] = cvRound(_dist_coeffs[1] * SCALE1);
		left_cam_params.mimd_int[6] = cvRound(_dist_coeffs[2] * SCALE1);
		left_cam_params.mimd_int[7] = cvRound(_dist_coeffs[3] * SCALE1);

		left_cam_params.mr_int[0] = cvRound(_extr_params[0] * SCALE3);
		left_cam_params.mr_int[1] = cvRound(_extr_params[1] * SCALE3);
		left_cam_params.mr_int[2] = cvRound(_extr_params[2] * SCALE3);
		left_cam_params.mt_int[0] = cvRound(_extr_params[3] * SCALE2);
		left_cam_params.mt_int[1] = cvRound(_extr_params[4] * SCALE2);
		left_cam_params.mt_int[2] = cvRound(_extr_params[5] * SCALE2);
	}
	else
	{
		extCalibError = extCalibError | (0x1 << (2));
		printf("leftCam calib error!\n");
	}
	parking_assistant_params.car_length = carLength / 2 - CAR_F_BLIND - CAR_B_BLIND; //$)A5wJTSC5D353$#,6TFk5=5wJT2<:ZI+8qWS1_
	parking_assistant_params.car_width = carWidth / 2 - 2 * CAR_LR_BLIND;						 //$)A5wJTSC5D35?m#,6TFk5=5wT2<:ZI?qWS1?
	parking_assistant_params.LRchess2carFront_distance = chess2carFront / 2 - CAR_F_BLIND;
	parking_assistant_params.square_size = FIELD_CHESSBOARD_SQUARE_SIZE;									//$)A5wJT<8qWS4sP? 8qWS1_3$20cm 1XPkSkJ5<J5wJT2<O`7{
	parking_assistant_params.chessboard_length_corners = FIELD_CHESSBOARD_LENGTH_CORNERS; //$)A5wJT2<8qWSJ}
	parking_assistant_params.chessboard_width_corners = FIELD_CHESSBOARD_WIDTH_CORNERS;		//$)A5wJT2<8qWSJ}
	parking_assistant_params.car_world_x = 250;
	parking_assistant_params.car_world_x2 = 250;
	parking_assistant_params.car_world_y = 250;
	parking_assistant_params.car_world_y2 = 250;
	printf("calculate right\n");
	
	if (ret = run_calib_InstExt(rightImage, right_image_points_seq, &camera, &dist_coeffs,
															&extr_params, 3, parking_assistant_params))
	{
		printf("rightCam calib success!\n");
		right_cam_params.mimd_int[0] = cvRound(_camera[0] * SCALE2 * 2);
		right_cam_params.mimd_int[2] = cvRound(_camera[2] * SCALE2 * 2);
		right_cam_params.mimd_int[1] = cvRound(_camera[4] * SCALE2 * 2);
		right_cam_params.mimd_int[3] = cvRound(_camera[5] * SCALE2 * 2);
		right_cam_params.mimd_int[4] = cvRound(_dist_coeffs[0] * SCALE1);
		right_cam_params.mimd_int[5] = cvRound(_dist_coeffs[1] * SCALE1);
		right_cam_params.mimd_int[6] = cvRound(_dist_coeffs[2] * SCALE1);
		right_cam_params.mimd_int[7] = cvRound(_dist_coeffs[3] * SCALE1);

		right_cam_params.mr_int[0] = cvRound(_extr_params[0] * SCALE3);
		right_cam_params.mr_int[1] = cvRound(_extr_params[1] * SCALE3);
		right_cam_params.mr_int[2] = cvRound(_extr_params[2] * SCALE3);
		right_cam_params.mt_int[0] = cvRound(_extr_params[3] * SCALE2);
		right_cam_params.mt_int[1] = cvRound(_extr_params[4] * SCALE2);
		right_cam_params.mt_int[2] = cvRound(_extr_params[5] * SCALE2);
	}
	else
	{
		extCalibError = extCalibError | (0x1 << (3));
		printf("rightCam calib error!\n");
	}

	cvReleaseMemStorage(&frontStorage);
	cvReleaseMemStorage(&rearStorage);
	cvReleaseMemStorage(&leftStorage);
	cvReleaseMemStorage(&rightStorage);
	cvFree(&image_points_buf);
	cvReleaseMat(&frontImage);
	cvReleaseMat(&rearImage);
	cvReleaseMat(&leftImage);
	cvReleaseMat(&rightImage);

	printf("extCalibError = %d\n", extCalibError);
	if (extCalibError & 0xf)
	{
		printf("extCalibError ERROR!!! \n");
		return -1;
	}
	else if ((extCalibError & 0xf) == 0) // all camera calib Sucess! write the params
	{
		printf("all camera calibrate success!\n");
		parking_assistant_params.car_length = carLength / 2 - CAR_F_BLIND - CAR_B_BLIND; //$)A5wJTSC5D353$#,6TFk5=5wJT2<:ZI+8qWS1_
		parking_assistant_params.car_width = carWidth / 2 - 2 * CAR_LR_BLIND;						 //$)A5wJTSC5D35?m#,6TFk5=5wT2<:ZI?qWS1?
		parking_assistant_params.LRchess2carFront_distance = chess2carFront / 2 - CAR_F_BLIND;
		parking_assistant_params.square_size = FIELD_CHESSBOARD_SQUARE_SIZE;									//$)A5wJT<8qWS4sP? 8qWS1_3$20cm 1XPkSkJ5<J5wJT2<O`7{
		parking_assistant_params.chessboard_length_corners = FIELD_CHESSBOARD_LENGTH_CORNERS; //$)A5wJT2<8qWSJ}
		parking_assistant_params.chessboard_width_corners = FIELD_CHESSBOARD_WIDTH_CORNERS;		//$)A5wJT2<8qWSJ}
		parking_assistant_params.car_world_x = 250;
		parking_assistant_params.car_world_x2 = 250;
		parking_assistant_params.car_world_y = 250;
		parking_assistant_params.car_world_y2 = 250;
		WriteParamsXML();
		return 0;
	}
#endif
}
