/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

/************************************************************************************\
    This is improved variant of chessboard corner detection algorithm that
    uses a graph of connected quads. It is based on the code contributed
    by Vladimir Vezhnevets and Philip Gruebele.
    Here is the copyright notice from the original Vladimir's code:
    ===============================================================

    The algorithms developed and implemented by Vezhnevets Vldimir
    aka Dead Moroz (vvp@graphics.cs.msu.ru)
    See http://graphics.cs.msu.su/en/research/calibration/opencv.html
    for detailed information.

    Reliability additions and modifications made by Philip Gruebele.
    <a href="mailto:pgruebele@cox.net">pgruebele@cox.net</a>

\************************************************************************************/

#include "_cv.h"
//#include "IQMATH.H"
//#include "IQmath_inline.h"
#include "cvrectify.h"
//#include "highgui.h"

#define Uint8 unsigned char
//=====================================================================================
// Implementation for the enhanced calibration object detection
//=====================================================================================

#define MAX_CONTOUR_APPROX  7
#define max_corners 50
#define CV_ALLOC


typedef struct CvContourEx
{
    CV_CONTOUR_FIELDS()
    int counter;
}
CvContourEx;

//=====================================================================================

/// Corner info structure
/** This structure stores information about the chessboard corner.*/
typedef struct CvCBCorner
{
    CvPoint2D32f pt; // Coordinates of the corner
    int row;         // Board row index
    int count;       // Number of neighbor corners
    struct CvCBCorner* neighbors[4]; // Neighbor corners
}
CvCBCorner;

//=====================================================================================
/// Quadrangle contour info structure
/** This structure stores information about the chessboard quadrange.*/
typedef struct CvCBQuad
{
    int count;      // Number of quad neibors
    int group_idx;  // quad group ID
    float edge_len; // quad size characteristic
    CvCBCorner *corners[4]; // Coordinates of quad corners
    struct CvCBQuad *neighbors[4]; // Pointers of quad neighbors
}
CvCBQuad;

//=====================================================================================
extern "C" CvPoint2D32f debug_out_corners[24]; //$)AMb2NWT6/5wJT9}3LVP#,SCSZOTJ>RQ>-UR5=5D=G5c#,1cSZH7HODD8v=G5cC;UR5=!#
extern "C" int debug_corner_count;

static int icvGenerateQuads( CvCBQuad **quads, CvCBCorner **corners,
                             CvMemStorage *storage, CvMat *image, int flags );

static void icvFindQuadNeighbors( CvCBQuad *quads, int quad_count );

static int icvFindConnectedQuads( CvCBQuad *quads, int quad_count,
                                  CvCBQuad **quad_group, int group_idx,
                                  CvMemStorage* storage );

static int icvCheckQuadGroup( CvCBQuad **quad_group, int count,
                              CvCBCorner **out_corners, CvSize pattern_size );

static int icvCleanFoundConnectedQuads( int quad_count,
                CvCBQuad **quads, CvSize pattern_size );

static int repairQuads(CvCBQuad **quad_group, int quad_count, int corner_count,
						CvCBCorner *quad_corners,CvCBQuad *quads,
						int org_quad_count,CvSize pattern_size);
int predictCorner(CvCBQuad *quad_4neighbors,CvCBQuad *quad_3neighbors,
						int lost_corner_idx,int connect4_corner_idx );

CV_IMPL
int cvFindChessboardCorners_fix( const void* arr, CvSize pattern_size,
                             CvPoint2D32f* out_corners, int* out_corner_count,
                             int flags )
{
	int k;
	const int min_dilations = 1;
    const int max_dilations = 1;
    int found = 0;
    CvMat* img = 0;
    CvMat* thresh_img = 0;
    CvMemStorage* storage = 0;

    CvCBQuad *quads = 0, **quad_group = 0;
    CvCBCorner *corners = 0, **corner_group = 0;

    if( out_corner_count )
        *out_corner_count = 0;

    CV_FUNCNAME( "cvFindChessBoardCornerGuesses2" );

    __BEGIN__;

     int quad_count, group_idx, i, dilations;
    CvMat stub, *input_img = (CvMat*)arr;

    CV_CALL( input_img = cvGetMat( input_img, &stub ));
    //debug_img = img;

    if( CV_MAT_DEPTH( input_img->type ) != CV_8U || CV_MAT_CN( input_img->type ) == 2 )
        CV_ERROR( CV_StsUnsupportedFormat, "Only 8-bit grayscale or color images are supported" );

    if( pattern_size.width <= 2 || pattern_size.height <= 2 )
        CV_ERROR( CV_StsOutOfRange, "pattern should have at least 2x2 size" );

    if( !out_corners )
        CV_ERROR( CV_StsNullPtr, "Null pointer to corners" );

    CV_CALL( storage = cvCreateMemStorage(0) );
    CV_CALL( thresh_img = cvCreateMat( input_img->rows, input_img->cols, CV_8UC1 ));
    CV_CALL( img = cvCreateMat( input_img->rows, input_img->cols, CV_8UC1 ));

    if( CV_MAT_CN(input_img->type) != 1 || (flags & CV_CALIB_CB_NORMALIZE_IMAGE) )
    {
        // equalize the input image histogram -
        // that should make the contrast between "black" and "white" areas big enoug

        if( CV_MAT_CN(input_img->type) != 1 )
        {
            CV_CALL( cvCvtColor( input_img, img, CV_BGR2GRAY ));
            //img = norm_img;
        }

        if( flags & CV_CALIB_CB_NORMALIZE_IMAGE )
        {
            cvEqualizeHist( input_img, img );
            //img = norm_img;
        }
    }
	else
	{
		cvCopy(input_img, img,0);
	}

	
	//gbb. pre-process the image
	

    // Try our standard "1" dilation, but if the pattern is not found, iterate the whole procedure with higher dilations.
    // This is necessary because some squares simply do not separate properly with a single dilation.  However,
    // we want to use the minimum number of dilations possible since dilations cause the squares to become smaller,
    // making it difficult to detect smaller squares.

	//gbb,20140730, add resize part, resize the bellow two-thirds part of the img to get larger chessboards.

	for( dilations = min_dilations; dilations <= max_dilations; dilations++ )
	{
		for(k = 0; k<3; k++)
		{		
			// convert the input grayscale image to binary (black-n-white)
			int block_size = cvRound(MIN(img->cols,img->rows)*0.3)|1; //gbb 0.2 -> 0.3

			// convert to binary
			cvAdaptiveThreshold( img, thresh_img, 255,
				CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, block_size, k*20);//   CV_ADAPTIVE_THRESH_MEAN_C
		   	cvDilate( thresh_img, thresh_img, 0, dilations );

			// So we can find rectangles that go to the edge, we draw a white line around the image edge.
			// Otherwise FindContours will miss those clipped rectangle contours.
			// The border color will be the image mean, because otherwise we risk screwing up filters like cvSmooth()...
			cvRectangle( thresh_img, cvPoint(0,0), cvPoint(thresh_img->cols-1,
						 thresh_img->rows-1), CV_RGB(255,255,255), 3, 8);

			CV_CALL( quad_count = icvGenerateQuads( &quads, &corners, storage, thresh_img, flags ));
			if( quad_count <= 0 )
			{			
				continue;
			}

			// Find quad's neighbors
			CV_CALL( icvFindQuadNeighbors( quads, quad_count ));
#ifdef CV_ALLOC
			CV_CALL( quad_group = (CvCBQuad**)cvAlloc( sizeof(quad_group[0]) * quad_count));
			CV_CALL( corner_group = (CvCBCorner**)cvAlloc( sizeof(corner_group[0]) * quad_count*4 ));
#else
			CV_CALL( quad_group = (CvCBQuad**)malloc( sizeof(quad_group[0]) * quad_count));
			CV_CALL( corner_group = (CvCBCorner**)malloc( sizeof(corner_group[0]) * quad_count*4 ));
#endif

			for( group_idx = 0; ; group_idx++ )
			{
				int count;
				CV_CALL( count = icvFindConnectedQuads( quads, quad_count, quad_group, group_idx, storage ));

				if( count == 0 )
					break;

				// If count is more than it should be, this will remove those quads
				// which cause maximum deviation from a nice square pattern.
				CV_CALL( count = icvCleanFoundConnectedQuads( count, quad_group, pattern_size ));
				CV_CALL( count = icvCheckQuadGroup( quad_group, count, corner_group, pattern_size ));

				if( count > 0 || (out_corner_count && -count > *out_corner_count) )
				{
					int n = count > 0 ? pattern_size.width * pattern_size.height : -count;
					n = MIN( n, pattern_size.width * pattern_size.height );

					// copy corners to output array
					for( i = 0; i < n; i++ )
					{
						out_corners[i] = corner_group[i]->pt;
					}
					if( out_corner_count )
						*out_corner_count = n;

					if( count > 0 )
					{
						found = 1;
						EXIT;
					}
				}
			}
#ifdef CV_ALLOC
			cvFree( &quads );
			cvFree( &corners );
#else
			free( quads );
			free( corners );
			quads = NULL;
			corners = NULL;
#endif
		}  
	}
    __END__;


    cvReleaseMemStorage( &storage );
    cvReleaseMat( &thresh_img );
	cvReleaseMat( &img );
#ifdef CV_ALLOC
	if(quads)
		cvFree( &quads);
	if(corners)
		cvFree( &corners);
	if(quad_group)
		cvFree( &quad_group);
	if(corner_group)
		cvFree( &corner_group);
#else
	if(quads)
		free( quads );
	if(corners)
		free( corners );
	if(quad_group)
		free( quad_group );
	if(corner_group)
		free( corner_group );
#endif 

    return found;
}


//this chessboard corners finding function is designed for extrinsic parameter calibration.
CV_IMPL
int cvFindChessboardCorners_ext( const void* arr, CvSize pattern_size,
                             CvPoint2D32f* out_corners, int* out_corner_count,
                             int cam_id, int extInFlag )
{
	int k,j,l;
	const int min_dilations = 1;
    const int max_dilations = 2;
    int found = 0;
    CvMat* img = 0,* resize_img = NULL,* sub_img = NULL;
    CvMat* thresh_img = 0;
    CvMemStorage* storage = 0;

    CvCBQuad *quads = 0, **quad_group = 0;
    CvCBCorner *corners = 0, **corner_group = 0;

    if( out_corner_count )
        *out_corner_count = 0;

    CV_FUNCNAME( "cvFindChessBoardCornerGuesses2" );

    __BEGIN__;

     int quad_count, group_idx, i, dilations;
    CvMat *img_in = (CvMat*)arr;

    CV_CALL( storage = cvCreateMemStorage(0) );
    CV_CALL( thresh_img = cvCreateMat( img_in->rows, img_in->cols, CV_8UC1 ));
	CV_CALL( img = cvCreateMat( img_in->rows, img_in->cols, CV_8UC1 ));
	CV_CALL( resize_img = cvCreateMat( img->rows, img->cols, CV_8UC1 ));

	//gbb. pre-process the image
	

    // Try our standard "1" dilation, but if the pattern is not found, iterate the whole procedure with higher dilations.
    // This is necessary because some squares simply do not separate properly with a single dilation.  However,
    // we want to use the minimum number of dilations possible since dilations cause the squares to become smaller,
    // making it difficult to detect smaller squares.

	//gbb,20140730, add resize part, resize the bellow two-thirds part of the img to get larger chessboards.
	for(j = 0; j <2; j++)// predict corners at the second loop
	{

	int min_l = 0;
	int max_l = 0; 
	int offset_rows =0;
	cvCopy(img_in,img);
	if (cam_id == 0 || cam_id ==1) // for front/rear cam, resize img firt
	{
		if (extInFlag)
		{
			min_l = 0;
			max_l = 1;	
		}
		else
		{
			min_l = 2;
			max_l = 4;
		}
	}
	else
	{
		min_l = 0;
		max_l = 1;
	}
	for (l = min_l; l<max_l; l++)
	{
		//printf("l: %d\n",l);
		//gbb,20140730,resize the button part of the org image to enlarge the chessboards
		if(l > 0)
		{
			offset_rows = cvRound(img->rows / l);
			CV_CALL( sub_img = cvCreateMatHeader( img->rows-offset_rows, img->cols, CV_8UC1 ));
			cvGetSubRect(img,sub_img,cvRect(0,offset_rows,img->cols,img->rows-offset_rows));
			cvResize(sub_img,resize_img,CV_INTER_AREA);
			//cvCopy(resize_img,img,NULL);
		}
		else
			cvCopy(img,resize_img);

		for(k = 0; k<3; k++)
		{
			for( dilations = min_dilations; dilations <= max_dilations; dilations++ )
			{
				int block_size = cvRound(MIN(img->cols,img->rows)*0.2)|1; //gbb 0.2 -> 0.3

				// convert to binary
				cvAdaptiveThreshold( resize_img, thresh_img, 255,
					CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, block_size, k*20);//   CV_ADAPTIVE_THRESH_MEAN_C
			   	cvDilate( thresh_img, thresh_img, 0, dilations );

				// So we can find rectangles that go to the edge, we draw a white line around the image edge.
				// Otherwise FindContours will miss those clipped rectangle contours.
				// The border color will be the image mean, because otherwise we risk screwing up filters like cvSmooth()...
				cvRectangle( thresh_img, cvPoint(0,0), cvPoint(thresh_img->cols-1,
							 thresh_img->rows-1), CV_RGB(255,255,255), 3, 8);

				CV_CALL( quad_count = icvGenerateQuads( &quads, &corners, storage, thresh_img, 0 ));
				if( quad_count <= 0 )
				{			
					continue;
				}

				// Find quad's neighbors
				CV_CALL( icvFindQuadNeighbors( quads, quad_count ));
				
				//printf("sizeof(quad_group[0]) %d\n",sizeof(quad_group[0]));
#ifdef CV_ALLOC
				CV_CALL( quad_group = (CvCBQuad**)cvAlloc( sizeof(quad_group[0]) * (10+ quad_count)));
				CV_CALL( corner_group = (CvCBCorner**)cvAlloc( sizeof(corner_group[0]) * (10+quad_count)*10 ));
#else
				CV_CALL( quad_group = (CvCBQuad**)malloc( sizeof(quad_group[0]) *(10+ quad_count))); // add 4 lost quad
				CV_CALL( corner_group = (CvCBCorner**)malloc( sizeof(corner_group[0]) * (10+quad_count)*10 )); // add 4*4 lost corner
#endif
				for( group_idx = 0; ; group_idx++ )
				{
					int count = 0,q_count = 0;
					int lost_count =0;
					CV_CALL( q_count = icvFindConnectedQuads( quads, quad_count, quad_group, group_idx, storage ));

					if( q_count == 0)
						break;

					// If count is more than it should be, this will remove those quads
					// which cause maximum deviation from a nice square pattern.
                    CV_CALL( q_count = icvCleanFoundConnectedQuads( q_count, quad_group, pattern_size ));
					CV_CALL( count = icvCheckQuadGroup( quad_group, q_count, corner_group, pattern_size ));

					lost_count = pattern_size.width * pattern_size.height -abs(count);
					if (j==1 && lost_count >0 && lost_count <6) // lest then 4 corners not found, then guess the lost corners
					{
						printf("lost corners: %d\n",lost_count);
						q_count = repairQuads(quad_group,q_count,abs(count),corners,quads,quad_count,pattern_size);
						printf("q_count by repairQuads: %d\n",q_count);
						CV_CALL( count = icvCheckQuadGroup( quad_group, q_count, corner_group, pattern_size ));
					}

					if( count > 0 || (out_corner_count && -count > *out_corner_count) )
					{
						int n = count > 0 ? pattern_size.width * pattern_size.height : -count;
						n = MIN( n, pattern_size.width * pattern_size.height );

						// copy corners to output array
						for( i = 0; i < n; i++ )
						{
							float temp_y = 0.;
							out_corners[i] = corner_group[i]->pt;
							//out_corners[i].y = img->rows - _IQ20toF(_IQ20mpy(_FtoIQ20(img->rows - out_corners[i].y)
							//										, _IQ20pow(_FtoIQ20(0.666666),_IQ20(l))));	
							out_corners[i].y = offset_rows + float(img->rows - offset_rows) * out_corners[i].y /img->rows;						 
							{
								// boundary check
								if(out_corners[i] .x <2)
									out_corners[i].x  =2;
								if(out_corners[i] .x > WIDVPFE -2)
									out_corners[i].x  =WIDVPFE -2;
								if(out_corners[i] .y <2)
									out_corners[i].y  =2;
								if(out_corners[i] .y > HEIVPFE -2)
									out_corners[i].y  =HEIVPFE -2;
								//debug_out_corners[i] = out_corners[i];
								//debug_out_corners[i].x *= (float)WIDVPBE/WIDVPFE;
								//debug_out_corners[i].y *= (float)HEIVPBE/HEIVPFE;

							}
						}
						if( out_corner_count )
						{
							*out_corner_count = n;
							debug_corner_count = n;
						}

						if( count > 0 )
						{
							found = 1;
							EXIT;
						}
					}
				}
#ifdef CV_ALLOC
			    cvFree( &quads );
			    cvFree( &corners );
#else
				free( quads );
				free( corners );
				quads = NULL;
				corners = NULL;
#endif
			}  
		}
		 
	}
	}
    __END__;
    cvReleaseMemStorage( &storage );
    cvReleaseMat( &thresh_img );
	
	cvReleaseMat( &img );
	cvReleaseMat( &resize_img);
#ifdef CV_ALLOC
	if(quads)
		cvFree( &quads);
	if(corners)
		cvFree( &corners);
	if(quad_group)
		cvFree( &quad_group);
	if(corner_group)
		cvFree( &corner_group);
#else
	if(quads)
		free( quads );
	if(corners)
		free( corners );
	if(quad_group)
		free( quad_group );
	if(corner_group)
		free( corner_group );
#endif 

    return found;
}

CV_IMPL
int cvFindSquareCorners( const void* arr, CvPoint2D32f *square0, Parking_Assistant_Params parking_assistant_params,
						float real2pix_ratio,int cam_id, int flag)
{
	int i,j;
    int found = 0;
    CvMemStorage* storage = NULL;

    CvCBQuad *quads = 0;
    CvCBCorner *corners = 0;

    CvMat  *img = (CvMat*)arr;
	CvMat *img_thresh =0;
	IplConvKernel* element = NULL;
	element = cvCreateStructuringElementEx(5, 5, 2, 2,CV_SHAPE_ELLIPSE, 0 );

	int rect_width = img->cols, rect_height = img->rows;
    int quad_count;

	//int block_size = cvRound(MIN(img->cols,img->rows)*0.5)|1; //gbb 0.2 -> 0.3
	int block_size = 81;//cvRound(MIN(img->cols,img->rows)*0.5)|1; //gbb 0.2 -> 0.3
	//  searching the square corners
	CvCBQuad quads0;
	int idea_cx0 =rect_width/2, idea_cy0 =rect_height/2;
	float quads_cx =0.,quads_cy =0.;
	float min_edge_len = pow(float(30)/real2pix_ratio,2), max_edge_len = pow(float(80)/real2pix_ratio,2) ;
	float min_dist0 = 10000., min_dist1 = 10000.;
	float dist0 =0.,dist1 = 0.;
	float q_v = 0.,q_h = 0.;

    CV_FUNCNAME( "cvFindSquareCorners" );

    __BEGIN__;


	img_thresh = cvCreateMat( img->rows, img->cols, CV_8UC1 );
    storage = cvCreateMemStorage(0) ;

	cvAdaptiveThreshold( img, img_thresh, 255,CV_ADAPTIVE_THRESH_GAUSSIAN_C, //CV_ADAPTIVE_THRESH_GAUSSIAN_C  CV_ADAPTIVE_THRESH_MEAN_C
						CV_THRESH_BINARY, block_size, 0   ); 

	//cvThreshold(img, img_thresh,
                          //10, 255,
                          //CV_THRESH_BINARY_INV );
	if(0)//flag
	{
		cvDilate( img_thresh, img_thresh, element, 2 );
		cvErode( img_thresh, img_thresh, element, 2 );
	}
	
	cvReleaseStructuringElement( &element);
	// So we can find rectangles that go to the edge, we draw a white line around the image edge.
	// Otherwise FindContours will miss those clipped rectangle contours.
	// The border color will be the image mean, because otherwise we risk screwing up filters like cvSmooth()...
	cvRectangle( img_thresh, cvPoint(0,0), cvPoint(img_thresh->cols-1,
				 img_thresh->rows-1), CV_RGB(255,255,255), 3, 4);
	CV_CALL( quad_count = icvGenerateQuads( &quads, &corners, storage, img_thresh, 0 ));

	if (cam_id == 0 || cam_id == 1)
	{
		q_v = 0.8;			
	}
	else
	{
		q_v = 0.2;
	}
	q_h = 1 - q_v;

	switch(cam_id)
	{
		case 0:
		{
			min_edge_len = pow(float(30)/real2pix_ratio,2);
			max_edge_len = pow(float(200)/real2pix_ratio,2) ;
			break;							
		}
		case 1:
		{
			min_edge_len = pow(float(30)/real2pix_ratio,2);
			max_edge_len = pow(float(200)/real2pix_ratio,2) ;
			break;							
		}
		case 2:
		{
			min_edge_len = pow(float(20)/real2pix_ratio,2);
			max_edge_len = pow(float(300)/real2pix_ratio,2) ;
			break;				
		}
		case 3:
		{
			min_edge_len = pow(float(20)/real2pix_ratio,2);
			max_edge_len = pow(float(300)/real2pix_ratio,2) ;
			break;				
		}
	}
	//LOG_printf(&trace,"min_edge_len: %f\n",min_edge_len);
	//LOG_printf(&trace,"max_edge_len: %f\n",max_edge_len);


	printf("cam_id=%d flag=%d quad_count: %d %f %f\n",cam_id, flag, quad_count, min_edge_len, max_edge_len);
	for (i = 0; i <quad_count; i++)
	{				
		quads_cx = (quads[i].corners[0]->pt.x + quads[i].corners[1]->pt.x 
					+ quads[i].corners[2]->pt.x + quads[i].corners[3]->pt.x)*0.25;
		quads_cy = (quads[i].corners[0]->pt.y + quads[i].corners[1]->pt.y 
					+ quads[i].corners[2]->pt.y + quads[i].corners[3]->pt.y)*0.25;

		//dist0 = sqrt(q_h * (idea_cx0 - quads_cx)*(idea_cx0 - quads_cx)
				   //+ q_v * (idea_cy0 - quads_cy)*(idea_cy0 - quads_cy));

		dist0 = sqrt((idea_cx0 - quads_cx)*(idea_cx0 - quads_cx)
				   + (idea_cy0 - quads_cy)*(idea_cy0 - quads_cy));

		//printf("dist0 = %f\n", dist0);
		if ( dist0 < min_dist0 )
		{
			//printf("near square quads[%d].edge_len: %f\n",i,quads[i].edge_len);
			if (quads[i].edge_len > min_edge_len && quads[i].edge_len < max_edge_len ) 
			{
				min_dist0 = dist0;
				//LOG_printf(&trace,"good square!\n");
				quads0 = quads[i];							
			}					
		}			
	}
	//printf("min_dist0 = %f thresh = %f\n",min_dist0, (200.0/real2pix_ratio));
	if(min_dist0 < (200.0/real2pix_ratio))
	{
		found =1;
	
		CvCBQuad temp_quads;
		float avg_x,avg_y  ;
		temp_quads = quads0;
		
		avg_x = (temp_quads.corners[0]->pt.x + temp_quads.corners[1]->pt.x + 
						temp_quads.corners[2]->pt.x + temp_quads.corners[3]->pt.x)*0.25 ;
		avg_y = (temp_quads.corners[0]->pt.y + temp_quads.corners[1]->pt.y + 
					temp_quads.corners[2]->pt.y + temp_quads.corners[3]->pt.y)*0.25 ;


		// rearrange the square corners
		for(j =0; j <4; j++)
		{
			float temp_x = temp_quads.corners[j]->pt.x ,
				temp_y = temp_quads.corners[j]->pt.y ;
			if(temp_x < avg_x && temp_y < avg_y)
			{
				(square0[0]).x = temp_x ;
				(square0[0]).y = temp_y ;
			}
			else if(temp_x > avg_x && temp_y < avg_y)
			{
				(square0[1]).x = temp_x ;
				(square0[1]).y = temp_y ;
			}
			else if(temp_x > avg_x && temp_y > avg_y)
			{
				(square0[2]).x = temp_x ;
				(square0[2]).y = temp_y ;
			}
			else if(temp_x < avg_x && temp_y > avg_y)
			{
				(square0[3]).x = temp_x ;
				(square0[3]).y = temp_y ;
			}
		}
	}
	else
	{
		printf("min_dist0 = %f\n",min_dist0);
		found = 0;
	}
    __END__;

#ifdef CV_ALLOC
	if(quads)
		cvFree( &quads);
	if(corners)
		cvFree( &corners);
#else
	if(quads)
		free( quads );
	if(corners)
		free( corners );
#endif 

    cvReleaseMemStorage( &storage );
    cvReleaseMat( &img_thresh );

    return found;
}


// if we found too many connect quads, remove those which probably do not belong.
static int
icvCleanFoundConnectedQuads( int quad_count, CvCBQuad **quad_group, CvSize pattern_size )
{
    CvMemStorage *temp_storage = 0;
    CvPoint2D32f *centers = 0;

    CV_FUNCNAME( "icvCleanFoundConnectedQuads" );

    __BEGIN__;

    CvPoint2D32f center = {0,0};
    int i, j, k;
    // number of quads this pattern should contain
    int count = ((pattern_size.width + 1)*(pattern_size.height + 1) + 1)/2;

    // if we have more quadrangles than we should,
    // try to eliminate duplicates or ones which don't belong to the pattern rectangle...
    if( quad_count <= count )
        EXIT;

    // create an array of quadrangle centers
#ifdef CV_ALLOC
    CV_CALL( centers = (CvPoint2D32f *)cvAlloc( sizeof(centers[0])*quad_count ));
#else
	CV_CALL( centers = (CvPoint2D32f *)malloc( sizeof(centers[0])*quad_count ));
#endif

    CV_CALL( temp_storage = cvCreateMemStorage(0));

    for( i = 0; i < quad_count; i++ )
    {
        CvPoint2D32f ci = {0,0};
        CvCBQuad* q = quad_group[i];

        for( j = 0; j < 4; j++ )
        {
            CvPoint2D32f pt = q->corners[j]->pt;
            ci.x += pt.x;
            ci.y += pt.y;
        }

        ci.x *= 0.25f;
        ci.y *= 0.25f;

        centers[i] = ci;
        center.x += ci.x;
        center.y += ci.y;
    }
    center.x /= quad_count;
    center.y /= quad_count;

    // If we still have more quadrangles than we should,
    // we try to eliminate bad ones based on minimizing the bounding box.
    // We iteratively remove the point which reduces the size of
    // the bounding box of the blobs the most
    // (since we want the rectangle to be as small as possible)
    // remove the quadrange that causes the biggest reduction
    // in pattern size until we have the correct number
    for( ; quad_count > count; quad_count-- )
    {
        double min_box_area = DBL_MAX;
        int skip, min_box_area_index = -1;
        CvCBQuad *q0, *q;

        // For each point, calculate box area without that point
        for( skip = 0; skip < quad_count; skip++ )
        {
            // get bounding rectangle
            CvPoint2D32f temp = centers[skip]; // temporarily make index 'skip' the same as
            centers[skip] = center;            // pattern center (so it is not counted for convex hull)
            CvMat pointMat = cvMat(1, quad_count, CV_32FC2, centers);
            CvSeq *hull = cvConvexHull2( &pointMat, temp_storage, CV_CLOCKWISE, 1 );
            centers[skip] = temp;
            double hull_area = fabs(cvContourArea(hull, CV_WHOLE_SEQ));

            // remember smallest box area
            if( hull_area < min_box_area )
            {
                min_box_area = hull_area;
                min_box_area_index = skip;
            }
            cvClearMemStorage( temp_storage );
        }

        q0 = quad_group[min_box_area_index];

        // remove any references to this quad as a neighbor
        for( i = 0; i < quad_count; i++ )
        {
            q = quad_group[i];
            for( j = 0; j < 4; j++ )
            {
                if( q->neighbors[j] == q0 )
                {
                    q->neighbors[j] = 0;
                    q->count--;
                    for( k = 0; k < 4; k++ )
                        if( q0->neighbors[k] == q )
                        {
                            q0->neighbors[k] = 0;
                            q0->count--;
                            break;
                        }
                    break;
                }
            }
        }

        // remove the quad
        quad_count--;
        quad_group[min_box_area_index] = quad_group[quad_count];
        centers[min_box_area_index] = centers[quad_count];
    }

    __END__;

    cvReleaseMemStorage( &temp_storage );
#ifdef CV_ALLOC
    cvFree( &centers );
#else
	free( centers);
#endif 

    return quad_count;
}

//=====================================================================================

static int
icvFindConnectedQuads( CvCBQuad *quad, int quad_count, CvCBQuad **out_group,
                       int group_idx, CvMemStorage* storage )
{
    CvMemStorage* temp_storage = cvCreateChildMemStorage( storage );
    CvSeq* stack = cvCreateSeq( 0, sizeof(*stack), sizeof(void*), temp_storage );
    int i, count = 0;

    // Scan the array for a first unlabeled quad
    for( i = 0; i < quad_count; i++ )
    {
        if( quad[i].count > 0 && quad[i].group_idx < 0)
            break;
    }

    // Recursively find a group of connected quads starting from the seed quad[i]
    if( i < quad_count )
    {
        CvCBQuad* q = &quad[i];
        cvSeqPush( stack, &q );
        out_group[count++] = q;
        q->group_idx = group_idx;

        while( stack->total )
        {
            cvSeqPop( stack, &q );
            for( i = 0; i < 4; i++ )
            {
                CvCBQuad *neighbor = q->neighbors[i];
                if( neighbor && neighbor->count > 0 && neighbor->group_idx < 0 )
                {
                    cvSeqPush( stack, &neighbor );
                    out_group[count++] = neighbor;
                    neighbor->group_idx = group_idx;
                }
            }
        }
    }

    cvReleaseMemStorage( &temp_storage );
    return count;
}


//=====================================================================================

static int
icvCheckQuadGroup( CvCBQuad **quad_group, int quad_count,
                   CvCBCorner **out_corners, CvSize pattern_size )
{
    const int ROW1 = 1000000;
    const int ROW2 = 2000000;
    const int ROW_ = 3000000;
    int result = 0;
    int i, out_corner_count = 0, corner_count = 0;
    CvCBCorner** corners = 0;

    CV_FUNCNAME( "icvCheckQuadGroup" );

    __BEGIN__;

    int j, k, kk;
    int width = 0, height = 0;
    int hist[5] = {0,0,0,0,0};
    CvCBCorner* first = 0, *first2 = 0, *right, *cur, *below, *c;
#ifdef CV_ALLOC
    CV_CALL( corners = (CvCBCorner**)cvAlloc( quad_count*4*sizeof(corners[0]) ));
#else
    CV_CALL( corners = (CvCBCorner**)malloc( quad_count*4*sizeof(corners[0]) ));
#endif

    // build dual graph, which vertices are internal quad corners
    // and two vertices are connected iff they lie on the same quad edge
    for( i = 0; i < quad_count; i++ )
    {
        CvCBQuad* q = quad_group[i];
        /*CvScalar color = q->count == 0 ? cvScalar(0,255,255) :
                         q->count == 1 ? cvScalar(0,0,255) :
                         q->count == 2 ? cvScalar(0,255,0) :
                         q->count == 3 ? cvScalar(255,255,0) :
                                         cvScalar(255,0,0);*/

        for( j = 0; j < 4; j++ )
        {
            //cvLine( debug_img, cvPointFrom32f(q->corners[j]->pt), cvPointFrom32f(q->corners[(j+1)&3]->pt), color, 1, CV_AA, 0 );
            if( q->neighbors[j] )
            {
                CvCBCorner *a = q->corners[j], *b = q->corners[(j+1)&3];
                // mark internal corners that belong to:
                //   - a quad with a single neighbor - with ROW1,
                //   - a quad with two neighbors     - with ROW2
                // make the rest of internal corners with ROW_
                int row_flag = q->count == 1 ? ROW1 : q->count == 2 ? ROW2 : ROW_;

                if( a->row == 0 )
                {
                    corners[corner_count++] = a;
                    a->row = row_flag;
                }
                else if( a->row > row_flag )
                    a->row = row_flag;

                if( q->neighbors[(j+1)&3] )
                {
                    if( a->count >= 4 || b->count >= 4 )
                        EXIT;
                    for( k = 0; k < 4; k++ )
                    {
                        if( a->neighbors[k] == b )
                            EXIT;
                        if( b->neighbors[k] == a )
                            EXIT;
                    }
                    a->neighbors[a->count++] = b;
                    b->neighbors[b->count++] = a;
                }
            }
        }
    }

    if( corner_count != pattern_size.width*pattern_size.height )
        EXIT;

    for( i = 0; i < corner_count; i++ )
    {
        int n = corners[i]->count;
        assert( 0 <= n && n <= 4 );
        hist[n]++;
        if( !first && n == 2 )
        {
            if( corners[i]->row == ROW1 )
                first = corners[i];
            else if( !first2 && corners[i]->row == ROW2 )
                first2 = corners[i];
        }
    }

    // start with a corner that belongs to a quad with a signle neighbor.
    // if we do not have such, start with a corner of a quad with two neighbors.
    if( !first )
        first = first2;

    if( !first || hist[0] != 0 || hist[1] != 0 || hist[2] != 4 ||
        hist[3] != (pattern_size.width + pattern_size.height)*2 - 8 )
        EXIT;

    cur = first;
    right = below = 0;
    out_corners[out_corner_count++] = cur;

    for( k = 0; k < 4; k++ )
    {
        c = cur->neighbors[k];
        if( c )
        {
            if( !right )
                right = c;
            else if( !below )
                below = c;
        }
    }

    if( !right || right->count != 2 && right->count != 3 ||
        !below || below->count != 2 && below->count != 3 )
        EXIT;

    cur->row = 0;
    //cvCircle( debug_img, cvPointFrom32f(cur->pt), 3, cvScalar(0,255,0), -1, 8, 0 );

    first = below; // remember the first corner in the next row
    // find and store the first row (or column)
    for(j=1;;j++)
    {
        right->row = 0;
        out_corners[out_corner_count++] = right;
        //cvCircle( debug_img, cvPointFrom32f(right->pt), 3, cvScalar(0,255-j*10,0), -1, 8, 0 );
        if( right->count == 2 )
            break;
        if( right->count != 3 || out_corner_count >= MAX(pattern_size.width,pattern_size.height) )
            EXIT;
        cur = right;
        for( k = 0; k < 4; k++ )
        {
            c = cur->neighbors[k];
            if( c && c->row > 0 )
            {
                for( kk = 0; kk < 4; kk++ )
                {
                    if( c->neighbors[kk] == below )
                        break;
                }
                if( kk < 4 )
                    below = c;
                else
                    right = c;
            }
        }
    }

    width = out_corner_count;
    if( width == pattern_size.width )
        height = pattern_size.height;
    else if( width == pattern_size.height )
        height = pattern_size.width;
    else
        EXIT;

    // find and store all the other rows
    for( i = 1; ; i++ )
    {
        if( !first )
            break;
        cur = first;
        first = 0;
        for( j = 0;; j++ )
        {
            cur->row = i;
            out_corners[out_corner_count++] = cur;
            //cvCircle( debug_img, cvPointFrom32f(cur->pt), 3, cvScalar(0,0,255-j*10), -1, 8, 0 );
            if( cur->count == 2 + (i < height-1) && j > 0 )
                break;

            right = 0;

            // find a neighbor that has not been processed yet
            // and that has a neighbor from the previous row
            for( k = 0; k < 4; k++ )
            {
                c = cur->neighbors[k];
                if( c && c->row > i )
                {
                    for( kk = 0; kk < 4; kk++ )
                    {
                        if( c->neighbors[kk] && c->neighbors[kk]->row == i-1 )
                            break;
                    }
                    if( kk < 4 )
                    {
                        right = c;
                        if( j > 0 )
                            break;
                    }
                    else if( j == 0 )
                        first = c;
                }
            }
            if( !right )
                EXIT;
            cur = right;
        }

        if( j != width - 1 )
            EXIT;
    }

    if( out_corner_count != corner_count )
        EXIT;

    // check if we need to transpose the board
    if( width != pattern_size.width )
    {
        CV_SWAP( width, height, k );

        memcpy( corners, out_corners, corner_count*sizeof(corners[0]) );
        for( i = 0; i < height; i++ )
            for( j = 0; j < width; j++ )
                out_corners[i*width + j] = corners[j*height + i];
    }

    // check if we need to revert the order in each row
    {
        CvPoint2D32f p0 = out_corners[0]->pt, p1 = out_corners[pattern_size.width-1]->pt,
                     p2 = out_corners[pattern_size.width]->pt;
        if( (p1.x - p0.x)*(p2.y - p1.y) - (p1.y - p0.y)*(p2.x - p1.x) < 0 )
        {
            if( width % 2 == 0 )
            {
                for( i = 0; i < height; i++ )
                    for( j = 0; j < width/2; j++ )
                        CV_SWAP( out_corners[i*width+j], out_corners[i*width+width-j-1], c );
            }
            else
            {
                for( j = 0; j < width; j++ )
                    for( i = 0; i < height/2; i++ )
                        CV_SWAP( out_corners[i*width+j], out_corners[(height - i - 1)*width+j], c );
            }
        }
    }

    result = corner_count;

    __END__;

    if( result <= 0 && corners )
    {
        corner_count = MIN( corner_count, pattern_size.width*pattern_size.height );
        for( i = 0; i < corner_count; i++ )
            out_corners[i] = corners[i];
        result = -corner_count;
    }
#ifdef CV_ALLOC
	cvFree( &corners );
#else
    free( corners );
#endif

    return result;
}

//=====================================================================================
#if 1
static void icvFindQuadNeighbors( CvCBQuad *quads, int quad_count )
{
    const float thresh_scale = 1.f;
    int idx, i, k, j;
    float dx, dy, dist;

    // find quad neighbors
    for( idx = 0; idx < quad_count; idx++ )
    {
        CvCBQuad* cur_quad = &quads[idx];

        // choose the points of the current quadrangle that are close to
        // some points of the other quadrangles
        // (it can happen for split corners (due to dilation) of the
        // checker board). Search only in other quadrangles!

        // for each corner of this quadrangle
        for( i = 0; i < 4; i++ )
        {
            CvPoint2D32f pt;
            float min_dist = FLT_MAX;
            int closest_corner_idx = -1;
            CvCBQuad *closest_quad = 0;
            CvCBCorner *closest_corner = 0;

            if( cur_quad->neighbors[i] )
                continue;

            pt = cur_quad->corners[i]->pt;

            // find the closest corner in all other quadrangles
            for( k = 0; k < quad_count; k++ )
            {
                if( k == idx )
                    continue;

                for( j = 0; j < 4; j++ )
                {
                    if( quads[k].neighbors[j] )
                        continue;

                    dx = pt.x - quads[k].corners[j]->pt.x;
                    dy = pt.y - quads[k].corners[j]->pt.y;
                    dist = dx * dx + dy * dy;

                    if( dist < min_dist &&
                        dist <= cur_quad->edge_len*thresh_scale &&
                        dist <= quads[k].edge_len*thresh_scale )
                    {
                        closest_corner_idx = j;
                        closest_quad = &quads[k];
                        min_dist = dist;
                    }
                }
            }

            // we found a matching corner point?
            if( closest_corner_idx >= 0 && min_dist < FLT_MAX )
            {
                // If another point from our current quad is closer to the found corner
                // than the current one, then we don't count this one after all.
                // This is necessary to support small squares where otherwise the wrong
                // corner will get matched to closest_quad;
                closest_corner = closest_quad->corners[closest_corner_idx];

                for( j = 0; j < 4; j++ )
                {
                    if( cur_quad->neighbors[j] == closest_quad )
                        break;

                    dx = closest_corner->pt.x - cur_quad->corners[j]->pt.x;
                    dy = closest_corner->pt.y - cur_quad->corners[j]->pt.y;

                    if( dx * dx + dy * dy < min_dist )
                        break;
                }

                if( j < 4 || cur_quad->count >= 4 || closest_quad->count >= 4 )
                    continue;

                // Check that each corner is a neighbor of different quads
                for( j = 0; j < closest_quad->count; j++ )
                {
                    if( closest_quad->neighbors[j] == cur_quad )
                        break;
                }
                if( j < closest_quad->count )
                    continue;

                // check whether the closest corner to closest_corner
                // is different from cur_quad->corners[i]->pt
                for( k = 0; k < quad_count; k++ )
                {
                    CvCBQuad* q = &quads[k];
                    if( k == idx || q == closest_quad )
                        continue;

                    for( j = 0; j < 4; j++ )
                        if( !q->neighbors[j] )
                        {
                            dx = closest_corner->pt.x - q->corners[j]->pt.x;
                            dy = closest_corner->pt.y - q->corners[j]->pt.y;
                            dist = dx*dx + dy*dy;
                            if( dist < min_dist )
                                break;
                        }
                    if( j < 4 )
                        break;
                }

                if( k < quad_count )
                    continue;

                closest_corner->pt.x = (pt.x + closest_corner->pt.x) * 0.5f;
                closest_corner->pt.y = (pt.y + closest_corner->pt.y) * 0.5f;

                // We've found one more corner - remember it
                cur_quad->count++;
                cur_quad->neighbors[i] = closest_quad;
                cur_quad->corners[i] = closest_corner;

                closest_quad->count++;
                closest_quad->neighbors[closest_corner_idx] = cur_quad;
            }
        }
    }
}

#else // fixed point optimization
static void icvFindQuadNeighbors( CvCBQuad *quads, int quad_count )
{
    //const float thresh_scale = 1.f;
	const _iq10 thresh_scale = _FtoIQ10(1.f);
    int idx, i, k, j;
	//float dx, dy, dist;
	_iq10 dx,dy,dist;
	_iq10 iq10_max = _iq10(2097151);

    // find quad neighbors
    for( idx = 0; idx < quad_count; idx++ )
    {
        CvCBQuad* cur_quad = &quads[idx];

        // choose the points of the current quadrangle that are close to
        // some points of the other quadrangles
        // (it can happen for split corners (due to dilation) of the
        // checker board). Search only in other quadrangles!

        // for each corner of this quadrangle
        for( i = 0; i < 4; i++ )
        {
            CvPoint2D32f pt;
            //float min_dist = FLT_MAX;
			_iq10 min_dist = iq10_max;
            int closest_corner_idx = -1;
            CvCBQuad *closest_quad = 0;
            CvCBCorner *closest_corner = 0;

            if( cur_quad->neighbors[i] )
                continue;

            pt = cur_quad->corners[i]->pt;

            // find the closest corner in all other quadrangles
            for( k = 0; k < quad_count; k++ )
            {
                if( k == idx )
                    continue;

                for( j = 0; j < 4; j++ )
                {
                    if( quads[k].neighbors[j] )
                        continue;

                    //dx = pt.x - quads[k].corners[j]->pt.x;
                    //dy = pt.y - quads[k].corners[j]->pt.y;
                    //dist = dx * dx + dy * dy;
					dx = _FtoIQ10(pt.x) - _FtoIQ10(quads[k].corners[j]->pt.x);
                    dy = _FtoIQ10(pt.y) - _FtoIQ10(quads[k].corners[j]->pt.y);
                    dist = _IQ10mpy(dx , dx) + _IQ10mpy(dy , dy);


                    if( dist < min_dist &&
                        dist <= _IQ10mpy(_FtoIQ10(cur_quad->edge_len) , thresh_scale) &&
                        dist <= _IQ10mpy(_FtoIQ10(quads[k].edge_len) , thresh_scale) )
                    {
                        closest_corner_idx = j;
                        closest_quad = &quads[k];
                        min_dist = dist;
                    }
                }
            }

            // we found a matching corner point?
            if( closest_corner_idx >= 0 && min_dist < iq10_max )
            {
                // If another point from our current quad is closer to the found corner
                // than the current one, then we don't count this one after all.
                // This is necessary to support small squares where otherwise the wrong
                // corner will get matched to closest_quad;
                closest_corner = closest_quad->corners[closest_corner_idx];

                for( j = 0; j < 4; j++ )
                {
                    if( cur_quad->neighbors[j] == closest_quad )
                        break;

                    //dx = closest_corner->pt.x - cur_quad->corners[j]->pt.x;
                    //dy = closest_corner->pt.y - cur_quad->corners[j]->pt.y;
					dx = _FtoIQ10(closest_corner->pt.x) - _FtoIQ10(cur_quad->corners[j]->pt.x);
                    dy = _FtoIQ10(closest_corner->pt.y) - _FtoIQ10(cur_quad->corners[j]->pt.y);

                    if( _IQ10mpy(dx,dx) + _IQ10mpy(dy,dy)  < min_dist )
                        break;
                }

                if( j < 4 || cur_quad->count >= 4 || closest_quad->count >= 4 )
                    continue;

                // Check that each corner is a neighbor of different quads
                for( j = 0; j < closest_quad->count; j++ )
                {
                    if( closest_quad->neighbors[j] == cur_quad )
                        break;
                }
                if( j < closest_quad->count )
                    continue;

                // check whether the closest corner to closest_corner
                // is different from cur_quad->corners[i]->pt
                for( k = 0; k < quad_count; k++ )
                {
                    CvCBQuad* q = &quads[k];
                    if( k == idx || q == closest_quad )
                        continue;

                    for( j = 0; j < 4; j++ )
                        if( !q->neighbors[j] )
                        {
                            //dx = closest_corner->pt.x - q->corners[j]->pt.x;
                            //dy = closest_corner->pt.y - q->corners[j]->pt.y;
                            //dist = dx*dx + dy*dy;
							dx = _FtoIQ10(closest_corner->pt.x) - _FtoIQ10(q->corners[j]->pt.x);
                            dy = _FtoIQ10(closest_corner->pt.y) - _FtoIQ10(q->corners[j]->pt.y);
                            dist = _IQ10mpy(dx,dx) + _IQ10mpy(dy,dy);
                            if( dist < min_dist )
                                break;
                        }
                    if( j < 4 )
                        break;
                }

                if( k < quad_count )
                    continue;
                //closest_corner->pt.x = (pt.x + closest_corner->pt.x) * 0.5f;
                //closest_corner->pt.y = (pt.y + closest_corner->pt.y) * 0.5f;
                closest_corner->pt.x = _IQ10toF(_IQ10mpy((_FtoIQ10(pt.x) + _FtoIQ10(closest_corner->pt.x)) , _FtoIQ10(0.5f) ));
                closest_corner->pt.y = _IQ10toF(_IQ10mpy((_FtoIQ10(pt.y) + _FtoIQ10(closest_corner->pt.y)) , _FtoIQ10(0.5f) ));

                // We've found one more corner - remember it
                cur_quad->count++;
                cur_quad->neighbors[i] = closest_quad;
                cur_quad->corners[i] = closest_corner;

                closest_quad->count++;
                closest_quad->neighbors[closest_corner_idx] = cur_quad;
            }
        }
    }
}
#endif
//=====================================================================================
#if 1
static int
icvGenerateQuads( CvCBQuad **out_quads, CvCBCorner **out_corners,
                  CvMemStorage *storage, CvMat *image, int flags )
{
    int quad_count = 0;
    CvMemStorage *temp_storage = 0;

    if( out_quads )
        *out_quads = 0;

    if( out_corners )
        *out_corners = 0;

    CV_FUNCNAME( "icvGenerateQuads" );

    __BEGIN__;

    CvSeq *src_contour = 0;
    CvSeq *root;
    CvContourEx* board = 0;
    CvContourScanner scanner;
    int i, idx, min_size;

    CV_ASSERT( out_corners && out_quads );

    // empiric bound for minimal allowed perimeter for squares
    min_size = cvRound( image->cols * image->rows * .03 * 0.01 * 0.92 );//0.92

    // create temporary storage for contours and the sequence of pointers to found quadrangles
    CV_CALL( temp_storage = cvCreateChildMemStorage( storage ));
    CV_CALL( root = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvSeq*), temp_storage ));

    // initialize contour retrieving routine
    CV_CALL( scanner = cvStartFindContours( image, temp_storage, sizeof(CvContourEx),
                                            CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE ));

    // get all the contours one by one
    while( (src_contour = cvFindNextContour( scanner )) != 0 )
    {
        CvSeq *dst_contour = 0;
        CvRect rect = ((CvContour*)src_contour)->rect;

        // reject contours with too small perimeter
        if( CV_IS_SEQ_HOLE(src_contour) && rect.width*rect.height >= min_size )
        {
            const int min_approx_level = 2, max_approx_level = MAX_CONTOUR_APPROX;
            int approx_level;
            for( approx_level = min_approx_level; approx_level <= max_approx_level; approx_level++ )
            {
                dst_contour = cvApproxPoly( src_contour, sizeof(CvContour), temp_storage,
                                            CV_POLY_APPROX_DP, (float)approx_level );
                // we call this again on its own output, because sometimes
                // cvApproxPoly() does not simplify as much as it should.
                dst_contour = cvApproxPoly( dst_contour, sizeof(CvContour), temp_storage,
                                            CV_POLY_APPROX_DP, (float)approx_level );

                if( dst_contour->total == 4 )
                    break;
            }

            // reject non-quadrangles
            if( dst_contour->total == 4 && cvCheckContourConvexity(dst_contour) )
            {
                CvPoint pt[4];
                double d1, d2, p = cvContourPerimeter(dst_contour);
                double area = fabs(cvContourArea(dst_contour, CV_WHOLE_SEQ));
                double dx, dy;

                for( i = 0; i < 4; i++ )
                    pt[i] = *(CvPoint*)cvGetSeqElem(dst_contour, i);

                dx = pt[0].x - pt[2].x;
                dy = pt[0].y - pt[2].y;
                d1 = sqrt(dx*dx + dy*dy);

                dx = pt[1].x - pt[3].x;
                dy = pt[1].y - pt[3].y;
                d2 = sqrt(dx*dx + dy*dy);

                // philipg.  Only accept those quadrangles which are more square
                // than rectangular and which are big enough
                double d3, d4;
                dx = pt[0].x - pt[1].x;
                dy = pt[0].y - pt[1].y;
                d3 = sqrt(dx*dx + dy*dy);
                dx = pt[1].x - pt[2].x;
                dy = pt[1].y - pt[2].y;
                d4 = sqrt(dx*dx + dy*dy);
                if( !(flags & CV_CALIB_CB_FILTER_QUADS) ||
                    d3*4 > d4 && d4*4 > d3 && d3*d4 < area*1.5 && area > min_size &&
                    d1 >= 0.15 * p && d2 >= 0.15 * p )
                {
                    CvContourEx* parent = (CvContourEx*)(src_contour->v_prev);
                    parent->counter++;
                    if( !board || board->counter < parent->counter )
                        board = parent;
                    dst_contour->v_prev = (CvSeq*)parent;
                    //for( i = 0; i < 4; i++ ) cvLine( debug_img, pt[i], pt[(i+1)&3], cvScalar(200,255,255), 1, CV_AA, 0 );
                    cvSeqPush( root, &dst_contour );
                }
            }
        }
    }

    // finish contour retrieving
    cvEndFindContours( &scanner );

    // allocate quad & corner buffers
    CV_CALL( *out_quads = (CvCBQuad*)cvAlloc((root->total+6) * sizeof((*out_quads)[0])));
    CV_CALL( *out_corners = (CvCBCorner*)cvAlloc((root->total+6) * 4 * sizeof((*out_corners)[0])));
    //CV_CALL( *out_quads = (CvCBQuad*)malloc((root->total+6) * sizeof((*out_quads)[0])));
    //CV_CALL( *out_corners = (CvCBCorner*)malloc((root->total+6) * 4 * sizeof((*out_corners)[0])));

    // Create array of quads structures
    for( idx = 0; idx < root->total; idx++ )
    {
        CvCBQuad* q = &(*out_quads)[quad_count];
        src_contour = *(CvSeq**)cvGetSeqElem( root, idx );
        if( (flags & CV_CALIB_CB_FILTER_QUADS) && src_contour->v_prev != (CvSeq*)board )
            continue;

        // reset group ID
        memset( q, 0, sizeof(*q) );
        q->group_idx = -1;
        assert( src_contour->total == 4 );
        for( i = 0; i < 4; i++ )
        {
            CvPoint2D32f pt = cvPointTo32f(*(CvPoint*)cvGetSeqElem(src_contour, i));
            CvCBCorner* corner = &(*out_corners)[quad_count*4 + i];

            memset( corner, 0, sizeof(*corner) );
            corner->pt = pt;
            q->corners[i] = corner;
        }
        q->edge_len = FLT_MAX;
        for( i = 0; i < 4; i++ )
        {
            float dx = q->corners[i]->pt.x - q->corners[(i+1)&3]->pt.x;
            float dy = q->corners[i]->pt.y - q->corners[(i+1)&3]->pt.y;
            float d = dx*dx + dy*dy;
            if( q->edge_len > d )
                q->edge_len = d;
        }
        quad_count++;
    }

    __END__;

    if( cvGetErrStatus() < 0 )
    {
        if( out_quads )
            cvFree( out_quads );
        if( out_corners )
            cvFree( out_corners );
        quad_count = 0;
    }

    cvReleaseMemStorage( &temp_storage );
    return quad_count;
}
#else //fixed point optimization
static int
icvGenerateQuads( CvCBQuad **out_quads, CvCBCorner **out_corners,
                  CvMemStorage *storage, CvMat *image, int flags )
{
    int quad_count = 0;
    CvMemStorage *temp_storage = 0;

     if( out_quads )
        *out_quads = 0;

    if( out_corners )
        *out_corners = 0;

    CV_FUNCNAME( "icvGenerateQuads" );

    __BEGIN__;

    CvSeq *src_contour = 0;
    CvSeq *root;
    CvContourEx* board = 0;
    CvContourScanner scanner;
    int i, idx, min_size;

    CV_ASSERT( out_corners && out_quads );

    // empiric bound for minimal allowed perimeter for squares
    min_size = cvRound( image->cols * image->rows * .03 * 0.01 * 0.92 );//0.92

    // create temporary storage for contours and the sequence of pointers to found quadrangles
    CV_CALL( temp_storage = cvCreateChildMemStorage( storage ));
    CV_CALL( root = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvSeq*), temp_storage ));

    // initialize contour retrieving routine
    CV_CALL( scanner = cvStartFindContours( image, temp_storage, sizeof(CvContourEx),
                                            CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE ));

    // get all the contours one by one
    while( (src_contour = cvFindNextContour( scanner )) != 0 )
    {
        CvSeq *dst_contour = 0;
        CvRect rect = ((CvContour*)src_contour)->rect;

        // reject contours with too small perimeter
        if( CV_IS_SEQ_HOLE(src_contour) && rect.width*rect.height >= min_size )
        {
            const int min_approx_level = 2, max_approx_level = MAX_CONTOUR_APPROX;
            int approx_level;
            for( approx_level = min_approx_level; approx_level <= max_approx_level; approx_level++ )
            {
                dst_contour = cvApproxPoly( src_contour, sizeof(CvContour), temp_storage,
                                            CV_POLY_APPROX_DP, (float)approx_level );
                // we call this again on its own output, because sometimes
                // cvApproxPoly() does not simplify as much as it should.
                dst_contour = cvApproxPoly( dst_contour, sizeof(CvContour), temp_storage,
                                            CV_POLY_APPROX_DP, (float)approx_level );

                if( dst_contour->total == 4 )
                    break;
            }

            // reject non-quadrangles
            if( dst_contour->total == 4 && cvCheckContourConvexity(dst_contour) )
            {
                CvPoint pt[4];
                //double d1, d2, p = cvContourPerimeter(dst_contour);
                //double area = fabs(cvContourArea(dst_contour, CV_WHOLE_SEQ));
                //double dx, dy;
				_iq15 d1,d2,p = _FtoIQ15((double)cvContourPerimeter(dst_contour));
				_iq15 area = _FtoIQ15((double)fabs(cvContourArea(dst_contour, CV_WHOLE_SEQ)));
				_iq15 dx, dy;

                for( i = 0; i < 4; i++ )
                    pt[i] = *(CvPoint*)cvGetSeqElem(dst_contour, i);

                dx = _IQ15(pt[0].x) - _IQ15(pt[2].x);
                dy = _IQ15(pt[0].y) - _IQ15(pt[2].y);
                //d1 = sqrt(dx*dx + dy*dy);
				d1 = _IQ15sqrt(_IQ15mpy(dx,dx)+_IQ15mpy(dy,dy));

                dx = _IQ15(pt[1].x) - _IQ15(pt[3].x);
                dy = _IQ15(pt[1].y) - _IQ15(pt[3].y);
                //d2 = sqrt(dx*dx + dy*dy);
				d2 = _IQ15sqrt(_IQ15mpy(dx,dx)+_IQ15mpy(dy,dy));


                // philipg.  Only accept those quadrangles which are more square
                // than rectangular and which are big enough
                //double d3, d4;
				_iq15 d3,d4;
                dx = _IQ15(pt[0].x) - _IQ15(pt[1].x);
                dy = _IQ15(pt[0].y) - _IQ15(pt[1].y);
                //d3 = sqrt(dx*dx + dy*dy);
				d3 = _IQ15sqrt(_IQ15mpy(dx,dx)+_IQ15mpy(dy,dy));

                dx = _IQ15(pt[1].x) - _IQ15(pt[2].x);
                dy = _IQ15(pt[1].y) - _IQ15(pt[2].y);
                //d4 = sqrt(dx*dx + dy*dy);
				d4 = _IQ15sqrt(_IQ15mpy(dx,dx)+_IQ15mpy(dy,dy));
                //if( !(flags & CV_CALIB_CB_FILTER_QUADS) ||
                //    (d3*4 > d4 && d4*4 > d3 && d3*d4 < area*1.5 && area > min_size &&
                //    d1 >= 0.15 * p && d2 >= 0.15 * p) )
				if( !(flags & CV_CALIB_CB_FILTER_QUADS) ||
					(_IQ15mpy(d3,_IQ15(4)) > d4 
					&& _IQ15mpy(d4,_IQ15(4)) > d3
					&& _IQ15mpy(d3,d4) < _IQ15mpy(area,_IQ(1.5)) 
					&& _IQ15toF(area) > (float)min_size 
					&& d1 >= _IQ15mpy(_FtoIQ15(0.15),p) 
					&& d2 >= _IQ15mpy(_FtoIQ15(0.15),p) ) )
                {
                    CvContourEx* parent = (CvContourEx*)(src_contour->v_prev);
                    parent->counter++;
                    if( !board || board->counter < parent->counter )
                        board = parent;
                    dst_contour->v_prev = (CvSeq*)parent;
                    //for( i = 0; i < 4; i++ ) cvLine( debug_img, pt[i], pt[(i+1)&3], cvScalar(200,255,255), 1, CV_AA, 0 );
                    cvSeqPush( root, &dst_contour );
                }
            }
        }
    }

    // finish contour retrieving
    cvEndFindContours( &scanner );

    // allocate quad & corner buffers
#ifdef CV_ALLOC
    CV_CALL( *out_quads = (CvCBQuad*)cvAlloc(root->total * sizeof((*out_quads)[0])));
    CV_CALL( *out_corners = (CvCBCorner*)cvAlloc(root->total * 4 * sizeof((*out_corners)[0])));
#else
    CV_CALL( *out_quads = (CvCBQuad*)malloc((root->total+4) * sizeof((*out_quads)[0])));
    CV_CALL( *out_corners = (CvCBCorner*)malloc((root->total+4) * 4 * sizeof((*out_corners)[0])));
#endif

    // Create array of quads structures
    for( idx = 0; idx < root->total; idx++ )
    {
        CvCBQuad* q = &(*out_quads)[quad_count];
        src_contour = *(CvSeq**)cvGetSeqElem( root, idx );
        if( (flags & CV_CALIB_CB_FILTER_QUADS) && src_contour->v_prev != (CvSeq*)board )
            continue;

        // reset group ID
        memset( q, 0, sizeof(*q) );
        q->group_idx = -1;
        assert( src_contour->total == 4 );
        for( i = 0; i < 4; i++ )
        {
            CvPoint2D32f pt = cvPointTo32f(*(CvPoint*)cvGetSeqElem(src_contour, i));
            CvCBCorner* corner = &(*out_corners)[quad_count*4 + i];

            memset( corner, 0, sizeof(*corner) );
            corner->pt = pt;
            q->corners[i] = corner;
        }
        q->edge_len = FLT_MAX;
        for( i = 0; i < 4; i++ )
        {
            //float dx = q->corners[i]->pt.x - q->corners[(i+1)&3]->pt.x;
            //float dy = q->corners[i]->pt.y - q->corners[(i+1)&3]->pt.y;
            //float d = dx*dx + dy*dy;
			_iq15 dx = _FtoIQ15(q->corners[i]->pt.x) - _FtoIQ15(q->corners[(i+1)&3]->pt.x);
            _iq15 dy = _FtoIQ15(q->corners[i]->pt.y) - _FtoIQ15(q->corners[(i+1)&3]->pt.y);
            _iq15 d = _IQ15mpy(dx,dx) + _IQ15mpy(dy,dy);
            if( q->edge_len > _IQ15toF(d) )
                q->edge_len = _IQ15toF(d);
        }
        quad_count++;
    }

    __END__;

    if( cvGetErrStatus() < 0 )
    {
#ifdef CV_ALLOC
        if( out_quads )
            cvFree( &out_quads );
        if( out_corners )
            cvFree( &out_corners );
#else
        if( out_quads )
            free( out_quads );
        if( out_corners )
            free( out_corners );
#endif
        quad_count = 0;
    }

    cvReleaseMemStorage( &temp_storage );
    return quad_count;
}
#endif 

// 20141127 gbb,
/**********************************************************************
* [function]: repair the lost corners or lost quads
* 	--for lost quad corners: in condition that the neighbor quads are found 
*		but some quads corners are not merged
* 	--for lost quads: in condition that some quads are lost
* [algorithm]: 
*	--start from the quads with 3 neighbors: predict the forth corner and 
*		its neighbor quad. if the neighbor is not found, rebuild one quad 
*		and its corners.
*	--start searching the neighbors of the rebuild quad (after all 
*		3-corner-quad found), merge the corners near by and rebuild the 
*		relationship of neighbors.
**********************************************************************/
static int repairQuads(CvCBQuad **quad_group, int quad_count, int corner_count,
						CvCBCorner *quad_corners,CvCBQuad *quads,
						int org_quad_count,CvSize pattern_size)
{
	int i,ii,j,k;
	int q_count = quad_count,c_count = corner_count,lost_count =0,
		full_quad_count = org_quad_count;

	CvPoint2D32f *quad_centers = 0;
	printf("repairQuads***\n");
#ifdef CV_ALLOC
	quad_centers = (CvPoint2D32f *)cvAlloc( sizeof(quad_centers[0])*(quad_count+12) );
#else
	quad_centers = (CvPoint2D32f *)malloc( sizeof(quad_centers[0])*(quad_count+12) );
#endif

check_count:	lost_count = pattern_size.width * pattern_size.height -abs(c_count);  
	//printf("lost_count : %d\n",lost_count);

	if (lost_count ==0)
	{
		printf("all corners found!\n");
		// clear all the flag of the corners in quad group
		for( i = 0; i < q_count; i++ )
	    {
			for(j = 0; j<4; j++)
			{
				quad_group[i]->corners[j]->row =0;
				quad_group[i]->corners[j]->count = 0;
				for (k = 0; k<4; k++)
				{
					quad_group[i]->corners[j]->neighbors[k] = NULL;
				}
			}
	    }
#ifdef CV_ALLOC
		cvFree( &quad_centers);
#else
		free( quad_centers);
#endif
		return q_count;
	}
	else if (lost_count <0)
	{
#ifdef CV_ALLOC
		cvFree( &quad_centers);
#else
		free( quad_centers);
#endif 
		return quad_count;
	}

	for( i = 0; i < q_count; i++ )
    {
		CvPoint2D32f ci = {0,0};
        CvCBQuad* q = quad_group[i];
        for( j = 0; j < 4; j++ )
        {
            CvPoint2D32f pt = q->corners[j]->pt;
            ci.x += pt.x;
            ci.y += pt.y;
        }
        ci.x *= 0.25f;
        ci.y *= 0.25f;
        quad_centers[i] = ci;
    }

	for(i= 0; i < q_count; i++)
	{
		if(quad_group[i]->count ==3) 
		{
			// *********************************************************
			// step1:find the 3-neighbor quads with 4-neighbor neighbor
			// 		 and predict the 4th corner. 
			// **********************************************************
			// find the lost neighbor index of 3-neighbor quad
			CvCBCorner* lostCorner = NULL;
			CvPoint2D32f center,guess_center;
			int found_quad_idx = -1,found_corner_idx = -1,found_neighbor_idx = -1,
				lost_neighbor_idx = -1,lost_corner_idx= -1;
			float min_dist = FLT_MAX, thresh_dist = 0.;				
			//CvPoint2D32f corner_neighbor0, corner_neighbor1,corner_diag;
			int t = 0,connect4_corner_idx =-1;
			float max_dist = FLT_MIN;
			
			for (j=0; j<4; j++)
			{
				if(quad_group[i]->neighbors[j] == NULL)
				{
					lost_neighbor_idx = j;
					lostCorner = quad_group[i]->corners[j];
				}
			}
			c_count+=1;
			lost_corner_idx = lost_neighbor_idx;

			// refine the corner predict by symmetry of the quad
			// start with the 3-neighbor quad that has a neighbor quad of 4 neighbors			
			for(j = 0; j<4; j++)
			{
				if(j !=lost_neighbor_idx)
				{
				if(quad_group[i]->neighbors[j]->count == 4) 
				{
					connect4_corner_idx = j;
					break;
				}
				}				
			}
			if (connect4_corner_idx == -1)
				continue;

			// refine the 4th corner of the 3-neighbor quad
			CvCBQuad* quad_4neighbors = quad_group[i]->neighbors[connect4_corner_idx];
			CvCBQuad* quad_3neighbors = quad_group[i];
			predictCorner(quad_4neighbors,quad_3neighbors,lost_corner_idx,connect4_corner_idx);
			quad_group[i]->count +=1;

			center.x = (quad_3neighbors->corners[0]->pt.x + quad_3neighbors->corners[1]->pt.x +
						quad_3neighbors->corners[2]->pt.x + quad_3neighbors->corners[3]->pt.x)*0.25;
			center.y = (quad_3neighbors->corners[0]->pt.y + quad_3neighbors->corners[1]->pt.y +
						quad_3neighbors->corners[2]->pt.y + quad_3neighbors->corners[3]->pt.y)*0.25;			

			
			// *********************************************************
			// step2: find the lost neighbor connect with the lost corner
			// **********************************************************
			
			guess_center.x = 2*lostCorner->pt.x - center.x;// guess the lost neighbor center
			guess_center.y = 2*lostCorner->pt.y - center.y;

			min_dist = FLT_MAX;
			for( ii = 0; ii < q_count; ii++ ) // find the nearest quad center
			{
				if (ii == i)
					continue;
				float dist = sqrt( pow(guess_center.x -quad_centers[ii].x,2) + pow(guess_center.y -quad_centers[ii].y,2));
				if(min_dist > dist)
				{
					min_dist = dist;
					found_quad_idx = ii;
				}				
			}
			
			thresh_dist = sqrt( pow(guess_center.x -lostCorner->pt.x,2) + pow(guess_center.y -lostCorner->pt.y,2));

			if(min_dist > thresh_dist )// neighbor quad not exit, create a virtual quads
			{
				//create a virtual quad and push into the quad group
				//printf("neighbor quad not exist! ADD ONE!\n");
				//return q_count;
				
				//
				quad_group[q_count] = &(quads[full_quad_count]); // use the extended quads;
				quad_group[q_count]->count = 0;
				quad_group[q_count]->group_idx = quad_group[q_count-1]->group_idx;
				quad_group[q_count]->edge_len = min_dist*min_dist*0.5;
				for(k = 0; k<4; k++)
				{
					quad_group[q_count]->corners[k] = &(quad_corners[4*full_quad_count+k]);
					quad_group[q_count]->neighbors[k] = NULL;
				}
				// predict the virtual quad by the near by corners.
				/**************************
				* 
				***************************/
				float L0 = sqrt( pow(quad_group[i]->corners[(lost_corner_idx+0)&0x3]->pt.x -
								 	 quad_group[i]->corners[(lost_corner_idx+1)&0x3]->pt.x,2) 
							   + pow(quad_group[i]->corners[(lost_corner_idx+0)&0x3]->pt.y -
								 	 quad_group[i]->corners[(lost_corner_idx+1)&0x3]->pt.y,2) ),
					  L1 = sqrt( pow(quad_group[i]->corners[(lost_corner_idx+1)&0x3]->pt.x -
								 	 quad_group[i]->corners[(lost_corner_idx+2)&0x3]->pt.x,2) 
							   + pow(quad_group[i]->corners[(lost_corner_idx+1)&0x3]->pt.y -
								 	 quad_group[i]->corners[(lost_corner_idx+2)&0x3]->pt.y,2) ),
					  L2 = sqrt( pow(quad_group[i]->corners[(lost_corner_idx+2)&0x3]->pt.x -
								 	 quad_group[i]->corners[(lost_corner_idx+3)&0x3]->pt.x,2) 
							   + pow(quad_group[i]->corners[(lost_corner_idx+2)&0x3]->pt.y -
								 	 quad_group[i]->corners[(lost_corner_idx+3)&0x3]->pt.y,2) ),
					  L3 = sqrt( pow(quad_group[i]->corners[(lost_corner_idx+3)&0x3]->pt.x -
								 	 quad_group[i]->corners[(lost_corner_idx+0)&0x3]->pt.x,2) 
							   + pow(quad_group[i]->corners[(lost_corner_idx+3)&0x3]->pt.y -
								 	 quad_group[i]->corners[(lost_corner_idx+0)&0x3]->pt.y,2) );
				quad_group[q_count]->corners[0] = lostCorner;
				quad_group[q_count]->corners[1]->pt.x = lostCorner->pt.x + (L3/L1) * (lostCorner->pt.x 
														- quad_group[i]->corners[(lost_corner_idx+1)&0x3]->pt.x);
				quad_group[q_count]->corners[1]->pt.y = lostCorner->pt.y + (L3/L1) * (lostCorner->pt.y 
														- quad_group[i]->corners[(lost_corner_idx+1)&0x3]->pt.y);
				quad_group[q_count]->corners[3]->pt.x = lostCorner->pt.x + (L0/L2) * (lostCorner->pt.x 
														- quad_group[i]->corners[(lost_corner_idx+3)&0x3]->pt.x);
				quad_group[q_count]->corners[3]->pt.y = lostCorner->pt.y + (L0/L2) * (lostCorner->pt.y 
														- quad_group[i]->corners[(lost_corner_idx+3)&0x3]->pt.y);
				//quad_group[q_count]->corners[1]->pt.y = lostCorner->pt.y - quad_group[i]->corners[(lost_corner_idx+1)&0x3]->pt.y;
				//quad_group[q_count]->corners[3]->pt.x = lostCorner->pt.x - quad_group[i]->corners[(lost_corner_idx+3)&0x3]->pt.x;
				//quad_group[q_count]->corners[3]->pt.y = lostCorner->pt.y - quad_group[i]->corners[(lost_corner_idx+3)&0x3]->pt.y;
				predictCorner(quad_group[i],quad_group[q_count],2,0);
				//quad_group[q_count]->corners[2]->pt.x = 2*guess_center.x  - lostCorner->pt.x;
				//quad_group[q_count]->corners[2]->pt.y = 2*guess_center.y  - lostCorner->pt.y;
				found_quad_idx = q_count;
				found_corner_idx = 0;
				q_count+=1;
				full_quad_count +=1;
				
				//
			}
			else// neighbor quad exist,
			{
				float least_dist = FLT_MAX;
				CvPoint2D32f pts ={0,0} ;
				CvPoint2D32f pts_lost_corner = quad_group[i]->corners[lost_corner_idx]->pt;

				//printf("neighbor exit!\n");
				if(quad_group[found_quad_idx]->count ==4)
				{
					//printf("ERROR!!!!neighbors :%d\n",quad_group[found_quad_idx]->count);
#ifdef CV_ALLOC
					cvFree( &quad_centers);
#else
					free( quad_centers);
#endif
					return quad_count;
				}

				for(j = 0; j<4; j++)//find the nearest corner of the neighbor. update it.
				{
					float temp_dist;
					if(quad_group[found_quad_idx]->neighbors[j])
						continue;
					pts = quad_group[found_quad_idx]->corners[j]->pt;
					temp_dist = sqrt( pow(pts_lost_corner.x -pts.x,2) + pow(pts_lost_corner.y -pts.y,2));
					if(least_dist > temp_dist)
					{
						least_dist = temp_dist;
						found_corner_idx = j;
					}					
				}
			}

			quad_group[i]->neighbors[lost_neighbor_idx] = quad_group[found_quad_idx];
			//quad_group[i]->count +=1;
			quad_group[found_quad_idx]->corners[found_corner_idx] = lostCorner; // combine the seperated corners.
			quad_group[found_quad_idx]->neighbors[found_corner_idx] = quad_group[i];
			quad_group[found_quad_idx]->count+=1;

			// if the neighbor quad has 3-neighbors, rebuild the 4th corner
			if(quad_group[found_quad_idx]->count == 3)
			{
				lost_corner_idx = -1;
				for(j = 0; j<4; j++)
				{
					if (quad_group[found_quad_idx]->neighbors[j] == NULL)
					{
						lost_corner_idx = j;
						break;
					}
				}			
				predictCorner(quad_group[i],quad_group[found_quad_idx],lost_corner_idx,found_corner_idx);
			}

			// for the rebuilded quad, go on searching the neighbors
			if(q_count >found_quad_idx )
			{
				CvPoint2D32f guess_center2 = {0,0},guess_corner = {0,0};
				int found_quad_idx2 =0,found_corner_idx2 =0;
				float least_dist = 0.;
				CvPoint2D32f new_quad_center = {0,0};
				for(j = 0; j<4; j++)
				{
					new_quad_center.x += quad_group[found_quad_idx]->corners[j]->pt.x;
					new_quad_center.y += quad_group[found_quad_idx]->corners[j]->pt.y;
				}
				new_quad_center.x = new_quad_center.x * 0.25;
				new_quad_center.y = new_quad_center.y * 0.25;

				for(j = 0; j<4; j++)
				{
					if(quad_group[found_quad_idx]->neighbors[j])
						continue;
					
					guess_center2.x = 2*quad_group[found_quad_idx]->corners[j]->pt.x - new_quad_center.x;// guess the lost neighbor center
					guess_center2.y = 2*quad_group[found_quad_idx]->corners[j]->pt.y - new_quad_center.y;
					
					min_dist = FLT_MAX;
					for( ii = 0; ii < q_count-1; ii++ ) // find the nearest quad center, in the old centers data base;
					{
						if (ii == i)
							continue;
						float dist = sqrt( pow(guess_center2.x -quad_centers[ii].x,2) + 
											pow(guess_center2.y -quad_centers[ii].y,2));
						if(min_dist > dist)
						{
							min_dist = dist;
							found_quad_idx2 = ii;
						}				
					}					
					thresh_dist = 1.5*sqrt( pow(new_quad_center.x -lostCorner->pt.x,2) + 
											pow(new_quad_center.y -lostCorner->pt.y,2));

					if(min_dist < thresh_dist) // neighbor exist
					{
						if(quad_group[found_quad_idx2]->count ==4)
						{
							printf("ERROR!!!!neighbors :%d\n",quad_group[found_quad_idx]->count);
#ifdef CV_ALLOC
							cvFree( &quad_centers);
#else
							free( quad_centers);
#endif
							return quad_count;
						}
						guess_corner.x = (guess_center2.x + new_quad_center.x)*0.5;
						guess_corner.y = (guess_center2.y + new_quad_center.y)*0.5;
						least_dist = FLT_MAX;
						for(k = 0; k<4; k++)//find the nearest corner of the neighbor. update it.
						{
							float temp_dist;
							if(quad_group[found_quad_idx2]->neighbors[k])
								continue;
							temp_dist = sqrt( pow(quad_group[found_quad_idx2]->corners[k]->pt.x -guess_corner.x,2) + 
												pow(quad_group[found_quad_idx2]->corners[k]->pt.y -guess_corner.y,2));
							if(least_dist > temp_dist)
							{
								least_dist = temp_dist;
								found_corner_idx2 = k;
							}					
						}
						quad_group[found_quad_idx2]->corners[found_corner_idx2]->pt = guess_corner;
						quad_group[found_quad_idx2]->neighbors[found_corner_idx2] = quad_group[found_quad_idx];
						quad_group[found_quad_idx]->neighbors[j] = quad_group[found_quad_idx2];
						quad_group[found_quad_idx]->corners[j] = quad_group[found_quad_idx2]->corners[found_corner_idx2];	
						quad_group[found_quad_idx2]->count +=1;
						quad_group[found_quad_idx]->count +=1;
						c_count +=1;											
					}
				}							
			}

			goto check_count;
		}
	}
	return quad_count;
}


int predictCorner(CvCBQuad *quad_4neighbors,CvCBQuad *quad_3neighbors,int lost_corner_idx,int connect4_corner_idx )
{
	// find the cross center for the 4-neighbors quad
	int i;
	int connect3_corner_idx = -1,new_corner_idx = -1;
	CvCBCorner *corner0,*corner1,*corner2,*corner3;
	CvPoint2D32f pos_cross3;

	//printf("lost_corner_idx: %d connect4_corner_idx: %d\n",lost_corner_idx,connect4_corner_idx);
	// find the idx of the connected corner in 4-neighbors quad
	for(i = 0; i<4; i++)
	{
		if(quad_3neighbors->corners[connect4_corner_idx] == quad_4neighbors->corners[i])
		{
			connect3_corner_idx = i;
			//printf("connect3_corner_idx : %d\n",connect3_corner_idx);
			break;
		}
	}
	corner0 = quad_4neighbors->corners[connect3_corner_idx];
	corner1 = quad_4neighbors->corners[((connect3_corner_idx+1)&0x3)];
	corner2 = quad_4neighbors->corners[((connect3_corner_idx+2)&0x3)];
	corner3 = quad_4neighbors->corners[((connect3_corner_idx+3)&0x3)];
	//printf("corner0 :[%f %f]\n",corner0->pt.x,corner0->pt.y);
	//printf("corner1 :[%f %f]\n",corner1->pt.x,corner1->pt.y);
	//printf("corner2 :[%f %f]\n",corner2->pt.x,corner2->pt.y);
	//printf("corner3 :[%f %f]\n",corner3->pt.x,corner3->pt.y);

	/*******************************************
	* line1 of corner1 and corner3: y1 = k1*x1 +b1
	* we have vector Len_3x = k * Len_13
	* line2 of corner0 and corner2: y2 = k2*x2 +b2
	* we have vector Len_0x = q * Len_02
	* then we get the cross center pos_cross4;
	********************************************/
	double _q[2],_a[4],_b[2];
	CvMat Q = cvMat( 2, 1, CV_64F, _q ),
		A = cvMat(2,2,CV_64F,_a),
		B = cvMat(2,1,CV_64F,_b);
	_a[0] = corner0->pt.x - corner2->pt.x;
	_a[1] = corner1->pt.x - corner3->pt.x;
	_a[2] = corner0->pt.y - corner2->pt.y;
	_a[3] = corner1->pt.y - corner3->pt.y;
	_b[0] = corner0->pt.x - corner3->pt.x;
	_b[1] = corner0->pt.y - corner3->pt.y;

	cvSolve (&A,&B,&Q,CV_SVD);
	//LOG_printf(&trace,"q0 q1: [%f %f]\n",_q[0],_q[1]);

	// switch the corner1-corner3 to 3-corners quad.
	corner1 = quad_3neighbors->corners[((connect4_corner_idx+1)&0x3)];
	corner2 = quad_3neighbors->corners[((connect4_corner_idx+2)&0x3)];	
	corner3 = quad_3neighbors->corners[((connect4_corner_idx+3)&0x3)];
	//printf("corner0 :[%f %f]\n",corner0->pt.x,corner0->pt.y);
	//printf("corner1 :[%f %f]\n",corner1->pt.x,corner1->pt.y);
	//printf("corner2 :[%f %f]\n",corner2->pt.x,corner2->pt.y);
	//printf("corner3 :[%f %f]\n",corner3->pt.x,corner3->pt.y); 
	for(i = 0; i<4; i++)
	{
		if(quad_3neighbors->corners[lost_corner_idx] == 
			quad_3neighbors->corners[((connect4_corner_idx+i)&0x3)])
		{
			new_corner_idx = i;
		}
	}
	if (new_corner_idx ==2)
	{
		pos_cross3.x = _q[1] * (corner3->pt.x - corner1->pt.x) + corner1->pt.x ;
		pos_cross3.y = _q[1] * (corner3->pt.y - corner1->pt.y) + corner1->pt.y ;
		corner2->pt.x = -(_q[0]/(1-_q[0])) * (corner0->pt.x - pos_cross3.x) + pos_cross3.x;
		corner2->pt.y = -(_q[0]/(1-_q[0])) * (corner0->pt.y - pos_cross3.y) + pos_cross3.y;
	}
	else if(new_corner_idx == 1)
	{
		pos_cross3.x = _q[0] * (corner0->pt.x - corner2->pt.x) + corner2->pt.x ;
		pos_cross3.y = _q[0] * (corner0->pt.y - corner2->pt.y) + corner2->pt.y ;
		corner1->pt.x = -(_q[1]/(1-_q[1])) * (corner3->pt.x - pos_cross3.x) + pos_cross3.x;
		corner1->pt.y = -(_q[1]/(1-_q[1])) * (corner3->pt.y - pos_cross3.y) + pos_cross3.y;
	}
	else if (new_corner_idx == 3)
	{
		pos_cross3.x = _q[0] * (corner0->pt.x - corner2->pt.x) + corner2->pt.x ;
		pos_cross3.y = _q[0] * (corner0->pt.y - corner2->pt.y) + corner2->pt.y ;
		corner3->pt.x = -((1-_q[1])/_q[1]) * (corner1->pt.x - pos_cross3.x) + pos_cross3.x;
		corner3->pt.y = -((1-_q[1])/_q[1]) * (corner1->pt.y - pos_cross3.y) + pos_cross3.y;
	}
	else
	{
		return -1;
	}
	return 0;
}

#if 0
void
cvUnDistortMap(CvMat* undistort_Points, CvSize size,
                     const double* intrinsic_matrix)
{	
	//gbb: add zoom para.
	_iq zoom = _FtoIQ(1.2f);
    int u, v;
    _iq u0 = _FtoIQ(intrinsic_matrix[2]), v0 = _FtoIQ(intrinsic_matrix[5]);
    _iq fx = _FtoIQ(intrinsic_matrix[0]), fy = _FtoIQ(intrinsic_matrix[4]);
    _iq _fx = _IQdiv(_FtoIQ(1.f),fx), _fy = _IQdiv(_FtoIQ(1.f),fy);
	CvPoint2D64f* dst = (CvPoint2D64f*)undistort_Points->data.db;

	for( v = 0; v < size.height; v++ )
    {
        _iq y = _IQmpy(_IQmpy(zoom,(_IQ(v) - v0)),_fy);
        _iq y2 = _IQmpy(y,y);		
        for( u = 0; u < size.width; u++ )
        {
			_iq x = _IQmpy(_IQmpy(zoom,(_IQ(u) - u0)),_fx);
            _iq x2 = _IQmpy(x,x);
			_iq r = _IQsqrt(x2 + y2);
			_iq theta = _IQatan(r);
			_iq scaling = _IQdiv(theta,r);
			_iq _u = _IQmpy(_IQmpy(fx,x),scaling) + u0;
			_iq _v = _IQmpy(_IQmpy(fy,y),scaling) + v0;
			
			dst->x = _IQtoF(_u);
			dst->y = _IQtoF(_v);
			dst++;
		}
	}

}
#endif

/* End of file. */
