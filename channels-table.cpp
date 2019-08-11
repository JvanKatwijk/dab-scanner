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

#include	"channels-table.h"
#include	"band-handler.h"
#include	<QHeaderView>
#include	<QString>


	channelsTable::channelsTable	(QSettings *si,
	                                 bandHandler	*theBand):
	                                    theTable (NULL) {
	this	-> theSettings	= si;
	this	-> theBand	= theBand;

	theTable. setColumnCount (2);
	QStringList header;
	header	<< tr ("channel") << tr ("scan");
	theTable. setHorizontalHeaderLabels (header);
	theTable. verticalHeader() -> hide ();
	theTable. setShowGrid (true);
	theSettings -> beginGroup ("Channels");
	for (int i = 0; i < theBand -> channels (); i ++) {
	   theTable. insertRow (i);
	   theTable. setItem (i, 0, new QTableWidgetItem (theBand -> channel (i)));
	   QString s1 = theSettings -> value (theBand -> channel (i), "+").
	                                                 toString ();
	   theTable. setItem (i, 1, new QTableWidgetItem (s1));
	}
	theSettings -> endGroup ();
	connect (&theTable, SIGNAL (cellDoubleClicked (int, int)),
	         this, SLOT (cellSelected (int, int)));
	isVisible	= false;
}

	channelsTable::~channelsTable () {
	if (isVisible)
	   theTable. hide ();

	theSettings	-> beginGroup ("Channels");
	for (int i = 0; i < theBand -> channels (); i ++) {
	   theSettings -> setValue (theBand -> channel (i),
	                       theTable. item (i, 1) -> text ());
	}
	theSettings	-> endGroup ();
}

void    channelsTable::cellSelected (int row, int column) {
        QString s1 = theTable. item (row, 0) ->text ();
        QString s2 = theTable. item (row, 1) ->text ();
        if (s2 == "+")
           theTable. item (row, 1) -> setText ("-");
        else
           theTable. item (row, 1) -> setText ("+");
}

void	channelsTable::show () {
	theTable. show ();
	isVisible	= true;
}

void	channelsTable::hide () {
	theTable. hide ();
	isVisible	= false;
}

bool	channelsTable::channel (int chNumber) {
	QString s1 = theTable. item (chNumber, 1) -> text ();
	return s1 == "+";
}
