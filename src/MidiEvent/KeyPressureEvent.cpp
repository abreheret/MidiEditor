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

#include "KeyPressureEvent.h"

#include <QWidget>
#include <QLabel>
#include <QLayout>

KeyPressureEvent::KeyPressureEvent(int channel, int value, int note) :
		MidiEvent(channel)
{
	_value = value;
	_note = note;
}

KeyPressureEvent::KeyPressureEvent(KeyPressureEvent &other) :
		MidiEvent(other)
{
	_value = other._value;
	_note = other._note;
}

int KeyPressureEvent::line(){
	return KEY_PRESSURE_LINE;
}

QString KeyPressureEvent::toMessage(){
	return "";
}

QByteArray KeyPressureEvent::save(){
	QByteArray array = QByteArray();
	array.append(0xA0 | channel());
	array.append(_note);
	array.append(_value);
	return array;
}

ProtocolEntry *KeyPressureEvent::copy(){
	return new KeyPressureEvent(*this);
}

void KeyPressureEvent::reloadState(ProtocolEntry *entry){
	KeyPressureEvent *other = dynamic_cast<KeyPressureEvent*>(entry);
	if(!other){
		return;
	}
	MidiEvent::reloadState(entry);
	_value = other->_value;
	_note = other->_note;
}

void KeyPressureEvent::setValue(int v){
	ProtocolEntry *toCopy = copy();
	_value = v;
	protocol(toCopy, this);
}

void KeyPressureEvent::setNote(int n){
	ProtocolEntry *toCopy = copy();
	_note = n;
	protocol(toCopy, this);
}

QString KeyPressureEvent::typeString(){
	return "Key Pressure Event";
}

// Widgets for EventWidget
QWidget *KeyPressureEvent::_val_widget = 0;
QSpinBox *KeyPressureEvent::_val_box = 0;
QLabel *KeyPressureEvent::_val_label = 0;
QWidget *KeyPressureEvent::_note_widget = 0;
QSpinBox *KeyPressureEvent::_note_box = 0;
QLabel *KeyPressureEvent::_note_label = 0;

void KeyPressureEvent::generateWidget(QWidget *widget){
	// general data
	MidiEvent::generateWidget(widget);

	// first call
	if(_val_widget == 0) _val_widget = new QWidget();
	if(_val_box == 0) _val_box = new QSpinBox();
	if(_val_label == 0) _val_label = new QLabel();
	if(_note_widget == 0) _note_widget = new QWidget();
	if(_note_box == 0) _note_box = new QSpinBox();
	if(_note_label == 0) _note_label = new QLabel();

	// set Parents
	_val_widget->setParent(widget);
	_val_label->setParent(_val_widget);
	_val_box->setParent(_val_widget);
	_note_widget->setParent(widget);
	_note_label->setParent(_note_widget);
	_note_box->setParent(_note_widget);

	// unhide
	_val_widget->setHidden(false);
	_val_label->setHidden(false);
	_val_box->setHidden(false);
	_note_widget->setHidden(false);
	_note_label->setHidden(false);
	_note_box->setHidden(false);

	// Edit Note
	_note_label->setText("Note: ");
	QLayout *noteL = _note_widget->layout();
	if(!noteL){
		noteL = new QBoxLayout(QBoxLayout::LeftToRight, _note_widget);
		_note_widget->setLayout(noteL);
	}
	noteL->addWidget(_note_label);

	// box
	_note_box->setMaximum(127);
	_note_box->setMinimum(0);
	_note_box->setValue(note());
	noteL->addWidget(_note_box);

	// Edit value
	_val_label->setText("Value: ");
	QLayout *valL = _val_widget->layout();
	if(!valL){
		valL = new QBoxLayout(QBoxLayout::LeftToRight, _val_widget);
		_val_widget->setLayout(valL);
	}
	valL->addWidget(_val_label);

	// box
	_val_box->setMaximum(127);
	_val_box->setMinimum(0);
	_val_box->setValue(value());
	valL->addWidget(_val_box);

	// Add Widgets
	QLayout *layout = widget->layout();
	layout->addWidget(_note_widget);
	layout->addWidget(_val_widget);
}

void KeyPressureEvent::editByWidget(){
	if(note()!=_note_box->value()){
		setNote(_note_box->value());
	}
	if(value()!=_val_box->value()){
		setValue(_val_box->value());
	}
	MidiEvent::editByWidget();
}

int KeyPressureEvent::value(){
	return _value;
}

int KeyPressureEvent::note(){
	return _note;
}
