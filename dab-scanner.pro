######################################################################
# Automatically generated by qmake (2.01a) Tue Oct 6 19:48:14 2009
# but modified by me to accomodate for the includes for qwt, hamlib and
# portaudio
######################################################################

TEMPLATE	= app
TARGET		= dab-scanner-1.8

QT		+= widgets xml
CONFIG		-= console
#CONFIG		+= console
QMAKE_CXXFLAGS	+= -std=c++11
QMAKE_CFLAGS	+=  -lfto -ffast-math
QMAKE_CXXFLAGS	+=  -lfto -ffast-math
#QMAKE_CFLAGS	+=  -g
#QMAKE_CXXFLAGS	+=  -g
#QMAKE_LFLAGS	+=  -g
QMAKE_CXXFLAGS += -isystem $$[QT_INSTALL_HEADERS]
RC_ICONS	=  dab-scanner.ico
RESOURCES	+= resources.qrc

TRANSLATIONS = i18n/de_DE.ts i18n/it_IT.ts i18n/hu_HU.ts

DEPENDPATH += . \
	      ./src \
	      ./service-description \
	      ./includes \
	      ./src/ofdm \
	      ./src/protection \
	      ./src/support \
	      ./src/support/viterbi-spiral \
	      ./includes/scopes-qwt6 \
	      ./devices \
	      ./spectrum-viewer \
	      ./includes/ofdm \
	      ./includes/support \
	      ./includes/support/viterbi-spiral
	

INCLUDEPATH += . \
	      ./ \
	      ./src \
	      ./includes \
	      ./includes/protection \
	      ./includes/ofdm \
	      ./includes/support \
	      ./includes/support/viterbi-spiral \
	      ./includes/scopes-qwt6 \
	      ./devices \
	      ./spectrum-viewer

# Input
HEADERS += ./radio.h \
	   ./dab-processor.h \
	   ./channels-table.h \
	   ./output-table.h \
	   ./includes/dab-constants.h \
	   ./includes/country-codes.h \
	   ./includes/ofdm/timesyncer.h \
	   ./includes/ofdm/sample-reader.h \
	   ./includes/ofdm/ofdm-decoder.h \
	   ./includes/ofdm/phasereference.h \
	   ./includes/ofdm/phasetable.h \
	   ./includes/ofdm/freq-interleaver.h \
	   ./includes/ofdm/fic-handler.h \
	   ./includes/ofdm/fib-decoder.h  \
	   ./includes/ofdm/tii_detector.h \
	   ./includes/protection/protTables.h \
#	   ./includes/protection/protection.h \
#	   ./includes/protection/eep-protection.h \
#	   ./includes/protection/uep-protection.h \
           ./includes/support/fft-handler.h \
	   ./includes/support/ringbuffer.h \
	   ./includes/support/dab-params.h \
	   ./includes/support/band-handler.h \
	   ./includes/support/dab_tables.h \
	   ./includes/support/charsets.h \
	   ./includes/support/ensemble-printer.h \
	   ./includes/support/viterbi-spiral/viterbi-spiral.h \
	   ./devices/device-handler.h \
	   ./includes/scopes-qwt6/spectrogramdata.h \
           ./includes/scopes-qwt6/iqdisplay.h \
	   ./spectrum-viewer/spectrum-viewer.h 

FORMS	+= ./forms/dabscanner.ui 
FORMS   += ./spectrum-viewer/scopewidget.ui

SOURCES += ./main.cpp \
	   ./radio.cpp \
	   ./dab-processor.cpp \
	   ./channels-table.cpp \
	   ./output-table.cpp \
	   ./src/ofdm/timesyncer.cpp \
	   ./src/ofdm/sample-reader.cpp \
	   ./src/ofdm/ofdm-decoder.cpp \
	   ./src/ofdm/phasereference.cpp \
	   ./src/ofdm/phasetable.cpp \
	   ./src/ofdm/freq-interleaver.cpp \
	   ./src/ofdm/fic-handler.cpp \
	   ./src/ofdm/fib-decoder.cpp  \
	   ./src/ofdm/tii_detector.cpp \
	   ./src/protection/protTables.cpp \
#	   ./src/protection/protection.cpp \
#	   ./src/protection/eep-protection.cpp \
#	   ./src/protection/uep-protection.cpp \
	   ./src/support/viterbi-spiral/viterbi-spiral.cpp \
           ./src/support/fft-handler.cpp \
	   ./src/support/dab-params.cpp \
	   ./src/support/band-handler.cpp \
	   ./src/support/dab_tables.cpp \
	   ./src/support/charsets.cpp \
	   ./src/support/ensemble-printer.cpp \
	   ./src/support/text-mapper.cpp \
	   ./devices/device-handler.cpp \
	   ./src/scopes-qwt6/iqdisplay.cpp \
	   ./spectrum-viewer/spectrum-viewer.cpp 


#
#	for unix systems this is about it. Adapt when needed for naming
#	and locating libraries. If you do not need a device as
#	listed, just comment the line out.
#
unix {
DESTDIR		= ./linux-bin
exists ("./.git") {
   GITHASHSTRING = $$system(git rev-parse --short HEAD)
   !isEmpty(GITHASHSTRING) {
       message("Current git hash = $$GITHASHSTRING")
       DEFINES += GITHASH=\\\"$$GITHASHSTRING\\\"
   }
}
isEmpty(GITHASHSTRING) {
    DEFINES += GITHASH=\\\"------\\\"
}

INCLUDEPATH	+= /usr/local/include
INCLUDEPATH     += /usr/local/include /usr/include/qt4/qwt /usr/include/qt5/qwt /usr/include/qt4/qwt /usr/include/qwt /usr/local/qwt-6.1.4-svn/
LIBS		+= -lfftw3  -lusb-1.0 -ldl  #
LIBS		+= -lz
#correct this for the correct path to the qwt6 library on your system
#LIBS           += -lqwt
LIBS            += -lqwt-qt5

#
# comment or uncomment for the devices you want to have support for
# (you obviously have libraries installed for the selected ones)
CONFIG		+= dabstick
CONFIG		+= sdrplay_v2
CONFIG		+= sdrplay_v3
CONFIG		+= airspy
CONFIG		+= hackrf
CONFIG		+= lime
#For x64 linux system uncomment SSE
#For any other system comment SSE out and uncomment NO_SSE
#CONFIG += SSE
CONFIG  += NO_SSE
}
#
# an attempt to have it run under W32 through cross compilation
win32 {
DESTDIR		= ../../windows-scanner
# includes in mingw differ from the includes in fedora linux

exists ("./.git") {
   GITHASHSTRING = $$system(git rev-parse --short HEAD)
   !isEmpty(GITHASHSTRING) {
       message("Current git hash = $$GITHASHSTRING")
       DEFINES += GITHASH=\\\"$$GITHASHSTRING\\\"
   }
}
isEmpty(GITHASHSTRING) {
    DEFINES += GITHASH=\\\"------\\\"
}

INCLUDEPATH += /usr/i686-w64-mingw32/sys-root/mingw/include
INCLUDEPATH += /usr/i686-w64-mingw32/sys-root/mingw/include/qt5/qwt
INCLUDEPATH	+= /mingw32/include
INCLUDEPATH	+=/usr/local/include
#INCLUDEPATH	+= /mingw32/include/qwt
LIBS		+= -L/usr/i686-w64-mingw32/sys-root/mingw/lib
LIBS		+= -lfftw3
LIBS		+= -lole32
LIBS		+= -lwinpthread
LIBS		+= -lwinmm
LIBS 		+= -lstdc++
LIBS		+= -lws2_32
LIBS		+= -lusb-1.0
LIBS		+= -lz
LIBS		+= -lqwt-qt5
#
#	devices
CONFIG		+= airspy
CONFIG		+= dabstick
CONFIG		+= sdrplay_v2
CONFIG		+= sdrplay_v3
CONFIG		+= hackrf
CONFIG		+= NO_SSE
}

#	devices
#
#	dabstick
dabstick {
	DEFINES         += HAVE_RTLSDR
        DEPENDPATH      += ./devices/rtlsdr-handler
        INCLUDEPATH     += ./devices/rtlsdr-handler
        HEADERS         += ./devices/rtlsdr-handler/rtlsdr-handler.h \
                           ./devices/rtlsdr-handler/rtl-dongleselect.h
        SOURCES         += ./devices/rtlsdr-handler/rtlsdr-handler.cpp \
                           ./devices/rtlsdr-handler/rtl-dongleselect.cpp
        FORMS           += ./devices/rtlsdr-handler/rtlsdr-widget.ui
}
#
#	the SDRplay: using the "old" v2.13 library
#
sdrplay_v2 {
	DEFINES         += HAVE_SDRPLAY_V2
        DEPENDPATH      += ./devices/sdrplay-handler-v2
        INCLUDEPATH     += ./devices/sdrplay-handler-v2
        HEADERS         += ./devices/sdrplay-handler-v2/sdrplay-handler-v2.h \
                           ./devices/sdrplay-handler-v2/sdrplayselect.h
        SOURCES         += ./devices/sdrplay-handler-v2/sdrplay-handler-v2.cpp \
                           ./devices/sdrplay-handler-v2/sdrplayselect.cpp
        FORMS           += ./devices/sdrplay-handler-v2/sdrplay-widget-v2.ui
}
#
#	the SDRplay: using the new v3.xx library
sdrplay_v3 {
        DEFINES         += HAVE_SDRPLAY_V3
        DEPENDPATH      += ./devices/sdrplay-handler-v3
        INCLUDEPATH     += ./devices/sdrplay-handler-v3
        HEADERS         += ./devices/sdrplay-handler-v3/sdrplay-handler-v3.h \
                           ./devices/sdrplay-handler-v3/sdrplay-commands.h
        SOURCES         += ./devices/sdrplay-handler-v3/sdrplay-handler-v3.cpp
        FORMS           += ./devices/sdrplay-handler-v3/sdrplay-widget-v3.ui
}

#
# airspy support
#
airspy {
	DEFINES		+= HAVE_AIRSPY
	DEPENDPATH	+= ./devices/airspy 
	INCLUDEPATH	+= ./devices/airspy-handler \
	                   ./devices/airspy-handler/libairspy
	HEADERS		+= ./devices/airspy-handler/airspy-handler.h \
	                   ./devices/airspy-handler/airspyfilter.h \
	                   ./devices/airspy-handler/libairspy/airspy.h
	SOURCES		+= ./devices/airspy-handler/airspy-handler.cpp \
	                   ./devices/airspy-handler/airspyfilter.cpp
	FORMS           += ./devices/airspy-handler/airspy-widget.ui

}
#
#       the HACKRF One
#
hackrf {
        DEFINES         += HAVE_HACKRF
        DEPENDPATH      += ./devices/hackrf-handler
        INCLUDEPATH     += ./devices/hackrf-handler
        HEADERS         += ./devices/hackrf-handler/hackrf-handler.h
        SOURCES         += ./devices/hackrf-handler/hackrf-handler.cpp
	FORMS           += ./devices/hackrf-handler/hackrf-widget.ui
}
#
# lime
#
lime  {
        DEFINES         += HAVE_LIME
        INCLUDEPATH     += ./devices/lime-handler
        HEADERS         += ./devices/lime-handler/lime-handler.h
        SOURCES         += ./devices/lime-handler/lime-handler.cpp
        FORMS           += ./devices/lime-handler/lime-widget.ui
}
NO_SSE  {
        HEADERS         += ./src/support/viterbi-spiral/spiral-no-sse.h
        SOURCES         += ./src/support/viterbi-spiral/spiral-no-sse.c
}

SSE     {
        DEFINES         += SSE_AVAILABLE
        HEADERS         += ./src/support/viterbi-spiral/spiral-sse.h
        SOURCES         += ./src/support/viterbi-spiral/spiral-sse.c
}

