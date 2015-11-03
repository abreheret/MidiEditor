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

#include "MidiInput.h"

#include "../MidiEvent/MidiEvent.h"
#include "../MidiEvent/OffEvent.h"
#include "../MidiEvent/OnEvent.h"

#include <QTextStream>
#include <QByteArray>

#include <cstdlib>

#include "rtmidi/RtError.h"
#include "rtmidi/RtMidi.h"

RtMidiIn *MidiInput::_midiIn = 0;
QString MidiInput::_inPort = "";
QMultiMap<int, std::vector<unsigned char> > *MidiInput::_messages =
		new QMultiMap<int, std::vector<unsigned char> >;
int MidiInput::_currentTime = 0;
bool MidiInput::_recording = false;

void MidiInput::init(){

	// RtMidiIn constructor
	try {
        _midiIn = new RtMidiIn(QString("MidiEditor Input").toStdString());
		_midiIn->setQueueSizeLimit(65535);
		_midiIn->ignoreTypes(false, true, true);
		_midiIn->setCallback(&receiveMessage);
	}
    catch (RtError &error) {
        qWarning("%s", error.getMessageString());
	}
}

void MidiInput::receiveMessage(double deltatime, std::vector<unsigned char>
		*message, void *userData)
{
	if(message->size()>1){
		_messages->insert(_currentTime, *message);
	}
}

QStringList MidiInput::inputPorts(){

	QStringList ports;

	// Check outputs.
	unsigned int nPorts = _midiIn->getPortCount();

	for(unsigned int i = 0; i < nPorts; i++){

		try {
			ports.append(QString::fromStdString(_midiIn->getPortName(i)));
		}
		catch (RtError &error) {}
	}

	return ports;
}

bool MidiInput::setInputPort(QString name){

	// try to find the port
	unsigned int nPorts = _midiIn->getPortCount();

	for(unsigned int i = 0; i < nPorts; i++){

		try {

			// if the current port has the given name, select it and close
			// current port
			if(_midiIn->getPortName(i) == name.toStdString()){

				_midiIn->closePort();
				_midiIn->openPort(i);
				_inPort = name;
				return true;
			}

		}
		catch (RtError &error) {}
	}

	// port not found
	return false;
}

QString MidiInput::inputPort(){
	return _inPort;
}

void MidiInput::startInput(){

	// clear eventlist
	_messages->clear();

	_recording = true;
}

QMultiMap<int, MidiEvent*> MidiInput::endInput(){

	QMultiMap<int, MidiEvent*> eventList;
	QByteArray array;

	QMultiMap<int, std::vector<unsigned char> >::iterator it =
			_messages->begin();

	bool ok = true;
	bool endEvent = false;

	_recording = false;

	QMultiMap<int, OffEvent*> emptyOffEvents;

	while(ok && it != _messages->end()){

		array.clear();

		for(unsigned int i = 0; i<it.value().size(); i++){
			array.append(it.value().at(i));
		}

		QDataStream tempStream(array);

		MidiEvent *event = MidiEvent::loadMidiEvent(&tempStream,&ok,&endEvent);
		OffEvent *off = dynamic_cast<OffEvent*>(event);
		if(off && !off->onEvent()){
			emptyOffEvents.insert(it.key(), off);
			it++;
			continue;
		}
		if(ok){
			eventList.insert(it.key(), event);
		}
		// if on event, check whether the off event has been loaded before.
		// this occurs when RTMidi fails to send the correct order
		OnEvent *on = dynamic_cast<OnEvent*>(event);
		if(on && emptyOffEvents.contains(it.key())){
			QMultiMap<int, OffEvent*>::iterator emptyIt =
					emptyOffEvents.lowerBound(it.key());
			while(emptyIt != emptyOffEvents.end() && emptyIt.key()==it.key()){
				if(emptyIt.value()->line() == on->line()){
					emptyIt.value()->setOnEvent(on);
					OffEvent::removeOnEvent(on);
					emptyOffEvents.remove(emptyIt.key(), emptyIt.value());
					// add offEvent
					eventList.insert(it.key()+100, emptyIt.value());
					break;
				}
				emptyIt++;
			}
		}
		it++;
	}
	QMultiMap<int, MidiEvent*>::iterator it2 = eventList.begin();
	while(it2!=eventList.end()){
		OnEvent *on = dynamic_cast<OnEvent*>(it2.value());
		if(on && !on->offEvent()){
			eventList.remove(it2.key(), it2.value());
		}
		it2++;
	}
	_messages->clear();

	_currentTime = 0;


	return eventList;
}

void MidiInput::setTime(int ms){
	_currentTime = ms;
}

bool MidiInput::recording(){
	return _recording;
}
