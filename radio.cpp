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
#include	"spectrum-viewer.h"
#include	"dab_tables.h"
#ifdef	HAVE_RTLSDR
#include	"rtlsdr-handler.h"
#endif
#ifdef	HAVE_SDRPLAY_V2
#include	"sdrplay-handler-v2.h"
#endif
#ifdef	HAVE_SDRPLAY_V3
#include	"sdrplay-handler-v3.h"
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

#define	NOT_RUNNING	0100
#define	CONTROLLED	0101
#define	CONTINUOUS	0102

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
	running. store (NOT_RUNNING);
	threshold_1	=
	           dabSettings -> value ("threshold", 3). toInt ();
	threshold_2	=
	           dabSettings -> value ("threshold", 3). toInt ();
//
	diff_length	=
	           dabSettings	-> value ("diff_length", DIFF_LENGTH). toInt ();

	dabMode		= dabSettings   -> value ("dabMode", 1). toInt ();
	if ((dabMode != 1) && (dabMode != 2))
	   dabMode = 1;
//
	channelFileName		= 
	              dabSettings -> value ("channelFile", ""). toString ();

//	we maintain the name of the directory of the last selection
	dirName		= dabSettings -> value ("dirName",
	                                        QDir::homePath ()). toString ();
	isSynced	= false;
	tii_Value. resize  (0);

	spectrumBuffer	= new RingBuffer<std::complex<double>> (2 * 32768);
	iqBuffer	= new RingBuffer<std::complex<double>> (2 * 1536);
///////////////////////////////////////////////////////////////////////////
//	The settings are done, now creation of the GUI parts
	setupUi (this);
	skipfileName		-> setText (channelFileName);
	channelTable		= nullptr;
	displayTable		= new outputTable (this);
//
//	... and the device selector

#ifdef	HAVE_RTLSDR
	deviceSelector	-> addItem ("rtlsdr");
#endif
#ifdef	HAVE_SDRPLAY_V2
	deviceSelector	-> addItem ("sdrplay v2");
#endif
#ifdef	HAVE_SDRPLAY_V3
	deviceSelector	-> addItem ("sdrplay v3");
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

	theDevice		= nullptr;
	my_dabProcessor		= nullptr;
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
	channelNumber. store (0);

	connect (skipTableSelect, SIGNAL (clicked (void)),
	         this, SLOT (handle_skipTableSelect ()));
	connect (startButton, SIGNAL (clicked (void)),
	         this, SLOT (handle_controlledButton (void)));
	connect (continuousButton, SIGNAL (clicked (void)),
	         this, SLOT (handle_continuousButton (void)));

	my_spectrumViewer       = new spectrumViewer (this, dabSettings,
                                               spectrumBuffer,
                                               iqBuffer);
	connect (spectrumSwitch, SIGNAL (clicked (void)),
                 this, SLOT (set_spectrumSwitch (void)));
	running. store (NOT_RUNNING);
}

	RadioInterface::~RadioInterface (void) {
}

/////////////////////////////////////////////////////////////////////////
//	CONTROLLED SECTION
////////////////////////////////////////////////////////////////////////

void	RadioInterface::handle_controlledButton (void) {
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
//	here we really start
	   my_dabProcessor	= new dabProcessor (this,
	                                            theDevice,
	                                            dabMode,
	                                            threshold_1,
	                                            threshold_2,
	                                            diff_length,
	                                            spectrumBuffer,
                                                    iqBuffer);

	   connect (my_dabProcessor, SIGNAL (show_snr (int)),
	            this, SLOT (show_snr (int)));
	   connect (my_dabProcessor, SIGNAL (setSynced (bool)),
	            this, SLOT (setSynced (bool)));
	   connect (my_dabProcessor, SIGNAL (show_tii (int)),
	            this, SLOT (show_tii (int)));
	}

	QString reportName      = find_fileName ();
        fileP           = fopen (reportName. toUtf8(). data(), "w");
        if (fileP == nullptr) {
           fprintf (stderr, "Could not open file %s\n",
                                   reportName. toUtf8(). data());
           return;
        }

	continuousButton	-> hide ();
	skipTableSelect	-> hide ();
	displayTable	-> clear ();
	displayTable	-> show ();
	nrCycles	-> show	();
	showTable	-> hide ();
	Services	= QStringList ();
	channelNumber. store (0);
	channelDisplay -> setText (theBand -> channel (channelNumber. load ()));
	connect (my_dabProcessor, SIGNAL (noSignal_Found (void)),
	         this, SLOT (nextcontrolled_noSignal (void)));
	connect (&channelTimer, SIGNAL (timeout (void)),
	         this, SLOT (nextcontrolled_withSignal (void)));
	disconnect (startButton, SIGNAL (clicked (void)),
                 this, SLOT (handle_controlledButton (void)));
	connect (startButton, SIGNAL (clicked (void)),
                 this, SLOT (stopControlled (void)));
	startButton	-> setText ("stop");

	channelTimer. start (channelDelay -> value () * 1000);
	
	my_dabProcessor -> start (theBand -> Frequency (channelNumber));
	running. store (CONTROLLED);
}

void	RadioInterface::stopControlled () {
	disconnect (my_dabProcessor, SIGNAL (noSignal_Found (void)),
	            this, SLOT (nextcontrolled_noSignal (void)));
	disconnect (&channelTimer, SIGNAL (timeout (void)),
	            this, SLOT (nextcontrolled_withSignal (void)));
	theDevice     		-> stopReader ();
	my_dabProcessor		-> stop ();
	channelTimer. stop ();
	running. store (NOT_RUNNING);	
        disconnect (startButton, SIGNAL (clicked (void)),
                 this, SLOT (stopControlled (void)));
	connect (startButton, SIGNAL (clicked (void)),
                 this, SLOT (handle_controlledButton (void)));

	startButton		-> setText ("start controlled");
	startButton		-> show ();
	continuousButton	-> show ();
	nrCycles		-> show ();
	skipTableSelect		-> show ();
}

void	RadioInterface::nextcontrolled_noSignal () {
	nextcontrolled ();
}

void	RadioInterface::nextcontrolled_withSignal () {
	nextcontrolled ();
}

void	RadioInterface::nextcontrolled () {
int	frequency;
	disconnect (my_dabProcessor, SIGNAL (noSignal_Found (void)),
	            this, SLOT (nextcontrolled_noSignal (void)));
	disconnect (&channelTimer, SIGNAL (timeout (void)),
	            this, SLOT (nextcontrolled_withSignal (void)));
	theDevice	-> stopReader ();
	my_dabProcessor -> stop ();
	channelTimer. stop ();

//	in case we are called as "noSignal", we already know that
//      the size is 0
        if ((Services. size () != 0) &&
            (ensembleDisplay -> text () != QString (""))) {
           showEnsembleData     (snrDisplay -> value (), tii_Value);
        }

	ensembleDisplay	-> setText ("");
	Services	= QStringList ();
	tii_Label	-> setText (" ");
	tii_Value. resize (0);
	serviceCountDisplay	-> display (0);

	channelNumber . store (channelNumber. load () + 1);

	if (channelNumber. load () >= theBand -> channels ()) {
	   nrCycles -> setValue (nrCycles -> value () - 1);
	   if (nrCycles -> value () < 1) {
	      channelNumber . store (0);
	      stopControlled ();
	      return;
	   }
	}
	
	frequency	= theBand -> Frequency (channelNumber);
	channelDisplay -> setText (theBand -> channel (channelNumber));
	connect (my_dabProcessor, SIGNAL (noSignal_Found (void)),
	         this, SLOT (nextcontrolled_noSignal (void)));
	connect (&channelTimer, SIGNAL (timeout (void)),
	         this, SLOT (nextcontrolled_withSignal (void)));
	channelTimer. start (channelDelay -> value () * 1000);
	my_dabProcessor -> start (frequency);
}


//////////////////////////////////////////////////////////////////////////
//	CONTINUOUS SECTION
//////////////////////////////////////////////////////////////////////////

void	RadioInterface::handle_continuousButton  () {
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
//	here we really start
	   my_dabProcessor	= new dabProcessor (this,
	                                            theDevice,
	                                            dabMode,
	                                            threshold_1,
	                                            threshold_2,
	                                            diff_length,
	                                            spectrumBuffer,
                                                    iqBuffer);

	   connect (my_dabProcessor, SIGNAL (show_snr (int)),
	            this, SLOT (show_snr (int)));
	   connect (my_dabProcessor, SIGNAL (setSynced (bool)),
	            this, SLOT (setSynced (bool)));
	   connect (my_dabProcessor, SIGNAL (show_tii (int)),
	            this, SLOT (show_tii (int)));
	}

	Services	= QStringList ();
	QString reportName	= find_fileName ();
	fileP		= fopen (reportName. toUtf8(). data(), "w");
	if (fileP == nullptr) {
	   fprintf (stderr, "Could not open file %s\n",
	                           reportName. toUtf8(). data());
	   return;
	}

	QString timeString = QDate::currentDate (). toString ();
        timeString. append ("-");
        timeString. append (QTime::currentTime (). toString ());
        localTimeDisplay        -> setText (timeString);
        QString theTime = localTimeDisplay -> text ();
        theTime. replace (":", "-");
        theTime. replace (" ", "-");
        QString summaryName = dirName;
        summaryName. append ("/dab-summary-");
        summaryName. append (theTime);
        summaryName. append (".txt");
	fprintf (stderr, "summaryName = %s\n", summaryName. toUtf8 (). data ());
	summaryP	= fopen (summaryName. toUtf8(). data(), "w");
	if (summaryP == nullptr) {
	   fprintf (stderr, "Could not open  summary file %s\n",
	                           summaryName. toUtf8(). data());
	   fclose (fileP);
	   return;
	}

	displayTable	-> hide ();
	startButton	-> hide ();
	nrCycles	-> hide ();
	skipTableSelect	-> hide ();
	showTable	-> show ();
	connect (showTable, SIGNAL (clicked ()),
	         this, SLOT (handle_showTable ()));
	if (channelTable == nullptr)
	   channelTable	= new channelsTable (this,
                                             theBand,
                                             channelFileName);
	operationsLabel	-> setText ("running continuously");
	continuousButton	-> setText ("stop");
	disconnect (continuousButton, SIGNAL (clicked ()),
	            this, SLOT (handle_continuousButton ()));
	connect (continuousButton, SIGNAL (clicked ()),
	         this, SLOT (stopContinuous ()));
	channelTable	-> show ();
	channelNumber. store (-1);
	for (int i = 0; i < theBand -> channels (); i ++) {
	   if (channelTable -> channel (i)) {
	      channelNumber = i;
	      break;
	   }
	}
	if (channelNumber. load () == -1) {
	   QMessageBox::warning (this, tr ("Warning"),
                                       tr ("no channels in scanlist\n"));
	   stopContinuous ();
	   return;
	}

	channelDisplay -> setText (theBand -> channel (channelNumber. load ()));
	connect (my_dabProcessor, SIGNAL (noSignal_Found (void)),
	         this, SLOT (nextcontinuous_noSignal (void)));
	connect (&channelTimer, SIGNAL (timeout (void)),
	         this, SLOT (nextcontinuous_withSignal (void)));
	channelTimer. start (channelDelay -> value () * 1000);
	
	my_dabProcessor -> start (theBand -> Frequency (channelNumber));
	running. store (CONTINUOUS);
}

void	RadioInterface::stopContinuous	() {
	showTable	-> hide ();
	disconnect (showTable, SIGNAL (clicked ()),
	            this, SLOT (handle_showTable ()));
	disconnect (my_dabProcessor, SIGNAL (noSignal_Found (void)),
	            this, SLOT (nextcontinuous_noSignal (void)));
	disconnect (&channelTimer, SIGNAL (timeout (void)),
	            this, SLOT (nextcontinuous_withSignal (void)));
	disconnect (continuousButton, SIGNAL (clicked ()),
	            this, SLOT (stopContinuous ()));
	connect (continuousButton, SIGNAL (clicked ()),
	            this, SLOT (handle_continuousButton ()));
	theDevice     		-> stopReader ();
	my_dabProcessor		-> stop ();
	channelTimer. stop ();
	running. store (NOT_RUNNING);	
	startButton		-> show ();
	continuousButton	-> setText ("start continuous");
	continuousButton	-> show ();
	nrCycles		-> show ();
	skipTableSelect		-> show ();
	channelTable		-> hide ();
}

void	RadioInterface::nextcontinuous_noSignal (void) {
	nextcontinuous ();
}

void	RadioInterface::nextcontinuous_withSignal (void) { 
	nextcontinuous ();
}

void	RadioInterface::nextcontinuous	() {
	disconnect (my_dabProcessor, SIGNAL (noSignal_Found (void)),
	            this, SLOT (nextcontinuous_noSignal (void)));
	disconnect (&channelTimer, SIGNAL (timeout (void)),
	            this, SLOT (nextcontinuous_withSignal (void)));
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
	for (int i = 0; i < nrChannels; i ++) {
	   if (skipChannel ((channelNumber. load () + i) % nrChannels)) {
	      skipCount ++;
	      fprintf (stderr, "skipping channel %d\n",
	                           (channelNumber. load () + i) % nrChannels);
	   }
	   else {
	      channelNumber. store ((channelNumber. load () + i) % nrChannels);
	      break;
	   }
	}
	if (skipCount >= nrChannels) {
	   QMessageBox::warning (this, tr ("Warning"),
	                         tr ("no channels in scanlist\n"));
	   stopContinuous ();
	   return;
	}
//	resume scanning
	int32_t frequency	= theBand -> Frequency (channelNumber);
	channelDisplay -> setText (theBand -> channel (channelNumber));
	connect (my_dabProcessor, SIGNAL (noSignal_Found (void)),
	         this, SLOT (nextcontinuous_noSignal (void)));
	connect (&channelTimer, SIGNAL (timeout (void)),
	         this, SLOT (nextcontinuous_withSignal (void)));
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
	if (channelTable != nullptr) {
	   channelTable	-> hide ();
           dabSettings -> setValue ("channelFile",
	                             channelTable -> FileName ());
           dabSettings -> sync ();
	   delete channelTable;
        }

	delete displayTable;
	if (my_dabProcessor != NULL) {
	   my_dabProcessor	-> stop ();	// definitely concurrent
//	everything should be halted by now
	   delete	my_dabProcessor;
	}
	if (theDevice != NULL)
	   delete	theDevice;
	delete my_spectrumViewer;
	dabSettings	-> setValue ("device", deviceSelector -> currentText ());
	dabSettings	-> sync ();
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

	my_spectrumViewer	-> setBitDepth (theDevice -> bitDepth ());
	my_dabProcessor	= new dabProcessor (this,
	                                    theDevice,
	                                    dabMode,
	                                    threshold_1,
	                                    threshold_2,
	                                    diff_length,
	                                    spectrumBuffer,
	                                    iqBuffer);
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
#ifdef	HAVE_SDRPLAY_V2
	if (s == "sdrplay v2") {
	   try {
	      return  new sdrplayHandler_v2 (dabSettings);
	   } catch (int e) {}
	}
	else
#endif
#ifdef	HAVE_SDRPLAY_V3
	if (s == "sdrplay v3") {
	   try {
	      QString temp = "dab-scanner";
	      return  new sdrplayHandler_v3 (dabSettings, temp);
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
	return nullptr;
}

QString extractTransmitters (std::vector<int> tiiValue) {
QString transmitters;

	for (uint16_t i = 0; i < tiiValue. size (); i ++) {
           char buffer [20];
           sprintf (buffer, " (%d %d)",
	                   tiiValue. at (i) >> 8, tiiValue. at (i) & 0xFF);
           transmitters. append (buffer);
        }
	return transmitters;
}

void	RadioInterface::showEnsembleData	(int snr,
	                                         std::vector<int> tiiValue) {
QString currentChannel	= theBand -> channel (channelNumber);
int32_t	frequency	= theDevice -> getVFOFrequency();
QString theTime		= localTimeDisplay -> text ();
ensemblePrinter	my_Printer;

	if (!running. load())
	   return;

	if ((running. load () == CONTROLLED) && (nrCycles -> value () == 1)) {
	   QString transmitters = extractTransmitters (tiiValue);
	   displayTable ->  newEnsemble (currentChannel,
	                          ensembleDisplay -> text (),
	                          QString::number (frequency / 1000),
	                          QString::number (snr),
	                          transmitters);
	   for (QString& audioService: Services) {
	      audiodata d;
	      my_dabProcessor -> dataforAudioService (audioService, &d, 0);
	      if (!d. defined)
	         continue;
	      QString bitRate	= QString::number (d. bitRate);
	      QString protL     = getProtectionLevel (d. shortForm,
	                                              d. protLevel);
	      QString codeRate  = getCodeRate (d. shortForm,
	                                       d. protLevel);
	      displayTable ->
	          add_to_Ensemble (audioService, 
	                           d. ASCTy == 077 ? "DAB+" : "DAB",
	                           bitRate, protL, codeRate);
	   }
	}
	   
	my_Printer. showEnsembleData (currentChannel,
	                              frequency,
	                              snr,
	                              tiiValue,
	                              theTime,
	                              Services, my_dabProcessor, fileP);
	if (running. load () == CONTINUOUS) {
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
	for (uint i = 0; i < tii_Value. size (); i ++)
	   if (tii_Value. at (i) == tii)
	      return;
	tii_Value. push_back (tii);
	for (uint i = 0; i < tii_Value. size (); i ++) {
	   sprintf (buffer, "(%d %d) ", tii_Value. at (i) >> 8,
	                                tii_Value. at (i) & 0xFF);
	   s. append (buffer);
	};
	tii_Label -> setText (s);
}

QString	RadioInterface::find_fileName () {
QString timeString = QDate::currentDate (). toString ();

	timeString. append ("-");
	timeString. append (QTime::currentTime (). toString ());
	localTimeDisplay	-> setText (timeString);
	QString theTime		= localTimeDisplay -> text ();
	theTime. replace (":", "-");
	theTime. replace (" ", "-");
	QString suggestedFileName = dirName;
	fprintf (stderr, "dirName = %s\n", dirName. toUtf8 (). data ());
	suggestedFileName. append ("./dab-scanner-");
	suggestedFileName. append (theTime);
	suggestedFileName. append (".txt");
	fprintf (stderr, "suggested filename = %s\n",
	             suggestedFileName. toUtf8 (). data ());
	QString fileName = QFileDialog::getSaveFileName (this,
	                                        tr ("Save file ..."),
	                                        suggestedFileName,
	                                        tr ("Text (*.txt)"));
	int ind         = fileName. lastIndexOf ("/");
        fileName        = QDir::toNativeSeparators (fileName);
        dirName         = fileName;
	dirName. remove (ind, 100); 
	dabSettings     -> setValue ("dirName", dirName);
	return fileName;
}

bool	RadioInterface::skipChannel (int channelNumber) {
	if (channelTable == NULL)	// should not happen
	   return false;
	return !channelTable	-> channel (channelNumber);
}

///////////////////////////////////////////////////////////////////////////
//                                                                       //
//   Looking at the spectrum may give some indications on the signal     //
//   and the setting of the device                                       //
///////////////////////////////////////////////////////////////////////////

//      signal, received from ofdm_decoder that a buffer is filled
//      with amount values ready for display
void    RadioInterface::showIQ  (int amount) {
        if (running. load())
           my_spectrumViewer    -> showIQ (amount);
}

void    RadioInterface::showSpectrum    (int32_t amount) {
        if (running. load())
           my_spectrumViewer -> showSpectrum (amount,
                                            theDevice -> getVFOFrequency());
}

void    RadioInterface::showQuality     (double q) {
        if (running. load())
           my_spectrumViewer    -> showQuality (q);
}

void    RadioInterface::set_spectrumSwitch() {
        if (my_spectrumViewer -> isHidden())
           my_spectrumViewer -> show();
        else
           my_spectrumViewer -> hide();
}

void	RadioInterface::handle_skipTableSelect () {
	QString file   = QFileDialog::getSaveFileName (this,
                                                       tr ("Open file ..."),
                                                       QDir::homePath(),
                                                       tr ("XML files (*.xml)"), 0, QFileDialog::DontConfirmOverwrite);
	if (file == QString ("")) {
	   return;
	}
	channelFileName         = QDir::toNativeSeparators (file);
	skipfileName	-> setText (channelFileName);
}

void	RadioInterface::handle_showTable	() {
	if ((running. load () == CONTINUOUS) && (channelTable != nullptr)) {
	   if (channelTable -> isHidden ())
	      channelTable -> show ();
	   else
	      channelTable -> hide ();
	}
}

