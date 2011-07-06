#include "FileLengthDialog.h"

#include <QLabel>
#include <QSpinBox>
#include <QGridLayout>
#include <QPushButton>
#include "../midi/MidiFile.h"
#include "../protocol/Protocol.h"

FileLengthDialog::FileLengthDialog(MidiFile *f, QWidget *parent) :
		QDialog(parent)
{
	_file = f;
	QLabel *text = new QLabel("Max. time: ", this);
	_box = new QSpinBox(this);
	_box->setMinimum(1);
	_box->setMaximum(2147483647);
	_box->setValue(_file->maxTime());
	QPushButton *breakButton = new QPushButton("break");
	connect(breakButton, SIGNAL(clicked()), this, SLOT(hide()));
	QPushButton *acceptButton = new QPushButton("accept");
	connect(acceptButton, SIGNAL(clicked()), this, SLOT(accept()));

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(text,0,0,1,1);
	layout->addWidget(_box, 0, 1, 1, 2);
	layout->addWidget(breakButton, 1, 0, 1, 1);
	layout->addWidget(acceptButton, 1, 2, 1, 1);
	layout->setColumnStretch(1, 1);
}

void FileLengthDialog::accept(){
	_file->protocol()->startNewAction("Change max. Length");
	_file->setMaxLengthMs(_box->value());
	_file->protocol()->endAction();
	hide();
}
