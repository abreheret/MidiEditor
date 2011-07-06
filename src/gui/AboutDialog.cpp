#include "AboutDialog.h"

#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>

AboutDialog::AboutDialog(QWidget *parent):QDialog(parent)
{
	setWindowIcon(QIcon("graphics/icon.png"));

	QGridLayout *layout = new QGridLayout;

	//Label starttext
	QLabel *title = new QLabel("<h2><u>MidiEditor</u></h2><font" \
		" color='#666666'><br>About<br><br>Copyright by Markus Schwenk" \
		 " (kontakt@markus-schwenk.de)<br><br>Visit my Homepage: <a href=\"" \
		 "www.markus-schwenk.de\">www.markus-schwenk.de</a></font> ");

	layout->addWidget(title, 0,0,1,4);

	QFrame *f0 = new QFrame( this );
	f0->setFrameStyle( QFrame::HLine | QFrame::Sunken );
	layout->addWidget( f0, 1, 0, 1, 4);


	QLabel *version = new QLabel("Version: MidiEditor, v. 1.0.1");
	layout->addWidget(version, 2,0,1,4);

	QLabel *date = new QLabel("(8.4.2011)");
	layout->addWidget(date, 3, 0, 1, 4);

	QFrame *f3 = new QFrame( this );
	f3->setFrameStyle( QFrame::HLine | QFrame::Sunken );
	layout->addWidget( f3, 4, 0, 1, 4);

	QLabel *comment = new QLabel("Bugs:");
	layout->addWidget(comment, 5, 0, 1, 4);

	QLabel *mail = new QLabel("bugs@markus-schwenk.de");
	layout->addWidget(mail, 6, 0, 1, 4);

	QFrame *f = new QFrame( this );
	f->setFrameStyle( QFrame::HLine | QFrame::Sunken );
	layout->addWidget( f, 7, 0, 1, 4);

	// button close
	QPushButton *close = new QPushButton("Close");
	layout->addWidget(close, 8, 3, 1, 1);
	connect(close, SIGNAL(clicked()), this, SLOT(hide()));

	setLayout(layout);
	setWindowTitle(tr("About"));
}
