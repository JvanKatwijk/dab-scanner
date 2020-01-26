#
/*
 *    Copyright (C) 2017 .. 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the dab-scanner
 *
 *    dab-scanner is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    dab-scanner is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with dab-scanner; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _COMMON_FFT
#define _COMMON_FFT

//  Simple wrapper around fftw
#include	"dab-constants.h"
#include	"dab-params.h"

#define FFTW_MALLOC		fftw_malloc
#define FFTW_PLAN_DFT_1D	fftw_plan_dft_1d
#define FFTW_DESTROY_PLAN	fftw_destroy_plan
#define FFTW_FREE		fftw_free
#define FFTW_PLAN		fftw_plan
#define FFTW_EXECUTE		fftw_execute
#include    <fftw3.h>

/*
 *  a simple wrapper
 */

class   fftHandler {
public:
		fftHandler	(uint8_t);
        	~fftHandler	(void);
        std::complex<double>  *getVector (void);
        void		do_FFT (void);
        void		do_IFFT (void);
    private:
	dabParams	p;
        int32_t		fftSize;
        std::complex<double>  *vector;
        FFTW_PLAN   plan;
};
#endif

