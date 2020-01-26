#include        <QDataStream>
#include	"output-table.h"
#include        "radio.h"


	outputTable::outputTable (RadioInterface *) {
	myWidget        = new QScrollArea (NULL);
        myWidget        -> resize (200, 200);
        myWidget        -> setWidgetResizable(true);

        outputWidget     = new QTableWidget (0, 5);
	outputWidget	-> setColumnWidth (0, 200);
        myWidget        -> setWidget (outputWidget);
        outputWidget     -> setHorizontalHeaderLabels (
	                  QStringList () << tr ("scan result"));
}

	outputTable::~outputTable () {
int16_t i;
int16_t rows    = outputWidget -> rowCount ();

        for (i = rows; i > 0; i --)
           outputWidget -> removeRow (i);
        delete  outputWidget;
        delete  myWidget;
}

void	outputTable::show	() {
	myWidget	-> show ();
}

void	outputTable::hide	() {
	myWidget	-> hide ();
}

int16_t	outputTable::addRow () {
int16_t row     = outputWidget -> rowCount ();

        outputWidget     -> insertRow (row);
        QTableWidgetItem *item0 = new QTableWidgetItem;
        item0           -> setTextAlignment (Qt::AlignLeft |Qt::AlignVCenter);
        outputWidget     -> setItem (row, 0, item0);

        QTableWidgetItem *item1 = new QTableWidgetItem;
        item1           -> setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        outputWidget     -> setItem (row, 1, item1);

        QTableWidgetItem *item2 = new QTableWidgetItem;
        item2           -> setTextAlignment (Qt::AlignRight |Qt::AlignVCenter);
        outputWidget    -> setItem (row, 2, item2);

        QTableWidgetItem *item3 = new QTableWidgetItem;
        item3           -> setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        outputWidget    -> setItem (row, 3, item3);

        QTableWidgetItem *item4 = new QTableWidgetItem;
        item4           -> setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        outputWidget    -> setItem (row, 4, item4);
	return row;
}

void	outputTable::newEnsemble	(QString currentChannel,
	                                 QString ensembleName,
	                                 QString freq,
	                                 QString snr,
	                                 QString transmitters) {
int16_t row	= addRow ();

	outputWidget	-> item (row, 0) -> setText (currentChannel);
	outputWidget	-> item (row, 1) -> setText (ensembleName);
	outputWidget	-> item (row, 2) -> setText (freq);
	outputWidget	-> item (row, 3) -> setText (snr);
	outputWidget	-> item (row, 4) -> setText (transmitters);

	row	= addRow ();
	outputWidget	-> item (row, 0) -> setText ("serviceName");
	outputWidget	-> item (row, 1) -> setText ("type ");
	outputWidget	-> item (row, 2) -> setText ("bit rate");
	outputWidget	-> item (row, 3) -> setText ("protection level");
	outputWidget	-> item (row, 4) -> setText ("code rate");
}

void	outputTable::add_to_Ensemble	(QString serviceName,
	                                 QString dabType,
	                                 QString bitRate,
	                                 QString protLevel,
	                                 QString codeRate) {
int16_t	row = addRow ();

	outputWidget	-> item (row, 0) -> setText (serviceName);
	outputWidget	-> item (row, 1) -> setText (dabType);
	outputWidget	-> item (row, 2) -> setText (bitRate);
	outputWidget	-> item (row, 3) -> setText (protLevel);
	outputWidget	-> item (row, 4) -> setText (codeRate);
}


