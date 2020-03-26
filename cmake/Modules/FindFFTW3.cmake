# http://tim.klingt.org/code/projects/supernova/repository/revisions/d336dd6f400e381bcfd720e96139656de0c53b6a/entry/cmake_modules/FindFFTW3f.cmake
# Modified to use pkg config and use standard var names

# Find double-precision (float) version of FFTW3

INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_FFTW3F "fftw3 >= 3.0")

FIND_PATH(
    FFTW3_INCLUDE_DIRS
    NAMES fftw3.h
    HINTS $ENV{FFTW3_DIR}/include
        ${PC_FFTW3_INCLUDE_DIR}
    PATHS /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    FFTW3_LIBRARIES
    NAMES fftw3 libfftw3
    HINTS $ENV{FFTW3_DIR}/lib
        ${PC_FFTW3_LIBDIR}
    PATHS /usr/local/lib
          /usr/lib
          /usr/lib64
)

FIND_LIBRARY(
    FFTW3_THREADS_LIBRARIES
    NAMES fftw3_threads libfftw3_threads
    HINTS $ENV{FFTW3_DIR}/lib
        ${PC_FFTW3_LIBDIR}
    PATHS /usr/local/lib
          /usr/lib
          /usr/lib64
)


INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FFTW3 DEFAULT_MSG FFTW3_LIBRARIES FFTW3_INCLUDE_DIRS)
MARK_AS_ADVANCED(FFTW3_LIBRARIES FFTW3_INCLUDE_DIRS FFTW3_THREADS_LIBRARIES)
