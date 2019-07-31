#
/*
 *    Copyright (C) 2013, 2014, 2015, 2016, 2017
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
#include	<QSettings>
#include	<QMessageBox>
#include	<QDebug>
#include	<QDateTime>
#include	<QTime>
#include	<QFile>
#include	<QDir>
#include	<QFileDialog>
#include	"dab-constants.h"
#include <iostream>
#include <string>
#include <stdio.h>
#include <time.h>
#include	<numeric>
#include	<unistd.h>
#include	<vector>
#include        <QMouseEvent>
#include	"radio.h"
#include	"band-handler.h"
#include	<mutex>

#ifdef	HAVE_RTLSDR
#include	"rtlsdr-handler.h"
#endif
#ifdef	HAVE_SDRPLAY
#include	"sdrplay-handler.h"
#endif
#ifdef	HAVE_AIRSPY
#include	"airspy-handler.h"
#endif
#ifdef	HAVE_HACKRF
#include	"hackrf-handler.h"
#endif
#ifdef	HAVE_LIME
#include	"lime-handler.h"
#endif
/**
  *	We use the creation function merely to set up the
  *	user interface and make the connections between the
  *	gui elements and the handling agents. All real action
  *	is initiated by gui buttons
  */

	RadioInterface::RadioInterface (QSettings	*Si,
	                                bandHandler	*theBand,
	                                QWidget		*parent):
	                                        QMainWindow (parent) {
QString h;

	dabSettings		= Si;
	this	-> theBand	= theBand;
	channels		= theBand	-> channels ();
	running. store (false);
	isSynced		= UNSYNCED;
	threshold	=
	           dabSettings -> value ("threshold", 3). toInt ();
//
	diff_length	=
	           dabSettings	-> value ("diff_length", DIFF_LENGTH). toInt ();

        dabMode		= dabSettings   -> value ("dabMode", 1). toInt ();
	if ((dabMode != 1) && (dabMode != 2))
	   dabMode = 1;

	isSynced	= false;
	tii_Value	= -1;
///////////////////////////////////////////////////////////////////////////

//	The settings are done, now creation of the GUI parts
	setupUi (this);
//
//	... and the device
	inputDevice		= setDevice (dabSettings);
	if (inputDevice == NULL) {
	   throw (33);
	}

	ficBlocks		= 0;
	ficSuccess		= 0;
//	
//	and start the timer(s)
//	The displaytimer is there to show the number of
//	seconds running 
//
//	timer for channel settings
	channelTimer. setSingleShot (true);
	channelTimer. setInterval   (channelDelay -> value () * 1000);

	channelNumber		= 0;
//
	my_dabProcessor	= new dabProcessor (this,
	                                    inputDevice,
	                                    dabMode,
	                                    threshold,
	                                    diff_length);
	connect (my_dabProcessor, SIGNAL (show_snr (int)),
	         this, SLOT (show_snr (int)));
	connect (my_dabProcessor, SIGNAL (setSynced (bool)),
	         this, SLOT (setSynced (bool)));
	connect (my_dabProcessor, SIGNAL (show_tii (int)),
	         this, SLOT (show_tii (int)));
	connect (startButton, SIGNAL (clicked (void)),
	         this, SLOT (handle_startButton (void)));
}

	RadioInterface::~RadioInterface (void) {
	fprintf (stderr, "radioInterface is deleted\n");
}
//
void	RadioInterface:: startScanning (void) {
int	frequency	= theBand -> Frequency (channelNumber);
	channelDisplay -> setText (theBand -> channel (channelNumber));
	connect (my_dabProcessor, SIGNAL (noSignal_Found (void)),
	         this, SLOT (nextChannel_noSignal (void)));
	connect (&channelTimer, SIGNAL (timeout (void)),
	         this, SLOT (nextChannel_withSignal (void)));
	channelTimer. start (channelDelay -> value () * 1000);
	my_dabProcessor -> start (frequency);
	running. store (true);
}

void	RadioInterface::stopScanning (void) {
	disconnect (my_dabProcessor, SIGNAL (noSignal_Found (void)),
                    this, SLOT (nextChannel_noSignal (void)));
        disconnect (&channelTimer, SIGNAL (timeout (void)),
                    this, SLOT (nextChannel_withSignal (void)));
        inputDevice     -> stopReader ();
        my_dabProcessor -> stop ();
        channelTimer. stop ();
	running. store (false);
}

void	RadioInterface::nextChannel_noSignal (void) {
int	frequency;
	disconnect (my_dabProcessor, SIGNAL (noSignal_Found (void)),
	            this, SLOT (nextChannel_noSignal (void)));
	disconnect (&channelTimer, SIGNAL (timeout (void)),
	            this, SLOT (nextChannel_withSignal (void)));
	inputDevice	-> stopReader ();
	my_dabProcessor -> stop ();
	channelTimer. stop ();
	ensembleDisplay	-> setText ("");
	Services	= QStringList ();
	serviceCountDisplay	-> display (0);
	tii_Label	-> setText (" ");
	tii_Value	= -1;
	channelNumber . store (channelNumber. load () + 1);
	if (channelNumber. load () >= theBand -> channels ()) {
	   channelNumber . store (0);
	   nrCycles	-> setValue (nrCycles -> value () - 1);
	   if (nrCycles -> value () < 1) {
	      running. store (false);
	      fclose (fileP);
	      startButton	-> setText ("start");
	      return;
	   }
	}
	frequency	= theBand -> Frequency (channelNumber);
	channelDisplay -> setText (theBand -> channel (channelNumber));
        connect (my_dabProcessor, SIGNAL (noSignal_Found (void)),
                 this, SLOT (nextChannel_noSignal (void)));
	connect (&channelTimer, SIGNAL (timeout (void)),
	         this, SLOT (nextChannel_withSignal (void)));
        channelTimer. start (channelDelay -> value () * 1000);
        my_dabProcessor -> start (frequency);
}

void	RadioInterface::nextChannel_withSignal (void) {
int	frequency;
	disconnect (my_dabProcessor, SIGNAL (noSignal_Found (void)),
                    this, SLOT (nextChannel_noSignal (void)));
        disconnect (&channelTimer, SIGNAL (timeout (void)),
                    this, SLOT (nextChannel_withSignal (void)));
        inputDevice     -> stopReader ();
        my_dabProcessor -> stop ();
        channelTimer. stop ();
	showEnsembleData	(snrDisplay -> value (), tii_Value);
	ensembleDisplay	-> setText ("");
	Services	= QStringList ();
	tii_Label	-> setText (" ");
	tii_Value	= -1;
	serviceCountDisplay	-> display (0);
	channelNumber. store (channelNumber. load () + 1);
	if (channelNumber. load () >= theBand -> channels ()) {
	   fprintf (stderr, "channelNumber = %d\n", channelNumber. load ());
	   channelNumber . store (0);
	   nrCycles	-> setValue (nrCycles -> value () - 1);
	   if (nrCycles -> value () < 1) {
	      running. store (false);
	      fclose (fileP);
	      startButton	-> setText ("start");
	      return;
	   }
	}
	channelDisplay	-> setText (theBand -> channel (channelNumber));
	frequency	= theBand -> Frequency (channelNumber);
        connect (my_dabProcessor, SIGNAL (noSignal_Found (void)),
                 this, SLOT (nextChannel_noSignal (void)));
	connect (&channelTimer, SIGNAL (timeout (void)),
	         this, SLOT (nextChannel_withSignal (void)));
        channelTimer. start (channelDelay -> value () * 1000);
        my_dabProcessor -> start (frequency);
}

void	RadioInterface::reset (void) {
}

void    RadioInterface::showTime        (const QString &s) {
        localTimeDisplay        -> setText (s);
}

void	RadioInterface::addtoEnsemble (const QString &s) {
	if (!running. load())
           return;

        Services << s;
        Services. removeDuplicates();
	serviceCountDisplay	-> display (Services. size ());
}

void	RadioInterface::nameofEnsemble (int Id, const QString &s) {
	(void)Id;
	ensembleDisplay -> setText (s);
}

//
///////////////////////////////////////////////////////////////////////

void	RadioInterface::show_snr		(int s) {
	snrDisplay	-> display (s);
}

void	RadioInterface::TerminateProcess (void) {
	running. store (false);
	inputDevice	-> stopReader ();
	my_dabProcessor	-> stop ();		// definitely concurrent
//	everything should be halted by now
	fprintf (stderr, "going to delete dabProcessor\n");
	delete	my_dabProcessor;
	fprintf (stderr, "deleted dabProcessor\n");
	if (inputDevice != NULL)
	   delete	inputDevice;
	close ();
	fprintf (stderr, ".. end the radio silences\n");
}

//
#include <QCloseEvent>
void RadioInterface::closeEvent (QCloseEvent *event) {

	QMessageBox::StandardButton resultButton =
	                QMessageBox::question (this, "dabRadio",
                                               tr("Are you sure?\n"),
                                               QMessageBox::No | QMessageBox::Yes,
                                               QMessageBox::Yes);
	if (resultButton != QMessageBox::Yes) {
	   event -> ignore();
	} else {
	   TerminateProcess ();
	   event -> accept ();
	}
}

bool	RadioInterface::eventFilter (QObject *obj, QEvent *event) {
	return QMainWindow::eventFilter (obj, event);
}

deviceHandler	*RadioInterface::setDevice (QSettings	*dabSettings) {
deviceHandler	*inputDevice	= NULL;
#ifdef	HAVE_AIRSPY
	try {
	   inputDevice	= new airspyHandler (dabSettings);
	   return inputDevice;
	} catch (int e) {
	}
#endif
#ifdef	HAVE_SDRPLAY
	try {
	   inputDevice	= new sdrplayHandler (dabSettings);
	   return inputDevice;
	} catch (int e) {}
#endif
#ifdef	HAVE_RTLSDR
	try {
	   inputDevice	= new rtlsdrHandler (dabSettings);
	   return inputDevice;
	} catch (int e) {}
#endif
#ifdef	HAVE_HACKRF
	try {
	   inputDevice	= new hackrfHandler (dabSettings);
	   return inputDevice;
	} catch (int e) {}
#endif
#ifdef	HAVE_LIME
	try {
	   inputDevice	= new limeHandler (dabSettings);
	   return inputDevice;
	} catch (int e) {}
#endif
	return NULL;
}

void	RadioInterface::handle_startButton (void) {
	if (nrCycles -> value () < 1)
	   return;
	if (!running. load ()) {
	   QString timeString = QDate::currentDate (). toString ();
	   timeString. append (":");
	   timeString. append (QTime::currentTime (). toString ());
	   localTimeDisplay	-> setText (timeString);
	   QString theTime	= localTimeDisplay -> text ();
	   QString suggestedFileName = QDir::homePath ();
	   suggestedFileName. append ("/dab-scanner-");
	   suggestedFileName. append (theTime);
	   suggestedFileName. append (".txt");
	   fprintf (stderr, "suggested filename = %s\n",
	             suggestedFileName. toLatin1 (). data ());
	   QString fileName = QFileDialog::getSaveFileName (this,
	                                        tr ("Save file ..."),
	                                        suggestedFileName,
	                                        tr ("Text (*.txt)"));
	   fileName	= QDir::toNativeSeparators (fileName);
	   fileP	= fopen (fileName. toUtf8(). data(), "w");

	   if (fileP == nullptr) {
	      fprintf (stderr, "Could not open file %s\n",
	                              fileName. toUtf8(). data());
	      return;
	   }
	   startButton -> setText ("stop");
	   startScanning ();
	   return;
	}
//
//	handling stop
	fclose (fileP);
	stopScanning ();
	startButton	-> setText ("start");
}

void	RadioInterface::showEnsembleData	(int snr, int tii) {
QString currentChannel	= theBand -> channel (channelNumber);
int32_t	frequency	= inputDevice -> getVFOFrequency();
QString theTime		= localTimeDisplay -> text ();
ensemblePrinter	my_Printer;

	if (!running. load())
	   return;

	my_Printer. showEnsembleData (currentChannel,
	                              frequency,
	                              snr, tii,
	                              theTime,
                                      Services, my_dabProcessor, fileP);

}

void    RadioInterface::setSynced       (bool b) {
        if (!running. load())
           return;
        if (isSynced == b)
           return;

        isSynced = b;
        switch (isSynced) {
           case SYNCED:
              syncedLabel ->
                       setStyleSheet ("QLabel {background-color : green}");
              break;

           default:
              syncedLabel ->
                       setStyleSheet ("QLabel {background-color : red}");
              break;
        }
}


void	RadioInterface::set_CorrectorDisplay	(int c) {
	offsetDisplay	-> display (c);
}

void    RadioInterface::show_ficSuccess (bool b) {
        if (!running. load())
           return;
        if (b) 
           ficSuccess ++;

        if (++ficBlocks >= 100) {
           ficSuccessDisplay   -> setValue (ficSuccess);
           ficSuccess   = 0;
           ficBlocks    = 0;
        }
}

void	RadioInterface::show_tii	(int tii) {
char	buffer [20];
	sprintf (buffer, "%d %d \n", tii >> 8, tii & 0xFF);
	tii_Label -> setText (buffer);
	tii_Value	= tii;
}

