#include "DonateDialog.h"

#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>

DonateDialog::DonateDialog(QWidget *parent):QDialog(parent)
{
	setWindowIcon(QIcon("graphics/icon.png"));

	QGridLayout *layout = new QGridLayout;

	//Label starttext
	QLabel *title = new QLabel("<h2><u>Donate</u></h2><font" \
		" color='#666666'>Please donate if this software is useful for you. " \
		 "<br>To donate visit my Homepage:<br><a href=\"www.markus-schwenk.de?"\
		 "category=donate\">www.markus-schwenk.de/index.php?category=donate"\
		 "</a></font> ");

	layout->addWidget(title, 0,0,1,4);

	// button close
	QPushButton *close = new QPushButton("Close");
	layout->addWidget(close, 1, 3, 1, 1);
	connect(close, SIGNAL(clicked()), this, SLOT(hide()));

	setLayout(layout);
	setWindowTitle(tr("Donate"));
}
