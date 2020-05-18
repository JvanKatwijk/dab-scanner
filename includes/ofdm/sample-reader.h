#
/*
 *    Copyright (C) 2013 .. 2017
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
#
#ifndef	__SAMPLE_READER__
#define	__SAMPLE_READER__
/*
 *	Reading the samples from the input device. Since it has its own
 *	"state", we embed it into its own class
 */
#include	"dab-constants.h"
#include	<QObject>
#include	<stdint.h>
#include	<atomic>
#include	<vector>
#include	"device-handler.h"
#include	"ringbuffer.h"
//

class	RadioInterface;
class	sampleReader : public QObject {
Q_OBJECT
public:
			sampleReader	(RadioInterface *mr,
	                   	 deviceHandler *theRig,
	                           RingBuffer<std::complex<double>> *spectrumBuffer);

			~sampleReader		(void);
		void	setRunning	(bool b);
		double	get_sLevel	(void);
		std::complex<double>	getSample	(int32_t);
	  void	getSamples	(std::complex<double> *v,
	                           int16_t n, int32_t phase);
private:
	RadioInterface	*myRadioInterface;
	deviceHandler	*theRig;
	RingBuffer<std::complex<double>> *spectrumBuffer;
	std::vector<std::complex<double>> localBuffer;
	int32_t		localCounter;
	int32_t		bufferSize;
	int32_t		currentPhase;
	std::atomic<bool>	running;
	int32_t		bufferContent;
	double		sLevel;
	int32_t		sampleCount;
	int32_t		corrector;
signals:
	void		show_Corrector	(int);
	void		show_Spectrum	(int);
};

#endif
