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
#include "../MidiEvent/TextEvent.h"
#include "MidiFile.h"

MidiTrack::MidiTrack(MidiFile *file) {
	_name = "not named";
	_number = 0;
	_nameEvent = 0;
	_file = file;
}

QString MidiTrack::name(){
	return _name;
}

void MidiTrack::setName(QString name){
	_name = name;
	if(_nameEvent){
		// set nameEVents data
	} else {
		// create one?
	}
}

int MidiTrack::number(){
	return _number;
}

void MidiTrack::setNumber(int number){
	_number = number;
}

void MidiTrack::setNameEvent(TextEvent *nameEvent){
	_nameEvent = nameEvent;

	// geht the name
	_name = nameEvent->text();
}

TextEvent *MidiTrack::nameEvent(){
	return _nameEvent;
}
