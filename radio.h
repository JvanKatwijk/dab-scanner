#
/*
 *    Copyright (C) 2013, 2014, 2015, 2016, 2017
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
 *	Main program
 */

#ifndef __DAB_SCANNER__
#define __DAB_SCANNER__

#include	"dab-constants.h"
#include	<QMainWindow>
#include	<QStringList>
#include	<QComboBox>
#include	<QLabel>
#include	<QTimer>
#include	<QList>
#include	<atomic>
#include	<stdio.h>
#include	"ui_dabscanner.h"
#include	"dab-processor.h"
#include	"ensemble-printer.h"
#include        "band-handler.h"
#include	"channels-table.h"
#include	"ringbuffer.h"
#include	"output-table.h"
class	QSettings;
class	virtualInput;
class	spectrumViewer;
/*
 *	GThe main gui object. It inherits from
 *	QDialog and the generated form
 */

class RadioInterface: public QMainWindow,
		      private Ui_dabscanner {
Q_OBJECT
public:
		RadioInterface		(QSettings	*,
	                                 bandHandler	*,
	                                 QWidget	*parent = NULL);
		~RadioInterface		(void);

private:
	QSettings	*dabSettings;
	outputTable	*displayTable;
	FILE		*fileP;
	FILE		*summaryP;
	QString		dirName;
	channelsTable	*channelTable;
	std::atomic<int>	channelNumber;
	int		serviceCount;
        QStringList     Services;
	uint8_t		dabBand;
	uint8_t		dabMode;
	uint8_t		isSynced;
	int16_t		threshold_1;
	int16_t		threshold_2;
	int16_t		diff_length;
	bandHandler	*theBand;
	int		channels;
	std::atomic<uint8_t>	running;
	deviceHandler	*theDevice;
	dabProcessor	*my_dabProcessor;
	QTimer		channelTimer;
	int16_t		ficBlocks;
	int16_t		ficSuccess;
	RingBuffer<std::complex<double>>  *spectrumBuffer;
	RingBuffer<std::complex<double>>  *iqBuffer;

	spectrumViewer	*my_spectrumViewer;
	std::vector<int>	tii_Value;
	void		TerminateProcess	(void);
	void		startScanning		(void);
	void		stopScanning		(void);
	void		showEnsembleData	(int, std::vector <int>);
	deviceHandler	*setDevice		(QString);
	QString		find_fileName		(void);
	bool		skipChannel		(int);
	void		process_nextChannel	(void);
	QString		channelFileName;
protected:
        bool    eventFilter (QObject *obj, QEvent *event);

public slots:
	void		handle_skipTableSelect	(void);
	void		nextcontrolled_withSignal	(void);
	void		nextcontrolled_noSignal	(void);
	void		nextcontrolled		();
	void		nextcontinuous_withSignal	(void);
	void		nextcontinuous_noSignal	(void);
	void		nextcontinuous		();
	void		addtoEnsemble		(const QString &);
	void		nameofEnsemble		(int, const QString &);
	void		setSynced		(bool);
	void		closeEvent		(QCloseEvent *event);
	void		show_snr		(int);
	void		showTime		(const QString &s);
	void		show_tii		(int);
	void		set_CorrectorDisplay	(int);
	void		show_ficSuccess		(bool);
	void            showSpectrum            (int);
        void            showIQ                  (int);
        void            showQuality             (double);

//	Somehow, these must be connected to the GUI
private slots:
	void		selectDevice		(QString);
	void		handle_controlledButton	(void);
	void		handle_continuousButton	(void);
	void		reset			(void);
	void		set_spectrumSwitch	(void);

	void		stopControlled		(void);
	void		stopContinuous		(void);
	void		handle_showTable	(void);
};
#endif

