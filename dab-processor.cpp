#
/*
 *    Copyright (C) 2014 .. 2017
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the dab scanner
 *
 *    dabscanner is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    dabscanner is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with dabscanner if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include	"dab-processor.h"
#include	"fic-handler.h"
#include	"radio.h"
#include	"dab-params.h"
#include	"timesyncer.h"
//
/**
  *	\brief dabProcessor
  *	The dabProcessor class is the driver of the processing
  *	of the samplestream.
  *	It is the main interface to the qt-dab program,
  *	local are classes ofdmDecoder, ficHandler and mschandler.
  */

#define	C_LEVEL_SIZE	50
	dabProcessor::dabProcessor	(RadioInterface	*mr,
	                                 deviceHandler	*theDevice,
	                                 uint8_t	dabMode,
	                                 int16_t	threshold_1,
	                                 int16_t	threshold_2,
	                                 int16_t	diff_length,
	                                 RingBuffer<std::complex<double>> *
                                                                 spectrumBuffer,
                                         RingBuffer<std::complex<double>> *
                                                                 iqBuffer):
	                                 params (dabMode),
	                                 myReader (mr,
	                                           theDevice,
	                                           spectrumBuffer),
	                                 my_ficHandler (mr, dabMode),
	                                 phaseSynchronizer (mr,
	                                                    dabMode, 
	                                                    diff_length),
	                                 my_TII_Detector (dabMode, 4),
	                                 my_ofdmDecoder (mr,
	                                                 dabMode,
	                                                 theDevice -> bitDepth (),
	                                                 iqBuffer) {

	this	-> myRadioInterface	= mr;
	this	-> theDevice		= theDevice;
	this	-> threshold_1		= threshold_1;
	this	-> threshold_2		= threshold_2;
	this	-> T_null		= params. get_T_null ();
	this	-> T_s			= params. get_T_s ();
	this	-> T_u			= params. get_T_u ();
	this	-> T_F			= params. get_T_F ();
	this	-> nrBlocks		= params. get_L ();
	this	-> carriers		= params. get_carriers ();
	this	-> carrierDiff		= params. get_carrierDiff ();
	this    -> tii_delay            = 10;
        this    -> tii_counter          = 0;

	ofdmBuffer. resize (2 * T_s);
	ofdmBufferIndex			= 0;
	ofdmSymbolCount			= 0;
	fineOffset			= 0;	
	correctionNeeded		= true;
	my_TII_Detector. reset();
	myReader. setRunning  (false);
}

	dabProcessor::~dabProcessor	(void) {
	if (isRunning ()) {
	   myReader. setRunning (false);
	                                // exception to be raised
	                        	// through the getSample(s) functions.
	   msleep (100);
	   while (isRunning ()) {
	      usleep (1000);
	   }
	}
}

void	dabProcessor::start (int frequency) {
	this		-> frequency	= frequency;
	my_ficHandler.  reset ();
	this -> QThread::start ();
}
	
/***
   *	\brief run
   *	The main thread, reading samples,
   *	time synchronization and frequency synchronization
   *	Identifying blocks in the DAB frame
   *	and sending them to the ofdmDecoder who will transfer the results
   *	Finally, estimating the small freqency error
   */
void	dabProcessor::run	(void) {
int32_t		startIndex;
int32_t		i;
int		false_dipStarts;
int		false_dipEnds;
int		false_frameStarts;

std::complex<double>	FreqCorr;
timeSyncer	myTimeSyncer (&myReader);
std::vector<int16_t> ibits (2 * params. get_carriers ());


	theDevice  -> resetBuffer ();
	theDevice	-> restartReader (frequency);
	false_dipStarts         = 0;
        false_dipEnds           = 0;
        false_frameStarts       = 0;
	coarseOffset	= 0;
	fineOffset 	  = 0;
	correctionNeeded	= true;
//
//	start the reader
	myReader. setRunning (true);
//
//	to get some idea of the signal strength
	try {
	   for (i = 0; i < T_F / 5; i ++) {
	      myReader. getSample (0);
	   }
//Initing:
notSynced:
	   setSynced (false);
	   my_TII_Detector. reset();

	   switch (myTimeSyncer. sync (T_null, T_F)) {
	      case TIMESYNC_ESTABLISHED:
	         break;                 // yes, we are ready

	      case NO_DIP_FOUND:
	         if (++ false_dipStarts >= 25) {
	            emit (noSignal_Found ());
	            false_dipStarts	= 0;
	            false_dipEnds	= 0;
	            false_frameStarts	= 0;
	         }
	         goto notSynced;

	      default:                  // does not happen
	      case NO_END_OF_DIP_FOUND:
	         if (++ false_dipEnds >= 25) {
	            emit (noSignal_Found ());
	            false_dipStarts     = 0;
                    false_dipEnds       = 0;
                    false_frameStarts   = 0;
	            goto notSynced;
	         }
	   }
	  

	   myReader. getSamples (ofdmBuffer. data (),
	                         T_u, coarseOffset + fineOffset);
//
//	and then, call upon the phase synchronizer to verify/compute
//	the real "first" sample
	   startIndex = phaseSynchronizer. findIndex (ofdmBuffer, threshold_1);
	   if (startIndex < 0) { // no sync, try again
	      if (++false_frameStarts >= 25) {
	         emit (noSignal_Found ());
	         false_dipStarts	= 0;
	         false_dipEnds		= 0;
	         false_frameStarts	= 0;
	      }
	      goto notSynced;
	   }
//
//	When here, we have a timesync
	   goto SyncOnPhase;

Check_endofNULL:

	   myReader. getSamples (ofdmBuffer. data(),
	                        T_u, coarseOffset + fineOffset);
/**
  *     We now have to find the exact first sample of the non-null period.
  *     We use a correlation that will find the first sample after the
  *     cyclic prefix.
  *	Since we were at sync, we know that the value of the
  *	correlation is an order higher than it was when not yet synced.
  */
	   startIndex = phaseSynchronizer.
	                   findIndex (ofdmBuffer, threshold_2);
	   if (startIndex < 0) { // no sync, try again
	      if (!correctionNeeded) {
//	         fprintf (stderr, "%d\n", startIndex);
	      }
	      goto notSynced;
	   }
//         fprintf (stderr, "startIndex = %d\n", startIndex);

SyncOnPhase:
	   false_dipStarts	= 0;
	   false_dipEnds	= 0;
	   false_frameStarts	= 0;
/**
  *	Once here, we are synchronized, we need to copy the data we
  *	used for synchronization for block 0
  */
	   memmove (ofdmBuffer. data (),
	            &((ofdmBuffer. data ()) [startIndex]),
	                  (T_u - startIndex) * sizeof (std::complex<double>));
	   ofdmBufferIndex	= T_u - startIndex;

//Block_0:
/**
  *	Block 0 is special in that it is used for fine time synchronization,
  *	for coarse frequency synchronization
  *	and its content is used as a reference for decoding the
  *	first datablock.
  *	We read the missing samples in the ofdm buffer
  */
	   setSynced (true);
	   myReader. getSamples (&((ofdmBuffer. data ()) [ofdmBufferIndex]),
	                           T_u - ofdmBufferIndex,
	                           coarseOffset + fineOffset);
	   my_ofdmDecoder. processBlock_0 (ofdmBuffer);

//	Here we look only at the block_0 when we need a coarse
//	frequency synchronization.
	   correctionNeeded	= !my_ficHandler. syncReached ();
	   if (correctionNeeded) {
	      int correction	=
	            phaseSynchronizer. estimate_CarrierOffset (ofdmBuffer);
	      if (correction != 100) {
	         coarseOffset	+= correction * carrierDiff;
	         if (abs (coarseOffset) > Khz (35))
	            coarseOffset = 0;
	      }
	   }
/**
  *	after block 0, we will just read in the other (params -> L - 1) blocks
  */
//Data_blocks:
/**
  *	The first ones are the FIC blocks. We immediately
  *	start with building up an average of the phase difference
  *	between the samples in the cyclic prefix and the
  *	corresponding samples in the datapart.
  */
	   FreqCorr	= std::complex<double> (0, 0);
	   for (int ofdmSymbolCount = 1;
	        ofdmSymbolCount < nrBlocks; ofdmSymbolCount ++) {
	      myReader. getSamples (ofdmBuffer. data (),
	                              T_s, coarseOffset + fineOffset);
	      for (i = (int)T_u; i < (int)T_s; i ++)
	         FreqCorr += ofdmBuffer [i] * conj (ofdmBuffer [i - T_u]);

	      if (ofdmSymbolCount < 4) {
	         my_ofdmDecoder. decode (ofdmBuffer,
	                                 ofdmSymbolCount, ibits. data ());
	         my_ficHandler. process_ficBlock (ibits, ofdmSymbolCount);
	      }
	   }

//NewOffset:
//	we integrate the newly found frequency error with the
//	existing frequency error.
	   fineOffset += 0.1 * arg (FreqCorr) / (2 * M_PI) * carrierDiff;
//
/**
  *	OK,  here we are at the end of the frame
  *	Assume everything went well and skip T_null samples
  */
	   myReader. getSamples (ofdmBuffer. data (),
	                         T_null, coarseOffset);

	   double sum    = 0;
	   for (i = 0; i < T_null; i ++)
	      sum += abs (ofdmBuffer [i]);
	   sum /= T_null;

	   static    float snr       = 0;
	   snr = 0.9 * snr +
	         0.1 * 20 * log10 ((myReader. get_sLevel () + 0.005) / sum);
	   static int ccc       = 0;
	   if (++ccc > 10) {
	      ccc = 0;
	      show_snr ((int)snr);
	   }

/*
 *      The TII data is encoded in the null period of the
 *      odd frames
 */
	   if (params. get_dabMode() == 1) {
	      if (wasSecond (my_ficHandler. get_CIFcount(), &params)) {
	         my_TII_Detector. addBuffer (ofdmBuffer);
	         if (++tii_counter >= tii_delay) {
	            std::vector <int> transmitterIds =
	                              my_TII_Detector. processNULL();
	            if (transmitterIds . size ()> 0) 
	               for (uint i = 0; i <  transmitterIds. size (); i ++)
	                  show_tii (transmitterIds. at (i));
	            tii_counter = 0;
	            my_TII_Detector. reset();
	         }
	      }
	   }
/**
  *	The first sample to be found for the next frame should be T_g
  *	samples ahead. Before going for the next frame, we
  *	we just check the fineCorrector
  */
	   if (fineOffset > carrierDiff / 2) {
	      coarseOffset	+= carrierDiff;
	      fineOffset	-= carrierDiff;
	   }
	   else
	   if (fineOffset < -carrierDiff / 2) {
	      coarseOffset -= carrierDiff;
	      fineOffset += carrierDiff;
	   }
//ReadyForNewFrame:
///	and off we go, up to the next frame
	   goto Check_endofNULL;
	}
	catch (int e) {
//	   fprintf (stderr, "dabProcessor is stopping\n");
	   ;
	}
	my_ficHandler.  stop ();
}

void	dabProcessor:: reset	(void) {
	myReader. setRunning (false);
	while (isRunning ())
	   usleep (1000);
	usleep (10000);
	my_ficHandler.  reset ();
	QThread::start ();
}

void	dabProcessor::stop	(void) {
	myReader. setRunning (false);
	while (isRunning ())
	   usleep (1000);
	theDevice	-> stopReader ();
	usleep (10000);
}

void	dabProcessor::coarseCorrectorOn (void) {
	correctionNeeded 	= true;
	coarseOffset	= 0;
}

void	dabProcessor::coarseCorrectorOff (void) {
	correctionNeeded	= false;
}

//
//	just a convenience function
bool	dabProcessor::is_audioService	(QString &s) {
audiodata ad;
	my_ficHandler. dataforAudioService (s, &ad, 0);
	return ad. defined;
}

bool	dabProcessor::is_packetService	(QString &s) {
packetdata pd;
	my_ficHandler. dataforPacketService (s, &pd, 0);
	return pd. defined;
}

void	dabProcessor::dataforAudioService	(QString &s,
	                                          audiodata *d, int16_t c) {
	my_ficHandler. dataforAudioService (s, d, c);
}

void	dabProcessor::dataforPacketService	(QString &s,
	                                         packetdata *d, int16_t c) {
	my_ficHandler. dataforPacketService (s, d, c);
}

uint8_t	dabProcessor::get_ecc() {
	return my_ficHandler. get_ecc();
}

int32_t dabProcessor::get_ensembleId() {
	return my_ficHandler. get_ensembleId();
}

QString dabProcessor::get_ensembleName	(void) {
	return my_ficHandler. get_ensembleName ();
}

void	dabProcessor::clearEnsemble	(void) {
	my_ficHandler. clearEnsemble ();
}

bool    dabProcessor::wasSecond (int16_t cf, dabParams *p) {
	switch (p -> get_dabMode()) {
	   default:
	   case 1:
	      return (cf & 07) >= 4;
	   case 2:
	   case 3:
	      return (cf & 02);
	   case 4:
	      return (cf & 03) >= 2;
	}
}

