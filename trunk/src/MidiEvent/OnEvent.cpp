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

#include "OnEvent.h"

#include "OffEvent.h"
#include <QBoxLayout>
#include "../midi/MidiFile.h"

OnEvent::OnEvent(int ch) : MidiEvent(ch){
	_offEvent = 0;

	return;
}

OnEvent::OnEvent(OnEvent &other) : MidiEvent(other){
	_offEvent = other._offEvent;
	return;
}

void OnEvent::setOffEvent(OffEvent *event){
	_offEvent = event;
}

OffEvent *OnEvent::offEvent(){
	return _offEvent;
}

ProtocolEntry *OnEvent::copy(){
	return new OnEvent(*this);
}

void OnEvent::reloadState(ProtocolEntry *entry){
	OnEvent *other = dynamic_cast<OnEvent*>(entry);
	if(!other){
		return;
	}
	MidiEvent::reloadState(entry);
	_offEvent = other->_offEvent;
}

QByteArray OnEvent::saveOffEvent(){
	return QByteArray();
}

QString OnEvent::offEventMessage(){
	return "";
}

QWidget *OnEvent::_off_tick_widget = 0;
QSpinBox *OnEvent::_off_tick_box = 0;
QLabel *OnEvent::_off_tick_label = 0;

void OnEvent::generateWidget(QWidget *widget){
	// general data
	MidiEvent::generateWidget(widget);

	if(!offEvent()) return;

	// first call
	if(_off_tick_widget == 0) _off_tick_widget = new QWidget();
	if(_off_tick_box == 0) _off_tick_box = new QSpinBox();
	if(_off_tick_label == 0) _off_tick_label = new QLabel();

	// set Parents
	_off_tick_widget->setParent(widget);
	_off_tick_label->setParent(_off_tick_widget);
	_off_tick_box->setParent(_off_tick_widget);

	// unhide
	_off_tick_widget->setHidden(false);
	_off_tick_label->setHidden(false);
	_off_tick_box->setHidden(false);

	// Edit text of onTick
	_timePos_label->setText("On Tick: ");

	// Edit offTick
	_off_tick_label->setText("Off Tick: ");
	QLayout *timeL = _off_tick_widget->layout();
	if(!timeL){
		timeL = new QBoxLayout(QBoxLayout::LeftToRight, _off_tick_widget);
		_off_tick_widget->setLayout(timeL);
	}
	timeL->addWidget(_off_tick_label);

	// box
	_off_tick_box->setMaximum(file()->endTick());
	_off_tick_box->setMinimum(0);
	_off_tick_box->setValue(offEvent()->midiTime());
	timeL->addWidget(_off_tick_box);

	// Add Widgets
	QLayout *layout = widget->layout();
	layout->addWidget(_off_tick_widget);
}

void OnEvent::editByWidget(){
	OffEvent *off = offEvent();

	if(off->midiTime()!=_off_tick_box->value()){
		off->setMidiTime(_off_tick_box->value(), true);
	}
	if(off->track()!=_track_spinBox->value()){
		off->setTrack(_track_spinBox->value());
	}
	if(off->channel()!=_channel_spinBox->value()){
		off->setChannel(_channel_spinBox->value());
	}
	MidiEvent::editByWidget();
}


