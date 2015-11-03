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

#include "PitchBendEvent.h"

#include <QSpinBox>
#include <QBoxLayout>
#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include "../midi/MidiFile.h"

PitchBendEvent::PitchBendEvent(int channel, int value) :
		MidiEvent(channel)
{
	_value = value;
}

PitchBendEvent::PitchBendEvent(PitchBendEvent &other) :
		MidiEvent(other)
{
	_value = other._value;
}

int PitchBendEvent::line(){
	return PITCH_BEND_LINE;
}

QString PitchBendEvent::toMessage(){
	return "cc "+QString::number(channel())+" "+
			QString::number(_value);
}

QByteArray PitchBendEvent::save(){
	QByteArray array = QByteArray();
	array.append(0xE0 | channel());
	array.append(_value & 0x7F);
	array.append((_value >> 7) & 0x7F);
	return array;
}

ProtocolEntry *PitchBendEvent::copy(){
	return new PitchBendEvent(*this);
}

void PitchBendEvent::reloadState(ProtocolEntry *entry){
	PitchBendEvent *other = dynamic_cast<PitchBendEvent*>(entry);
	if(!other){
		return;
	}
	MidiEvent::reloadState(entry);
	_value = other->_value;
}

QString PitchBendEvent::typeString(){
	return "Pitch Bend Event";
}

int PitchBendEvent::value(){
	return _value;
}

void PitchBendEvent::setValue(int v){
	ProtocolEntry *toCopy = copy();
	_value = v;
	protocol(toCopy, this);
	if(shownInEventWidget()){
		_value_box->setValue(v);
	}
}

// Widgets for EventWidget
QWidget *PitchBendEvent::_value_widget = 0;
QSpinBox *PitchBendEvent::_value_box = 0;
QLabel *PitchBendEvent::_value_label = 0;

void PitchBendEvent::generateWidget(QWidget *widget){

	// general data
	MidiEvent::generateWidget(widget);

	// first call
	if(_value_widget == 0) _value_widget = new QWidget();
	if(_value_box == 0) _value_box = new QSpinBox();
	if(_value_label == 0) _value_label = new QLabel();

	// set Parents
	_value_widget->setParent(widget);
	_value_box->setParent(_value_widget);
	_value_label->setParent(_value_widget);

	// unhide
	_value_widget->setHidden(false);
	_value_box->setHidden(false);
	_value_label->setHidden(false);


	// value
	_value_label->setText("Value: ");
	QLayout *valL = _value_widget->layout();
	if(!valL){
		valL = new QBoxLayout(QBoxLayout::LeftToRight, _value_widget);
		_value_widget->setLayout(valL);
	}
	valL->addWidget(_value_label);

	// box
	_value_box->setMaximum(16383);
	_value_box->setMinimum(0);
	_value_box->setValue(_value);
	valL->addWidget(_value_box);

	// Add Widgets
	QLayout *layout = widget->layout();
	layout->addWidget(_value_widget);
}

void PitchBendEvent::editByWidget(){
	if(_value_box->value()!=value()){
		setValue(_value_box->value());
	}
	MidiEvent::editByWidget();
}

bool PitchBendEvent::isOnEvent(){
//	return (_control < 64 && _control> 69) || _value > 64;
	return false;
}
