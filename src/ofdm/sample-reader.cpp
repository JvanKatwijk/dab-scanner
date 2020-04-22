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
 */
#
#include	"sample-reader.h"
#include	"radio.h"

static  inline
int16_t valueFor (int16_t b) {
int16_t res     = 1;
        while (--b > 0)
           res <<= 1;
        return res;
}

	sampleReader::sampleReader (RadioInterface *mr,
	                            deviceHandler   *theRig,
	                            RingBuffer<std::complex <double>> *spectrumBuffer) {
int	i;
	this	-> theRig	= theRig;
        bufferSize		= 32768;
        localBuffer. resize (bufferSize);
	localCounter		= 0;
	this    -> spectrumBuffer       = spectrumBuffer;
        connect (this, SIGNAL (show_Spectrum (int)),
                 mr, SLOT (showSpectrum (int)));

	
        connect (this, SIGNAL (show_Corrector (int)),
                 mr, SLOT (set_CorrectorDisplay (int)));
	currentPhase	= 0;
	sLevel		= 0;
	sampleCount	= 0;
	oscillatorTable = new std::complex<double> [INPUT_RATE];
        for (i = 0; i < INPUT_RATE; i ++)
           oscillatorTable [i] = std::complex<double>
	                            (cos (2.0 * M_PI * i / INPUT_RATE),
                                     sin (2.0 * M_PI * i / INPUT_RATE));

	bufferContent	= 0;
	corrector	= 0;
}

	sampleReader::~sampleReader (void) {
	delete[] oscillatorTable;
}

void	sampleReader::setRunning (bool b) {
	running. store (b);
}

double	sampleReader::get_sLevel (void) {
	return sLevel;
}

std::complex<double> sampleReader::getSample (int32_t phaseOffset) {
std::complex<float> temp;
std::complex<double> dtemp;
	corrector	= phaseOffset;
	if (!running. load ()) 
	   throw 21;

///	bufferContent is an indicator for the value of ... -> Samples ()
	if (bufferContent == 0) {
	   bufferContent = theRig -> Samples ();
	   while ((bufferContent <= 2048) && running. load ()) {
	      usleep (10);
	      bufferContent = theRig -> Samples (); 
	   }
	}

	if (!running. load ())	
	   throw 20;
//
//	so here, bufferContent > 0
	theRig -> getSamples (&temp, 1);
	dtemp	= temp;
	bufferContent --;
//	OK, we have a sample!!
	if (localCounter < bufferSize)
	   localBuffer [localCounter ++] = dtemp;
//	first: adjust frequency. We need Hz accuracy
	currentPhase	-= phaseOffset;
	currentPhase	= (currentPhase + INPUT_RATE) % INPUT_RATE;

	dtemp		*= oscillatorTable [currentPhase];
	sLevel		= 0.00001 * abs (temp) + (1 - 0.00001) * sLevel;
#define	N	5
	if (++ sampleCount > INPUT_RATE / N) {
           show_Corrector       (corrector);
           sampleCount = 0;
           spectrumBuffer -> putDataIntoBuffer (localBuffer. data(),
                                                               localCounter);
           emit show_Spectrum (bufferSize);
           localCounter = 0;
	}
	return dtemp;
}

void	sampleReader::getSamples (std::complex<double>  *v,
	                          int16_t n, int32_t phaseOffset) {
int32_t		i;
std::complex<float> temp [n];

	corrector	= phaseOffset;
	if (!running. load ())
	   throw 21;
	if (n > bufferContent) {
	   bufferContent = theRig -> Samples ();
	   while ((bufferContent < n) && running. load ()) {
	      usleep (10);
	      bufferContent = theRig -> Samples ();
	   }
	}

	if (!running. load ())	
	   throw 20;
//
//	so here, bufferContent >= n
	n	= theRig -> getSamples (temp, n);
	if (!running. load ())	
	   throw 20;
	bufferContent -= n;
//	OK, we have samples!!
//	first: adjust frequency. We need Hz accuracy
	for (i = 0; i < n; i ++) {
	   if (localCounter < bufferSize)
	      localBuffer [localCounter ++] = temp [i];
	   currentPhase	-= phaseOffset;
//
//	Note that "phase" itself might be negative
	   currentPhase	= (currentPhase + INPUT_RATE) % INPUT_RATE;
	   sLevel	= 0.00001 * abs (temp [i]) + (1 - 0.00001) * sLevel;
	   v [i]	= temp [i];
	   v [i]	*= oscillatorTable [currentPhase];
	}
	sampleCount	+= n;
	if (sampleCount > INPUT_RATE / N) {
	   show_Corrector	(corrector);
	   spectrumBuffer -> putDataIntoBuffer (localBuffer. data(),
                                                               localCounter);
	   emit show_Spectrum (bufferSize);
           localCounter = 0;
	   sampleCount = 0;
	}
}

