#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "cv.h" 
#include "cvrectify.h"

#define DEADTIME  15


/*$)ADZ2N1j6(J12I</M<Oq;:3eGx*/
unsigned char* pu8CamMBvir[4] = {NULL, NULL, NULL, NULL};


int extCalibError = 0;


int findChessBoardPoints(CvMat* image_D1,CvSeq* image_points_seq,
		CvPoint2D32f* image_points_buf,int cam_id,CvSize board_size,int corners_count)
{
	int i;
	int count = board_size.width * board_size.height;
	unsigned char *tmpPtr = NULL;
	int find =0;
	struct timeval start, end;
				
	find =0;

	gettimeofday(&start, NULL);

	while(find !=1) 
	{

		gettimeofday(&end, NULL);
		
		if((end.tv_sec - start.tv_sec) > DEADTIME)
		{
			extCalibError = extCalibError | (0x1 << (cam_id));
			return 0; // time out
		}

		memcpy(image_D1->data.ptr, pu8CamMBvir[cam_id],HEIVPFE*WIDVPFE);
		find = cvFindChessboardCorners_ext( image_D1, board_size,image_points_buf, 
			&corners_count, cam_id, 1);

		if (find) 			
		{
			//resort image points direction,in case of turning 180 degree of image.
			if(image_points_buf[0].x > image_points_buf[corners_count-1].x)
			{
				CvPoint2D32f pts = {0,0};
				printf("corners reorder!");
				for(i = 0; i< corners_count*0.5; i++)
				{
					pts = image_points_buf[i];
					image_points_buf[i] = image_points_buf[corners_count-1 -i];
					image_points_buf[corners_count-1 -i] = pts;
				}
			}
			cvSeqPush( image_points_seq, image_points_buf );
			printf("get all chessPoint\n");

			return 1;	
		}
	}

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
	CvPoint2D32f* image_points_buf = 0;
	CvMemStorage* frontStorage= NULL, *rearStorage = NULL,
		 *leftStorage = NULL,  *rightStorage = NULL;
	CvSeq *front_image_points_seq = 0, *rear_image_points_seq  = 0,
		  *left_image_points_seq  = 0, *right_image_points_seq = 0;
	CvSize board_size = {FIELD_CHESSBOARD_LENGTH_CORNERS-1, FIELD_CHESSBOARD_WIDTH_CORNERS-1}; // $)A8y>]5wJT2<6x6(!#
	int elem_size;
	int find[4];

	double _camera[9], _dist_coeffs[4], _extr_params[6];
	CvMat camera = cvMat( 3, 3, CV_64F, _camera );
	CvMat dist_coeffs = cvMat( 1, 4, CV_64F, _dist_coeffs );
	CvMat extr_params = cvMat( 1, 6, CV_64F, _extr_params );
	int corners_count=0;
	int ret;

	extCalibError =0;

	para_field.car_length = carLength / 2 - CAR_F_BLIND - CAR_B_BLIND;//$)A5wJTSC5D353$#,6TFk5=5wJT2<:ZI+8qWS1_
	para_field.car_width  = carWidth / 2 - 2 * CAR_LR_BLIND;//$)A5wJTSC5D35?m#,6TFk5=5wT2<:ZI?qWS1?			
	para_field.LRchess2carFront_distance = chess2carFront / 2 - CAR_F_BLIND;
	para_field.square_size = FIELD_CHESSBOARD_SQUARE_SIZE; //$)A5wJT<8qWS4sP? 8qWS1_3$20cm 1XPkSkJ5<J5wJT2<O`7{
	para_field.chessboard_length_corners = FIELD_CHESSBOARD_LENGTH_CORNERS;//$)A5wJT2<8qWSJ}
	para_field.chessboard_width_corners = FIELD_CHESSBOARD_WIDTH_CORNERS;//$)A5wJT2<8qWSJ}
	para_field.carWorldX = 250;
	para_field.carWorldX2 = 250;
	para_field.carWorldY = 250;
	para_field.carWorldY2 = 250;

	elem_size = board_size.width*board_size.height*sizeof(CvPoint2D32f);

	printf("three value %d %d %d\n", carLength, carWidth, chess2carFront);
	printf("cameraCalib three value %d %d %d\n", para_field.car_length, para_field.car_width, para_field.LRchess2carFront_distance);

	frontImage = cvCreateMat(HEIVPFE,WIDVPFE,CV_8UC1);
	rearImage  = cvCreateMat(HEIVPFE,WIDVPFE,CV_8UC1);
	leftImage  = cvCreateMat(HEIVPFE,WIDVPFE,CV_8UC1);
	rightImage = cvCreateMat(HEIVPFE,WIDVPFE,CV_8UC1);
	frontStorage = cvCreateMemStorage( MAX( elem_size, 1 << 16 ));
	rearStorage  = cvCreateMemStorage( MAX( elem_size, 1 << 16 ));
	leftStorage  = cvCreateMemStorage( MAX( elem_size, 1 << 16 ));
	rightStorage = cvCreateMemStorage( MAX( elem_size, 1 << 16 ));
	image_points_buf = (CvPoint2D32f*)cvAlloc( elem_size );
	front_image_points_seq = cvCreateSeq( 0, sizeof(CvSeq), elem_size, frontStorage );
	rear_image_points_seq  = cvCreateSeq( 0, sizeof(CvSeq), elem_size, rearStorage );
	left_image_points_seq  = cvCreateSeq( 0, sizeof(CvSeq), elem_size, leftStorage  );
	right_image_points_seq = cvCreateSeq( 0, sizeof(CvSeq), elem_size, rightStorage );
	
	find[0] = findChessBoardPoints(frontImage,front_image_points_seq,image_points_buf,0,
							board_size,corners_count);

	find[1] = findChessBoardPoints(rearImage,rear_image_points_seq,image_points_buf,1,
							board_size,corners_count);

	find[2] = findChessBoardPoints(leftImage,left_image_points_seq,image_points_buf,2,
							board_size,corners_count);	
	
	find[3] = findChessBoardPoints(rightImage,right_image_points_seq,image_points_buf,3,
							board_size,corners_count);
	#if 1
	printf("calculate front\n");	

	if(find[0])
	{
		ret = run_calib_InstExt(frontImage,front_image_points_seq,&camera, &dist_coeffs,
								&extr_params,0,para_field);
	}
	else
	{
		ret = 0;
	}
	
	if(ret)
	{
		printf("frontCam calib success!\n");
		frontCamParams.mimdInt[0] = cvRound(_camera[0] * SCALE2 * 2);
		frontCamParams.mimdInt[2] = cvRound(_camera[2] * SCALE2 * 2);
		frontCamParams.mimdInt[1] = cvRound(_camera[4] * SCALE2 * 2);
		frontCamParams.mimdInt[3] = cvRound(_camera[5] * SCALE2 * 2);
		frontCamParams.mimdInt[4] = cvRound(_dist_coeffs[0] * SCALE1);
		frontCamParams.mimdInt[5] = cvRound(_dist_coeffs[1] * SCALE1);
		frontCamParams.mimdInt[6] = cvRound(_dist_coeffs[2] * SCALE1);
		frontCamParams.mimdInt[7] = cvRound(_dist_coeffs[3] * SCALE1);
		
		frontCamParams.mrInt[0] = cvRound(_extr_params[0]*SCALE3);
		frontCamParams.mrInt[1] = cvRound(_extr_params[1]*SCALE3);
		frontCamParams.mrInt[2] = cvRound(_extr_params[2]*SCALE3);		
		frontCamParams.mtInt[0] = cvRound(_extr_params[3]*SCALE2);
		frontCamParams.mtInt[1] = cvRound(_extr_params[4]*SCALE2);
		frontCamParams.mtInt[2] = cvRound(_extr_params[5]*SCALE2);
	}
	else
	{
		extCalibError = extCalibError|(0x1<<(0));
		printf("frontCam calib error !!!\n");
	}

	printf("calculate back\n");
	if(find[1])
	{
		ret = run_calib_InstExt(rearImage,rear_image_points_seq,&camera, &dist_coeffs,
							&extr_params,1,para_field);
	}
	else
	{
		ret = 0;
	}
	
	if(ret)
	{
		printf("rearCam calib success!\n");

		rearCamParams.mimdInt[0] = cvRound(_camera[0] * SCALE2 * 2);
		rearCamParams.mimdInt[2] = cvRound(_camera[2] * SCALE2 * 2);
		rearCamParams.mimdInt[1] = cvRound(_camera[4] * SCALE2 * 2);
		rearCamParams.mimdInt[3] = cvRound(_camera[5] * SCALE2 * 2);
		rearCamParams.mimdInt[4] = cvRound(_dist_coeffs[0] * SCALE1);
		rearCamParams.mimdInt[5] = cvRound(_dist_coeffs[1] * SCALE1);
		rearCamParams.mimdInt[6] = cvRound(_dist_coeffs[2] * SCALE1);
		rearCamParams.mimdInt[7] = cvRound(_dist_coeffs[3] * SCALE1);
		
		rearCamParams.mrInt[0] = cvRound(_extr_params[0]*SCALE3);
		rearCamParams.mrInt[1] = cvRound(_extr_params[1]*SCALE3);
		rearCamParams.mrInt[2] = cvRound(_extr_params[2]*SCALE3);		
		rearCamParams.mtInt[0] = cvRound(_extr_params[3]*SCALE2);
		rearCamParams.mtInt[1] = cvRound(_extr_params[4]*SCALE2);
		rearCamParams.mtInt[2] = cvRound(_extr_params[5]*SCALE2);
	}
	else
	{
		extCalibError = extCalibError|(0x1<<(1));
		printf("rearCam calib error!\n");
	}	

	printf("calculate left\n");
	if(find[2])
	{
		ret = run_calib_InstExt(leftImage,left_image_points_seq,&camera, &dist_coeffs,
							&extr_params,2,para_field);
	}
	else
	{
		ret = 0;
	}
	
	if(ret)
	{
		printf("leftCam calib success!\n");
		leftCamParams.mimdInt[0] = cvRound(_camera[0] * SCALE2 * 2);
		leftCamParams.mimdInt[2] = cvRound(_camera[2] * SCALE2 * 2);
		leftCamParams.mimdInt[1] = cvRound(_camera[4] * SCALE2 * 2);
		leftCamParams.mimdInt[3] = cvRound(_camera[5] * SCALE2 * 2);
		leftCamParams.mimdInt[4] = cvRound(_dist_coeffs[0] * SCALE1);
		leftCamParams.mimdInt[5] = cvRound(_dist_coeffs[1] * SCALE1);
		leftCamParams.mimdInt[6] = cvRound(_dist_coeffs[2] * SCALE1);
		leftCamParams.mimdInt[7] = cvRound(_dist_coeffs[3] * SCALE1);
		
		leftCamParams.mrInt[0] = cvRound(_extr_params[0]*SCALE3);
		leftCamParams.mrInt[1] = cvRound(_extr_params[1]*SCALE3);
		leftCamParams.mrInt[2] = cvRound(_extr_params[2]*SCALE3);		
		leftCamParams.mtInt[0] = cvRound(_extr_params[3]*SCALE2);
		leftCamParams.mtInt[1] = cvRound(_extr_params[4]*SCALE2);
		leftCamParams.mtInt[2] = cvRound(_extr_params[5]*SCALE2);
	}
	else
	{
		extCalibError = extCalibError|(0x1<<(2));
		printf("leftCam calib error!\n");
	}

	printf("calculate right\n");

	if(find[3])
	{
		ret = run_calib_InstExt(rightImage,right_image_points_seq,&camera, &dist_coeffs,
							&extr_params,3,para_field);
	}
	else
	{
		ret = 0;
	}
	
	if(ret)
	{
		printf("rightCam calib success!\n");
		rightCamParams.mimdInt[0] = cvRound(_camera[0] * SCALE2 * 2);
		rightCamParams.mimdInt[2] = cvRound(_camera[2] * SCALE2 * 2);
		rightCamParams.mimdInt[1] = cvRound(_camera[4] * SCALE2 * 2);
		rightCamParams.mimdInt[3] = cvRound(_camera[5] * SCALE2 * 2);
		rightCamParams.mimdInt[4] = cvRound(_dist_coeffs[0] * SCALE1);
		rightCamParams.mimdInt[5] = cvRound(_dist_coeffs[1] * SCALE1);
		rightCamParams.mimdInt[6] = cvRound(_dist_coeffs[2] * SCALE1);
		rightCamParams.mimdInt[7] = cvRound(_dist_coeffs[3] * SCALE1);
		
		rightCamParams.mrInt[0] = cvRound(_extr_params[0]*SCALE3);
		rightCamParams.mrInt[1] = cvRound(_extr_params[1]*SCALE3);
		rightCamParams.mrInt[2] = cvRound(_extr_params[2]*SCALE3);		
		rightCamParams.mtInt[0] = cvRound(_extr_params[3]*SCALE2);
		rightCamParams.mtInt[1] = cvRound(_extr_params[4]*SCALE2);
		rightCamParams.mtInt[2] = cvRound(_extr_params[5]*SCALE2);
	}
	else
	{
		extCalibError = extCalibError|(0x1<<(3));
		printf("rightCam calib error!\n");
	}
	
	cvReleaseMemStorage( &frontStorage);
	cvReleaseMemStorage( &rearStorage);
	cvReleaseMemStorage( &leftStorage);
	cvReleaseMemStorage( &rightStorage);
	cvFree( &image_points_buf);
	cvReleaseMat( &frontImage);
	cvReleaseMat( &rearImage);
	cvReleaseMat( &leftImage);
	cvReleaseMat( &rightImage);

	printf("extCalibError = %d\n",extCalibError);

	writeParamsXML();
	if(extCalibError&0xf) 
	{
		printf("extCalibError ERROR!!! \n");
		return -1;
	}
	else// ( (extCalibError & 0xf) == 0) // all camera calib Sucess! write the params
	{
		printf("all camera calibrate success!\n");
		
		return 0;
	}
	return -1;
	#endif
}



