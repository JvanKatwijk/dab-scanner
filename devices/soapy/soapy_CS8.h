#
/*
 *    Copyright (C) 2013, 2014, 2015, 2016, 2017, 2018, 2019
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the dab-scanner
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

#ifndef	SOAPY_CS8
#define	SOAPY_CS8

#include	<SoapySDR/Device.hpp>
#include	<SoapySDR/Formats.hpp>
#include        <SoapySDR/Errors.hpp>
#include	<stdio.h> //printf
#include	<stdlib.h> //free
#include	<complex.h>

#include	"soapy-worker.h";


class	soapy_CS8: public soapyWorker {
public:
	soapy_CS8	(SoapySDR::Device *);
	~soapy_CS8	(void);
int	Samples		(void);
int	getSamples	(std::complex<float> *, int);
void	run		(void);
private:
SoapySDR::Device	*theDevice;
SoapySDR::Stream	*stream;
RingBuffer<int8_t> *theBuffer;
bool	running;
};

#endif

