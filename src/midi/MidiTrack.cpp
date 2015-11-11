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
	_hidden = false;
	_muted = false;
	_color = new QColor(Qt::red);
}

MidiTrack::MidiTrack(MidiTrack &other) : QObject(), ProtocolEntry(other) {
	_number = other._number;
	_nameEvent = other._nameEvent;
	_file = other._file;
	_hidden = other._hidden;
	_muted = other._muted;
	_color = other._color;
}

MidiTrack::~MidiTrack(){

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
		_nameEvent = new TextEvent(16, this);
		_nameEvent->setFile(_file);
		_nameEvent->setMidiTime(0, false);
		_nameEvent->setType(TextEvent::TRACKNAME);
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

	switch((number-1)%16){
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
	_hidden = other->_hidden;
	_muted = other->_muted;
}


void MidiTrack::setHidden(bool hidden){
	ProtocolEntry *toCopy = copy();
	_hidden = hidden;
	protocol(toCopy, this);
	emit trackChanged();
}

bool MidiTrack::hidden(){
	return _hidden;
}

void MidiTrack::setMuted(bool muted){
	ProtocolEntry *toCopy = copy();
	_muted = muted;
	protocol(toCopy, this);
	emit trackChanged();
}

bool MidiTrack::muted(){
	return _muted;
}

QColor *MidiTrack::color(){
	return _color;
}

MidiTrack *MidiTrack::copyToFile(MidiFile *file){

	file->addTrack();
	MidiTrack *newTrack = file->tracks()->last();
	newTrack->setName(this->name());

	file->registerCopiedTrack(this, newTrack, this->file());

	return newTrack;
}
