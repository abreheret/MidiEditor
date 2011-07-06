/*
 * SenderThread.cpp
 *
 *  Created on: 08.04.2011
 *      Author: markus
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
