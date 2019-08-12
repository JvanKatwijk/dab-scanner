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
#include	<QStringList>
#include	<QDebug>
#include	<QDateTime>
#include	<QTime>
#include	<QFile>
#include	<QDir>
#include	<QFileDialog>
#include	"dab-constants.h"
#include	<iostream>
#include	<string>
#include	<stdio.h>
#include	<time.h>
#include	<numeric>
#include	<unistd.h>
#include	<vector>
#include        <QMouseEvent>
#include	"radio.h"
#include	"band-handler.h"

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
	channelTable		= new channelsTable (Si, this, theBand);
	running. store (false);
	threshold	=
	           dabSettings -> value ("threshold", 3). toInt ();
//
	diff_length	=
	           dabSettings	-> value ("diff_length", DIFF_LENGTH). toInt ();

	dabMode		= dabSettings   -> value ("dabMode", 1). toInt ();
	if ((dabMode != 1) && (dabMode != 2))
	   dabMode = 1;
//
//	we maintain the name of the directory of the last selection
	dirName		= dabSettings -> value ("dirName",
	                                        QDir::homePath ()). toString ();
	isSynced	= false;
	tii_Value. resize  (0);
///////////////////////////////////////////////////////////////////////////

//	The settings are done, now creation of the GUI parts
	setupUi (this);
//
//	... and the device selector

#ifdef	HAVE_RTLSDR
	deviceSelector	-> addItem ("rtlsdr");
#endif
#ifdef	HAVE_SDRPLAY
	deviceSelector	-> addItem ("sdrplay");
#endif
#ifdef	HAVE_AIRSPY
	deviceSelector	-> addItem ("airspy");
#endif
#ifdef	HAVE_HACKRF
	deviceSelector	-> addItem ("hackrf");
#endif
#ifdef	HAVE_LIME
	deviceSelector	-> addItem ("lime");
#endif

	h		=
	            dabSettings -> value ("device", "no device"). toString();
	int k		= deviceSelector -> findText (h);
//      fprintf (stderr, "%d %s\n", k, h. toUtf8(). data());
	if (k != -1) {
	   deviceSelector       -> setCurrentIndex (k);
	}

//	connect (deviceSelector, SIGNAL (activated (QString)),
//	         this, SLOT (selectDevice (QString)));

	theDevice		= NULL;
	my_dabProcessor		= NULL;
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
	connect (startButton, SIGNAL (clicked (void)),
	         this, SLOT (handle_startcontrolledButton (void)));
	connect (continuousButton, SIGNAL (clicked (void)),
	         this, SLOT (handle_continuousButton (void)));
//
}

	RadioInterface::~RadioInterface (void) {
}
//
void	RadioInterface:: startScanning (void) {
int	frequency	= theBand -> Frequency (channelNumber);

	if (go_continuously) {
	   int nrChannels	= theBand -> channels ();
	   int skipCount	= 0;
	   if (channelNumber. load () < nrChannels) {
	      while  (go_continuously &&
	                      skipChannel (channelNumber. load ())) {
	         fprintf (stderr, "skipping channel %d\n", channelNumber. load ());
	         if (++skipCount >= nrChannels) {
	            QMessageBox::warning (this, tr ("Warning"),
	                               tr ("no channels in scanlist\n"));
	            stopScanning ();
	            return;
	         }
	         channelNumber.
	               store ((channelNumber. load () + 1) % nrChannels);
	      }
	   }
	}
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
	theDevice     		-> stopReader ();
	my_dabProcessor		-> stop ();
	channelTimer. stop ();
	running. store (false);	
	startButton		-> setText ("start controlled");
	startButton		-> show ();
	continuousButton	-> setText ("start continuous");
	continuousButton	-> show ();
	nrCycles		-> show ();
}

void	RadioInterface::nextChannel_noSignal (void) {
	process_nextChannel ();
}

void	RadioInterface::nextChannel_withSignal (void) { 
	process_nextChannel ();
}

void	RadioInterface::process_nextChannel (void) {
int	frequency;
	disconnect (my_dabProcessor, SIGNAL (noSignal_Found (void)),
	            this, SLOT (nextChannel_noSignal (void)));
	disconnect (&channelTimer, SIGNAL (timeout (void)),
	            this, SLOT (nextChannel_withSignal (void)));
	theDevice	-> stopReader ();
	my_dabProcessor -> stop ();

	channelTimer. stop ();
//
//	in case we are called as "noSignal", we already know that
//	the size is 0
	if ((Services. size () != 0) &&
	    (ensembleDisplay -> text () != QString (""))) {
	   showEnsembleData     (snrDisplay -> value (), tii_Value);
	}

	ensembleDisplay	-> setText ("");
	Services	= QStringList ();
	tii_Label	-> setText (" ");
	tii_Value. resize (0);
	serviceCountDisplay	-> display (0);

	int nrChannels	= theBand -> channels ();
	channelNumber . store (channelNumber. load () + 1);
	int skipCount	= 0;
	if (channelNumber. load () < nrChannels) {
	   while  (go_continuously && skipChannel (channelNumber. load ())) {
	      fprintf (stderr, "skipping channel %d\n", channelNumber. load ());
	      if (++skipCount >= nrChannels) {       
	         QMessageBox::warning (this, tr ("Warning"),
	                               tr ("no channels in scanlist\n"));
	         stopScanning ();
	         return;
	      }

	      channelNumber. store ((channelNumber. load () + 1) % nrChannels);
	   }
	}
//
//	end of cycle
	if (channelNumber. load () >= theBand -> channels ()) {
	   channelNumber . store (0);
	   if (!go_continuously) {
	      nrCycles -> setValue (nrCycles -> value () - 1);
	      if (nrCycles -> value () < 1) {
	         running. store (false);
	         fclose (fileP);
	         startButton		-> setText ("start controlled");
	         continuousButton	-> show ();
	         return;
	      }
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
	if (theDevice != NULL) 
	   theDevice	-> stopReader ();
	if (my_dabProcessor != NULL) {
	   my_dabProcessor	-> stop ();		// definitely concurrent
//	everything should be halted by now
	   delete	my_dabProcessor;
	}
	if (theDevice != NULL)
	   delete	theDevice;
	dabSettings	-> setValue ("device", deviceSelector -> currentText ());
	channelTable	-> hide ();
	delete channelTable;
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

//
//	In this setting, the user just select one device
//	per program invocation
void	RadioInterface::selectDevice (QString s) {
	theDevice	= setDevice (s);
	if (theDevice == NULL)
	   return;
	deviceSelector	-> hide ();

	my_dabProcessor	= new dabProcessor (this,
	                                    theDevice,
	                                    dabMode,
	                                    threshold,
	                                    diff_length);
	connect (my_dabProcessor, SIGNAL (show_snr (int)),
	         this, SLOT (show_snr (int)));
	connect (my_dabProcessor, SIGNAL (setSynced (bool)),
	         this, SLOT (setSynced (bool)));
	connect (my_dabProcessor, SIGNAL (show_tii (int)),
	         this, SLOT (show_tii (int)));
}

deviceHandler	*RadioInterface::setDevice (QString s) {
#ifdef	HAVE_AIRSPY
	if (s == "airspy") {
	   try {
	      return new airspyHandler (dabSettings);
	   } catch (int e) {}
	}
	else
#endif
#ifdef	HAVE_SDRPLAY
	if (s == "sdrplay") {
	   try {
	      return  new sdrplayHandler (dabSettings);
	   } catch (int e) {}
	}
	else
#endif
#ifdef	HAVE_RTLSDR
	if (s == "rtlsdr") {
	   try {
	      return new rtlsdrHandler (dabSettings);
	   } catch (int e) {}
	}
	else
#endif
#ifdef	HAVE_HACKRF
	if (s == "hackrf") {
	   try {
	      return new hackrfHandler (dabSettings);
	   } catch (int e) {}
	}
	else
#endif
#ifdef	HAVE_LIME
	if (s == "lime") {
	   try {
	      return new limeHandler (dabSettings);
	   } catch (int e) {}
	}
#endif
	return NULL;
}

void	RadioInterface::handle_startcontrolledButton (void) {
QString reportName;

	if (theDevice == NULL) {	// initialize first
	   if (deviceSelector -> currentText () == "select device") {
	      QMessageBox::warning (this, tr ("Warning"),
	                               tr ("Select a device first\n"));
	      return;
	   }

	   theDevice	= setDevice (deviceSelector -> currentText ());
	   if (theDevice == NULL) {
	      QMessageBox::warning (this, tr ("Warning"),
                                          tr ("Select a connected device"));
	      return;
	   }

	   deviceSelector	-> hide ();
//
//	here we really start
	   my_dabProcessor	= new dabProcessor (this,
	                                    theDevice,
	                                    dabMode,
	                                    threshold,
	                                    diff_length);
	   connect (my_dabProcessor, SIGNAL (show_snr (int)),
	            this, SLOT (show_snr (int)));
	   connect (my_dabProcessor, SIGNAL (setSynced (bool)),
	            this, SLOT (setSynced (bool)));
	   connect (my_dabProcessor, SIGNAL (show_tii (int)),
	            this, SLOT (show_tii (int)));
	}

	if (nrCycles -> value () < 1)
	   return;
	if (!running. load ()) {
	   continuousButton	-> hide ();
	   reportName	= find_fileName ();
	   fileP	= fopen (reportName. toUtf8(). data(), "w");
	   if (fileP == nullptr) {
	      fprintf (stderr, "Could not open file %s\n",
	                              reportName. toUtf8(). data());
	      continuousButton	-> show ();
	      return;
	   }
	   startButton -> setText ("stop");
	   operationsLabel	-> setText ("running controlled");
	   go_continuously	= false;
	   startScanning ();
	   return;
	}
//
//	handling stop
	fclose (fileP);
	stopScanning ();
	startButton		-> setText ("start controlled");
	operationsLabel		-> setText ("");
	continuousButton	-> show ();
}

void	RadioInterface::showEnsembleData	(int snr,
	                                         std::vector<int> tiiValue) {
QString currentChannel	= theBand -> channel (channelNumber);
int32_t	frequency	= theDevice -> getVFOFrequency();
QString theTime		= localTimeDisplay -> text ();
ensemblePrinter	my_Printer;

	if (!running. load())
	   return;

	my_Printer. showEnsembleData (currentChannel,
	                              frequency,
	                              snr,
	                              tiiValue,
	                              theTime,
	                              Services, my_dabProcessor, fileP);
	if (go_continuously) {
	   my_Printer. addSummary (currentChannel,
	                           frequency,
	                           snr,
	                           tiiValue,
	                           theTime,
	                           Services, my_dabProcessor, summaryP);
	}
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
QString s;
char buffer [20];
	sprintf (buffer, " (%d %d) ", tii >> 8, tii & 0xFF);
	s. append (buffer);
	tii_Label -> setText (s);
	for (int i = 0; i < tii_Value. size (); i ++)
	   if (tii_Value. at (i) == tii)
	      return;
	tii_Value. push_back (tii);
}

void	RadioInterface::handle_continuousButton (void) {
QString reportName;
QString	summaryName;

	if (theDevice == NULL) {
	   if (deviceSelector -> currentText () == "select device") {
	      QMessageBox::warning (this, tr ("Warning"),
	                               tr ("Select a device first\n"));
	      return;
	   }

	   theDevice	= setDevice (deviceSelector -> currentText ());
	   if (theDevice == NULL) {
	      QMessageBox::warning (this, tr ("Warning"),
	                               tr ("select a connected device\n"));
	      return;
	   }
	   deviceSelector	-> hide ();
//
//	here we really start
	   my_dabProcessor	= new dabProcessor (this,
	                                    theDevice,
	                                    dabMode,
	                                    threshold,
	                                    diff_length);
	   connect (my_dabProcessor, SIGNAL (show_snr (int)),
	            this, SLOT (show_snr (int)));
	   connect (my_dabProcessor, SIGNAL (setSynced (bool)),
	            this, SLOT (setSynced (bool)));
	   connect (my_dabProcessor, SIGNAL (show_tii (int)),
	            this, SLOT (show_tii (int)));
	}

	if (!running. load ()) {
	   startButton	-> hide ();
	   nrCycles	-> hide ();
	   reportName = find_fileName ();
	   fileP        = fopen (reportName. toUtf8(). data(), "w");
	   if (fileP == nullptr) {
	      fprintf (stderr, "Could not open file %s\n",
	                              reportName. toUtf8(). data());
	      startButton	-> show ();
	      nrCycles		-> show ();
	      return;
	   }

	   summaryName	= reportName;
	   if (summaryName. indexOf ("dab-scanner") != -1)
	      summaryName. replace ("dab-scanner", "dab-summary");
	   else
	      summaryName. append ("-summary.txt");
	   fprintf (stderr, "summaryName = %s\n", summaryName. toLatin1 (). data ());
	   summaryP	= fopen (summaryName. toUtf8(). data(), "w");
	   if (summaryP == nullptr) {
	      fprintf (stderr, "Could not open  summary file %s\n",
	                              summaryName. toUtf8(). data());
	      fclose (fileP);
	      startButton       -> show ();
	      nrCycles          -> show ();
	      return;
	   }
	   
	   channelTable		-> show ();
	   operationsLabel	-> setText ("running continuously");
	   continuousButton	-> setText ("stop");
	   go_continuously	= true;
	   startScanning ();	
	   running. store (true);
	   return;
	}
//	and in case we need to stop:
	go_continuously = false;
	stopScanning ();
	operationsLabel	-> setText ("");
	fclose (fileP);
	fclose (summaryP);
	running. store (false);
	startButton	-> show ();
	nrCycles	-> show ();
	channelTable	-> hide ();
	continuousButton -> setText ("start continuous");
}

QString	RadioInterface::find_fileName () {
QString timeString = QDate::currentDate (). toString ();

	timeString. append ("-");
	timeString. append (QTime::currentTime (). toString ());
	localTimeDisplay	-> setText (timeString);
	QString theTime	= localTimeDisplay -> text ();
	theTime. replace (":", "-");
	QString suggestedFileName = dirName;
	suggestedFileName. append ("/dab-scanner-");
	suggestedFileName. append (theTime);
	suggestedFileName. append (".txt");
	fprintf (stderr, "suggested filename = %s\n",
	             suggestedFileName. toLatin1 (). data ());
	QString fileName = QFileDialog::getSaveFileName (this,
	                                        tr ("Save file ..."),
	                                        suggestedFileName,
	                                        tr ("Text (*.txt)"));
	int ind		= fileName. lastIndexOf ("/");
	fileName	= QDir::toNativeSeparators (fileName);
	dirName		= fileName;
	dirName. remove (ind, 100);
	dabSettings	-> setValue ("dirName", dirName);
	return fileName;
}

bool	RadioInterface::skipChannel (int channelNumber) {
	return !channelTable	-> channel (channelNumber);
}
