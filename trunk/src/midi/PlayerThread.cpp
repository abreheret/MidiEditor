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

#include "PlayerThread.h"
#include "MidiOutput.h"
#include <QMultiMap>
#include "MidiFile.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../MidiEvent/OffEvent.h"
#include "../MidiEvent/TimeSignatureEvent.h"
#include "../MidiEvent/KeySignatureEvent.h"
#include "MidiInput.h"
#include <QTime>

#define INTERVAL_TIME 15
#define TIMEOUTS_PER_SIGNAL 1

PlayerThread::PlayerThread() : QThread() {
	file = 0;
	timer = 0;
	timeoutSinceLastSignal = 0;
	time = 0;
}

void PlayerThread::setFile(MidiFile *f){
	file = f;
}

void PlayerThread::stop(){
	stopped = true;
}

void PlayerThread::setInterval(int i){
	interval = i;
}

void PlayerThread::run(){

	if(timer){
		delete timer;
	}
	if(time){
		delete time;
		time = 0;
	}
	timer = new QTimer();

	events = file->playerData();

	position = file->msOfTick(file->cursorTick());

	if(file->pauseTick() >= 0){
		position = file->msOfTick(file->pauseTick());
	}

	// Reset all Controllers
	for(int i = 0; i<16; i++){
		QByteArray array;
		array.append(0xB0 | i);
		array.append(121);
		array.append(char(0));
		MidiOutput::sendCommand(array);
	}

	// All Events before position should be sent, progChanges and ControlChanges
	QMultiMap<int, MidiEvent*>::iterator it = events->begin();
	while(it!=events->end()){
		if(it.key()>=position){
			break;
		}
		MidiOutput::sendCommand(it.value());
		it++;
	}

	setInterval(INTERVAL_TIME);

	connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
	timer->start(INTERVAL_TIME);

	stopped = false;

	QList<TimeSignatureEvent*> *list = 0;
	measure = file->measure(file->cursorTick(), file->cursorTick(), &list);
	emit(measureChanged(measure));

	if(exec() == 0){
		timer->stop();
		emit playerStopped();
	}
}

void PlayerThread::timeout(){

	if(!time){
		time = new QTime();
		time->start();
	}

	disconnect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
	if(stopped){
		disconnect(timer, SIGNAL(timeout()), this, SLOT(timeout()));

		// AllNotesOff // All SoundsOff
		for(int i = 0; i<16; i++){
			// value (third number) should be 0, but doesnt work
			QByteArray array;
			array.append(0xB0 | i);
			array.append(char(123));
			array.append(char(127));
			MidiOutput::sendCommand(array);
		}

		quit();

	} else {

		int newPos = position + time->elapsed();
		int tick = file->tick(newPos);
		QList<TimeSignatureEvent*> *list = 0;
		int new_measure = file->measure(tick, tick, &list);
		if(new_measure > measure){
			emit measureChanged(new_measure);
			measure = new_measure;
		}
		time->restart();
		QMultiMap<int, MidiEvent*>::iterator it = events->lowerBound(position);

		while(it!=events->end() && it.key()<newPos){


			// save events for the given tick
			QList<MidiEvent*> onEv, offEv;
			int sendPosition = it.key();

			do {
				if(it.value()->isOnEvent()){
					onEv.append(it.value());
				} else {
					offEv.append(it.value());
				}
				it++;
			} while(it!=events->end() && it.key() == sendPosition);

			foreach(MidiEvent *ev, offEv){
				MidiOutput::sendCommand(ev);
			}
			foreach(MidiEvent *ev, onEv){
				if(ev->line() == MidiEvent::KEY_SIGNATURE_EVENT_LINE){
					KeySignatureEvent *keySig = dynamic_cast<KeySignatureEvent*>(ev);
					if(keySig){
						emit tonalityChanged(keySig->tonality());
					}
				}
				if(ev->line() == MidiEvent::TIME_SIGNATURE_EVENT_LINE){
					TimeSignatureEvent *timeSig = dynamic_cast<TimeSignatureEvent*>(ev);
					if(timeSig){
						emit meterChanged(timeSig->num(), timeSig->denom());
					}
				}
				MidiOutput::sendCommand(ev);
			}

			//MidiOutput::sendCommand(it.value());
			//it++;
		}

		// end if it was last event, but only if not recording
		if(it == events->end() && !MidiInput::recording()){
			stop();
		}
		position = newPos;
		timeoutSinceLastSignal++;
		MidiInput::setTime(position);
		if(timeoutSinceLastSignal==TIMEOUTS_PER_SIGNAL){
			emit timeMsChanged(position);
			timeoutSinceLastSignal = 0;
		}
	}
	connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

int PlayerThread::timeMs(){
	return position;
}
