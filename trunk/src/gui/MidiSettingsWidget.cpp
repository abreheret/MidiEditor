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
#include "../midi/MidiOutput.h"
#include "../midi/MidiInput.h"
#include "../Terminal.h"

AdditionalMidiSettingsWidget::AdditionalMidiSettingsWidget(QSettings *settings, QWidget *parent) : SettingsWidget("Midi Settings", parent) {

	_settings = settings;

	QGridLayout *layout = new QGridLayout(this);
	setLayout(layout);

	_alternativePlayerModeBox = new QCheckBox("Manually stop notes", this);
	_alternativePlayerModeBox->setChecked(MidiOutput::isAlternativePlayer);

	connect(_alternativePlayerModeBox, SIGNAL(toggled(bool)), this, SLOT(manualModeToggled(bool)));
	layout->addWidget(_alternativePlayerModeBox, 0, 0, 1, 6);

	QWidget *playerModeInfo = createInfoBox("Note: the above option should not be enabled in general. It is only required if the stop button does not stop playback as expected (e.g. when some notes are not stopped correctly).");
	layout->addWidget(playerModeInfo, 1, 0, 1, 6);

	layout->addWidget(separator(), 2, 0, 1, 6);

	layout->addWidget(new QLabel("Start command:", this), 3, 0, 1, 1);
	startCmd = new QLineEdit(this);
	layout->addWidget(startCmd, 3, 1, 1, 5);

	QWidget *startCmdInfo = createInfoBox("The start command can be used to start additional software components (e.g. Midi synthesizers) each time, MidiEditor is started. You can see the output of the started software / script in the field below.");
	layout->addWidget(startCmdInfo, 4, 0, 1, 6);

	layout->addWidget(Terminal::terminal()->console(), 5, 0, 1, 6);

	startCmd->setText(_settings->value("start_cmd", "").toString());
	layout->setRowStretch(5, 1);
}

void AdditionalMidiSettingsWidget::manualModeToggled(bool enable){
	MidiOutput::isAlternativePlayer = enable;
}

bool AdditionalMidiSettingsWidget::accept(){
	QString text = startCmd->text();
	if(!text.isEmpty()){
		_settings->setValue("start_cmd", text);
	}
	return true;
}

MidiSettingsWidget::MidiSettingsWidget(QWidget *parent) : SettingsWidget("Midi I/O", parent) {

	QGridLayout *layout = new QGridLayout(this);
	setLayout(layout);

	QWidget *playerModeInfo = createInfoBox("Choose the Midi ports on your machine to which MidiEditor connects in order to play and record Midi data.");
	layout->addWidget(playerModeInfo, 0, 0, 1, 6);

	// output
	layout->addWidget(new QLabel("Midi Output: ", this), 1,0,1,2);
	_outList = new QListWidget(this);
	connect(_outList, SIGNAL(itemChanged(QListWidgetItem*)), this,
			SLOT(outputChanged(QListWidgetItem*)));

	layout->addWidget(_outList, 2, 0, 1, 3);
	QPushButton *reloadOutputList = new QPushButton();
	reloadOutputList->setToolTip("Refresh port list");
	reloadOutputList->setFlat(true);
	reloadOutputList->setIcon(QIcon("graphics/tool/refresh.png"));
	reloadOutputList->setFixedSize(30, 30);
	layout->addWidget(reloadOutputList, 1, 2, 1, 1);
	connect(reloadOutputList, SIGNAL(clicked()), this,
			SLOT(reloadOutputPorts()));
	reloadOutputPorts();

	// input
	layout->addWidget(new QLabel("Midi Input: ", this), 1,3,1,2);
	_inList = new QListWidget(this);
	connect(_inList, SIGNAL(itemChanged(QListWidgetItem*)), this,
			SLOT(inputChanged(QListWidgetItem*)));

	layout->addWidget(_inList, 2, 3, 1, 3);
	QPushButton *reloadInputList = new QPushButton();
	reloadInputList->setFlat(true);
	layout->addWidget(reloadInputList, 1, 5, 1, 1);
	reloadInputList->setToolTip("Refresh port list");
	reloadInputList->setIcon(QIcon("graphics/tool/refresh.png"));
	reloadInputList->setFixedSize(30, 30);
	connect(reloadInputList, SIGNAL(clicked()), this,
			SLOT(reloadInputPorts()));
	reloadInputPorts();
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
