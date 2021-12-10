/*********************************************************************************
* $)A0fH(KySP (C) ::;*025@9I7]SPO^TpHN9+K>
* 
* $)AND<~C{3F#: cvrectity.cpp
* $)AND<~1jJ6#: 
* $)ADZH]U*R*#: M(9}5XCfFLIh5DFeEL8q#,UR5==G5c#,;q5CMb2N#,Wn:sM(9}1d;;#,;q5C
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

//#include "_cv.h"
//#define PI 3.1415926f
#include "_cv.h"
#include "cv.h"
#include "cvrectify.h"
#include "stdio.h"

#define Uint8 unsigned char

#define MATLAB_DISTORTION

/*
    This is stright forward port v2 of Matlab calibration engine by Jean-Yves Bouguet
    that is (in a large extent) based on the paper:
    Z. Zhang. "A flexible new technique for camera calibration".
    IEEE Transactions on Pattern Analysis and Machine Intelligence, 22(11):1330-1334, 2000.

    The 1st initial port was done by Valery Mosyagin.
*/


CV_IMPL void
cvProjectPoints_fisheye( const CvMat* obj_points,
                  const CvMat* r_vec,
                  const CvMat* t_vec,
                  const CvMat* A,
                  const CvMat* dist_coeffs,
                  CvMat* img_points, CvMat* dpdr,
                  CvMat* dpdt, CvMat* dpdf,
                  CvMat* dpdc, CvMat* dpdk )
{
    CvMat *_M = 0, *_m = 0;
    CvMat *_dpdr = 0, *_dpdt = 0, *_dpdc = 0, *_dpdf = 0, *_dpdk = 0;
    
    CV_FUNCNAME( "cvProjectPoints_fisheye" );

    __BEGIN__;

    int i, j, count;
    int calc_derivatives;
    const CvPoint3D64f* M;
    CvPoint2D64f* m;
    double r[3], R[9], dRdr[27], t[3], a[9], k[4] = {0,0,0,0}, fx, fy, cx, cy;
    CvMat _r, _t, _a = cvMat( 3, 3, CV_64F, a ), _k;
    CvMat _R = cvMat( 3, 3, CV_64F, R ), _dRdr = cvMat( 3, 9, CV_64F, dRdr );
    double *dpdr_p = 0, *dpdt_p = 0, *dpdk_p = 0, *dpdf_p = 0, *dpdc_p = 0;
    int dpdr_step = 0, dpdt_step = 0, dpdk_step = 0, dpdf_step = 0, dpdc_step = 0;

    if( !CV_IS_MAT(obj_points) || !CV_IS_MAT(r_vec) ||
        !CV_IS_MAT(t_vec) || !CV_IS_MAT(A) ||
        /*!CV_IS_MAT(dist_coeffs) ||*/ !CV_IS_MAT(img_points) )
        CV_ERROR( CV_StsBadArg, "One of required arguments is not a valid matrix" );

    count = MAX(obj_points->rows, obj_points->cols);

    if( CV_IS_CONT_MAT(obj_points->type) && CV_MAT_DEPTH(obj_points->type) == CV_64F &&
        (obj_points->rows == 1 && CV_MAT_CN(obj_points->type) == 3 ||
        obj_points->rows == count && CV_MAT_CN(obj_points->type)*obj_points->cols == 3))
        _M = (CvMat*)obj_points;
    else
    {
        CV_CALL( _M = cvCreateMat( 1, count, CV_64FC3 ));
        CV_CALL( cvConvertPointsHomogenious( obj_points, _M ));
    }

    if( CV_IS_CONT_MAT(img_points->type) && CV_MAT_DEPTH(img_points->type) == CV_64F &&
        (img_points->rows == 1 && CV_MAT_CN(img_points->type) == 2 ||
        img_points->rows == count && CV_MAT_CN(img_points->type)*img_points->cols == 2))
        _m = img_points;
    else
        CV_CALL( _m = cvCreateMat( 1, count, CV_64FC2 ));

    M = (CvPoint3D64f*)_M->data.db;
    m = (CvPoint2D64f*)_m->data.db;

    if( CV_MAT_DEPTH(r_vec->type) != CV_64F && CV_MAT_DEPTH(r_vec->type) != CV_32F ||
        (r_vec->rows != 1 && r_vec->cols != 1 ||
        r_vec->rows*r_vec->cols*CV_MAT_CN(r_vec->type) != 3) &&
        (r_vec->rows != 3 && r_vec->cols != 3 || CV_MAT_CN(r_vec->type) != 1))
        CV_ERROR( CV_StsBadArg, "Rotation must be represented by 1x3 or 3x1 "
                  "floating-point rotation vector, or 3x3 rotation matrix" );

    if( r_vec->rows == 3 && r_vec->cols == 3 )
    {
        _r = cvMat( 3, 1, CV_64FC1, r );
        CV_CALL( cvRodrigues2( r_vec, &_r ));
        CV_CALL( cvRodrigues2( &_r, &_R, &_dRdr ));
        cvCopy( r_vec, &_R );
    }
    else
    {
        _r = cvMat( r_vec->rows, r_vec->cols, CV_MAKETYPE(CV_64F,CV_MAT_CN(r_vec->type)), r );
        CV_CALL( cvConvert( r_vec, &_r ));
        CV_CALL( cvRodrigues2( &_r, &_R, &_dRdr ) );
    }

    if( CV_MAT_DEPTH(t_vec->type) != CV_64F && CV_MAT_DEPTH(t_vec->type) != CV_32F ||
        t_vec->rows != 1 && t_vec->cols != 1 ||
        t_vec->rows*t_vec->cols*CV_MAT_CN(t_vec->type) != 3 )
        CV_ERROR( CV_StsBadArg,
            "Translation vector must be 1x3 or 3x1 floating-point vector" );

    _t = cvMat( t_vec->rows, t_vec->cols, CV_MAKETYPE(CV_64F,CV_MAT_CN(t_vec->type)), t );
    CV_CALL( cvConvert( t_vec, &_t ));

    if( CV_MAT_TYPE(A->type) != CV_64FC1 && CV_MAT_TYPE(A->type) != CV_32FC1 ||
        A->rows != 3 || A->cols != 3 )
        CV_ERROR( CV_StsBadArg, "Instrinsic parameters must be 3x3 floating-point matrix" );

    CV_CALL( cvConvert( A, &_a ));
    fx = a[0]; fy = a[4];
    cx = a[2]; cy = a[5];

    if( dist_coeffs )
    {
        if( !CV_IS_MAT(dist_coeffs) ||
            CV_MAT_DEPTH(dist_coeffs->type) != CV_64F &&
            CV_MAT_DEPTH(dist_coeffs->type) != CV_32F ||
            dist_coeffs->rows != 1 && dist_coeffs->cols != 1 ||
            dist_coeffs->rows*dist_coeffs->cols*CV_MAT_CN(dist_coeffs->type) != 4 )
            CV_ERROR( CV_StsBadArg,
                "Distortion coefficients must be 1x4 or 4x1 floating-point vector" );

        _k = cvMat( dist_coeffs->rows, dist_coeffs->cols,
                    CV_MAKETYPE(CV_64F,CV_MAT_CN(dist_coeffs->type)), k );
        CV_CALL( cvConvert( dist_coeffs, &_k ));
    }
    
    if( dpdr )
    {
        if( !CV_IS_MAT(dpdr) ||
            CV_MAT_TYPE(dpdr->type) != CV_32FC1 &&
            CV_MAT_TYPE(dpdr->type) != CV_64FC1 ||
            dpdr->rows != count*2 || dpdr->cols != 3 )
            CV_ERROR( CV_StsBadArg, "dp/drot must be 2Nx3 floating-point matrix" );

        if( CV_MAT_TYPE(dpdr->type) == CV_64FC1 )
            _dpdr = dpdr;
        else
            CV_CALL( _dpdr = cvCreateMat( 2*count, 3, CV_64FC1 ));
        dpdr_p = _dpdr->data.db;
        dpdr_step = _dpdr->step/sizeof(dpdr_p[0]);
    }

    if( dpdt )
    {
        if( !CV_IS_MAT(dpdt) ||
            CV_MAT_TYPE(dpdt->type) != CV_32FC1 &&
            CV_MAT_TYPE(dpdt->type) != CV_64FC1 ||
            dpdt->rows != count*2 || dpdt->cols != 3 )
            CV_ERROR( CV_StsBadArg, "dp/dT must be 2Nx3 floating-point matrix" );

        if( CV_MAT_TYPE(dpdt->type) == CV_64FC1 )
            _dpdt = dpdt;
        else
            CV_CALL( _dpdt = cvCreateMat( 2*count, 3, CV_64FC1 ));
        dpdt_p = _dpdt->data.db;
        dpdt_step = _dpdt->step/sizeof(dpdt_p[0]);
    }

    if( dpdf )
    {
        if( !CV_IS_MAT(dpdf) ||
            CV_MAT_TYPE(dpdf->type) != CV_32FC1 && CV_MAT_TYPE(dpdf->type) != CV_64FC1 ||
            dpdf->rows != count*2 || dpdf->cols != 2 )
            CV_ERROR( CV_StsBadArg, "dp/df must be 2Nx2 floating-point matrix" );

        if( CV_MAT_TYPE(dpdf->type) == CV_64FC1 )
            _dpdf = dpdf;
        else
            CV_CALL( _dpdf = cvCreateMat( 2*count, 2, CV_64FC1 ));
        dpdf_p = _dpdf->data.db;
        dpdf_step = _dpdf->step/sizeof(dpdf_p[0]);
    }

    if( dpdc )
    {
        if( !CV_IS_MAT(dpdc) ||
            CV_MAT_TYPE(dpdc->type) != CV_32FC1 && CV_MAT_TYPE(dpdc->type) != CV_64FC1 ||
            dpdc->rows != count*2 || dpdc->cols != 2 )
            CV_ERROR( CV_StsBadArg, "dp/dc must be 2Nx2 floating-point matrix" );

        if( CV_MAT_TYPE(dpdc->type) == CV_64FC1 )
            _dpdc = dpdc;
        else
            CV_CALL( _dpdc = cvCreateMat( 2*count, 2, CV_64FC1 ));
        dpdc_p = _dpdc->data.db;
        dpdc_step = _dpdc->step/sizeof(dpdc_p[0]);
    }

    if( dpdk )
    {
        if( !CV_IS_MAT(dpdk) ||
            CV_MAT_TYPE(dpdk->type) != CV_32FC1 && CV_MAT_TYPE(dpdk->type) != CV_64FC1 ||
            dpdk->rows != count*2 || (dpdk->cols != 4 && dpdk->cols != 2) )
            CV_ERROR( CV_StsBadArg, "dp/df must be 2Nx4 or 2Nx2 floating-point matrix" );

        if( !dist_coeffs )
            CV_ERROR( CV_StsNullPtr, "dist_coeffs is NULL while dpdk is not" );

        if( CV_MAT_TYPE(dpdk->type) == CV_64FC1 )
            _dpdk = dpdk;
        else
            CV_CALL( _dpdk = cvCreateMat( dpdk->rows, dpdk->cols, CV_64FC1 ));
        dpdk_p = _dpdk->data.db;
        dpdk_step = _dpdk->step/sizeof(dpdk_p[0]);
    }

    calc_derivatives = dpdr || dpdt || dpdf || dpdc || dpdk;

    for( i = 0; i < count; i++ )
    {
        double X = M[i].x, Y = M[i].y, Z = M[i].z;
        double x = R[0]*X + R[1]*Y + R[2]*Z + t[0];
        double y = R[3]*X + R[4]*Y + R[5]*Z + t[1];
        double z = R[6]*X + R[7]*Y + R[8]*Z + t[2];
        double r2, r0;
        double xd, yd;

		if(Z == -10000)
		{
			m[i].x = -10000;
	        m[i].y = -10000;
			continue;
		}

        z = z ? 1./z : 1;
        x *= z; y *= z;


		double x2 = x*x;
		double y2 = y*y;
		r2 = x2 + y2;
		r0 = sqrt(r2);
		
		//a1 = 2*x*y;
        //a2 = r2 + 2*x*x;
        //a3 = r2 + 2*y*y;
		//double r = sqrt(r2);
		double theta = atan(r0);
		double theta2 = theta*theta;
		double theta4 = theta2*theta2;
		double theta6 = theta4*theta2;
		double theta8 = theta6*theta2;
		double theta_d =0;
		theta_d = theta*(1 + k[0]*theta2 + k[1]*theta4 + k[2]*theta6 + k[3]*theta8);
		xd = x*theta_d/r0;
		yd = y*theta_d/r0;


        m[i].x = xd*fx + cx;
        m[i].y = yd*fy + cy;

        if( calc_derivatives ) //calc_derivatives
        {
            if( dpdc_p )
            {
                dpdc_p[0] = 1; dpdc_p[1] = 0;
                dpdc_p[dpdc_step] = 0;
                dpdc_p[dpdc_step+1] = 1;
                dpdc_p += dpdc_step*2;
            }

            if( dpdf_p )
            {
                dpdf_p[0] = xd; dpdf_p[1] = 0;
                dpdf_p[dpdf_step] = 0;
                dpdf_p[dpdf_step+1] = yd;
                dpdf_p += dpdf_step*2;
            }
			if( dpdk_p )
            {
                dpdk_p[0] = fx*x*pow(theta,3)/r0;
                dpdk_p[1] = fx*x*pow(theta,5)/r0;;
                dpdk_p[dpdk_step] = fy*y*pow(theta,3)/r0;
                dpdk_p[dpdk_step+1] = fy*y*pow(theta,5)/r0;
                if( _dpdk->cols > 2 )
                {
					dpdk_p[2] = fx*x*pow(theta,7)/r0;
                    dpdk_p[3] = fx*x*pow(theta,9)/r0;
                    dpdk_p[dpdk_step+2] = fy*y*pow(theta,7)/r0;
                    dpdk_p[dpdk_step+3] = fy*y*pow(theta,9)/r0;
                }
                dpdk_p += dpdk_step*2;
            }
            if( dpdt_p )
            {
                double dxdt[] = { z, 0, -x*z }, dydt[] = { 0, z, -y*z };
                for( j = 0; j < 3; j++ )
                {
					double dr0_dt = (x*dxdt[j] + y*dydt[j])/r0;
					double theta_dt = dr0_dt/(1 + r2);
					double theta_d_dt = theta_dt*(1 + k[0]*3*theta2 + k[1]*5*theta4 + k[2]*7*theta6 + k[3]*9*theta8);					
					double dmxdt = fx*((dxdt[j]*theta_d + theta_d_dt*x)/r0 - x*theta_d*dr0_dt/r2);
					double dmydt = fy*((dydt[j]*theta_d + theta_d_dt*y)/r0 - y*theta_d*dr0_dt/r2);

                    dpdt_p[j] = dmxdt;
                    dpdt_p[dpdt_step+j] = dmydt;
                }
                dpdt_p += dpdt_step*2;
            }

            if( dpdr_p )
            {
                double dx0dr[] =
                {
                    X*dRdr[0] + Y*dRdr[1] + Z*dRdr[2],
                    X*dRdr[9] + Y*dRdr[10] + Z*dRdr[11],
                    X*dRdr[18] + Y*dRdr[19] + Z*dRdr[20]
                };
                double dy0dr[] =
                {
                    X*dRdr[3] + Y*dRdr[4] + Z*dRdr[5],
                    X*dRdr[12] + Y*dRdr[13] + Z*dRdr[14],
                    X*dRdr[21] + Y*dRdr[22] + Z*dRdr[23]
                };
                double dz0dr[] =
                {
                    X*dRdr[6] + Y*dRdr[7] + Z*dRdr[8],
                    X*dRdr[15] + Y*dRdr[16] + Z*dRdr[17],
                    X*dRdr[24] + Y*dRdr[25] + Z*dRdr[26]
                };
                for( j = 0; j < 3; j++ )
                {
                    double dxdr = z*(dx0dr[j] - x*dz0dr[j]);
                    double dydr = z*(dy0dr[j] - y*dz0dr[j]);
					double dr0_dr = (x*dxdr + y*dydr)/r0;
					double theta_dr = dr0_dr/(1 + r2);
					double theta_d_dr = theta_dr*(1 + k[0]*3*theta2 + k[1]*5*theta4 + k[2]*7*theta6 + k[3]*9*theta8);//gbb					
					double dmxdr = fx*((dxdr*theta_d + theta_d_dr*x)/r0 - x*theta_d*dr0_dr/r2);
					double dmydr = fy*((dydr*theta_d + theta_d_dr*y)/r0 - y*theta_d*dr0_dr/r2);

                    dpdr_p[j] = dmxdr;
                    dpdr_p[dpdr_step+j] = dmydr;
                }
                dpdr_p += dpdr_step*2;
            }
        }
    }

    if( _m != img_points )
        cvConvertPointsHomogenious( _m, img_points );
    if( _dpdr != dpdr )
        cvConvert( _dpdr, dpdr );
    if( _dpdt != dpdt )
        cvConvert( _dpdt, dpdt );
    if( _dpdf != dpdf )
        cvConvert( _dpdf, dpdf );
    if( _dpdc != dpdc )
        cvConvert( _dpdc, dpdc );
    if( _dpdk != dpdk )
        cvConvert( _dpdk, dpdk );

    __END__;

    if( _M != obj_points )
        cvReleaseMat( &_M );
    if( _m != img_points )
        cvReleaseMat( &_m );
    if( _dpdr != dpdr )
        cvReleaseMat( &_dpdr );
    if( _dpdt != dpdt )
        cvReleaseMat( &_dpdt );
    if( _dpdf != dpdf )
        cvReleaseMat( &_dpdf );
    if( _dpdc != dpdc )
        cvReleaseMat( &_dpdc );
    if( _dpdk != dpdk )
        cvReleaseMat( &_dpdk );
}

CV_IMPL void
cvProjectPoints_fisheyeD1( const CvMat* obj_points,
                  const CvMat* r_vec,
                  const CvMat* t_vec,
                  const CvMat* A,
                  const CvMat* dist_coeffs,
                  CvMat* img_points, CvMat* dpdr,
                  CvMat* dpdt, CvMat* dpdf,
                  CvMat* dpdc, CvMat* dpdk )
{
    CvMat *_M = 0, *_m = 0;
    CvMat *_dpdr = 0, *_dpdt = 0, *_dpdc = 0, *_dpdf = 0, *_dpdk = 0;
    
    CV_FUNCNAME( "cvProjectPoints_fisheye" );

    __BEGIN__;

    int i, j, count;
    int calc_derivatives;
    const CvPoint3D64f* M;
    CvPoint2D64f* m;
    double r[3], R[9], dRdr[27], t[3], a[9], k[4] = {0,0,0,0}, fx, fy, cx, cy;
    CvMat _r, _t, _a = cvMat( 3, 3, CV_64F, a ), _k;
    CvMat _R = cvMat( 3, 3, CV_64F, R ), _dRdr = cvMat( 3, 9, CV_64F, dRdr );
    double *dpdr_p = 0, *dpdt_p = 0, *dpdk_p = 0, *dpdf_p = 0, *dpdc_p = 0;
    int dpdr_step = 0, dpdt_step = 0, dpdk_step = 0, dpdf_step = 0, dpdc_step = 0;

    if( !CV_IS_MAT(obj_points) || !CV_IS_MAT(r_vec) ||
        !CV_IS_MAT(t_vec) || !CV_IS_MAT(A) ||
        /*!CV_IS_MAT(dist_coeffs) ||*/ !CV_IS_MAT(img_points) )
        CV_ERROR( CV_StsBadArg, "One of required arguments is not a valid matrix" );

    count = MAX(obj_points->rows, obj_points->cols);

    if( CV_IS_CONT_MAT(obj_points->type) && CV_MAT_DEPTH(obj_points->type) == CV_64F &&
        (obj_points->rows == 1 && CV_MAT_CN(obj_points->type) == 3 ||
        obj_points->rows == count && CV_MAT_CN(obj_points->type)*obj_points->cols == 3))
        _M = (CvMat*)obj_points;
    else
    {
        CV_CALL( _M = cvCreateMat( 1, count, CV_64FC3 ));
        CV_CALL( cvConvertPointsHomogenious( obj_points, _M ));
    }

    if( CV_IS_CONT_MAT(img_points->type) && CV_MAT_DEPTH(img_points->type) == CV_64F &&
        (img_points->rows == 1 && CV_MAT_CN(img_points->type) == 2 ||
        img_points->rows == count && CV_MAT_CN(img_points->type)*img_points->cols == 2))
        _m = img_points;
    else
        CV_CALL( _m = cvCreateMat( 1, count, CV_64FC2 ));

    M = (CvPoint3D64f*)_M->data.db;
    m = (CvPoint2D64f*)_m->data.db;

    if( CV_MAT_DEPTH(r_vec->type) != CV_64F && CV_MAT_DEPTH(r_vec->type) != CV_32F ||
        (r_vec->rows != 1 && r_vec->cols != 1 ||
        r_vec->rows*r_vec->cols*CV_MAT_CN(r_vec->type) != 3) &&
        (r_vec->rows != 3 && r_vec->cols != 3 || CV_MAT_CN(r_vec->type) != 1))
        CV_ERROR( CV_StsBadArg, "Rotation must be represented by 1x3 or 3x1 "
                  "floating-point rotation vector, or 3x3 rotation matrix" );

    if( r_vec->rows == 3 && r_vec->cols == 3 )
    {
        _r = cvMat( 3, 1, CV_64FC1, r );
        CV_CALL( cvRodrigues2( r_vec, &_r ));
        CV_CALL( cvRodrigues2( &_r, &_R, &_dRdr ));
        cvCopy( r_vec, &_R );
    }
    else
    {
        _r = cvMat( r_vec->rows, r_vec->cols, CV_MAKETYPE(CV_64F,CV_MAT_CN(r_vec->type)), r );
        CV_CALL( cvConvert( r_vec, &_r ));
        CV_CALL( cvRodrigues2( &_r, &_R, &_dRdr ) );
    }

    if( CV_MAT_DEPTH(t_vec->type) != CV_64F && CV_MAT_DEPTH(t_vec->type) != CV_32F ||
        t_vec->rows != 1 && t_vec->cols != 1 ||
        t_vec->rows*t_vec->cols*CV_MAT_CN(t_vec->type) != 3 )
        CV_ERROR( CV_StsBadArg,
            "Translation vector must be 1x3 or 3x1 floating-point vector" );

    _t = cvMat( t_vec->rows, t_vec->cols, CV_MAKETYPE(CV_64F,CV_MAT_CN(t_vec->type)), t );
    CV_CALL( cvConvert( t_vec, &_t ));

    if( CV_MAT_TYPE(A->type) != CV_64FC1 && CV_MAT_TYPE(A->type) != CV_32FC1 ||
        A->rows != 3 || A->cols != 3 )
        CV_ERROR( CV_StsBadArg, "Instrinsic parameters must be 3x3 floating-point matrix" );

    CV_CALL( cvConvert( A, &_a ));
    fx = a[0]; fy = a[4];
    cx = a[2] + a[2]; cy = a[5] + a[5];

    if( dist_coeffs )
    {
        if( !CV_IS_MAT(dist_coeffs) ||
            CV_MAT_DEPTH(dist_coeffs->type) != CV_64F &&
            CV_MAT_DEPTH(dist_coeffs->type) != CV_32F ||
            dist_coeffs->rows != 1 && dist_coeffs->cols != 1 ||
            dist_coeffs->rows*dist_coeffs->cols*CV_MAT_CN(dist_coeffs->type) != 4 )
            CV_ERROR( CV_StsBadArg,
                "Distortion coefficients must be 1x4 or 4x1 floating-point vector" );

        _k = cvMat( dist_coeffs->rows, dist_coeffs->cols,
                    CV_MAKETYPE(CV_64F,CV_MAT_CN(dist_coeffs->type)), k );
        CV_CALL( cvConvert( dist_coeffs, &_k ));
    }
    
    if( dpdr )
    {
        if( !CV_IS_MAT(dpdr) ||
            CV_MAT_TYPE(dpdr->type) != CV_32FC1 &&
            CV_MAT_TYPE(dpdr->type) != CV_64FC1 ||
            dpdr->rows != count*2 || dpdr->cols != 3 )
            CV_ERROR( CV_StsBadArg, "dp/drot must be 2Nx3 floating-point matrix" );

        if( CV_MAT_TYPE(dpdr->type) == CV_64FC1 )
            _dpdr = dpdr;
        else
            CV_CALL( _dpdr = cvCreateMat( 2*count, 3, CV_64FC1 ));
        dpdr_p = _dpdr->data.db;
        dpdr_step = _dpdr->step/sizeof(dpdr_p[0]);
    }

    if( dpdt )
    {
        if( !CV_IS_MAT(dpdt) ||
            CV_MAT_TYPE(dpdt->type) != CV_32FC1 &&
            CV_MAT_TYPE(dpdt->type) != CV_64FC1 ||
            dpdt->rows != count*2 || dpdt->cols != 3 )
            CV_ERROR( CV_StsBadArg, "dp/dT must be 2Nx3 floating-point matrix" );

        if( CV_MAT_TYPE(dpdt->type) == CV_64FC1 )
            _dpdt = dpdt;
        else
            CV_CALL( _dpdt = cvCreateMat( 2*count, 3, CV_64FC1 ));
        dpdt_p = _dpdt->data.db;
        dpdt_step = _dpdt->step/sizeof(dpdt_p[0]);
    }

    if( dpdf )
    {
        if( !CV_IS_MAT(dpdf) ||
            CV_MAT_TYPE(dpdf->type) != CV_32FC1 && CV_MAT_TYPE(dpdf->type) != CV_64FC1 ||
            dpdf->rows != count*2 || dpdf->cols != 2 )
            CV_ERROR( CV_StsBadArg, "dp/df must be 2Nx2 floating-point matrix" );

        if( CV_MAT_TYPE(dpdf->type) == CV_64FC1 )
            _dpdf = dpdf;
        else
            CV_CALL( _dpdf = cvCreateMat( 2*count, 2, CV_64FC1 ));
        dpdf_p = _dpdf->data.db;
        dpdf_step = _dpdf->step/sizeof(dpdf_p[0]);
    }

    if( dpdc )
    {
        if( !CV_IS_MAT(dpdc) ||
            CV_MAT_TYPE(dpdc->type) != CV_32FC1 && CV_MAT_TYPE(dpdc->type) != CV_64FC1 ||
            dpdc->rows != count*2 || dpdc->cols != 2 )
            CV_ERROR( CV_StsBadArg, "dp/dc must be 2Nx2 floating-point matrix" );

        if( CV_MAT_TYPE(dpdc->type) == CV_64FC1 )
            _dpdc = dpdc;
        else
            CV_CALL( _dpdc = cvCreateMat( 2*count, 2, CV_64FC1 ));
        dpdc_p = _dpdc->data.db;
        dpdc_step = _dpdc->step/sizeof(dpdc_p[0]);
    }

    if( dpdk )
    {
        if( !CV_IS_MAT(dpdk) ||
            CV_MAT_TYPE(dpdk->type) != CV_32FC1 && CV_MAT_TYPE(dpdk->type) != CV_64FC1 ||
            dpdk->rows != count*2 || (dpdk->cols != 4 && dpdk->cols != 2) )
            CV_ERROR( CV_StsBadArg, "dp/df must be 2Nx4 or 2Nx2 floating-point matrix" );

        if( !dist_coeffs )
            CV_ERROR( CV_StsNullPtr, "dist_coeffs is NULL while dpdk is not" );

        if( CV_MAT_TYPE(dpdk->type) == CV_64FC1 )
            _dpdk = dpdk;
        else
            CV_CALL( _dpdk = cvCreateMat( dpdk->rows, dpdk->cols, CV_64FC1 ));
        dpdk_p = _dpdk->data.db;
        dpdk_step = _dpdk->step/sizeof(dpdk_p[0]);
    }

    calc_derivatives = dpdr || dpdt || dpdf || dpdc || dpdk;

    for( i = 0; i < count; i++ )
    {
        double X = M[i].x, Y = M[i].y, Z = M[i].z;
        double x = R[0]*X + R[1]*Y + R[2]*Z + t[0];
        double y = R[3]*X + R[4]*Y + R[5]*Z + t[1];
        double z = R[6]*X + R[7]*Y + R[8]*Z + t[2];
        double r2, r0;
        double xd, yd;

        z = z ? 1./z : 1;
        x *= z; y *= z;


		double x2 = x*x;
		double y2 = y*y;
		r2 = x2 + y2;
		r0 = sqrt(r2);
		
		//a1 = 2*x*y;
        //a2 = r2 + 2*x*x;
        //a3 = r2 + 2*y*y;
		//double r = sqrt(r2);
		double theta = atan(r0);
		double theta2 = theta*theta;
		double theta4 = theta2*theta2;
		double theta6 = theta4*theta2;
		double theta8 = theta6*theta2;
		double theta_d =0;
		theta_d = theta*(1 + k[0]*theta2 + k[1]*theta4 + k[2]*theta6 + k[3]*theta8);
		xd = x*theta_d/r0;
		yd = y*theta_d/r0;


        m[i].x = 2*xd*fx + cx;
        m[i].y = 2*yd*fy + cy;

        if( calc_derivatives ) //calc_derivatives
        {
            if( dpdc_p )
            {
                dpdc_p[0] = 1; dpdc_p[1] = 0;
                dpdc_p[dpdc_step] = 0;
                dpdc_p[dpdc_step+1] = 1;
                dpdc_p += dpdc_step*2;
            }

            if( dpdf_p )
            {
                dpdf_p[0] = xd; dpdf_p[1] = 0;
                dpdf_p[dpdf_step] = 0;
                dpdf_p[dpdf_step+1] = yd;
                dpdf_p += dpdf_step*2;
            }
			if( dpdk_p )
            {
                dpdk_p[0] = fx*x*pow(theta,3)/r0;
                dpdk_p[1] = fx*x*pow(theta,5)/r0;;
                dpdk_p[dpdk_step] = fy*y*pow(theta,3)/r0;
                dpdk_p[dpdk_step+1] = fy*y*pow(theta,5)/r0;
                if( _dpdk->cols > 2 )
                {
					dpdk_p[2] = fx*x*pow(theta,7)/r0;
                    dpdk_p[3] = fx*x*pow(theta,9)/r0;
                    dpdk_p[dpdk_step+2] = fy*y*pow(theta,7)/r0;
                    dpdk_p[dpdk_step+3] = fy*y*pow(theta,9)/r0;
                }
                dpdk_p += dpdk_step*2;
            }
            if( dpdt_p )
            {
                double dxdt[] = { z, 0, -x*z }, dydt[] = { 0, z, -y*z };
                for( j = 0; j < 3; j++ )
                {

					double theta_dt = (x*dxdt[j] + y*dydt[j])/(r0*(1 + r2));
					double theta_d_dt = theta_dt*theta_d/theta + theta_dt*(k[0]*2*theta2 + k[1]*4*theta4 + k[2]*6*theta6 + k[3]*8*theta8);
					//gbb,
					//double theta_d_dt = theta_dt*(1 + 3*k[0]*theta2 + 5*k[1]*theta4 + 7*k[2]*theta6 + 9*k[3]*theta8);
					double dmxdt = fx*(((dxdt[j]*theta_d + theta_d_dt*x)*r0 - x*theta_d*(x*dxdt[j] + y*dydt[j])/r0)/r2);
					double dmydt = fy*(((dydt[j]*theta_d + theta_d_dt*y)*r0 - y*theta_d*(x*dxdt[j] + y*dydt[j])/r0)/r2);

                    dpdt_p[j] = dmxdt;
                    dpdt_p[dpdt_step+j] = dmydt;
                }
                dpdt_p += dpdt_step*2;
            }

            if( dpdr_p )
            {
                double dx0dr[] =
                {
                    X*dRdr[0] + Y*dRdr[1] + Z*dRdr[2],
                    X*dRdr[9] + Y*dRdr[10] + Z*dRdr[11],
                    X*dRdr[18] + Y*dRdr[19] + Z*dRdr[20]
                };
                double dy0dr[] =
                {
                    X*dRdr[3] + Y*dRdr[4] + Z*dRdr[5],
                    X*dRdr[12] + Y*dRdr[13] + Z*dRdr[14],
                    X*dRdr[21] + Y*dRdr[22] + Z*dRdr[23]
                };
                double dz0dr[] =
                {
                    X*dRdr[6] + Y*dRdr[7] + Z*dRdr[8],
                    X*dRdr[15] + Y*dRdr[16] + Z*dRdr[17],
                    X*dRdr[24] + Y*dRdr[25] + Z*dRdr[26]
                };
                for( j = 0; j < 3; j++ )
                {
                    double dxdr = z*(dx0dr[j] - x*dz0dr[j]);
                    double dydr = z*(dy0dr[j] - y*dz0dr[j]);

					double theta_dr = (x*dxdr + y*dydr)/(r0*(1 + r2));
					double theta_d_dr = theta_dr*theta_d/theta + theta_dr*(k[0]*2*theta2 + k[1]*4*theta4 + k[2]*6*theta6 + k[3]*8*theta8);
					//gbb
					//double theta_d_dr = theta_dr*(1 + 3*k[0]*theta2 + 5*k[1]*theta4 + 7*k[2]*theta6 + 9*k[3]*theta8);
					double dmxdr = fx*(((dxdr*theta_d + theta_d_dr*x)*r0 - x*theta_d*(x*dxdr + y*dydr)/r0)/r2);
					double dmydr = fy*(((dydr*theta_d + theta_d_dr*y)*r0 - y*theta_d*(x*dxdr + y*dydr)/r0)/r2);

                    dpdr_p[j] = dmxdr;
                    dpdr_p[dpdr_step+j] = dmydr;
                }
                dpdr_p += dpdr_step*2;
            }
        }
    }

    if( _m != img_points )
        cvConvertPointsHomogenious( _m, img_points );
    if( _dpdr != dpdr )
        cvConvert( _dpdr, dpdr );
    if( _dpdt != dpdt )
        cvConvert( _dpdt, dpdt );
    if( _dpdf != dpdf )
        cvConvert( _dpdf, dpdf );
    if( _dpdc != dpdc )
        cvConvert( _dpdc, dpdc );
    if( _dpdk != dpdk )
        cvConvert( _dpdk, dpdk );

    __END__;

    if( _M != obj_points )
        cvReleaseMat( &_M );
    if( _m != img_points )
        cvReleaseMat( &_m );
    if( _dpdr != dpdr )
        cvReleaseMat( &_dpdr );
    if( _dpdt != dpdt )
        cvReleaseMat( &_dpdt );
    if( _dpdf != dpdf )
        cvReleaseMat( &_dpdf );
    if( _dpdc != dpdc )
        cvReleaseMat( &_dpdc );
    if( _dpdk != dpdk )
        cvReleaseMat( &_dpdk );
}


CV_IMPL void
cvFindExtrinsicCameraParams_fisheye( const CvMat* obj_points,
                  const CvMat* img_points, const CvMat* A,
                  const CvMat* dist_coeffs,
                  CvMat* r_vec, CvMat* t_vec )
{
    const int max_iter = 30;
    CvMat *_M = 0, *_Mxy = 0, *_m = 0, *_mn = 0, *_L = 0, *_J = 0;
    
    CV_FUNCNAME( "cvFindExtrinsicCameraParams_fisheye" );

    __BEGIN__;

    int i, j, count;
    double a[9], k[4] = { 0, 0, 0, 0 }, R[9], ifx, ify, cx, cy;
    double Mc[3] = {0, 0, 0}, MM[9], U[9], V[9], W[3];
    double JtJ[6*6], JtErr[6], JtJW[6], JtJV[6*6], delta[6], param[6];
    CvPoint3D64f* M = 0;
    CvPoint2D64f *m = 0, *mn = 0;
    CvMat _a = cvMat( 3, 3, CV_64F, a );
    CvMat _R = cvMat( 3, 3, CV_64F, R );
    CvMat _r = cvMat( 3, 1, CV_64F, param );
    CvMat _t = cvMat( 3, 1, CV_64F, param + 3 );
    CvMat _Mc = cvMat( 1, 3, CV_64F, Mc );
    CvMat _MM = cvMat( 3, 3, CV_64F, MM );
    CvMat _U = cvMat( 3, 3, CV_64F, U );
    CvMat _V = cvMat( 3, 3, CV_64F, V );
    CvMat _W = cvMat( 3, 1, CV_64F, W );
    CvMat _JtJ = cvMat( 6, 6, CV_64F, JtJ );
    CvMat _JtErr = cvMat( 6, 1, CV_64F, JtErr );
    CvMat _JtJW = cvMat( 6, 1, CV_64F, JtJW );
    CvMat _JtJV = cvMat( 6, 6, CV_64F, JtJV );
    CvMat _delta = cvMat( 6, 1, CV_64F, delta );
    CvMat _param = cvMat( 6, 1, CV_64F, param );
    CvMat _dpdr, _dpdt;

    if( !CV_IS_MAT(obj_points) || !CV_IS_MAT(img_points) ||
        !CV_IS_MAT(A) || !CV_IS_MAT(r_vec) || !CV_IS_MAT(t_vec) )
        CV_ERROR( CV_StsBadArg, "One of required arguments is not a valid matrix" );

    count = MAX(obj_points->cols, obj_points->rows);
    CV_CALL( _M = cvCreateMat( 1, count, CV_64FC3 ));
    CV_CALL( _Mxy = cvCreateMat( 1, count, CV_64FC2 ));
    CV_CALL( _m = cvCreateMat( 1, count, CV_64FC2 ));
    CV_CALL( _mn = cvCreateMat( 1, count, CV_64FC2 ));
    M = (CvPoint3D64f*)_M->data.db;
    m = (CvPoint2D64f*)_m->data.db;
    mn = (CvPoint2D64f*)_mn->data.db;

    CV_CALL( cvConvertPointsHomogenious( obj_points, _M ));
    CV_CALL( cvConvertPointsHomogenious( img_points, _m ));
    CV_CALL( cvConvert( A, &_a ));

    if( dist_coeffs )
    {
        CvMat _k;
        if( !CV_IS_MAT(dist_coeffs) ||
            CV_MAT_DEPTH(dist_coeffs->type) != CV_64F &&
            CV_MAT_DEPTH(dist_coeffs->type) != CV_32F ||
            dist_coeffs->rows != 1 && dist_coeffs->cols != 1 ||
            dist_coeffs->rows*dist_coeffs->cols*CV_MAT_CN(dist_coeffs->type) != 4 )
            CV_ERROR( CV_StsBadArg,
                "Distortion coefficients must be 1x4 or 4x1 floating-point vector" );

        _k = cvMat( dist_coeffs->rows, dist_coeffs->cols,
                    CV_MAKETYPE(CV_64F,CV_MAT_CN(dist_coeffs->type)), k );
        CV_CALL( cvConvert( dist_coeffs, &_k ));
    }

    if( CV_MAT_DEPTH(r_vec->type) != CV_64F && CV_MAT_DEPTH(r_vec->type) != CV_32F ||
        r_vec->rows != 1 && r_vec->cols != 1 ||
        r_vec->rows*r_vec->cols*CV_MAT_CN(r_vec->type) != 3 )
        CV_ERROR( CV_StsBadArg, "Rotation vector must be 1x3 or 3x1 floating-point vector" );

    if( CV_MAT_DEPTH(t_vec->type) != CV_64F && CV_MAT_DEPTH(t_vec->type) != CV_32F ||
        t_vec->rows != 1 && t_vec->cols != 1 ||
        t_vec->rows*t_vec->cols*CV_MAT_CN(t_vec->type) != 3 )
        CV_ERROR( CV_StsBadArg,
            "Translation vector must be 1x3 or 3x1 floating-point vector" );

    ifx = 1./a[0]; ify = 1./a[4];
    cx = a[2]; cy = a[5];

    // normalize image points
    // (unapply the intrinsic matrix transformation and distortion)
    for( i = 0; i < count; i++ )
    {
        double x = (m[i].x - cx)*ifx, y = (m[i].y - cy)*ify, x0 = x, y0 = y;

        // compensate distortion iteratively
        if( dist_coeffs )
		{			
			
			double theta_d = sqrt(x*x + y*y); 
			double theta = theta_d;
			for( j = 0; j < 10; j++ )
			{
				theta = theta_d/(1 + k[0]*pow(theta,2) + k[1]*pow(theta,4) + k[2]*pow(theta,6) + k[3]*pow(theta,8));
				
			}
			double scaling = tan(theta) / theta_d;
			x = x0*scaling;
			y = y0*scaling;
			
		}
        mn[i].x = x; mn[i].y = y;

        // calc mean(M)
        Mc[0] += M[i].x;
        Mc[1] += M[i].y;
        Mc[2] += M[i].z;
    }

    Mc[0] /= count;
    Mc[1] /= count;
    Mc[2] /= count;

    cvReshape( _M, _M, 1, count );
    cvMulTransposed( _M, &_MM, 1, &_Mc );
    cvSVD( &_MM, &_W, 0, &_V, CV_SVD_MODIFY_A + CV_SVD_V_T );

    // initialize extrinsic parameters
    if( W[2]/W[1] < 1e-3 || count < 4 )
    {
        // a planar structure case (all M's lie in the same plane)
        double tt[3], h[9], h1_norm, h2_norm;
        CvMat* R_transform = &_V;
        CvMat T_transform = cvMat( 3, 1, CV_64F, tt );
        CvMat _H = cvMat( 3, 3, CV_64F, h );
        CvMat _h1, _h2, _h3;

        if( V[2]*V[2] + V[5]*V[5] < 1e-10 )
            cvSetIdentity( R_transform );

        if( cvDet(R_transform) < 0 )
            cvScale( R_transform, R_transform, -1 );

        cvGEMM( R_transform, &_Mc, -1, 0, 0, &T_transform, CV_GEMM_B_T );

        for( i = 0; i < count; i++ )
        {
            const double* Rp = R_transform->data.db;
            const double* Tp = T_transform.data.db;
            const double* src = _M->data.db + i*3;
            double* dst = _Mxy->data.db + i*2;

            dst[0] = Rp[0]*src[0] + Rp[1]*src[1] + Rp[2]*src[2] + Tp[0];//LOG_printf(&trace,"%lf \n %lf\n %lf\n",src[0],src[1],src[2]);
            dst[1] = Rp[3]*src[0] + Rp[4]*src[1] + Rp[5]*src[2] + Tp[1];
        }

        cvFindHomography( _Mxy, _mn, &_H );

        cvGetCol( &_H, &_h1, 0 );
        _h2 = _h1; _h2.data.db++;
        _h3 = _h2; _h3.data.db++;
        h1_norm = sqrt(h[0]*h[0] + h[3]*h[3] + h[6]*h[6]);
        h2_norm = sqrt(h[1]*h[1] + h[4]*h[4] + h[7]*h[7]);

        cvScale( &_h1, &_h1, 1./h1_norm );
        cvScale( &_h2, &_h2, 1./h2_norm );
        cvScale( &_h3, &_t, 2./(h1_norm + h2_norm));
        cvCrossProduct( &_h1, &_h2, &_h3 );

        cvRodrigues2( &_H, &_r );
        cvRodrigues2( &_r, &_H );
        cvMatMulAdd( &_H, &T_transform, &_t, &_t );
        cvMatMul( &_H, R_transform, &_R );
        cvRodrigues2( &_R, &_r );
    }
    else
    {
        // non-planar structure. Use DLT method
        double* L;
        double LL[12*12], LW[12], LV[12*12], sc;
        CvMat _LL = cvMat( 12, 12, CV_64F, LL );
        CvMat _LW = cvMat( 12, 1, CV_64F, LW );
        CvMat _LV = cvMat( 12, 12, CV_64F, LV );
        CvMat _RRt, _RR, _tt;

        CV_CALL( _L = cvCreateMat( 2*count, 12, CV_64F ));
        L = _L->data.db;

        for( i = 0; i < count; i++, L += 24 )
        {
            double x = -mn[i].x, y = -mn[i].y;
            L[0] = L[16] = M[i].x;
            L[1] = L[17] = M[i].y;
            L[2] = L[18] = M[i].z;
            L[3] = L[19] = 1.;
            L[4] = L[5] = L[6] = L[7] = 0.;
            L[12] = L[13] = L[14] = L[15] = 0.;
            L[8] = x*M[i].x;
            L[9] = x*M[i].y;
            L[10] = x*M[i].z;
            L[11] = x;
            L[20] = y*M[i].x;
            L[21] = y*M[i].y;
            L[22] = y*M[i].z;
            L[23] = y;
        }

        cvMulTransposed( _L, &_LL, 1 );
        cvSVD( &_LL, &_LW, 0, &_LV, CV_SVD_MODIFY_A + CV_SVD_V_T );
        _RRt = cvMat( 3, 4, CV_64F, LV + 11*12 );
        cvGetCols( &_RRt, &_RR, 0, 3 );
        cvGetCol( &_RRt, &_tt, 3 );
        if( cvDet(&_RR) < 0 )
            cvScale( &_RRt, &_RRt, -1 );
        sc = cvNorm(&_RR);
        cvSVD( &_RR, &_W, &_U, &_V, CV_SVD_MODIFY_A + CV_SVD_U_T + CV_SVD_V_T );
        cvGEMM( &_U, &_V, 1, 0, 0, &_R, CV_GEMM_A_T );
        cvScale( &_tt, &_t, cvNorm(&_R)/sc );
        cvRodrigues2( &_R, &_r );
        cvReleaseMat( &_L );
    }

    CV_CALL( _J = cvCreateMat( 2*count, 6, CV_64FC1 ));
    cvGetCols( _J, &_dpdr, 0, 3 );
    cvGetCols( _J, &_dpdt, 3, 6 );

    // refine extrinsic parameters using iterative algorithm
    for( i = 0; i < max_iter; i++ )
    {
        double n1, n2;
        cvReshape( _mn, _mn, 2, 1 );
        cvProjectPoints_fisheye( _M, &_r, &_t, &_a, dist_coeffs,
                          _mn, &_dpdr, &_dpdt, 0, 0, 0 );
        cvSub( _m, _mn, _mn );
        cvReshape( _mn, _mn, 1, 2*count );

        cvMulTransposed( _J, &_JtJ, 1 );
        cvGEMM( _J, _mn, 1, 0, 0, &_JtErr, CV_GEMM_A_T );
        cvSVD( &_JtJ, &_JtJW, 0, &_JtJV, CV_SVD_MODIFY_A + CV_SVD_V_T );
        if( JtJW[5]/JtJW[0] < 1e-12 )
            break;
        cvSVBkSb( &_JtJW, &_JtJV, &_JtJV, &_JtErr,
                  &_delta, CV_SVD_U_T + CV_SVD_V_T );
        cvAdd( &_delta, &_param, &_param );
        n1 = cvNorm( &_delta );
        n2 = cvNorm( &_param );
        if( n1/n2 < 1e-10 )
            break;
    }

    _r = cvMat( r_vec->rows, r_vec->cols,
        CV_MAKETYPE(CV_64F,CV_MAT_CN(r_vec->type)), param );
    _t = cvMat( t_vec->rows, t_vec->cols,
        CV_MAKETYPE(CV_64F,CV_MAT_CN(t_vec->type)), param + 3 );

    cvConvert( &_r, r_vec );
    cvConvert( &_t, t_vec );

    __END__;

    cvReleaseMat( &_M );
    cvReleaseMat( &_Mxy );
    cvReleaseMat( &_m );
    cvReleaseMat( &_mn );
    cvReleaseMat( &_L );
    cvReleaseMat( &_J );
}




//
CV_IMPL void
initChessBoardPoints(int cam_id,const CvSeq* image_points_seq,CvMat* image_points, CvMat* object_points,
					 Parking_Assistant_Params parking_assistant_params)
{
	int j ,k;
	int corners_width = (parking_assistant_params.chessboard_length_corners-1),
		corners_height = (parking_assistant_params.chessboard_width_corners-1);
	int square_size = parking_assistant_params.square_size;
    CvSeqReader reader;		
	CvPoint2D32f *src_img_pt,*dst_img_pt;
	CvPoint3D32f *obj_pt;
	int offset_x =0;
	
	cvStartReadSeq( image_points_seq, &reader, 0);
	src_img_pt= (CvPoint2D32f*)reader.ptr;
	dst_img_pt = (CvPoint2D32f*)image_points->data.fl ;
	obj_pt = (CvPoint3D32f*)object_points->data.fl;
	
	switch (cam_id)
	{
	case 0:
	{
		offset_x = 0;
		for( j = 1; j <= corners_height; j++ )
			for( k = 1; k <= corners_width; k++ )
			{
				*obj_pt++ = cvPoint3D32f(k*square_size + offset_x, j*square_size - CAR_F_BLIND , FLT_EPSILON);
				printf("front chess = %d %d\n",k*square_size + offset_x, j*square_size);
				*dst_img_pt++ = *src_img_pt++;
			}
			CV_NEXT_SEQ_ELEM( image_points_seq->elem_size, reader );
		
		break;
	}
	case 1:
	{
		offset_x =0;// -20 * auto_flag; 
		for( j = corners_height; j > 0; j-- )
			for( k = corners_width; k > 0; k-- )
			{
				*obj_pt++ = cvPoint3D32f(k*square_size + offset_x, j*square_size + CAR_B_BLIND, FLT_EPSILON);
				printf("back chess = %d %d\n",k*square_size + offset_x, j*square_size);
				*dst_img_pt++ = *src_img_pt++;
			}
			CV_NEXT_SEQ_ELEM( image_points_seq->elem_size, reader );
		
		break;
	}
	case 2:
		for( k = 1; k <= corners_height; k++ )
			for( j =corners_width;j >0 ; j-- )
			{
				*obj_pt++ = cvPoint3D32f(k*square_size - CAR_LR_BLIND, j*square_size , FLT_EPSILON);
				printf("left chess = %d %d\n",k*square_size + offset_x, j*square_size);
				*dst_img_pt++ = *src_img_pt++;
			}
			CV_NEXT_SEQ_ELEM( image_points_seq->elem_size, reader );
			
		
		break;
	case 3:
		for( k = corners_height; k >0 ; k-- )
			for( j = 1; j <= corners_width; j++ )
			{
				*obj_pt++ = cvPoint3D32f(k*square_size + CAR_LR_BLIND, j*square_size , FLT_EPSILON);
				printf("right chess = %d %d\n",k*square_size + offset_x, j*square_size);
				*dst_img_pt++ = *src_img_pt++;
			}
			CV_NEXT_SEQ_ELEM( image_points_seq->elem_size, reader );
		
		break;
	default:
		break;
	}	
}
void searchSubSquareCorners2(const CvMat* image,CvSize winSize, CvPoint2D32f* square_corners,int corners_Count)
{
	int i,ii,j,jj,n;
	float max_val;
	CvPoint2D32f pts = cvPoint2D32f(0,0);
	int waste_w = 1, waste_h = 1;
	int win_width = 2*winSize.width +1,win_height = 2*winSize.height+1;
	CvMat* win32 = cvCreateMat(win_height,win_width,CV_32F),
		   *grad_x = cvCreateMat(win_height,win_width,CV_32F),
		   *grad_y = cvCreateMat(win_height,win_width,CV_32F),
		   *grad_xy = cvCreateMat(win_height,win_width,CV_32F);

	unsigned char* ptr_src = image->data.ptr;
	float  *ptr_win32 = win32->data.fl;
	for (n = 0; n < corners_Count; n++)
	{
		int base_x = square_corners[n].x;
		int base_y = square_corners[n].y;
		float kx[9]= {1,0,-1,0,0,0,-1,0,1},
			k[5] = {1,0,-2,0,1};
		CvMat km = cvMat(3,3,CV_32FC1,kx),
			k_h = cvMat(1,5,CV_32FC1,k),
			k_v = cvMat(5,1,CV_32FC1,k);


		max_val =0;
		//get the neighbourhood window
		for(i = 0; i< win_width; i++)
			for(j = 0; j<win_height; j++)
			{
				ptr_win32[j*win_width +i] = 255 - (float) ptr_src[WIDVPFE*(base_y-winSize.height + j) + base_x-winSize.width +i ];
			}
#if 0
		cvSobel( win32, grad_x, 1, 0, 3);						
		cvSobel( grad_x, grad_xy, 0, 1, 3);					
		cvAbs(grad_xy,grad_xy);	
#else
		//cvCornerHarris(win32, grad_xy, 7, 3, 0.04 );
		//cvSmooth(win32,win32,CV_GAUSSIAN,3,0,0,0);
		cvFilter2D(win32,grad_x,&k_h,cvPoint(-1,-1));
		cvFilter2D(win32,grad_y,&k_v,cvPoint(-1,-1));
		cvFilter2D(win32,grad_xy,&km,cvPoint(-1,-1));
		cvMul(grad_x,grad_y,grad_x);
		cvMul(grad_xy,grad_xy,grad_xy);
		cvSub(grad_xy,grad_x,grad_xy);
		cvAbs(grad_xy,grad_xy);
#endif

		/*cvFilter2D(win32,grad_xy,&km,cvPoint(-1,-1));
		cvSobel( win32, grad_x, 1, 0, 3);						
		cvSobel( win32, grad_y, 0, 1, 3);
		cvMul(grad_x,grad_y,grad_x);
		cvMul(grad_xy,grad_xy,grad_xy);
		cvSub(grad_xy,grad_x,grad_xy);
		//cvAbs(grad_xy,grad_xy);*/

		for(i = base_x -winSize.width+waste_w; i <= base_x + winSize.width-waste_w; i++)
		{
			ii = i - (base_x - winSize.width);
			for(j = base_y-winSize.height+waste_h; j <= base_y + winSize.height-waste_h; j++)
			{
				jj = j - (base_y - winSize.height);
				if(grad_xy->data.fl[jj*win_width + ii] > max_val)
				{					
					max_val = grad_xy->data.fl[jj*win_width + ii];
					pts = cvPoint2D32f(float(i),float(j));
				}
			}
		}
		square_corners[n] = cvPoint2D32f(pts.x,pts.y);
	}
	cvReleaseMat( &win32);
	cvReleaseMat( &grad_x);
	cvReleaseMat( &grad_y);
	cvReleaseMat( &grad_xy);
}



