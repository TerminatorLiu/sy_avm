include(openglwindow.pri)

SOURCES += \
    InitMosaic.cpp \
    esTransform.cpp \
    main.cpp \
    mtlloader.cpp \
    objloader.cpp \
    render.cpp \
    tinystr.cpp \
    tinyxml.cpp \
    tinyxmlerror.cpp \
    tinyxmlparser.cpp \
    xmltest.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/gui/openglwindow
INSTALLS += target

INCLUDEPATH += $$PWD/../libOpenCV/build-libOpenCV-Desktop_Qt_5_9_9_GCC_64bit-Release
DEPENDPATH += $$PWD/../libOpenCV/build-libOpenCV-Desktop_Qt_5_9_9_GCC_64bit-Release

INCLUDEPATH += $$PWD/../libOpenCV/build-libOpenCV-Desktop_Qt_5_9_9_GCC_64bit-Debug
DEPENDPATH += $$PWD/../libOpenCV/build-libOpenCV-Desktop_Qt_5_9_9_GCC_64bit-Debug

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../libOpenCV/build-libOpenCV-Desktop_Qt_5_9_9_GCC_64bit-Release/release/ -llibOpenCV
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../libOpenCV/build-libOpenCV-Desktop_Qt_5_9_9_GCC_64bit-Release/debug/ -llibOpenCV
else:unix: LIBS += -L$$PWD/../libOpenCV/build-libOpenCV-Desktop_Qt_5_9_9_GCC_64bit-Release/ -llibOpenCV

INCLUDEPATH += $$PWD/../libOpenCV/build-libOpenCV-Desktop_Qt_5_9_9_GCC_64bit-Release
DEPENDPATH += $$PWD/../libOpenCV/build-libOpenCV-Desktop_Qt_5_9_9_GCC_64bit-Release
