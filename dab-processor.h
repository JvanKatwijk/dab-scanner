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
#ifndef	__DAB_PROCESSOR__
#define	__DAB_PROCESSOR__
/*
 *	dabProcessor is the embodying of all functionality related
 *	to the actal DAB processing.
 */
#include	"dab-constants.h"
#include	<QThread>
#include	<QObject>
#include	<vector>
#include	"stdint.h"
#include	<sndfile.h>
#include	<ringbuffer.h>
#include	"sample-reader.h"
#include	"phasereference.h"
#include	"ofdm-decoder.h"
#include	"fic-handler.h"
#include	"device-handler.h"
#include	"tii_detector.h"

class	RadioInterface;
class	dabParams;

class dabProcessor: public QThread {
Q_OBJECT
public:
		dabProcessor  	(RadioInterface *,
	                         deviceHandler *,
	                         uint8_t,
	                         int16_t,
	                         int16_t,
	                         RingBuffer<std::complex<float>> *,
	                         RingBuffer<std::complex<float>> *);
		~dabProcessor	(void);
	void		reset			(void);
	void		start			(int);
	void		stop			(void);
	void		setOffset		(int32_t);
	void		coarseCorrectorOn	(void);
	void		coarseCorrectorOff	(void);
        int32_t		get_ensembleId          (void);
        QString		get_ensembleName        (void);
	bool            is_audioService         (QString &s);
        bool            is_packetService        (QString &s);
        void            dataforAudioService     (QString &,
                                                     audiodata *, int16_t);
        void            dataforPacketService    (QString &,
                                                     packetdata *, int16_t);
        void            set_audioChannel        (audiodata *,
                                                     RingBuffer<int16_t> *);
        void            set_dataChannel         (packetdata *,
                                                     RingBuffer<uint8_t> *);
        uint8_t         get_ecc();
	void		clearEnsemble		(void);
private:
	deviceHandler	*theDevice;
	dabParams	params;
	int16_t         tii_delay;
	int16_t         tii_counter;
	sampleReader	myReader;
	RadioInterface	*myRadioInterface;
	ficHandler	my_ficHandler;
	int32_t		frequency;
	int16_t		attempts;
	int16_t		startFailures;
	int32_t		T_null;
	int32_t		T_u;
	int32_t		T_s;
	int32_t		T_g;
	int32_t		T_F;
	int32_t		nrBlocks;
	int32_t		carriers;
	int32_t		carrierDiff;
	int16_t		fineOffset;
	int		threshold;
	int32_t		coarseOffset;

	bool		correctionNeeded;
	std::vector<std::complex<float>	>ofdmBuffer;
	uint32_t	ofdmBufferIndex;
	uint32_t	ofdmSymbolCount;
	phaseReference	phaseSynchronizer;
	TII_Detector	my_TII_Detector;
	ofdmDecoder	my_ofdmDecoder;
	bool		wasSecond		(int16_t, dabParams *);

virtual	void		run			(void);
	bool		isReset;
signals:
	void		setSynced		(bool);
	void		show_tii		(int);
	void		noSignal_Found		(void);
	void		show_snr		(int);
};
#endif

