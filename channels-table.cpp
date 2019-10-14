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
#include <QDomDocument>
#include	<QMessageBox>
#include	"radio.h"

	channelsTable::channelsTable	(RadioInterface *theRadio,
	                                 bandHandler	*theBand,
	                                 QString	fileName
	                                 ):
	                                    theTable (NULL) {
QDomDocument xml_bestand;
	this	-> theName		= fileName;
	this	-> myRadioInterface	= theRadio;
	this	-> theBand	= theBand;

	
	theTable. setColumnCount (2);
	QStringList header;
	header	<< tr ("channel") << tr ("scan");
	theTable. setHorizontalHeaderLabels (header);
	theTable. verticalHeader() -> hide ();
	theTable. setShowGrid (true);
	for (int i = 0; i < theBand -> channels (); i ++) {
	   theTable. insertRow (i);
	   theTable. setItem (i, 0, new QTableWidgetItem (theBand -> channel (i)));
	   QString s1 =  "+";
	   theTable. setItem (i, 1, new QTableWidgetItem (s1));
	}

	QFile f (fileName);
	if (f. open (QIODevice::ReadOnly)) {
	   xml_bestand. setContent (&f);
	   QDomElement root	= xml_bestand. documentElement ();
	   QDomElement component	= root. firstChild (). toElement ();
	   while (!component. isNull ()) {
	      if (component. tagName () == "BAND_ELEMENT") {
	         QString channel = component. attribute ("CHANNEL", "???");
	         QString skipItem = component. attribute ("VALUE", "+");
	         if (channel != "???")
	            update (channel, skipItem);
	      }
	      component = component. nextSibling (). toElement ();
	   }
	}
	connect (&theTable, SIGNAL (cellDoubleClicked (int, int)),
	         this, SLOT (cellSelected (int, int)));
	isVisible	= false;
}

	channelsTable::~channelsTable () {
QDomDocument skipList;
QDomElement root;

	if (isVisible)
	   theTable. hide ();

	if (theName == "")
	   return;
	root	= skipList. createElement ("skipList");
	skipList. appendChild (root);

	for (int i = 0; i < theBand -> channels (); i ++) {
	   QString channel = theTable. item (i, 0) -> text ();
	   QString theValue = theTable. item (i, 1) -> text ();
	   QDomElement skipElement = skipList.
	                                createElement ("BAND_ELEMENT");
	   skipElement. setAttribute ("CHANNEL", channel);
	   skipElement. setAttribute ("VALUE", theValue);
	   root. appendChild (skipElement);
	}
	QFile file (theName);
        if (!file. open (QIODevice::WriteOnly | QIODevice::Text))
           return;

        QTextStream stream (&file);
        stream << skipList. toString ();
        file. close ();
}

QString	channelsTable::FileName	() {
	return theName;
}

void	channelsTable::update (QString channel, QString value) {
	if ((value != "+") && (value != "-"))
	   return;

	for (int i = 0; i < theBand -> channels (); i ++) {
	   if (theTable. item (i, 0) -> text () == channel) {
	      theTable. item (i, 1) -> setText (value);
	      break;
	   }
	}
}

void    channelsTable::cellSelected (int row, int column) {
        QString s1 = theTable. item (row, 0) ->text ();
        QString s2 = theTable. item (row, 1) ->text ();
	int	amount_P	= 0;
	(void)column;
	for (int i = 0; i < theBand -> channels (); i ++)
	   if (theTable. item (i, 1) -> text () == "+")
	      amount_P ++;
        if (s2 == "-") 
           theTable. item (row, 1) -> setText ("+");
	else 
	   if (amount_P == 1) 
	      QMessageBox::warning (myRadioInterface, tr ("Warning"),
                                    tr ("do not skip all channels!!\n"));
	   else
              theTable. item (row, 1) -> setText ("-");
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
