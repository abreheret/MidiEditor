/*
 * MidiEditor
 * Copyright (C) 2010  Markus Schwenk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.+
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MidiTrack.h"
#include "MidiChannel.h"
#include "../MidiEvent/TextEvent.h"
#include "MidiFile.h"

MidiTrack::MidiTrack(MidiFile *file) : ProtocolEntry() {
	_number = 0;
	_nameEvent = 0;
	_file = file;
}

MidiTrack::MidiTrack(MidiTrack &other) : ProtocolEntry(other) {
	_number = other._number;
	_nameEvent = other._nameEvent;
	_file = other._file;
}


MidiFile *MidiTrack::file(){
	return _file;
}

QString MidiTrack::name(){
	if(_nameEvent){
		return _nameEvent->text();
	}
	return "untitled track";
}

void MidiTrack::setName(QString name){

	if(!_nameEvent){
		_nameEvent = new TextEvent(16);
		_nameEvent->setFile(_file);
		_nameEvent->setMidiTime(0, false);
		_nameEvent->setType(TextEvent::TRACKNAME);
		_nameEvent->setTrack(number(), true);
		_file->channel(16)->insertEvent(_nameEvent, 0);
	}

	_nameEvent->setText(name);
	emit trackChanged();
}

int MidiTrack::number(){
	return _number;
}

void MidiTrack::setNumber(int number){
	ProtocolEntry *toCopy = copy();
	_number = number;
	protocol(toCopy, this);
}

void MidiTrack::setNameEvent(TextEvent *nameEvent){
	ProtocolEntry *toCopy = copy();
	_nameEvent = nameEvent;
	protocol(toCopy, this);
	emit trackChanged();
}

TextEvent *MidiTrack::nameEvent(){
	return _nameEvent;
}

ProtocolEntry *MidiTrack::copy(){
	return new MidiTrack(*this);
}

void MidiTrack::reloadState(ProtocolEntry *entry){
	MidiTrack *other = dynamic_cast<MidiTrack*>(entry);
	if(!other){
		return;
	}
	_number = other->_number;
	_nameEvent = other->_nameEvent;
	_file = other->_file;
}

