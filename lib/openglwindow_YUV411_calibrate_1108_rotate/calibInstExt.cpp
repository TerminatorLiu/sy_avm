/*********************************************************************************
* $)A0fH(KySP (C) ::;*025@?F<<SPO^TpHN9+K>
* 
* $)AND<~C{3F#: calibInstExt.cpp
* $)AND<~1jJ6#: 
* $)ADZH]U*R*#: M(9}5XCfFLIh5DFeEL8q#,RT<08(Vz6(2N5D5wJT2<#,UR5==G5c#,M(9}P^U}DZMb2N
			 $)A@44o5=DZMb2NR;4N1j6(5DD?5D!#
* $)AFdK|K5Cw#: 
* $)A51G00f1>#: 
* $)AWw    U_#: 9(1y1y#(gbb#)
* $)A44=(HUFZ#: 20140828
* --------------------------------------------------------------------------------
* $)AP^8D<GB<1#:
* $)AP^8DHUFZ#:
* $)A0f 1> :E#:
* $)AP^ 8D HK#: 
* $)AP^8DDZH]#:
**********************************************************************************/
#include "_cv.h"
#include "cv.h"
//#include "c6x.h"
#include "cvrectify.h"
//#include "IQmath_inline.h"

#define Uint8   unsigned char

#define CALIBRION_IMG_COUNT  3
#define CHESSBOARD_WIDTH  7
#define CHESSBOARD_HEIGHT  5

#define max(a,b) (((a)>(b)) ? (a):(b))
#define min(a,b) (((a)<(b)) ? (a):(b))

CV_IMPL
int run_calib_InstExt(const CvMat* img_src,const CvSeq* image_points_seq,CvMat* camera_matrix, 
				  CvMat* dist_coeffs, CvMat* extr_params,int cam_id, Parking_Assistant_Params parking_assistant_params)
{
	int i =0,found0 = 0,found1;
	int ret = 0;
	CvMat* img_rectify0 = cvCreateMat(RECTIFY_HEIGHT,RECTIFY_WIDTH,CV_8UC1),
		* img_rectify1 = cvCreateMat(RECTIFY_HEIGHT,RECTIFY_WIDTH,CV_8UC1);
	CvSize board_size = cvSize(parking_assistant_params.chessboard_length_corners -1, parking_assistant_params.chessboard_width_corners -1);
	CvPoint2D32f *square0 = (CvPoint2D32f *) cvAlloc (4 * sizeof(CvPoint2D32f)),
				*square1 = (CvPoint2D32f *) cvAlloc (4 * sizeof(CvPoint2D32f));
	int point_count = board_size.width*board_size.height;
	CvMat *image_points =NULL, *object_points=NULL;
	CvMat* image_points0 = cvCreateMat( 1, point_count, CV_32FC2 );
   	CvMat* object_points0 = cvCreateMat( 1, point_count, CV_32FC3 );
	CvSize output_full_size = cvSize(RECTIFY_WIDTH,RECTIFY_HEIGHT);
	int pix_num = img_rectify0->cols * img_rectify0->rows;

	CvMat* obj_FullPoints = cvCreateMat(1,pix_num,CV_64FC3);//the world points.
	CvMat* project_Points = cvCreateMat(1,pix_num,CV_64FC2);//the project points of the world points.
	CvMat* rot_vects = cvCreateMat(1,3,CV_64FC1), *trans_vects = cvCreateMat(1,3,CV_64FC1);
	double _camera[9],_dist_coeffs[4];
	CvMat camera = cvMat( 3, 3, CV_64F, _camera );
	CvMat dist_coeffs0 = cvMat( 1, 4, CV_64F, _dist_coeffs );

	double reprojection_err = 1000.0;
	float real2pix_ratio = 1.5;
	float fx,fy;

	int center_x =( WIDVPFE>>2),center_y = (HEIVPFE>>2);

	fx = 366.6896908038024 / 2;
	fy = 356.1124332021072 / 2;
		
	_dist_coeffs[0]   = -0.03647013101842009;
	_dist_coeffs[1]   = 0.008072033197747085;
	_dist_coeffs[2]   = -0.01052453285682819;
	_dist_coeffs[3]   = 0.003438989378898739;

	_camera[0] = fx;
	_camera[4] = fy;
	_camera[2] = center_x;
	_camera[5] = center_y;


	printf("\ncam[%d] run_calib_InstExt *******************\n",cam_id);

	cvSet(obj_FullPoints,cvScalar(0.,0.,-10000.),0);

	// 1. init the chessboard corners
	initChessBoardPoints(cam_id,image_points_seq,image_points0,object_points0,parking_assistant_params);
#if 1
	CvPoint2D32f* pts_img = (CvPoint2D32f*)image_points0->data.fl;
	for(i = 0; i< point_count; i++)
	{
		//pts_img[i].x +=pts_img[i].x;
		//pts_img[i].y +=pts_img[i].y;
	}
	searchSubSquareCorners2(img_src,cvSize(7,7),pts_img,point_count  );
	for(i = 0; i< point_count; i++)
	{
		pts_img[i].x *= 0.5;
		pts_img[i].y *= 0.5;
	}
#endif
	//2. get the extrinsic parameter of Camera
	cvFindExtrinsicCameraParams_fisheye(object_points0,image_points0,&camera, 
		&dist_coeffs0,rot_vects, trans_vects );

	printf("R | %f %f %f\n", cvmGet(rot_vects, 0, 0), cvmGet(rot_vects, 0, 1), cvmGet(rot_vects, 0, 2));
	printf("T | %f %f %f\n", cvmGet(trans_vects, 0, 0), cvmGet(trans_vects, 0, 1), cvmGet(trans_vects, 0, 2));

	// 3. initialize arrays of object points
	//*************project square0***************
	initObjectPoints(obj_FullPoints,output_full_size,cam_id,parking_assistant_params,real2pix_ratio,0);
	cvProjectPoints_fisheyeD1(obj_FullPoints,rot_vects,trans_vects,&camera,&dist_coeffs0,project_Points,NULL,NULL,NULL,NULL,NULL);			
	interpolate_image(img_src,output_full_size,img_rectify0,project_Points);
	// find the square0 corners	
	found0 = cvFindSquareCorners( img_rectify0, square0, parking_assistant_params, real2pix_ratio,cam_id ,0);		
	//*************project square1***************
	initObjectPoints(obj_FullPoints,output_full_size,cam_id,parking_assistant_params,real2pix_ratio,1);
	cvProjectPoints_fisheyeD1(obj_FullPoints,rot_vects,trans_vects,&camera,&dist_coeffs0,project_Points,NULL,NULL,NULL,NULL,NULL);
	interpolate_image(img_src,output_full_size,img_rectify1,project_Points);
	// find the square1 corners		
	found1 = cvFindSquareCorners( img_rectify1, square1, parking_assistant_params, real2pix_ratio,cam_id ,0);

	printf("camid = %d %d %d\n", cam_id, found0, found1);

	int square_count = 0;
	if(square0 && square1)
	{
		image_points = cvCreateMat( 1, point_count + 8, CV_32FC2 );
		object_points = cvCreateMat( 1, point_count + 8, CV_32FC3 );
		square_count = 2;
	}
	else if(square0 || square1)
	{
		square_count = 1;
		image_points = cvCreateMat( 1, point_count + 4, CV_32FC2 );
		object_points = cvCreateMat( 1, point_count + 4, CV_32FC3 );
	}
	//*** copy the chessboard obj/img points
	CvPoint2D32f* img_pt_src0 = (CvPoint2D32f*) image_points0->data.fl;
	CvPoint2D32f* img_pt_dst  = (CvPoint2D32f*) image_points->data.fl;
	CvPoint3D32f* obj_pt_src0 = (CvPoint3D32f*) object_points0->data.fl;
	CvPoint3D32f* obj_pt_dst  = (CvPoint3D32f*) object_points->data.fl;
	memcpy(img_pt_dst,img_pt_src0,point_count * sizeof(CvPoint2D32f));
	memcpy(obj_pt_dst,obj_pt_src0,point_count * sizeof(CvPoint3D32f));


	// 7. $)ADZ2NP^U}
	// project the object point by the square corners.
	mergePoints(square0,square1,real2pix_ratio,cam_id,parking_assistant_params,rot_vects,
				trans_vects,&camera,&dist_coeffs0,img_pt_dst,obj_pt_dst);

	// instrisic parameters optimization
	reprojection_err = instParaOpt(object_points,image_points,image_points->cols, 
									&camera, &dist_coeffs0,rot_vects,trans_vects,0 );

	printf("reprojection_err : %lf \n",reprojection_err);
	
	cvReleaseMat( &image_points);
	cvReleaseMat( &object_points);
									

	// 8. $)A1#4f=a9{
	//extCalibError = extCalibError|(((square0_flag <<12)|(chessboard_flag <<8)|(square0_flag <<4))<<cam_id) ;

	if(reprojection_err <120)
	{
		printf("\ncam_id[%d] inExtcalib Success!\n",cam_id);
		extr_params->data.db[0] = rot_vects->data.db[0]   ;
		extr_params->data.db[1] = rot_vects->data.db[1]   ;
		extr_params->data.db[2] = rot_vects->data.db[2]   ;
		extr_params->data.db[3] = trans_vects->data.db[0]   ;
		extr_params->data.db[4] = trans_vects->data.db[1]   ;
		extr_params->data.db[5] = trans_vects->data.db[2]   ;
		camera_matrix->data.db[0] = _camera[0];
		camera_matrix->data.db[4] = _camera[4];
		camera_matrix->data.db[2] = _camera[2];
		camera_matrix->data.db[5] = _camera[5];
		dist_coeffs->data.db[0]	  = _dist_coeffs[0];
		dist_coeffs->data.db[1]	  = _dist_coeffs[1];
		dist_coeffs->data.db[2]	  = _dist_coeffs[2];
		dist_coeffs->data.db[3]	  = _dist_coeffs[3];
		ret = 1;
	}
	else // if calib_inExt fail, calib_ext instead
	{
		// get the extrinsic parameter of Camera
		printf("\ncam_id[%d] extrinsic calib\n",cam_id);
		cvFindExtrinsicCameraParams_fisheye(object_points0,image_points0,camera_matrix, 
			dist_coeffs,rot_vects, trans_vects );
		
		extr_params->data.db[0] = rot_vects->data.db[0]   ;
		extr_params->data.db[1] = rot_vects->data.db[1]   ;
		extr_params->data.db[2] = rot_vects->data.db[2]   ;
		extr_params->data.db[3] = trans_vects->data.db[0]   ;
		extr_params->data.db[4] = trans_vects->data.db[1]   ;
		extr_params->data.db[5] = trans_vects->data.db[2]   ;
		ret = 2;
		
	}

#if 1
	printf("camera_matrix->data.db[0] : %f\n",camera_matrix->data.db[0]);
	printf("camera_matrix->data.db[4] : %f\n",camera_matrix->data.db[4]);
	printf("camera_matrix->data.db[2] : %f\n",camera_matrix->data.db[2]);
	printf("camera_matrix->data.db[5] : %f\n",camera_matrix->data.db[5]);

	printf("extr_params->data.db[0] : %f\n",extr_params->data.db[0]);
	printf("extr_params->data.db[1] : %f\n",extr_params->data.db[1]);
	printf("extr_params->data.db[2] : %f\n",extr_params->data.db[2]);
	printf("extr_params->data.db[3] : %f\n",extr_params->data.db[3]);
	printf("extr_params->data.db[4] : %f\n",extr_params->data.db[4]);
	printf("extr_params->data.db[5] : %f\n",extr_params->data.db[5]);
#endif

	cvFree( &square0);
	cvFree( &square1);
	cvReleaseMat( &img_rectify0);
	cvReleaseMat( &img_rectify1);
	cvReleaseMat( &image_points0 );
	cvReleaseMat( &object_points0 );
	cvReleaseMat( &obj_FullPoints );
	cvReleaseMat( &project_Points );	
	cvReleaseMat( &rot_vects );
	cvReleaseMat( &trans_vects );

	return ret;

}

void initObjectPoints(CvMat* obj_FullPoints, CvSize output_size,int cam_id,
				 Parking_Assistant_Params parking_assistant_params,float real2pix_ratio,int square_idx)
{
	CvPoint3D64f* obj_pt = (CvPoint3D64f*)obj_FullPoints->data.db;
	int out_width = output_size.width,
		out_height = output_size.height;
	float offset_x = 0; 
	float offset_y = 0; 
	int i,j;	
	
	switch (cam_id)
	{
	case 0:
	{
		if(square_idx == 0)
		{
			offset_x = HALF_BIG_SQUARE + CHESS_BOARD_TO_LEFT_BIG_SQUARE_DISTANCE  + out_width*0.5 * real2pix_ratio; // move the square to the center of the image;
			offset_y = out_height* 0.5 * real2pix_ratio - HALF_BIG_SQUARE + CAR_F_BLIND;
		}
		else if(square_idx == 1)
		{
			offset_x = -(CHESS_BOARD_TO_RIGHT_BIG_SQUARE_DISTANCE + HALF_BIG_SQUARE) + out_width*0.5 * real2pix_ratio; // move the square1 to the center of the image;
			offset_y = out_height* 0.5 * real2pix_ratio - HALF_BIG_SQUARE + CAR_F_BLIND;
		}
		break;
	}
	case 1:
	{
		if(square_idx == 0)
		{
			offset_x = HALF_BIG_SQUARE + CHESS_BOARD_TO_LEFT_BIG_SQUARE_DISTANCE + out_width*0.5 * real2pix_ratio; // move the square to the center of the image;
			offset_y = out_height* 0.5 * real2pix_ratio - (HALF_BIG_SQUARE + CAR_B_BLIND) ;
		}
		else if(square_idx == 1)
		{
			offset_x = -(CHESS_BOARD_TO_RIGHT_BIG_SQUARE_DISTANCE + HALF_BIG_SQUARE) + out_width*0.5 * real2pix_ratio; // move the square1 to the center of the image;
			offset_y = out_height* 0.5 * real2pix_ratio - (HALF_BIG_SQUARE + CAR_B_BLIND) ;
		}
		break;
	}
	case 2:
	{
		if(square_idx == 0)
		{
			offset_x = out_width*0.5 * real2pix_ratio + (HALF_BIG_SQUARE+CHESS_BOARD_TO_LEFT_BIG_SQUARE_DISTANCE+parking_assistant_params.chessboard_length_corners * parking_assistant_params.square_size * 0.5)-(parking_assistant_params.chessboard_width_corners * parking_assistant_params.square_size+parking_assistant_params.car_width/2); // move the square to the center of the image;
			offset_y = HALF_BIG_SQUARE + CAR_F_BLIND + parking_assistant_params.LRchess2carFront_distance + out_height*0.5 * real2pix_ratio;
		}
		else if(square_idx == 1)
		{
			offset_x = (HALF_BIG_SQUARE+CHESS_BOARD_TO_LEFT_BIG_SQUARE_DISTANCE+parking_assistant_params.chessboard_length_corners * parking_assistant_params.square_size * 0.5)-(parking_assistant_params.chessboard_width_corners * parking_assistant_params.square_size+parking_assistant_params.car_width/2) + out_width*0.5 * real2pix_ratio; // move the square1 to the center of the image;
			offset_y = -(parking_assistant_params.car_length - parking_assistant_params.LRchess2carFront_distance + CAR_B_BLIND + HALF_BIG_SQUARE)+ out_height*0.5 * real2pix_ratio ;
		}
		break;
	}
	case 3:
	{
	 	/*if(square_idx == 0)
        {

            offset_x = out_width * 0.5 * real2pix_ratio + parking_assistant_params.car_width * 0.5  - (70 + 40) -50;				// move the square to the center of the image;
            offset_y = 50 + parking_assistant_params.LRchess2carFront_distance + out_height * 0.5 * real2pix_ratio;

        }
        else if(square_idx == 1)
        {

            offset_x = out_width * 0.5 * real2pix_ratio + parking_assistant_params.car_width * 0.5 - (70 + 40) -50; 				// move the square to the center of the image;
            offset_y = -(parking_assistant_params.car_length - parking_assistant_params.LRchess2carFront_distance + 50) + out_height * 0.5 * real2pix_ratio ;

        }*/

        if(square_idx == 0)
        {
                offset_x = out_width * 0.5 * real2pix_ratio + parking_assistant_params.car_width * 0.5  - (HALF_BIG_SQUARE + CHESS_BOARD_TO_LEFT_BIG_SQUARE_DISTANCE + parking_assistant_params.chessboard_length_corners * parking_assistant_params.square_size * 0.5); 				// move the square to the center of the image;
                offset_y = (HALF_BIG_SQUARE + CAR_F_BLIND) + parking_assistant_params.LRchess2carFront_distance + out_height * 0.5 * real2pix_ratio;
        }
        else if(square_idx == 1)
        {
                offset_x = out_width * 0.5 * real2pix_ratio + parking_assistant_params.car_width * 0.5 - (HALF_BIG_SQUARE + CHESS_BOARD_TO_LEFT_BIG_SQUARE_DISTANCE + parking_assistant_params.chessboard_length_corners * parking_assistant_params.square_size * 0.5); 				// move the square to the center of the image;
                offset_y = -(HALF_BIG_SQUARE + parking_assistant_params.car_length - parking_assistant_params.LRchess2carFront_distance + CAR_B_BLIND) + out_height * 0.5 * real2pix_ratio ;
        }
		break;
	}
	default:
		break;

	}

	for(j = 0; j < out_height; j++)
	{
		for(i = 0; i < out_width; i++)
		{
			*(obj_pt+j*out_width + i) = cvPoint3D64f((double)(i * real2pix_ratio - offset_x ),
				(double)(j * real2pix_ratio - offset_y),0 );
		}
	}
}



CV_IMPL void 
interpolate_image(const CvMat* img_src,CvSize output_size, CvMat* img_dst, CvMat* project_Points )
{
	int j, x, y,ix,iy;
	unsigned char f,f1, f2, f3, f4;
	int width = output_size.width,height = output_size.height;
	int src_width = img_src->width,src_height = img_src->height;
	CvPoint2D64f* proj_pt = (CvPoint2D64f*)project_Points->data.db;
	Uint8* img_src_pt = img_src->data.ptr;
	Uint8* img_dst_pt = img_dst->data.ptr;

	int pix_num = output_size.width * output_size.height;
	for(j =0 ; j< pix_num; j++)
	{
		double temp_x =0,
			   temp_y =0;

		temp_x	   = proj_pt[j].x;
		temp_y	   = proj_pt[j].y;
		x		= (int)(temp_x);
		y		= (int)(temp_y);
		
		// black the out of range points
		if(y> src_height-2 || y <0 || x > src_width-2 || x<0)
		{
			img_dst_pt[j] = 80;
			continue;
		}

		ix		= (int)((temp_x - x)*64);
		iy		= (int)((temp_y - y)*64);
		f1		= img_src_pt[y*src_width + x];
		f2		= img_src_pt[y*src_width + x +1];
		f3		= img_src_pt[(y+1)*src_width + x];
		f4		= img_src_pt[(y+1)*src_width + x +1];
		f		= ((64 - ix)*(64 - iy)*f1 + ix*(64 - iy)*f2 + (64 - ix)*iy*f3 + ix*iy*f4)>>12;
		img_dst_pt[j] = f;
	}
}

int mergePoints(CvPoint2D32f *square0,CvPoint2D32f *square1, float real2pix_ratio,
				int cam_id,Parking_Assistant_Params parking_assistant_params,CvMat* rot_vects,CvMat* trans_vects,
				CvMat* camera_matrix,CvMat* dist_coeffs, 
				CvPoint2D32f *img_pt_dst0,CvPoint3D32f *obj_pt_dst0)
{
	int i;
	CvMat *obj_points_temp,*img_points_temp ;
	CvPoint2D32f* img_pt_dst = img_pt_dst0;
	CvPoint2D32f*img_pt_src0 = NULL;
	CvPoint3D32f* obj_pt_src0 = NULL,* obj_pt_dst = obj_pt_dst0;
	int square_num =0;
	int point_count = (parking_assistant_params.chessboard_length_corners -1) * (parking_assistant_params.chessboard_width_corners -1);
	int out_width = RECTIFY_WIDTH;
	int out_height = RECTIFY_HEIGHT;

	float offset_x0, offset_y0,offset_x1,offset_y1;
	float square_base_x  = 0., square_base_y  = 0., square1_base_x = 0., square1_base_y = 0.;

	if(square0 && square1)
	{
		obj_points_temp = cvCreateMat(1, 8 , CV_32FC3);
		img_points_temp = cvCreateMat(1, 8 , CV_32FC2);
	}
	else if(square0 || square1)
	{
	    obj_points_temp = cvCreateMat(1, 4 , CV_32FC3);
		img_points_temp = cvCreateMat(1, 4 , CV_32FC2);
	}
	else
		return 0;

	switch (cam_id)
	{
	case 0:
		{	
			offset_x0 = HALF_BIG_SQUARE + CHESS_BOARD_TO_LEFT_BIG_SQUARE_DISTANCE  + out_width*0.5 * real2pix_ratio; // move the square to the center of the image;
			offset_y0 = out_height* 0.5 * real2pix_ratio - HALF_BIG_SQUARE + CAR_F_BLIND;
			offset_x1 = -(CHESS_BOARD_TO_RIGHT_BIG_SQUARE_DISTANCE + HALF_BIG_SQUARE) + out_width*0.5 * real2pix_ratio; // move the square1 to the center of the image;
			offset_y1 = out_height* 0.5 * real2pix_ratio - HALF_BIG_SQUARE + CAR_F_BLIND;	
			
			square_base_y  = BIG_SQUARE - CAR_F_BLIND;
			square_base_x  = -CHESS_BOARD_TO_LEFT_BIG_SQUARE_DISTANCE-BIG_SQUARE;
			square1_base_y = BIG_SQUARE - CAR_F_BLIND;
			square1_base_x = CHESS_BOARD_TO_RIGHT_BIG_SQUARE_DISTANCE;		
			break;
		}
	case 1:
		{
			offset_x0 = HALF_BIG_SQUARE + CHESS_BOARD_TO_LEFT_BIG_SQUARE_DISTANCE + out_width*0.5 * real2pix_ratio; // move the square to the center of the image;
			offset_y0 = out_height* 0.5 * real2pix_ratio - (HALF_BIG_SQUARE + CAR_B_BLIND) ;
			offset_x1 = -(CHESS_BOARD_TO_RIGHT_BIG_SQUARE_DISTANCE + HALF_BIG_SQUARE) + out_width*0.5 * real2pix_ratio; // move the square1 to the center of the image;
			offset_y1 = out_height* 0.5 * real2pix_ratio - (HALF_BIG_SQUARE + CAR_B_BLIND) ;
			
			square_base_y  = BIG_SQUARE + CAR_B_BLIND;
			square_base_x  = -CHESS_BOARD_TO_LEFT_BIG_SQUARE_DISTANCE-BIG_SQUARE;
			square1_base_y = BIG_SQUARE + CAR_B_BLIND;
			square1_base_x = CHESS_BOARD_TO_RIGHT_BIG_SQUARE_DISTANCE;	
			break;
		}
	case 2:
	    {
	        offset_x0 = out_width * 0.5 * real2pix_ratio  - parking_assistant_params.car_width * 0.5 - 				// move the square to the center of the image;
	                        parking_assistant_params.chessboard_width_corners * parking_assistant_params.square_size + (HALF_BIG_SQUARE + CHESS_BOARD_TO_LEFT_BIG_SQUARE_DISTANCE + parking_assistant_params.chessboard_length_corners * parking_assistant_params.square_size * 0.5);
	        offset_y0 = (HALF_BIG_SQUARE + CAR_F_BLIND) + parking_assistant_params.LRchess2carFront_distance + out_height * 0.5 * real2pix_ratio;
	        offset_x1 = out_width * 0.5 * real2pix_ratio  - parking_assistant_params.car_width * 0.5 - 				// move the square to the center of the image;
	                    parking_assistant_params.chessboard_width_corners * parking_assistant_params.square_size + (HALF_BIG_SQUARE + CHESS_BOARD_TO_LEFT_BIG_SQUARE_DISTANCE + parking_assistant_params.chessboard_length_corners * parking_assistant_params.square_size * 0.5);
	        offset_y1 = -(parking_assistant_params.car_length - parking_assistant_params.LRchess2carFront_distance + CAR_B_BLIND + HALF_BIG_SQUARE) + out_height * 0.5 * real2pix_ratio ;

	        square_base_x  = parking_assistant_params.chessboard_width_corners * parking_assistant_params.square_size + parking_assistant_params.car_width * 0.5 - (parking_assistant_params.chessboard_length_corners * parking_assistant_params.square_size * 0.5 + CHESS_BOARD_TO_LEFT_BIG_SQUARE_DISTANCE + BIG_SQUARE);
	        square_base_y  =  -(parking_assistant_params.LRchess2carFront_distance + CAR_F_BLIND);
	        square1_base_x = parking_assistant_params.chessboard_width_corners * parking_assistant_params.square_size + parking_assistant_params.car_width * 0.5 - (parking_assistant_params.chessboard_length_corners * parking_assistant_params.square_size * 0.5 + CHESS_BOARD_TO_LEFT_BIG_SQUARE_DISTANCE + BIG_SQUARE);
	        square1_base_y = (parking_assistant_params.car_length - parking_assistant_params.LRchess2carFront_distance + CAR_B_BLIND + BIG_SQUARE );
	    	break;
	    }
	case 3:
    {
        offset_x0 = out_width * 0.5 * real2pix_ratio + parking_assistant_params.car_width * 0.5 - (HALF_BIG_SQUARE + CHESS_BOARD_TO_LEFT_BIG_SQUARE_DISTANCE + parking_assistant_params.chessboard_length_corners * parking_assistant_params.square_size * 0.5); 				// move the square to the center of the image;
        offset_y0 = (HALF_BIG_SQUARE + CAR_F_BLIND) + parking_assistant_params.LRchess2carFront_distance + out_height * 0.5 * real2pix_ratio;
        offset_x1 = out_width * 0.5 * real2pix_ratio + parking_assistant_params.car_width * 0.5 - (HALF_BIG_SQUARE + CHESS_BOARD_TO_LEFT_BIG_SQUARE_DISTANCE + parking_assistant_params.chessboard_length_corners * parking_assistant_params.square_size * 0.5); 				// move the square to the center of the image;
        offset_y1 = -(parking_assistant_params.car_length - parking_assistant_params.LRchess2carFront_distance + CAR_B_BLIND + HALF_BIG_SQUARE) + out_height * 0.5 * real2pix_ratio ;

        square_base_x  = -parking_assistant_params.car_width * 0.5 + (parking_assistant_params.chessboard_length_corners * parking_assistant_params.square_size * 0.5 + CHESS_BOARD_TO_LEFT_BIG_SQUARE_DISTANCE);
        square_base_y  = -(parking_assistant_params.LRchess2carFront_distance + CAR_F_BLIND);
        square1_base_x = -parking_assistant_params.car_width * 0.5 + (parking_assistant_params.chessboard_length_corners * parking_assistant_params.square_size * 0.5 + CHESS_BOARD_TO_LEFT_BIG_SQUARE_DISTANCE);
        square1_base_y = (parking_assistant_params.car_length - parking_assistant_params.LRchess2carFront_distance + CAR_B_BLIND + BIG_SQUARE );
    
        break;
    }
	default:
		break;
	}
	
	// get the square corners obj points;
	CvPoint3D32f* obj_pt_temp = (CvPoint3D32f*)obj_points_temp->data.fl;
	if(square0)
	{
		CvPoint2D32f *square_corners = square0 ;
		for(i =0; i<4; i++)
		{
			*obj_pt_temp++ = cvPoint3D32f(square_corners[i].x * real2pix_ratio-offset_x0,
				square_corners[i].y * real2pix_ratio -offset_y0, 0);
		}
	}
	if(square1)
	{
		CvPoint2D32f *square_corners = square1 ;
		for(i =0; i<4; i++)
		{
			*obj_pt_temp++ = cvPoint3D32f(square_corners[i].x * real2pix_ratio-offset_x1,
				square_corners[i].y * real2pix_ratio -offset_y1, 0);
		}
	}
			
	cvProjectPoints_fisheyeD1(obj_points_temp,rot_vects,trans_vects,camera_matrix,dist_coeffs,img_points_temp,NULL,NULL,NULL,NULL,NULL);			
	//cvProjectPoints_fisheye(object_points_temp,rot_vects,trans_vects,camera_matrix,dist_coeffs,image_points,NULL,NULL,NULL,NULL,NULL);			


	// output the square img points
	CvPoint2D32f* img_pt_temp = (CvPoint2D32f*) img_points_temp->data.fl;
	img_pt_dst += point_count;
	memcpy(img_pt_dst,img_pt_temp,img_points_temp->cols * sizeof(CvPoint2D32f));

	// count the square corners obj points.
	obj_pt_dst += point_count;
	if(square0)
	{
		*obj_pt_dst++ = cvPoint3D32f(square_base_x      , square_base_y - BIG_SQUARE,0);
		*obj_pt_dst++ = cvPoint3D32f(square_base_x + BIG_SQUARE , square_base_y - BIG_SQUARE,0);
		*obj_pt_dst++ = cvPoint3D32f(square_base_x + BIG_SQUARE , square_base_y     ,0);
		*obj_pt_dst++ = cvPoint3D32f(square_base_x      , square_base_y     ,0);
		square_num += 1;
	}
	if(square1)
	{
		*obj_pt_dst++ = cvPoint3D32f(square1_base_x     , square1_base_y - BIG_SQUARE,0);
		*obj_pt_dst++ = cvPoint3D32f(square1_base_x + BIG_SQUARE, square1_base_y - BIG_SQUARE,0);
		*obj_pt_dst++ = cvPoint3D32f(square1_base_x + BIG_SQUARE, square1_base_y     ,0);
		*obj_pt_dst++ = cvPoint3D32f(square1_base_x     , square1_base_y     ,0);
		square_num += 1;
	}	

	for(i = point_count; i<point_count + 4*square_num; i++)
	{
		//printf("%d %f %f\n",i,img_pt_dst0[i].x,img_pt_dst0[i].y);
		img_pt_dst0[i].x *= 0.5;
		img_pt_dst0[i].y *= 0.5;
	}

	cvReleaseMat( &obj_points_temp);
	cvReleaseMat( &img_points_temp);
	return 1;

}

CV_IMPL double
instParaOpt( const CvMat* obj_points,
                    const CvMat* img_points,
                    const int point_counts,
                    CvMat* A, CvMat* dist_coeffs,
                    CvMat* r_vecs, CvMat* t_vecs,
                    int flags )
{
    double alpha_smooth = 0.1;// 0.02;//0.4;//0.02;
	double reproject_err =0;
    CvMat  *_M = 0, *_m = 0;
    CvMat *_Ji = 0, *_Je = 0, *_JtJ = 0, *_JtErr = 0, *_JtJW = 0, *_JtJV = 0;
    CvMat *_param = 0, *_param_innov = 0, *_err = 0;
    
    CV_FUNCNAME( "instParaOpt" );

    __BEGIN__;

    double a[9];
    CvMat _a = cvMat( 3, 3, CV_64F, a ), _k;
    CvMat _Mi, _mi, _ri, _ti, _part;
    CvMat _dpdr, _dpdt, _dpdf, _dpdc, _dpdk;
    CvMat _sr_part = cvMat( 1, 3, CV_64F ), _st_part = cvMat( 1, 3, CV_64F ), _r_part, _t_part;
    int i, j, pos, iter, img_count, count = 0, max_count = 0, total = 0, param_count;
    int r_depth = 0, t_depth = 0, r_step = 0, t_step = 0, cn, dims;
    int output_r_matrices = 0;


    img_count = 1;

    if( r_vecs )
    {
        r_depth = CV_MAT_DEPTH(r_vecs->type);
        r_step = r_vecs->rows == 1 ? 3*CV_ELEM_SIZE(r_depth) : r_vecs->step;
        cn = CV_MAT_CN(r_vecs->type);
        if( !CV_IS_MAT(r_vecs) || r_depth != CV_32F && r_depth != CV_64F ||
            (r_vecs->rows != img_count || r_vecs->cols*cn != 3 && r_vecs->cols*cn != 9) &&
            (r_vecs->rows != 1 || r_vecs->cols != img_count || cn != 3) )
            CV_ERROR( CV_StsBadArg, "the output array of rotation vectors must be 3-channel "
                "1xn or nx1 array or 1-channel nx3 or nx9 array, where n is the number of views" );
        output_r_matrices = r_vecs->rows == img_count && r_vecs->cols*cn == 9;
    }

    if( t_vecs )
    {
        t_depth = CV_MAT_DEPTH(t_vecs->type);
        t_step = t_vecs->rows == 1 ? 3*CV_ELEM_SIZE(t_depth) : t_vecs->step;
        cn = CV_MAT_CN(t_vecs->type);
        if( !CV_IS_MAT(t_vecs) || t_depth != CV_32F && t_depth != CV_64F ||
            (t_vecs->rows != img_count || t_vecs->cols*cn != 3) &&
            (t_vecs->rows != 1 || t_vecs->cols != img_count || cn != 3) )
            CV_ERROR( CV_StsBadArg, "the output array of translation vectors must be 3-channel "
                "1xn or nx1 array or 1-channel nx3 array, where n is the number of views" );
    }

    if( CV_MAT_TYPE(A->type) != CV_32FC1 && CV_MAT_TYPE(A->type) != CV_64FC1 ||
        A->rows != 3 || A->cols != 3 )
        CV_ERROR( CV_StsBadArg,
            "Intrinsic parameters must be 3x3 floating-point matrix" );

    if( CV_MAT_TYPE(dist_coeffs->type) != CV_32FC1 &&
        CV_MAT_TYPE(dist_coeffs->type) != CV_64FC1 ||
        (dist_coeffs->rows != 4 || dist_coeffs->cols != 1) &&
        (dist_coeffs->cols != 4 || dist_coeffs->rows != 1))
        CV_ERROR( CV_StsBadArg,
            "Distortion coefficients must be 4x1 or 1x4 floating-point matrix" );

	max_count = point_counts;
	total     = point_counts;
	count = point_counts ;

    dims = CV_MAT_CN(obj_points->type)*(obj_points->rows == 1 ? 1 : obj_points->cols);

    if( CV_MAT_DEPTH(obj_points->type) != CV_32F &&
        CV_MAT_DEPTH(obj_points->type) != CV_64F ||
        (obj_points->rows != total || dims != 3 && dims != 2) &&
        (obj_points->rows != 1 || obj_points->cols != total || (dims != 3 && dims != 2)) )
        CV_ERROR( CV_StsBadArg, "Object points must be 1xn or nx1, 2- or 3-channel matrix, "
                                "or nx3 or nx2 single-channel matrix" );

    cn = CV_MAT_CN(img_points->type);
    if( CV_MAT_DEPTH(img_points->type) != CV_32F &&
        CV_MAT_DEPTH(img_points->type) != CV_64F ||
        (img_points->rows != total || img_points->cols*cn != 2) &&
        (img_points->rows != 1 || img_points->cols != total || cn != 2) )
        CV_ERROR( CV_StsBadArg, "Image points must be 1xn or nx1, 2-channel matrix, "
                                "or nx2 single-channel matrix" );

    CV_CALL( _M = cvCreateMat( 1, total, CV_64FC3 ));
    CV_CALL( _m = cvCreateMat( 1, total, CV_64FC2 ));

    CV_CALL( cvConvertPointsHomogenious( obj_points, _M ));
    CV_CALL( cvConvertPointsHomogenious( img_points, _m ));

    param_count = 8 +6 ;//8 + img_count*6;
	param_count = 8 +6;
     _param = cvCreateMat( param_count, 1, CV_64FC1 );
     _param_innov = cvCreateMat( param_count, 1, CV_64FC1 );
     _JtJ = cvCreateMat( param_count, param_count, CV_64FC1 );
     _JtErr = cvCreateMat( param_count, 1, CV_64FC1 );
     _JtJW = cvCreateMat( param_count, 1, CV_64FC1 );
     _JtJV = cvCreateMat( param_count, param_count, CV_64FC1 );
     _Ji = cvCreateMat( max_count*2, 8, CV_64FC1 );
     _Je = cvCreateMat( max_count*2, 6, CV_64FC1 );
     _err = cvCreateMat( max_count*2, 1, CV_64FC1 );
    cvGetCols( _Je, &_dpdr, 0, 3 );
    cvGetCols( _Je, &_dpdt, 3, 6 );
    cvGetCols( _Ji, &_dpdf, 0, 2 );
    cvGetCols( _Ji, &_dpdc, 2, 4 );
    cvGetCols( _Ji, &_dpdk, 4, flags & CV_CALIB_ZERO_TANGENT_DIST ? 6 : 8 );
    cvSet(&_dpdk,cvScalar(DBL_EPSILON),NULL);

    // 1. initialize intrinsic parameters
	_k = cvMat( dist_coeffs->rows, dist_coeffs->cols, CV_64FC1, _param->data.db + 4 );
	cvConvert(dist_coeffs,&_k);
	cvConvert( A, &_a);
	_param->data.db[0] = a[0];
    _param->data.db[1] = a[4];
    _param->data.db[2] = a[2];
    _param->data.db[3] = a[5];

    // 2. initialize extrinsic parameters
	_param->data.db[8] = r_vecs->data.db[0];
	_param->data.db[9] = r_vecs->data.db[1];
	_param->data.db[10] = r_vecs->data.db[2];
	_param->data.db[11] = t_vecs->data.db[0];
	_param->data.db[12] = t_vecs->data.db[1];
	_param->data.db[13] = t_vecs->data.db[2];

	
    // 3. run the optimization
	for( iter = 0; iter < 30; iter++ )
	{
		double* jj = _JtJ->data.db;
		double change;
		for( i = 0, pos = 0; i < img_count; i++, pos += count )
		{
			_ri = cvMat( 1, 3, CV_64FC1, _param->data.db + 8 + i*6);
			_ti = cvMat( 1, 3, CV_64FC1, _param->data.db + 8 + i*6 + 3);

			cvGetCols( _M, &_Mi, pos, pos + count );
			_mi = cvMat( count*2, 1, CV_64FC1, _m->data.db + pos*2 );

			_dpdr.rows = _dpdt.rows = _dpdf.rows = _dpdc.rows = _dpdk.rows = count*2;

			_err->cols = 1;
			_err->rows = count*2;
			cvReshape( _err, _err, 2, count );						
				//cvProjectPoints_fisheye( &_Mi, &_ri, &_ti, &_a, &_k, _err, &_dpdr, &_dpdt, &_dpdf, &_dpdc, &_dpdk  );
			cvProjectPoints_fisheye( &_Mi, &_ri, &_ti, &_a, &_k, _err, &_dpdr, &_dpdt, &_dpdf, &_dpdc, NULL  );
			cvSet(&_dpdk,cvScalar(DBL_EPSILON),NULL);

			cvReshape( _err, _err, 1, count*2 );
			cvSub( &_mi, _err, _err );
        
			_Je->rows = _Ji->rows = count*2;
        
			cvGetSubRect( _JtJ, &_part, cvRect(0,0,8,8) );
			cvGEMM( _Ji, _Ji, 1, &_part, i > 0, &_part, CV_GEMM_A_T );

			cvGetSubRect( _JtJ, &_part, cvRect(8+i*6,8+i*6,6,6) );
			cvMulTransposed( _Je, &_part, 1 );
        
			cvGetSubRect( _JtJ, &_part, cvRect(8+i*6,0,6,8) );
			cvGEMM( _Ji, _Je, 1, 0, 0, &_part, CV_GEMM_A_T );

			cvGetRows( _JtErr, &_part, 0, 8 );
			cvGEMM( _Ji, _err, 1, &_part, i > 0, &_part, CV_GEMM_A_T );

			cvGetRows( _JtErr, &_part, 8 + i*6, 8 + (i+1)*6 );
			cvGEMM( _Je, _err, 1, 0, 0, &_part, CV_GEMM_A_T );
		}

		// make the matrix JtJ exactly symmetrical and add missing zeros
		for( i = 0; i < param_count; i++ )
		{
			int mj = i < 8 ? param_count : ((i - 8)/6)*6 + 14;
			for( j = i+1; j < mj; j++ )
				jj[j*param_count + i] = jj[i*param_count + j];
			for( ; j < param_count; j++ )
				jj[j*param_count + i] = jj[i*param_count + j] = 0;
		}

		cvSVD( _JtJ, _JtJW, 0, _JtJV, CV_SVD_MODIFY_A + CV_SVD_V_T );
		cvSVBkSb( _JtJW, _JtJV, _JtJV, _JtErr, _param_innov, CV_SVD_U_T + CV_SVD_V_T );

		cvScale( _param_innov, _param_innov, 1. - pow(1. - alpha_smooth, iter + 1.) );
		cvGetRows( _param_innov, &_part, 0, 4 );
		change = cvNorm( &_part );
		cvGetRows( _param, &_part, 0, 4);
		change /= cvNorm( &_part );

 		for (i = 4; i < 8; i++)
 		{
 			_param_innov->data.db[i]  = DBL_EPSILON;
 		}

		cvAdd( _param, _param_innov, _param );
    
		a[0] = _param->data.db[0];
		a[4] = _param->data.db[1];
		a[2] = _param->data.db[2];
		a[5] = _param->data.db[3];

		if(change < FLT_EPSILON ) //gbb DBL_EPSILON FLT_EPSILON
			break;
	}

	printf("iter : %d\n",iter);

	reproject_err = cvNorm( _err );
	printf("reproject_err: %lf\n",reproject_err);

    cvConvert( &_a, A );
    cvConvert( &_k, dist_coeffs );

    _r_part = cvMat( output_r_matrices ? 3 : 1, 3, r_depth );
    _t_part = cvMat( 1, 3, t_depth );
    for( i = 0; i < img_count; i++ )
    {
        if( r_vecs )
        {
            _sr_part.data.db = _param->data.db + 8 + i*6;
            _r_part.data.ptr = r_vecs->data.ptr + i*r_step;
            if( !output_r_matrices )
                cvConvert( &_sr_part, &_r_part );
            else
            {
                cvRodrigues2( &_sr_part, &_a );
                cvConvert( &_a, &_r_part );
            }
        }
        if( t_vecs )
        {
            _st_part.data.db = _param->data.db + 8 + i*6 + 3;
            _t_part.data.ptr = t_vecs->data.ptr + i*t_step;
            cvConvert( &_st_part, &_t_part );
        }
    }

    __END__;

    cvReleaseMat( &_M );
    cvReleaseMat( &_m );
    cvReleaseMat( &_param );
    cvReleaseMat( &_param_innov );
    cvReleaseMat( &_JtJ );
    cvReleaseMat( &_JtErr );
    cvReleaseMat( &_JtJW );
    cvReleaseMat( &_JtJV );
    cvReleaseMat( &_Ji );
    cvReleaseMat( &_Je );
    cvReleaseMat( &_err );
	return reproject_err;
}


