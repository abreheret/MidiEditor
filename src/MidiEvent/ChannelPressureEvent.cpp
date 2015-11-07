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

#include "ChannelPressureEvent.h"

#include <QWidget>
#include <QLabel>
#include <QLayout>

ChannelPressureEvent::ChannelPressureEvent(int channel, int value, MidiTrack *track) :
		MidiEvent(channel, track)
{
	_value = value;
}

ChannelPressureEvent::ChannelPressureEvent(ChannelPressureEvent &other) :
		MidiEvent(other)
{
	_value = other._value;
}

int ChannelPressureEvent::line(){
	return CHANNEL_PRESSURE_LINE;
}

QString ChannelPressureEvent::toMessage(){
	return "";
}

QByteArray ChannelPressureEvent::save(){
	QByteArray array = QByteArray();
	array.append(0xD0 | channel());
	array.append(_value);
	return array;
}

ProtocolEntry *ChannelPressureEvent::copy(){
	return new ChannelPressureEvent(*this);
}

void ChannelPressureEvent::reloadState(ProtocolEntry *entry){
	ChannelPressureEvent *other = dynamic_cast<ChannelPressureEvent*>(entry);
	if(!other){
		return;
	}
	MidiEvent::reloadState(entry);
	_value = other->_value;
}

QString ChannelPressureEvent::typeString(){
	return "Channel Pressure Event";
}

// Widgets for EventWidget
QWidget *ChannelPressureEvent::_val_widget = 0;
QSpinBox *ChannelPressureEvent::_val_box = 0;
QLabel *ChannelPressureEvent::_val_label = 0;

void ChannelPressureEvent::generateWidget(QWidget *widget){
	// general data
	MidiEvent::generateWidget(widget);

	// first call
	if(_val_widget == 0) _val_widget = new QWidget();
	if(_val_box == 0) _val_box = new QSpinBox();
	if(_val_label == 0) _val_label = new QLabel();

	// set Parents
	_val_widget->setParent(widget);
	_val_label->setParent(_val_widget);
	_val_box->setParent(_val_widget);

	// unhide
	_val_widget->setHidden(false);
	_val_label->setHidden(false);
	_val_box->setHidden(false);

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
	layout->addWidget(_val_widget);
}

void ChannelPressureEvent::editByWidget(){
	if(value()!=_val_box->value()){
		setValue(_val_box->value());
	}
	MidiEvent::editByWidget();
}

void ChannelPressureEvent::setValue(int v){
	ProtocolEntry *toCopy = copy();
	_value = v;
	protocol(toCopy, this);
	if(shownInEventWidget()){
		_val_box->setValue(v);
	}
}

int ChannelPressureEvent::value(){
	return _value;
}
