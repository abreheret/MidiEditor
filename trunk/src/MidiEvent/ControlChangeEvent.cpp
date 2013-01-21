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

#include "ControlChangeEvent.h"

#include <QSpinBox>
#include <QBoxLayout>
#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include "../midi/MidiFile.h"

ControlChangeEvent::ControlChangeEvent(int channel, int control, int value) :
		MidiEvent(channel)
{
	_control = control;
	_value = value;
}

ControlChangeEvent::ControlChangeEvent(ControlChangeEvent &other) :
		MidiEvent(other)
{
	_value = other._value;
	_control = other._control;
}

int ControlChangeEvent::line(){
	return CONTROLLER_LINE;
}

QString ControlChangeEvent::toMessage(){
	return "cc "+QString::number(channel())+" "+QString::number(_control)+" "+
			QString::number(_value);
}

QByteArray ControlChangeEvent::save(){
	QByteArray array = QByteArray();
	array.append(0xB0 | channel());
	array.append(_control);
	array.append(_value);
	return array;
}

ProtocolEntry *ControlChangeEvent::copy(){
	return new ControlChangeEvent(*this);
}

void ControlChangeEvent::reloadState(ProtocolEntry *entry){
	ControlChangeEvent *other = dynamic_cast<ControlChangeEvent*>(entry);
	if(!other){
		return;
	}
	MidiEvent::reloadState(entry);
	_control = other->_control;
	_value = other->_value;
}

QString ControlChangeEvent::typeString(){
	return "Control Event";
}

int ControlChangeEvent::value(){
	return _value;
}

int ControlChangeEvent::control(){
	return _control;
}

void ControlChangeEvent::setValue(int v){
	ProtocolEntry *toCopy = copy();
	_value = v;
	protocol(toCopy, this);
	if(shownInEventWidget()){
		_value_box->setValue(v);
	}
}

void ControlChangeEvent::setControl(int c){
	ProtocolEntry *toCopy = copy();
	_control = c;
	protocol(toCopy, this);
	if(shownInEventWidget()){
		_control_combo->setCurrentIndex(c);
	}
}

// Widgets for EventWidget
QWidget *ControlChangeEvent::_value_widget = 0;
QSpinBox *ControlChangeEvent::_value_box = 0;
QLabel *ControlChangeEvent::_value_label = 0;
QComboBox *ControlChangeEvent::_control_combo = 0;
QLabel *ControlChangeEvent::_control_label = 0;

void ControlChangeEvent::generateWidget(QWidget *widget){

	// general data
	MidiEvent::generateWidget(widget);

	// first call
	if(_value_widget == 0) _value_widget = new QWidget();
	if(_value_box == 0) _value_box = new QSpinBox();
	if(_value_label == 0) _value_label = new QLabel();
	if(_control_label == 0) _control_label = new QLabel();

	if(_control_combo == 0){
		_control_combo = new QComboBox();
		for(int i = 0; i<128; i++){
			_control_combo->addItem(QString::number(i)+" "+
					file()->controlChangeName(i));
		}
	}

	// set Parents
	_value_widget->setParent(widget);
	_value_box->setParent(_value_widget);
	_value_label->setParent(_value_widget);
	_control_combo->setParent(widget);
	_control_label->setParent(widget);

	// unhide
	_value_widget->setHidden(false);
	_value_box->setHidden(false);
	_value_label->setHidden(false);
	_control_combo->setHidden(false);
	_control_label->setHidden(false);

	// Control
	_control_label->setText("Controller:");
	_control_combo->setCurrentIndex(_control);


	// value
	_value_label->setText("Value: ");
	QLayout *valL = _value_widget->layout();
	if(!valL){
		valL = new QBoxLayout(QBoxLayout::LeftToRight, _value_widget);
		_value_widget->setLayout(valL);
	}
	valL->addWidget(_value_label);

	// box
	_value_box->setMaximum(127);
	_value_box->setMinimum(0);
	_value_box->setValue(_value);
	valL->addWidget(_value_box);

	// Add Widgets
	QLayout *layout = widget->layout();
	layout->addWidget(_control_label);
	layout->addWidget(_control_combo);
	layout->addWidget(_value_widget);
}

void ControlChangeEvent::editByWidget(){
	if(_value_box->value()!=value()){
		setValue(_value_box->value());
	}
	if(_control_combo->currentIndex()!=control()){
		setControl(_control_combo->currentIndex());
	}
	MidiEvent::editByWidget();
}

bool ControlChangeEvent::isOnEvent(){
//	return (_control < 64 && _control> 69) || _value > 64;
	return false;
}
