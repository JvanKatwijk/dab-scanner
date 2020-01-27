#
/*
 *    Copyright (C) 2019
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
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
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef	__OUTPUT_TABLE__
#define	__OUTPUT_TABLE__

#include	<QWidget>
#include	<QObject>
#include	<QScrollArea>
#include	<QTableWidget>
#include	<QStringList>
#include	<QTableWidgetItem>
#include	<QObject>
#include	<QString>

class	RadioInterface;


class	outputTable: QObject {
Q_OBJECT
public:
		outputTable	(RadioInterface *);
		~outputTable	();
	void	show		();
	void	hide		();
	void	newEnsemble	(QString, QString, QString, QString, QString);
	void	add_to_Ensemble	(QString, QString, QString, QString, QString);
	void	clear		();
private:
	QScrollArea	*myWidget;
	QTableWidget	*outputWidget;
	int16_t		addRow	();
};

#endif

