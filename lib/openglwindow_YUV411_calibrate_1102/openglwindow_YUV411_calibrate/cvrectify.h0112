/*********************************************************************************
* $)A0fH(KySP (C) ::;*025@9I7]SPO^TpHN9+K>
* 
* $)AND<~C{3F#: cvrectity.cpp
* $)AND<~1jJ6#: 
* $)ADZH]U*R*#: M(9}5XCfFLIh5DFeEL8q#,UR5==G5c2"JdHkDZ2N#,;q5CMb2N	#,Wn:sM(9}1d;;#,;q5C
			 $)AP#U}3IK.F=5D5XCfM<Oq
* $)AFdK|K5Cw#: 
* $)A51G00f1>#: 
* $)AWw    U_#: 9(1y1y#(gbb#)
* $)AMj3IHUFZ#: 2014052x
* --------------------------------------------------------------------------------
* $)AP^8D<GB<1#:
* $)AP^8DHUFZ#:
* $)A0f 1> :E#:
* $)AP^ 8D HK#: 
* $)AP^8DDZH]#:
**********************************************************************************/
#ifndef CVRECTIFY_H
#define CVRECTIFY_H

//#include <stdio.h>

#include "avmInit.hpp"

//#include "pku360.h"
//#include "_cv.h"

#define FIELD_CHESSBOARD_SQUARE_SIZE    20					/* $)A&L!B(:?2?2?(:y*/
#define FIELD_CHESSBOARD_LENGTH_CORNERS 7
#define FIELD_CHESSBOARD_WIDTH_CORNERS  5
#define CAM_CHESSBOARD_LENGTH_CORNERS 7
#define CAM_CHESSBOARD_WIDTH_CORNERS  5


#define WIDTH_WASTE          2      /* 4cif$)AT-J<M<OqVPWsSR6*Fz5D@,;xOqKX */
#define HEIGHT_WASTE         2       /* 4cif$)AT-J<M<OqVPIOOB6*Fz5D@,;xOqKX */

//#define RECTIFY_WIDTH		 256 //258 // count from mosaicImage
//#define RECTIFY_HEIGHT		 890 //448 // count from mosaicImage

#define CAR_WIDTH_PIX	     180 //100		//model car has 60 pixels width in the picture.

#define WIDVPFE         1280
#define HEIVPFE         720

#define HEIGHT          (HEIVPFE>>1)
#define WIDTH           (WIDVPFE>>1) 

#define RECTIFY_WIDTH 320
#define RECTIFY_HEIGHT 320

#define PI              3.1415926    

#define CAR_F_BLIND 0
#define CAR_B_BLIND 0
#define CAR_LR_BLIND 0

#define BIG_SQUARE 100									//$)A4s:Z8qWS4sP!
#define HALF_BIG_SQUARE 50								//$)A4s:Z8qWSVPPD5c5D>`@k
#define CHESS_BOARD_TO_LEFT_BIG_SQUARE_DISTANCE  40		//$)AFeEL8qWs2`5=Ws1_4s:Z8qWS>`@k
#define CHESS_BOARD_TO_RIGHT_BIG_SQUARE_DISTANCE  180	//$)AFeEL8qWs2`5=SR1_4s:Z8qWS>`@k

#define SCALE1          100000                
#define SCALE2          100                   
#define SCALE3          10000  

// extern for c++
#ifdef __cplusplus
extern "C" {
#endif



/* rectify the input image to the horizontal plane, with known distortion
	coeffs and corner-found image */
CVAPI(void)
initChessBoardPoints(int cam_id,const CvSeq* image_points_seq,CvMat* image_points, CvMat* object_points, Parking_Assistant_Params parking_assistant_params);
					 
CVAPI(int) cvFindChessboardCorners_ext( const void* image, CvSize pattern_size,
                                    CvPoint2D32f* corners,
                                    int* corner_count,
                                    int cam_id, int extInFlag );

CVAPI(int)
run_calib_InstExt(const CvMat* img_src,const CvSeq* image_points_seq,CvMat* camera_matrix, 
				  CvMat* dist_coeffs, CvMat* extr_params,int cam_id, Parking_Assistant_Params parking_assistant_params);

CVAPI(void)
initObjectPoints(CvMat* obj_FullPoints, CvSize output_size,int cam_id,
				 Parking_Assistant_Params parking_assistant_params,float real2pix_ratio,int square_idx);
					

CVAPI(void)
interpolate_image(const CvMat* src,CvSize output_size, CvMat* dst, CvMat* project_Points );


CVAPI(int)
cvFindChessboardCorners_instExt( const void* arr, CvSize pattern_size,CvPoint2D32f *out_corners,
								CvPoint2D32f **square0,CvPoint2D32f **square1, int* out_corner_count, 
								Parking_Assistant_Params parking_assistant_params,int cam_id );
CVAPI(int)
cvFindSquareCorners( const void* arr, CvPoint2D32f *square0,Parking_Assistant_Params parking_assistant_params,
						float real2pix_ratio,int cam_id, int flag);

CVAPI (void)
cvFindExtrinsicCameraParams_fisheye( const CvMat* obj_points,
                  const CvMat* img_points, const CvMat* A,
                  const CvMat* dist_coeffs,
                  CvMat* r_vec, CvMat* t_vec );

void
cvProjectPoints_fisheye( const CvMat* obj_points,
                  const CvMat* r_vec,
                  const CvMat* t_vec,
                  const CvMat* A,
                  const CvMat* dist_coeffs,
                  CvMat* img_points, CvMat* dpdr,
                  CvMat* dpdt, CvMat* dpdf,
                  CvMat* dpdc, CvMat* dpdk );
CVAPI(void) 
cvUndistort_fisheye( const CvArr* _src, CvArr* _dst, const CvMat* A, const CvMat* dist_coeffs );

void 
cvProjectPoints_fisheyeD1( const CvMat* obj_points,
					  const CvMat* r_vec,
					  const CvMat* t_vec,
					  const CvMat* A,
					  const CvMat* dist_coeffs,
					  CvMat* img_points, CvMat* dpdr,
	                  CvMat* dpdt, CvMat* dpdf,
	                  CvMat* dpdc, CvMat* dpdk );
void 
searchSubSquareCorners(CvMat* grad_xy,CvSize winSize, CvPoint2D32f* square_corners,int corners_Count);

void 
searchSubSquareCorners2(const CvMat* image,CvSize winSize, CvPoint2D32f* square_corners,int corners_Count);

int mergePoints(CvPoint2D32f *square0,CvPoint2D32f *square1, float real2pix_ratio,
				int cam_id,Parking_Assistant_Params parking_assistant_params,CvMat* rot_vects,CvMat* trans_vects,
				CvMat* camera_matrix,CvMat* dist_coeffs, 
				CvPoint2D32f *img_pt_dst0,CvPoint3D32f *obj_pt_dst0);

CVAPI(double)
cvCalibrateCamera_fisheye( const CvMat* obj_points,
                    const CvMat* img_points,
                    const CvMat* point_counts,
                    CvSize image_size,
                    CvMat* A, CvMat* dist_coeffs,
                    CvMat* r_vecs, CvMat* t_vecs,
                    int flags );

CVAPI(double)
instParaOpt( const CvMat* obj_points,
			const CvMat* img_points,
			const int point_counts,
			CvMat* A, CvMat* dist_coeffs,
			CvMat* r_vecs, CvMat* t_vecs,
			int flags );
void
cvUnDistortMap(CvMat* undistort_Points, CvSize size,
                     const double* intrinsic_matrix);

#ifdef __cplusplus
}
#endif //c_plusplus

#endif // CVRECTIFY_H

