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

#include "KeySignatureEvent.h"

#include <QLayout>

KeySignatureEvent::KeySignatureEvent(int channel, int tonality, bool minor) : MidiEvent(channel){
	_tonality = tonality;
	_minor = minor;
}

KeySignatureEvent::KeySignatureEvent(KeySignatureEvent &other) : MidiEvent(other){
	_tonality = other._tonality;
	_minor = other._minor;
}

int KeySignatureEvent::line(){
	return KEY_SIGNATURE_EVENT_LINE;
}

QString KeySignatureEvent::toMessage(){
	return "";
}

QByteArray KeySignatureEvent::save(){
	QByteArray array = QByteArray();
	array.append(0xFF);
	array.append(0x59 | channel());
	array.append(0x02);
	array.append(tonality());
	if(_minor){
		array.append(0x01);
	} else {
		array.append(char(0x00));
	}
	return array;
}

ProtocolEntry *KeySignatureEvent::copy(){
	return new KeySignatureEvent(*this);
}

void KeySignatureEvent::reloadState(ProtocolEntry *entry){
	KeySignatureEvent *other = dynamic_cast<KeySignatureEvent*>(entry);
	if(!other){
		return;
	}
	MidiEvent::reloadState(entry);
	_tonality = other->_tonality;
	_minor = other->_minor;
}

QString KeySignatureEvent::typeString(){
	return "Key Signature Event";
}

// Widgets for EventWidget
QWidget *KeySignatureEvent::_ton_widget = 0;
QSpinBox *KeySignatureEvent::_ton_box = 0;
QLabel *KeySignatureEvent::_ton_label = 0;
QWidget *KeySignatureEvent::_minor_widget = 0;
QCheckBox *KeySignatureEvent::_minor_check = 0;

void KeySignatureEvent::generateWidget(QWidget *widget){
	// general data
	MidiEvent::generateWidget(widget);

	// first call
	if(_ton_widget == 0) _ton_widget = new QWidget();
	if(_ton_box == 0) _ton_box = new QSpinBox();
	if(_ton_label == 0) _ton_label = new QLabel();
	if(_minor_widget == 0) _minor_widget = new QWidget();
	if(_minor_check == 0) _minor_check = new QCheckBox();

	// set Parents
	_ton_widget->setParent(widget);
	_ton_label->setParent(_ton_widget);
	_ton_box->setParent(_ton_widget);
	_minor_widget->setParent(widget);
	_minor_check->setParent(_minor_widget);

	// unhide
	_ton_widget->setHidden(false);
	_ton_label->setHidden(false);
	_ton_box->setHidden(false);
	_minor_widget->setHidden(false);
	_minor_check->setHidden(false);

	// Edit tonality
	_ton_label->setText("Tonality: ");
	QLayout *tonL = _ton_widget->layout();
	if(!tonL){
		tonL = new QBoxLayout(QBoxLayout::LeftToRight, _ton_widget);
		_ton_widget->setLayout(tonL);
	}
	tonL->addWidget(_ton_label);

	// box
	_ton_box->setMaximum(7);
	_ton_box->setMinimum(-7);
	_ton_box->setValue(tonality());
	tonL->addWidget(_ton_box);

	// Edit minor
	QLayout *minorL = _minor_widget->layout();
	if(!minorL){
		minorL = new QBoxLayout(QBoxLayout::LeftToRight, _minor_widget);
		_minor_widget->setLayout(minorL);
	}

	// box
	_minor_check->setText("minor");
	_minor_check->setChecked(minor());
	minorL->addWidget(_minor_check);

	// Add Widgets
	QLayout *layout = widget->layout();
	layout->addWidget(_ton_widget);
	layout->addWidget(_minor_widget);
}

void KeySignatureEvent::editByWidget(){
	if(tonality()!=_ton_box->value()){
		setTonality(_ton_box->value());
	}
	if(minor()!=_minor_check->isChecked()){
		setMinor(_minor_check->isChecked());
	}
	MidiEvent::editByWidget();
}

int KeySignatureEvent::tonality(){
	return _tonality;
}

bool KeySignatureEvent::minor(){
	return _minor;
}

void KeySignatureEvent::setTonality(int t){
	ProtocolEntry *toCopy = copy();
	_tonality = t;
	protocol(toCopy, this);
	if(shownInEventWidget()){
		_ton_box->setValue(t);
	}
}

void KeySignatureEvent::setMinor(bool minor){
	ProtocolEntry *toCopy = copy();
	_minor = minor;
	protocol(toCopy, this);
	if(shownInEventWidget()){
		_minor_check->setChecked(_minor);
	}
}
