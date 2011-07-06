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

/**
 * \file midi/MidiChannel.cpp
 *
 * \brief Implements the class MidiChannel.
 */


#include "MidiChannel.h"

#include <QColor>

#include "MidiFile.h"
#include "../MidiEvent/MidiEvent.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../MidiEvent/OffEvent.h"
#include "../MidiEvent/ProgChangeEvent.h"
#include "../MidiEvent/TempoChangeEvent.h"
#include "../MidiEvent/TimeSignatureEvent.h"

MidiChannel::MidiChannel(MidiFile *f, int num){

	_midiFile = f;
	_num = num;

	_visible = true;
	_mute = false;
	_solo = false;
	_edit = false;

	_events = new QMultiMap<int, MidiEvent*>;

	// the color only depends on the number
	switch(num){
		case 0: { _color = new QColor(241, 214, 107, 255);break; }
		case 1: { _color =  new QColor(205, 241, 142, 255);break; }
		case 2: { _color = new QColor(107, 241, 142, 255);break; }
		case 3: { _color = new QColor(107, 241, 231, 255);break; }
		case 4: { _color =  new QColor(200, 236, 255, 255);break; }
		case 5: { _color = new QColor(241, 107, 200, 255);break; }
		case 6: { _color = new QColor(170, 212, 170, 255);break; }
		case 7: { _color =  new QColor(212, 204, 170, 255);break; }
		case 8: { _color = new QColor(238, 233, 138, 255);break; }
		case 9: { _color = new QColor(243, 94, 54, 255);break; }
		case 10: { _color = new QColor(255, 145, 26, 255);break; }
		case 11: { _color = new QColor(181, 132, 80, 255);break; }
		case 12: { _color =  new QColor(102, 162, 37, 255);break; }
		case 13: { _color = new QColor(241, 164, 107, 255);break; }
		case 14: { _color = new QColor(222, 213, 66, 255);break; }
		case 15: { _color = new QColor(202, 222, 66, 255);break; }
		default: { _color = new QColor(50, 50, 255, 255); break; }
	}
}

MidiChannel::MidiChannel(MidiChannel &other){
	_midiFile = other._midiFile;
	_visible = other._visible;
	_mute = other._mute;
	_solo = other._solo;
	_events = new QMultiMap<int, MidiEvent*>(*(other._events));
	_color = other._color;
	_num = other._num;
	_edit = other._edit;
}

ProtocolEntry *MidiChannel::copy(){
	return new MidiChannel(*this);
}

void MidiChannel::reloadState(ProtocolEntry *entry){
	MidiChannel *other = dynamic_cast<MidiChannel*>(entry);
	if(!other){
		return;
	}
	_midiFile = other->_midiFile;
	_visible = other->_visible;
	_mute = other->_mute;
	_solo = other->_solo;
	_events = other->_events;
	_color = other->_color;
	_num = other->_num;
	_edit = other->_edit;

}

MidiFile *MidiChannel::file(){
	return _midiFile;
}

bool MidiChannel::visible(){
	return _visible;
}

void MidiChannel::setVisible(bool b){
	ProtocolEntry *toCopy = copy();
	_visible = b;
	protocol(toCopy, this);
}

bool MidiChannel::mute(){
	return _mute;
}

void MidiChannel::setMute(bool b){
	ProtocolEntry *toCopy = copy();
	_mute = b;
	protocol(toCopy, this);
}

bool MidiChannel::solo(){
	return _solo;
}

void MidiChannel::setSolo(bool b){
	ProtocolEntry *toCopy = copy();
	_solo = b;
	protocol(toCopy, this);
}

int MidiChannel::number(){
	return _num;
}

QMultiMap<int, MidiEvent*> *MidiChannel::eventMap(){
	return _events;
}

QColor *MidiChannel::color(){
	return _color;
}

void MidiChannel::insertNote(int note, int startTick, int endTick,int velocity){
	ProtocolEntry *toCopy = copy();
	NoteOnEvent *onEvent = new NoteOnEvent(note, velocity, number());
	OffEvent *off = new OffEvent(number(), 128-note);
	off->setFile(file());
	off->setMidiTime(endTick, false);
	onEvent->setFile(file());
	onEvent->setMidiTime(startTick, false);
	protocol(toCopy, this);
}

void MidiChannel::removeEvent(MidiEvent *event){

	// if its once TimeSig / TempoChange at 0, dont delete event
	if(number()==18 || number()==17){
		if(event->midiTime()==0 && _events->count(0) == 1){
			return;
		}
	}

	ProtocolEntry *toCopy = copy();
	_events->remove(event->midiTime(), event);
	OnEvent *on = dynamic_cast<OnEvent*>(event);
	if(on && on->offEvent()){
		_events->remove(on->offEvent()->midiTime(), on->offEvent());
	}
	protocol(toCopy, this);
}

void MidiChannel::setEdit(bool b){
	ProtocolEntry *toCopy = copy();
	_edit = b;
	protocol(toCopy, this);
}

bool MidiChannel::edit(){
	return _edit;
}

void MidiChannel::insertEvent(MidiEvent *event, int tick){
	ProtocolEntry *toCopy = copy();
	event->setFile(file());
	event->setMidiTime(tick, false);
	protocol(toCopy, this);
}

int MidiChannel::progAtTick(int tick){

	// search for the last ProgChangeEvent in the channel
	QMultiMap<int, MidiEvent*>::iterator it = _events->upperBound(tick);
	if(it == _events->end()){
		it--;
	}
	while(it != _events->begin()){

		ProgChangeEvent *ev = dynamic_cast<ProgChangeEvent*>(it.value());
		if(ev && it.key()<=tick){
			return ev->program();
		}
		it--;
	}

	// default: first
	foreach(MidiEvent *event, *_events){
		ProgChangeEvent *ev = dynamic_cast<ProgChangeEvent*>(event);
		if(ev){
			return ev->program();
		}
	}
	return 0;
}
