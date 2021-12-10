QT -= gui

TEMPLATE = lib
DEFINES += LIBOPENCV_LIBRARY

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    cvaccum.cpp \
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

HEADERS += \
    libOpenCV_global.h \
    libopencv.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
