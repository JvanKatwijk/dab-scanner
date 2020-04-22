#
/*
 *    Copyright (C) 2013 .. 2017
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
 *
 *	Once the bits are "in", interpretation and manipulation
 *	should reconstruct the data blocks.
 *	Ofdm_decoder is called once every Ts samples, and
 *	its invocation results in 2 * Tu bits
 */
#include	<vector>
#include	"ofdm-decoder.h"
#include	"radio.h"
#include	"phasetable.h"
#include	"fic-handler.h"
#include	"freq-interleaver.h"
#include	"dab-params.h"

/**
  *	\brief ofdmDecoder
  *	The class ofdmDecoder is
  *	taking the data from the ofdmProcessor class in, and
  *	will extract the Tu samples, do an FFT and extract the
  *	carriers and map them on (soft) bits
  *	In the threaded version, a thread will run, fetch the
  *	data blocks asynchronously and do all computations.
  *	Threading is needed for e.g. the RPI 2 version
  *	In the non-threaded version, the functions in the class
  *	are just executed in the caller's thread
  */
	ofdmDecoder::ofdmDecoder	(RadioInterface *mr,
	                                 uint8_t	dabMode,
	                                 int16_t	bitDepth,
	                                 RingBuffer<std::complex<double>> *iqBuffer):
	                                    params (dabMode),
	                                    my_fftHandler (dabMode),
	                                    myMapper (dabMode) {
	this	-> myRadioInterface	= mr;
	this    -> iqBuffer             = iqBuffer;
	(void)bitDepth;
        connect (this, SIGNAL (showIQ (int)),
                 myRadioInterface, SLOT (showIQ (int)));
        connect (this, SIGNAL (showQuality (double)),
                 myRadioInterface, SLOT (showQuality (double)));

	this	-> T_s			= params. get_T_s ();
	this	-> T_u			= params. get_T_u ();
	this	-> nrBlocks		= params. get_L ();
	this	-> carriers		= params. get_carriers ();

	this	-> T_g			= T_s - T_u;
	fft_buffer			= my_fftHandler. getVector ();
	phaseReference			.resize (T_u);
}

	ofdmDecoder::~ofdmDecoder	(void) {
}
//
//
void    ofdmDecoder::stop       (void) {
}

void    ofdmDecoder::reset      (void) {
}

/**
  */
void	ofdmDecoder::
	       processBlock_0 (std::vector <std::complex<double> > buffer) {
	for (int i = 0; i < T_u; i ++)
	   fft_buffer [i] = buffer [i];

	my_fftHandler. do_FFT ();
/**
  *	we are now in the frequency domain, and we keep the carriers
  *	as coming from the FFT as phase reference.
  */
	for (int i = 0; i < T_u; i ++)
	   phaseReference [i] = fft_buffer [i];
}
//
//	Just interested. In the ideal case the constellation of the
//	decoded symbols is precisely in the four points 
//	k * (1, 1), k * (1, -1), k * (-1, -1), k * (-1, 1)
//	To ease computation, we map all incoming values onto quadrant 1
double	ofdmDecoder::computeQuality (std::complex<double> *v) {
int16_t i;
std::complex<double>	avgPoint	= std::complex<double> (0, 0);
std::complex<double>	x [T_u];
double	avg	= 0;
double	S	= 0;

	for (i = 0; i < carriers; i ++) {
	   x [i]	= std::complex<float> (abs (real (v [T_u / 2 - carriers / 2 + i])), abs (imag (v [T_u / 2 - carriers / 2 + i])));
	   avgPoint	+= x [i];
	}

	avg	= arg (avgPoint * conj (std::complex<double> (1, 1)));

	for (i = 0; i < carriers; i ++) {
	   float f = arg (x [i] * conj (std::complex<double> (1, 1))) - avg;
	   f = f / M_PI * 360;
	   S += f * f;
	}

	S /= (carriers - 1);
	
	return sqrt (S);
}

/**
  *	for the other blocks of data, the first step is to go from
  *	time to frequency domain, to get the carriers.
  *	we distinguish between FIC blocks and other blocks,
  *	only to spare a test. The mapping code is the same
  */

static	int	cnt	= 0;
void	ofdmDecoder::decode (std::vector <std::complex<double>> buffer,
	                     int32_t blkno, int16_t *ibits) {
int16_t	i;
std::complex<double> conjVector [T_u];

	for (i = 0; i < T_u; i ++)
	   fft_buffer [i] = buffer [T_g + i];

//fftlabel:
/**
  *	first step: do the FFT
  */
	my_fftHandler. do_FFT ();
/**
  *	a little optimization: we do not interchange the
  *	positive/negative frequencies to their right positions.
  *	The de-interleaving understands this
  */
//toBitsLabel:
/**
  *	Note that from here on, we are only interested in the
  *	"carriers", the useful carriers of the FFT output
  */
	for (i = 0; i < carriers; i ++) {
	   int16_t	index	= myMapper.  mapIn (i);
	   if (index < 0) 
	      index += T_u;
/**
  *	decoding is computing the phase difference between
  *	carriers with the same index in subsequent blocks.
  *	The carrier of a block is the reference for the carrier
  *	on the same position in the next block
  */
	   std::complex<double>	r1 = fft_buffer [index] *
	                                    conj (phaseReference [index]);
	   conjVector [index] = r1;
	   double ab1	= abs (r1);
//	split the real and the imaginary part and scale it
//	we make the bits into softbits in the range -127 .. 127
	   ibits [i]		= (int16_t)( - real (r1) / ab1 * 2048.0);
	   ibits [carriers + i] = (int16_t)( - imag (r1) / ab1 * 2048.0);
	}

	for (i = 0; i < T_u; i ++)
	   phaseReference [i] = fft_buffer [i];

//	From time to time we show the constellation of symbol 2.
        if (blkno == 2) {
           if (++cnt > 7) {
              iqBuffer  -> putDataIntoBuffer (&conjVector [T_u / 2 - carriers / 2],
                                              carriers);
              showIQ    (carriers);
              showQuality (computeQuality (conjVector));
              cnt = 0;
           }
        }
}

