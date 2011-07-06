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

#include "SenderThread.h"
#include "../MidiEvent/MidiEvent.h"

SenderThread::SenderThread() {
	_eventQueue = new QQueue<MidiEvent*>;
}

void SenderThread::run(){

	while(true) {
		while(!_eventQueue->isEmpty()){
			// send command
			MidiOutput::sendEnqueuedCommand(_eventQueue->head()->save());
			_eventQueue->pop_front();
		}
		msleep(1);
	}
}

void SenderThread::enqueue(MidiEvent *event){
	_eventQueue->push_back(event);
}
