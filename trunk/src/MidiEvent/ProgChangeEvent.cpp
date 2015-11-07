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

#include "ProgChangeEvent.h"

#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include "../midi/MidiFile.h"

ProgChangeEvent::ProgChangeEvent(int channel, int prog, MidiTrack *track) : MidiEvent(channel, track){
	_program = prog;
}

ProgChangeEvent::ProgChangeEvent(ProgChangeEvent &other) : MidiEvent(other){
	_program = other._program;
}

int ProgChangeEvent::line(){
	return PROG_CHANGE_LINE;
}

QString ProgChangeEvent::toMessage(){
	return "prog "+QString::number(channel())+" "+QString::number(_program);
}

QByteArray ProgChangeEvent::save(){
	QByteArray array = QByteArray();
	array.append(0xC0 | channel());
	array.append(_program);
	return array;
}

ProtocolEntry *ProgChangeEvent::copy(){
	return new ProgChangeEvent(*this);
}

void ProgChangeEvent::reloadState(ProtocolEntry *entry){
	ProgChangeEvent *other = dynamic_cast<ProgChangeEvent*>(entry);
	if(!other){
		return;
	}
	MidiEvent::reloadState(entry);
	_program = other->_program;
}

QString ProgChangeEvent::typeString(){
	return "Program Change Event";
}

// Widgets for EventWidget
QLabel *ProgChangeEvent::_instr_label = 0;
QComboBox *ProgChangeEvent::_instr_combo = 0;

void ProgChangeEvent::generateWidget(QWidget *widget){

	// general data
	MidiEvent::generateWidget(widget);

	// first call
	if(_instr_label == 0) _instr_label = new QLabel();
	if(_instr_combo == 0){
		_instr_combo = new QComboBox();
		for(int i = 0; i<128; i++){
			_instr_combo->addItem(QString::number(i)+" "+
					file()->instrumentName(i));
		}
	}

	// set Parents
	_instr_label->setParent(widget);
	_instr_combo->setParent(widget);

	// unhide
	_instr_label->setHidden(false);
	_instr_combo->setHidden(false);

	// Program, Instrument
	_instr_label->setText("Instrument:");
	_instr_combo->setCurrentIndex(_program);

	// Add Widgets
	QLayout *layout = widget->layout();
	layout->addWidget(_instr_label);
	layout->addWidget(_instr_combo);
}

void ProgChangeEvent::editByWidget(){
	if(program()!=_instr_combo->currentIndex()){
		setProgram(_instr_combo->currentIndex());
	}
	MidiEvent::editByWidget();
}

int ProgChangeEvent::program(){
	return _program;
}

void ProgChangeEvent::setProgram(int p){
	ProtocolEntry *toCopy = copy();
	_program = p;
	protocol(toCopy, this);
	if(shownInEventWidget()){
		_instr_combo->setCurrentIndex(p);
	}
}
