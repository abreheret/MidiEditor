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

#include "MidiSettingsWidget.h"

#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QLineEdit>
#include <QGridLayout>
#include <QCheckBox>
#include <QSettings>
#include <QTextEdit>
#include <QSpinBox>
#include "../midi/MidiOutput.h"
#include "../midi/MidiInput.h"
#include "../midi/MidiFile.h"
#include "../Terminal.h"


MidiSettingsWidget::MidiSettingsWidget(QWidget *parent) : SettingsWidget("Midi I/O", parent) {

	QGridLayout *layout = new QGridLayout(this);
	setLayout(layout);

	int row = 0;

	QWidget *playerModeInfo = createInfoBox("Choose the Midi ports on your machine to which MidiEditor connects in order to play and record Midi data.");
	layout->addWidget(playerModeInfo, row++, 0, 1, 6);

	// output
	layout->addWidget(new QLabel("Midi output: ", this), row, 0, 1, 2);
	// input
	layout->addWidget(new QLabel("Midi input: ", this), row,3,1,2);

	_outList = new QListWidget(this);
	connect(_outList, SIGNAL(itemChanged(QListWidgetItem*)), this,
			SLOT(outputChanged(QListWidgetItem*)));

	_inList = new QListWidget(this);
	connect(_inList, SIGNAL(itemChanged(QListWidgetItem*)), this,
			SLOT(inputChanged(QListWidgetItem*)));

	QPushButton *reloadOutputList = new QPushButton();
	reloadOutputList->setToolTip("Refresh port list");
	reloadOutputList->setFlat(true);
	reloadOutputList->setIcon(QIcon(":/run_environment/graphics/tool/refresh.png"));
	reloadOutputList->setFixedSize(30, 30);
	layout->addWidget(reloadOutputList, row, 2, 1, 1);
	connect(reloadOutputList, SIGNAL(clicked()), this,
			SLOT(reloadOutputPorts()));
	reloadOutputPorts();

	QPushButton *reloadInputList = new QPushButton();
	reloadInputList->setFlat(true);
	layout->addWidget(reloadInputList, row++, 5, 1, 1);
	reloadInputList->setToolTip("Refresh port list");
	reloadInputList->setIcon(QIcon(":/run_environment/graphics/tool/refresh.png"));
	reloadInputList->setFixedSize(30, 30);
	connect(reloadInputList, SIGNAL(clicked()), this, SLOT(reloadInputPorts()));
	reloadInputPorts();

	layout->addWidget(_outList, row, 0, 1, 3);
	layout->addWidget(_inList, row++, 3, 1, 3);
}

void MidiSettingsWidget::reloadInputPorts(){

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

void MidiSettingsWidget::reloadOutputPorts(){

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

void MidiSettingsWidget::inputChanged(QListWidgetItem *item){

	if(item->checkState() == Qt::Checked){

		MidiInput::setInputPort(item->text());

		reloadInputPorts();
	}
}

void MidiSettingsWidget::outputChanged(QListWidgetItem *item){

	if(item->checkState() == Qt::Checked){

		MidiOutput::setOutputPort(item->text());

		reloadOutputPorts();
	}
}
