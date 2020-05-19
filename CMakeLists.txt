cmake_minimum_required( VERSION 2.8.11 )
set (objectName dab-scanner-1.8)
set (CMAKE_CXX_FLAGS "${CMAKE_XCC_FLAGS} -Wall -std=c++11 -flto")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")

set(CMAKE_AUTORCC ON)
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "/usr/local/bin" CACHE PATH "default install path" FORCE )
endif()
#set (CMAKE_INSTALL_PREFIX /usr/local/bin)

#	modify if you want

if(MINGW)
    add_definitions ( -municode)
#else (MINGW)
#	add_definitions (-DSSE_AVAILABLE)
endif()

IF(EXISTS (".git") AND IS_DIRECTORY (".git"))
   execute_process (
      COMMAND git rev-parse --short HEAD
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      OUTPUT_VARIABLE GIT_COMMIT_HASH
      OUTPUT_STRIP_TRAILING_WHITESPACE
   )
endif ()

if(GIT_COMMIT_HASH)
   add_definitions("-DGITHASH=\"${GIT_COMMIT_HASH}\"")
else ()
   add_definitions ("-DGITHASH=\"       \"")
endif()

########################################################################
# select the release build type by default to get optimization flags
########################################################################
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
   message(STATUS "Build type not specified: defaulting to release.")
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")

### make sure our local CMake Modules path comes first
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake/Modules)

######################################################################
#
#####################################################################
#
if(DEFINED SDRPLAY)
   set(SDRPLAY true)
endif ()

if(DEFINED SDRPLAY_V3)
   set(SDRPLAY_V3 true)
endif ()

if(DEFINED RTLSDR)
   set(RTLSDR true)
endif ()

if(DEFINED RTL_TCP)
   set(RTL_TCP true)
endif ()

if(DEFINED AIRSPY)
   set(AIRSPY true)
endif ()

if(DEFINED HACKRF)
	set(HACKRF true)
endif ()

if(DEFINED LIMESDR)
   set(LIMESDR true)
endif ()

if (DEFINED VITERBI_SSE)
   set(VITERBI_SSE true)
endif ()

#
########################################################################

	find_package (PkgConfig)
	find_package (LSBId)

	find_package (Qt5Core REQUIRED)
	find_package (Qt5Widgets REQUIRED)
	find_package (Qt5Xml REQUIRED)
	include_directories (
	  ${Qt5Network_INCLUDE_DIRS}
	  ${Qt5Xml_INCLUDE_DIRS}
	)

	find_package(Qwt)
	if (NOT QWT_FOUND)
	   message(FATAL_ERROR "please install qwt library")
	endif ()
	include_directories ( ${QWT_INCLUDE_DIRS})
	list(APPEND extraLibs ${QWT_LIBRARIES})

        find_package(FFTW3)
        if (NOT FFTW3_FOUND)
            message(FATAL_ERROR "please install FFTW3")
        endif ()

	find_package(LibSndFile)
        if (NOT LIBSNDFILE_FOUND)
            message(FATAL_ERROR "please install libsndfile")
        endif ()
        list(APPEND extraLibs ${LIBSNDFILE_LIBRARY})

	find_library (PTHREADS pthread)
	if (NOT(PTHREADS))
	   message (FATAL_ERROR "please install libpthread")
	else (NOT(PTHREADS))
	   set (extraLibs ${extraLibs} ${PTHREADS})
	endif (NOT(PTHREADS))

#######################################################################
#
#	Here we really start

	include_directories (
	           ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	           ${QT5Widgets_INCLUDES}
	           ${QT_QTCORE_INCLUDE_DIR}
	           ${QT_QTGUI_INCLUDE_DIR}
	           .
	           ./
	           ./service-description
	           ./includes
	           ./includes/ofdm
	           ./includes/protection
	           ./includes/support
	           ./includes/support/viterbi-spiral
	           ./includes/scopes-qwt6
	           ./devices
	           ./spectrum-viewer
	           /usr/include/
	)

	set (${objectName}_HDRS
	     ./dab-processor.h
	     ./channels-table.h 
	     ./output-table.h 
	     ./includes/dab-constants.h
             ./includes/country-codes.h
	     ./includes/ofdm/timesyncer.h
             ./includes/ofdm/sample-reader.h
	     ./includes/ofdm/phasereference.h
	     ./includes/ofdm/ofdm-decoder.h
	     ./includes/ofdm/phasetable.h
	     ./includes/ofdm/freq-interleaver.h
	     ./includes/ofdm/fib-decoder.h
	     ./includes/ofdm/dab-config.h
	     ./includes/ofdm/fib-table.h
	     ./includes/ofdm/fic-handler.h
	     ./includes/ofdm/tii_detector.h
	     ./includes/protection/protTables.h
#	     ./includes/protection/protection.h
#	     ./includes/protection/uep-protection.h
#	     ./includes/protection/eep-protection.h
             ./includes/support/fft-handler.h 
	     ./includes/support/ringbuffer.h 
	     ./includes/support/dab-params.h 
	     ./includes/support/band-handler.h 
	     ./includes/support/dab_tables.h 
	     ./includes/support/charsets.h 
	     ./includes/support/ensemble-printer.h 
	     ./includes/support/viterbi-spiral/viterbi-spiral.h
	     ./devices/device-handler.h 
	     ./includes/scopes-qwt6/spectrogramdata.h 
             ./includes/scopes-qwt6/iqdisplay.h 
	     ./spectrum-viewer/spectrum-viewer.h 
	)

	set (${objectName}_SRCS
	     ${${objectName}_SRCS}
	     main.cpp
	     ./radio.cpp 
	     ./dab-processor.cpp 
	     ./channels-table.cpp 
	     ./output-table.cpp 
	     ./src/ofdm/timesyncer.cpp 
	     ./src/ofdm/sample-reader.cpp 
	     ./src/ofdm/ofdm-decoder.cpp 
	     ./src/ofdm/phasereference.cpp 
	     ./src/ofdm/phasetable.cpp 
	     ./src/ofdm/freq-interleaver.cpp 
	     ./src/ofdm/fic-handler.cpp 
	     ./src/ofdm/fib-decoder.cpp  
	     ./src/ofdm/tii_detector.cpp 
	     ./src/protection/protTables.cpp 
#	     ./src/protection/protection.cpp 
#	     ./src/protection/eep-protection.cpp 
#	     ./src/protection/uep-protection.cpp 
             ./src/support/fft-handler.cpp 
	     ./src/support/dab-params.cpp 
	     ./src/support/band-handler.cpp 
	     ./src/support/dab_tables.cpp 
	     ./src/support/charsets.cpp 
	     ./src/support/ensemble-printer.cpp 
	     ./src/support/viterbi-spiral/viterbi-spiral.cpp
	     ./src/support/text-mapper.cpp 
	     ./devices/device-handler.cpp 
	     ./src/scopes-qwt6/iqdisplay.cpp 
	     ./spectrum-viewer/spectrum-viewer.cpp 
	)

	set (${objectName}_MOCS
	     ./dab-processor.h
	     ./channels-table.h 
	     ./output-table.h 
	     ./includes/ofdm/sample-reader.h
	     ./includes/ofdm/ofdm-decoder.h
	     ./includes/ofdm/phasereference.h
	     ./includes/ofdm/fib-decoder.h
	     ./includes/ofdm/fic-handler.h
	     ./includes/ofdm/tii_detector.h
	     ./spectrum-viewer/spectrum-viewer.h 
	     ./includes/scopes-qwt6/iqdisplay.h
	)

	set (${objectName}_UIS
	        ${${objectName}_UIS}
	        ./forms/dabscanner.ui
	        ./spectrum-viewer/scopewidget.ui
	   )

	set (${objectName}_MOCS
	        ${${objectName}_MOCS} ./radio.h)

	set ($(objectName)_HDRS
	        ${${objectName}_HDRS} ./radio.h)

	set (${objectName}_SRCS
	        ${${objectName}_SRCS} ./radio.cpp)

	if(VITERBI_SSE)
	   set ($(objectName)_HDRS
	        ${${objectName}_HDRS}
	        ./src/support/viterbi-spiral/spiral-sse.h
	   )
	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./src/support/viterbi-spiral/spiral-sse.c
	   )
	   add_definitions (-DSSE_AVAILABLE)
	else (VITERBI_SSE)
	   set ($(objectName)_HDRS
	        ${${objectName}_HDRS}
	        ./src/support/viterbi-spiral/spiral-no-sse.h
	   )
	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./src/support/viterbi-spiral/spiral-no-sse.c
	   )
	endif (VITERBI_SSE)

#########################################################################
#	The devices
#
	if (SDRPLAY)
	   set (${objectName}_UIS
	        ${${objectName}_UIS}
	        ./devices/sdrplay-handler-v2/sdrplay-widget-v2.ui
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/sdrplay-handler-v2/sdrplay-handler-v2.h
	        ./devices/sdrplay-handler-v2/sdrplayselect.h
	   )

	   include_directories (
	     ./devices/sdrplay-handler-v2
	   )

	   set ($(objectName)_HDRS
	        ${${objectName}_HDRS}
	        ./devices/sdrplay-handler-v2/sdrplay-handler-v2.h
	        ./devices/sdrplay-handler-v2/sdrplayselect.h
	        ./devices/sdrplay-handler-v2/mirsdrapi-rsp.h
           )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./devices/sdrplay-handler-v2/sdrplay-handler-v2.cpp
	        ./devices/sdrplay-handler-v2/sdrplayselect.cpp
	   )

	 add_definitions (-DHAVE_SDRPLAY_V2)
	endif (SDRPLAY)

	if (SDRPLAY_V3)
	   set (${objectName}_UIS
	        ${${objectName}_UIS}
	        ./devices/sdrplay-handler-v3/sdrplay-widget-v3.ui
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/sdrplay-handler-v3/sdrplay-handler-v3.h
	   )

	   include_directories (
	     ./devices/sdrplay-handler-v3
	   )

	   set ($(objectName)_HDRS
	        ${${objectName}_HDRS}
	        ./devices/sdrplay-handler-v3/sdrplay-handler-v3.h
	        ./devices/sdrplay-handler-v3/sdrplay-commands.h
	        ./devices/sdrplay-handler-v3/sdrplay_api.h
           )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./devices/sdrplay-handler-v3/sdrplay-handler-v3.cpp
	   )

	 add_definitions (-DHAVE_SDRPLAY_V3)
	endif (SDRPLAY_V3)

	if (AIRSPY)
#           find_package(LibAIRSPY)
#           if (NOT LIBAIRSPY_FOUND)
#               message(FATAL_ERROR "please install airspy library")
#           endif ()
#	   ## include_directories (${AIRSPYLIB_INCLUDE_DIR})

	   set (${objectName}_UIS
	        ${${objectName}_UIS}
	        ./devices/airspy-handler/airspy-widget.ui
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/airspy-handler/airspy-handler.h
	   )

	   include_directories (
	     ./devices/airspy-handler
	     ./devices/airspy-handler/libairspy
	   )

	   set ($(objectName)_HDRS
	        ${${objectName}_HDRS}
	        ./devices/airspy-handler/airspy-handler.h
	        ./devices/airspy-handler/airspyfilter.h
	        ./devices/airspy-handler/libairspy/airspy.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./devices/airspy-handler/airspy-handler.cpp
	        ./devices/airspy-handler/airspyfilter.cpp
	   )

	   add_definitions (-DHAVE_AIRSPY)
	endif (AIRSPY)

	if (HACKRF)
	   set (${objectName}_UIS
	        ${${objectName}_UIS}
	        ./devices/hackrf-handler/hackrf-widget.ui
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/hackrf-handler/hackrf-handler.h
	   )

	   include_directories (
	     ./devices/hackrf-handler
	     ./devices/hackrf-handler/libhackrf
	   )

	   set ($(objectName)_HDRS
	        ${${objectName}_HDRS}
	        ./devices/hackrf-handler/hackrf-handler.h
	        ./devices/hackrf-handler/libhackrf/hackrf.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./devices/hackrf-handler/hackrf-handler.cpp
	   )

	   add_definitions (-DHAVE_HACKRF)
	endif (HACKRF)
#
	if (LIMESDR)
	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/lime-handler/lime-handler.h
	        ./devices/lime-handler/lime-widget.h
	   )

	   include_directories (
	     ./devices/lime-handler
	   )

	   set ($(objectName)_HDRS
	        ${${objectName}_HDRS}
	        ./devices/lime-handler/lime-handler.h
	        ./devices/lime-handler/lime-widget.h
	        ./devices/lime-handler/LimeSuite.h
	        ./devices/lime-handler/LMS7002M_parameters.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./devices/lime-handler/lime-handler.cpp
	   )

	   add_definitions (-DHAVE_LIME)
	endif (LIMESDR)

	if (RTLSDR)
	   set (${objectName}_UIS
                ${${objectName}_UIS}
                ./devices/rtlsdr-handler/rtlsdr-widget.ui
           )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/rtlsdr-handler/rtl-dongleselect.h
	        ./devices/rtlsdr-handler/rtlsdr-handler.h
	   )

	   include_directories (
	        ./devices/rtlsdr-handler/
	   )

	   set (${objectName}_HDRS
	        ${${objectName}_HDRS}
	        ./devices/rtlsdr-handler/rtlsdr-handler.h
	        ./devices/rtlsdr-handler/rtl-dongleselect.h
	        ./devices/rtlsdr-handler/rtl-sdr.h
	        ./devices/rtlsdr-handler/rtl-sdr_export.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./devices/rtlsdr-handler/rtlsdr-handler.cpp
	        ./devices/rtlsdr-handler/rtl-dongleselect.cpp
	   )

	   add_definitions (-DHAVE_RTLSDR)
	endif()

#
	if (RTL_TCP)
	   find_package (Qt5Network REQUIRED)
	   set (${objectName}_UIS
	        ${${objectName}_UIS}
	        ./devices/rtl_tcp/rtl_tcp-widget.ui
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/rtl_tcp/rtl_tcp_client.h
	   )

	   include_directories (
	      ./devices/rtl_tcp
	      ${Qt5Network_INCLUDE_DIRS}
	   )

	   set ($(objectName)_HDRS
	        ${${objectName}_HDRS}
	        ./devices/rtl_tcp/rtl_tcp_client.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./devices/rtl_tcp/rtl_tcp_client.cpp
	   )

	   set (RTLTCP_lib Qt5::Network)
	   add_definitions (-DHAVE_RTL_TCP)
	endif (RTL_TCP)

	if (DATA_STREAMER)
	   include_directories (
	      ./server-thread
	   )

	   set ($(objectName)_HDRS
	        ${${objectName}_HDRS}
	             ./server-thread/tcp-server.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	             ./server-thread/tcp-server.cpp
	   )
	   add_definitions (-DDATA_STREAMER)
	endif (DATA_STREAMER)

	include_directories (
	          ${SDRPLAY_INCLUDES}
	          ${QT5Widgets_INCLUDES}
	          ${QT_QTCORE_INCLUDE_DIR}
	          ${QT_QTGUI_INCLUDE_DIR}
	          ${FFTW_INCLUDE_DIRS}
	          ${SNDFILES_INCLUDE_DIRS}
	)

	QT5_WRAP_UI (UIS ${${objectName}_UIS})

	QT5_WRAP_CPP (MOCS ${${objectName}_MOCS})

	add_executable (${objectName}
	                ${${objectName}_SRCS}
	                ${UIS}
	                ${RSCS}
	                ${TRS}
	                ${MOCS}
	                resources.qrc
	)

	target_link_libraries (${objectName}
	                       Qt5::Widgets
	                       Qt5::Xml
	                       ${RTLTCP_lib}
	                       ${FFTW3_LIBRARIES}
	                       ${extraLibs}
	                       ${CMAKE_DL_LIBS}
	)

	INSTALL (TARGETS ${objectName} DESTINATION  ${CMAKE_INSTALL_PREFIX})
#	INSTALL (TARGETS ${objectName} DESTINATION ./linux-bin)

########################################################################
# Create uninstall target
########################################################################

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
