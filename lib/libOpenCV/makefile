

#CC:=/opt/gcc-linaro-7.4.1-2019.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-g++	
CC = /home/lt/Desktop/T507/gcc-linaro-7.4.1-2019.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc
AR = /home/lt/Desktop/T507/gcc-linaro-7.4.1-2019.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-ar
SOURCE:=cvaccum.cpp \
    cvadapthresh.cpp \
    cvapprox.cpp \
    cvcalccontrasthistogram.cpp \
    cvcalcimagehomography.cpp \
    cvcalibinit.cpp \
    cvcalibration.cpp \
    cvcamshift.cpp \
    cvcanny.cpp \
    cvcolor.cpp \
    cvcondens.cpp \
    cvcontours.cpp \
    cvcontourtree.cpp \
    cvconvhull.cpp \
    cvconvolve.cpp \
    cvcorner.cpp \
    cvcornersubpix.cpp \
    cvderiv.cpp \
    cvdistransform.cpp \
    cvdominants.cpp \
    cvemd.cpp \
    cvfeatureselect.cpp \
    cvfilter.cpp \
    cvfloodfill.cpp \
    cvfundam.cpp \
    cvgeometry.cpp \
    cvhaar.cpp \
    cvhistogram.cpp \
    cvhough.cpp \
    cvimgwarp.cpp \
    cvinpaint.cpp \
    cvkalman.cpp \
    cvlinefit.cpp \
    cvlkpyramid.cpp \
    cvmatchcontours.cpp \
    cvmoments.cpp \
    cvmorph.cpp \
    cvmotempl.cpp \
    cvoptflowbm.cpp \
    cvoptflowhs.cpp \
    cvoptflowlk.cpp \
    cvpgh.cpp \
    cvposit.cpp \
    cvprecomp.cpp \
    cvpyramids.cpp \
    cvpyrsegmentation.cpp \
    cvrotcalipers.cpp \
    cvsamplers.cpp \
    cvsegmentation.cpp \
    cvshapedescr.cpp \
    cvsmooth.cpp \
    cvsnakes.cpp \
    cvsubdivision2d.cpp \
    cvsumpixels.cpp \
    cvtables.cpp \
    cvtemplmatch.cpp \
    cvthresh.cpp \
    cvundistort.cpp \
    cvutils.cpp \
    cxalloc.cpp \
    cxarithm.cpp \
    cxarray.cpp \
    cxcmp.cpp \
    cxconvert.cpp \
    cxcopy.cpp \
    cxdatastructs.cpp \
    cxdrawing.cpp \
    cxdxt.cpp \
    cxerror.cpp \
    cximage.cpp \
    cxjacobieigens.cpp \
    cxlogic.cpp \
    cxlut.cpp \
    cxmathfuncs.cpp \
    cxmatmul.cpp \
    cxmatrix.cpp \
    cxmean.cpp \
    cxmeansdv.cpp \
    cxminmaxloc.cpp \
    cxnorm.cpp \
    cxouttext.cpp \
    cxpersistence.cpp \
    cxprecomp.cpp \
    cxrand.cpp \
    cxsumpixels.cpp \
    cxsvd.cpp \
    cxtables.cpp \
    cxutils.cpp

OBJ=cvaccum.o \
    cvadapthresh.o \
    cvapprox.o \
    cvcalccontrasthistogram.o \
    cvcalcimagehomography.o \
    cvcalibinit.o \
    cvcalibration.o \
    cvcamshift.o \
    cvcanny.o \
    cvcolor.o \
    cvcondens.o \
    cvcontours.o \
    cvcontourtree.o \
    cvconvhull.o \
    cvconvolve.o \
    cvcorner.o \
    cvcornersubpix.o \
    cvderiv.o \
    cvdistransform.o \
    cvdominants.o \
    cvemd.o \
    cvfeatureselect.o \
    cvfilter.o \
    cvfloodfill.o \
    cvfundam.o \
    cvgeometry.o \
    cvhaar.o \
    cvhistogram.o \
    cvhough.o \
    cvimgwarp.o \
    cvinpaint.o \
    cvkalman.o \
    cvlinefit.o \
    cvlkpyramid.o \
    cvmatchcontours.o \
    cvmoments.o \
    cvmorph.o \
    cvmotempl.o \
    cvoptflowbm.o \
    cvoptflowhs.o \
    cvoptflowlk.o \
    cvpgh.o \
    cvposit.o \
    cvprecomp.o \
    cvpyramids.o \
    cvpyrsegmentation.o \
    cvrotcalipers.o \
    cvsamplers.o \
    cvsegmentation.o \
    cvshapedescr.o \
    cvsmooth.o \
    cvsnakes.o \
    cvsubdivision2d.o \
    cvsumpixels.o \
    cvtables.o \
    cvtemplmatch.o \
    cvthresh.o \
    cvundistort.o \
    cvutils.o \
    cxalloc.o \
    cxarithm.o \
    cxarray.o \
    cxcmp.o \
    cxconvert.o \
    cxcopy.o \
    cxdatastructs.o \
    cxdrawing.o \
    cxdxt.o \
    cxerror.o \
    cximage.o \
    cxjacobieigens.o \
    cxlogic.o \
    cxlut.o \
    cxmathfuncs.o \
    cxmatmul.o \
    cxmatrix.o \
    cxmean.o \
    cxmeansdv.o \
    cxminmaxloc.o \
    cxnorm.o \
    cxouttext.o \
    cxpersistence.o \
    cxprecomp.o \
    cxrand.o \
    cxsumpixels.o \
    cxsvd.o \
    cxtables.o \
    cxutils.o

OBJDIR=./obj/
CFLAGS=-O2 -Wall -Wfatal-errors -Wno-unused-result -Wno-unknown-pragmas -fPIC -Wno-narrowing


DEPS = $(wildcard *.h)

COMMON=-I.
OBJS = $(addprefix $(OBJDIR), $(OBJ))


#libOpenCV.a:$(OBJS)
#	$(CC)	 -o libOpenCV.a  -fPIC -static -O2 $(SOURCE) -Wno-narrowing
all: $(OBJDIR)  $(OBJS) libOpenCV.a 

$(OBJDIR):
	mkdir -p $(OBJDIR)
	
$(OBJS): $(SOURCE) $(DEPS)
	$(CC)  $(COMMON) $(CFLAGS) -c cvaccum.cpp -o  $(OBJDIR)/cvaccum.o
	$(CC)  $(COMMON) $(CFLAGS) -c cvadapthresh.cpp -o  $(OBJDIR)/cvadapthresh.o
	$(CC)  $(COMMON) $(CFLAGS) -c cvapprox.cpp -o  $(OBJDIR)/cvapprox.o
	$(CC)  $(COMMON) $(CFLAGS) -c cvcalccontrasthistogram.cpp -o  $(OBJDIR)/cvcalccontrasthistogram.o
	$(CC)  $(COMMON) $(CFLAGS) -c cvcalcimagehomography.cpp -o  $(OBJDIR)/cvcalcimagehomography.o
	$(CC)  $(COMMON) $(CFLAGS) -c cvcalibinit.cpp -o  $(OBJDIR)/cvcalibinit.o       
	$(CC)  $(COMMON) $(CFLAGS) -c cvcalibration.cpp -o  $(OBJDIR)/cvcalibration.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvcamshift.cpp -o  $(OBJDIR)/cvcamshift.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvcanny.cpp -o  $(OBJDIR)/cvcanny.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvcolor.cpp -o  $(OBJDIR)/cvcolor.o      
	$(CC)  $(COMMON) $(CFLAGS) -c cvcondens.cpp -o  $(OBJDIR)/cvcondens.o           
	$(CC)  $(COMMON) $(CFLAGS) -c cvcontours.cpp -o  $(OBJDIR)/cvcontours.o  
	$(CC)  $(COMMON) $(CFLAGS) -c cvcontourtree.cpp -o  $(OBJDIR)/cvcontourtree.o      
	$(CC)  $(COMMON) $(CFLAGS) -c cvconvhull.cpp -o  $(OBJDIR)/cvconvhull.o     
	$(CC)  $(COMMON) $(CFLAGS) -c cvconvolve.cpp -o  $(OBJDIR)/cvconvolve.o     
	$(CC)  $(COMMON) $(CFLAGS) -c cvcorner.cpp -o  $(OBJDIR)/cvcorner.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvcorner.cpp -o  $(OBJDIR)/cvcorner.o
	$(CC)  $(COMMON) $(CFLAGS) -c cvcornersubpix.cpp    -o  $(OBJDIR)/cvcornersubpix.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvderiv.cpp     -o  $(OBJDIR)/cvderiv.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvdistransform.cpp     -o  $(OBJDIR)/cvdistransform.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvdominants.cpp    -o  $(OBJDIR)/cvdominants.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvemd.cpp    -o  $(OBJDIR)/cvemd.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvfeatureselect.cpp      -o  $(OBJDIR)/cvfeatureselect.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvfilter.cpp      -o  $(OBJDIR)/cvfilter.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvfloodfill.cpp     -o  $(OBJDIR)/cvfloodfill.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvfundam.cpp      -o  $(OBJDIR)/cvfundam.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvgeometry.cpp    -o  $(OBJDIR)/cvgeometry.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvhaar.cpp    -o  $(OBJDIR)/cvhaar.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvhistogram.cpp    -o  $(OBJDIR)/cvhistogram.o
	$(CC)  $(COMMON) $(CFLAGS) -c cvhough.cpp    -o  $(OBJDIR)/cvhough.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvimgwarp.cpp     -o  $(OBJDIR)/cvimgwarp.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvinpaint.cpp      -o  $(OBJDIR)/cvinpaint.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvkalman.cpp     -o  $(OBJDIR)/cvkalman.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvlinefit.cpp     -o  $(OBJDIR)/cvlinefit.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvlkpyramid.cpp     -o  $(OBJDIR)/cvlkpyramid.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvmatchcontours.cpp    -o  $(OBJDIR)/cvmatchcontours.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvmoments.cpp   -o  $(OBJDIR)/cvmoments.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvmorph.cpp    -o  $(OBJDIR)/cvmorph.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvmotempl.cpp    -o  $(OBJDIR)/cvmotempl.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvoptflowbm.cpp    -o  $(OBJDIR)/cvoptflowbm.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvoptflowhs.cpp    -o  $(OBJDIR)/cvoptflowhs.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvoptflowlk.cpp   -o  $(OBJDIR)/cvoptflowlk.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvpgh.cpp    -o  $(OBJDIR)/cvpgh.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvposit.cpp    -o  $(OBJDIR)/cvposit.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvprecomp.cpp   -o  $(OBJDIR)/cvprecomp.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvpyramids.cpp   -o  $(OBJDIR)/cvpyramids.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvpyrsegmentation.cpp  -o  $(OBJDIR)/cvpyrsegmentation.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvrotcalipers.cpp   -o  $(OBJDIR)/cvrotcalipers.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvsamplers.cpp   -o  $(OBJDIR)/cvsamplers.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvsegmentation.cpp   -o  $(OBJDIR)/cvsegmentation.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvshapedescr.cpp     -o  $(OBJDIR)/cvshapedescr.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvsmooth.cpp    -o  $(OBJDIR)/cvsmooth.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvsnakes.cpp   -o  $(OBJDIR)/cvsnakes.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvsubdivision2d.cpp    -o  $(OBJDIR)/cvsubdivision2d.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvsumpixels.cpp    -o  $(OBJDIR)/cvsumpixels.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvtables.cpp    -o  $(OBJDIR)/cvtables.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvtemplmatch.cpp    -o  $(OBJDIR)/cvtemplmatch.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvthresh.cpp  -o  $(OBJDIR)/cvthresh.o
	$(CC)  $(COMMON) $(CFLAGS) -c cvundistort.cpp   -o  $(OBJDIR)/cvundistort.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cvutils.cpp     -o  $(OBJDIR)/cvutils.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxalloc.cpp    -o  $(OBJDIR)/cxalloc.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxarithm.cpp     -o  $(OBJDIR)/cxarithm.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxarray.cpp    -o  $(OBJDIR)/cxarray.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxcmp.cpp   -o  $(OBJDIR)/cxcmp.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxconvert.cpp    -o  $(OBJDIR)/cxconvert.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxcopy.cpp    -o  $(OBJDIR)/cxcopy.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxdatastructs.cpp   -o  $(OBJDIR)/cxdatastructs.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxdrawing.cpp   -o  $(OBJDIR)/cxdrawing.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxdxt.cpp  -o  $(OBJDIR)/cxdxt.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxerror.cpp    -o  $(OBJDIR)/cxerror.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cximage.cpp    -o  $(OBJDIR)/cximage.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxjacobieigens.cpp   -o  $(OBJDIR)/cxjacobieigens.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxlogic.cpp   -o  $(OBJDIR)/cxlogic.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxlut.cpp   -o  $(OBJDIR)/cxlut.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxmathfuncs.cpp    -o  $(OBJDIR)/cxmathfuncs.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxmatmul.cpp   -o  $(OBJDIR)/cxmatmul.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxmatrix.cpp   -o  $(OBJDIR)/cxmatrix.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxmean.cpp    -o  $(OBJDIR)/cxmean.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxmeansdv.cpp   -o  $(OBJDIR)/cxmeansdv.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxminmaxloc.cpp    -o  $(OBJDIR)/cxminmaxloc.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxnorm.cpp     -o  $(OBJDIR)/cxnorm.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxouttext.cpp    -o  $(OBJDIR)/cxouttext.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxpersistence.cpp     -o  $(OBJDIR)/cxpersistence.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxprecomp.cpp    -o  $(OBJDIR)/cxprecomp.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxrand.cpp      -o  $(OBJDIR)/cxrand.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxsumpixels.cpp      -o  $(OBJDIR)/cxsumpixels.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxsvd.cpp     -o  $(OBJDIR)/cxsvd.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxtables.cpp       -o  $(OBJDIR)/cxtables.o 
	$(CC)  $(COMMON) $(CFLAGS) -c cxutils.cpp     -o  $(OBJDIR)/cxutils.o


libOpenCV.a:
	$(AR) cr libOpenCV.a  $(OBJS)
	$(CC) -shared  -o libOpenCV.so   $(OBJS)
	cp libOpenCV.a ~/liutao_mnt
clean:
	rm -rf libOpenCV.a $(OBJDIR)
