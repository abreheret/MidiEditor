/*
 * MidiEditor
 * Copyright (C) 2010  Markus Schwenk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MidiSettingsDialog.h"

#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QLineEdit>
#include <QGridLayout>

#include "../midi/MidiOutput.h"
#include "../midi/MidiInput.h"

MidiSettingsDialog::MidiSettingsDialog(QWidget *parent) : QDialog(parent) {

	setWindowTitle("Midi Settings");

	QGridLayout *layout = new QGridLayout(this);
	setLayout(layout);

	// output
	layout->addWidget(new QLabel("Send Output to: ", this), 0,0,1,3);
	_outList = new QListWidget(this);
	connect(_outList, SIGNAL(itemChanged(QListWidgetItem*)), this,
			SLOT(outputChanged(QListWidgetItem*)));

	layout->addWidget(_outList, 1, 0, 1, 3);
	QPushButton *reloadOutputList = new QPushButton("Reload");
	layout->addWidget(reloadOutputList, 2, 2, 1, 1);
	connect(reloadOutputList, SIGNAL(clicked()), this,
			SLOT(reloadOutputPorts()));
	reloadOutputPorts();

	// input
	layout->addWidget(new QLabel("Receive Input from: ", this), 0,3,1,3);
	_inList = new QListWidget(this);
	connect(_inList, SIGNAL(itemChanged(QListWidgetItem*)), this,
			SLOT(inputChanged(QListWidgetItem*)));

	layout->addWidget(_inList, 1, 3, 1, 3);
	QPushButton *reloadInputList = new QPushButton("Reload");
	layout->addWidget(reloadInputList, 2, 5, 1, 1);
	connect(reloadInputList, SIGNAL(clicked()), this,
			SLOT(reloadInputPorts()));
	reloadInputPorts();

	// horizontal line
	QFrame *f0 = new QFrame( this );
	f0->setFrameStyle( QFrame::HLine | QFrame::Sunken );
	layout->addWidget( f0, 3, 0, 1, 6);

	// ok button
	QPushButton *okButton = new QPushButton("Ok", this);
	connect(okButton, SIGNAL(clicked()), this, SLOT(hide()));
	layout->addWidget(okButton, 4, 5, 1, 1);
}

void MidiSettingsDialog::reloadInputPorts(){

	disconnect(_inList, SIGNAL(itemChanged(QListWidgetItem*)), this,
			SLOT(inputChanged(QListWidgetItem*)));

	// clear the list
	_inList->clear();

	foreach(QString name, MidiInput::inputPorts()){

		QListWidgetItem *item = new QListWidgetItem(name, _inList,
				QListWidgetItem::UserType);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled |
				Qt::ItemIsUserCheckable);

		if(name == MidiInput::inputPort()){
			item->setCheckState(Qt::Checked);
		} else {
			item->setCheckState(Qt::Unchecked);
		}
		_inList->addItem(item);
	}
	connect(_inList, SIGNAL(itemChanged(QListWidgetItem*)), this,
			SLOT(inputChanged(QListWidgetItem*)));
}

void MidiSettingsDialog::reloadOutputPorts(){

	disconnect(_outList, SIGNAL(itemChanged(QListWidgetItem*)), this,
			SLOT(outputChanged(QListWidgetItem*)));

	// clear the list
	_outList->clear();

	foreach(QString name, MidiOutput::outputPorts()){

		QListWidgetItem *item = new QListWidgetItem(name, _outList,
				QListWidgetItem::UserType);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled |
				Qt::ItemIsUserCheckable);

		if(name == MidiOutput::outputPort()){
			item->setCheckState(Qt::Checked);
		} else {
			item->setCheckState(Qt::Unchecked);
		}
		_outList->addItem(item);
	}
	connect(_outList, SIGNAL(itemChanged(QListWidgetItem*)), this,
			SLOT(outputChanged(QListWidgetItem*)));
}

void MidiSettingsDialog::inputChanged(QListWidgetItem *item){

	if(item->checkState() == Qt::Checked){

		MidiInput::setInputPort(item->text());

		reloadInputPorts();
	}
}

void MidiSettingsDialog::outputChanged(QListWidgetItem *item){

	if(item->checkState() == Qt::Checked){

		MidiOutput::setOutputPort(item->text());

		reloadOutputPorts();
	}
}
