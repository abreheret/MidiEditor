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
#include "../MidiEvent/NoteOnEvent.h"
#include "../MidiEvent/OffEvent.h"
#include "MidiPlayer.h"

#include <QTimer>

// Credit to http://thesmithfam.org/blog/2010/02/07/talking-to-qt-threads/
SenderThread::SenderThread() {
	moveToThread(this);
	_eventQueue = new QQueue<QByteArray>();
	_noteQueue = new QQueue<QByteArray>();
}

/*void SenderThread::run(){

	forever {
		// First, send the misc events, such as control change and program change events.
		while(!_eventQueue->isEmpty()){
			// send command
			MidiOutput::instance()->sendEnqueuedCommand(_eventQueue->dequeue());
		}
		// Now send the note events.
		while(!_noteQueue->isEmpty()){
			// send command
			MidiOutput::instance()->sendEnqueuedCommand(_noteQueue->dequeue());
		}
		msleep(1);
	}
}*/
void SenderThread::run() {
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(sendCommands()));
	timer->start(1);
	//QTimer::singleShot(1, this, SLOT(sendCommands()));
	exec();
}
void SenderThread::sendCommands() {
	if (!timer) {
		timer = new QTimer(this);
		connect(timer, SIGNAL(timeout()), this, SLOT(sendCommands()));
		timer->start(1);
	}
	timer->blockSignals(true);
	while(!_eventQueue->isEmpty()){
		// send command
		MidiOutput::instance()->sendEnqueuedCommand(_eventQueue->dequeue());
	}
	// Now send the note events.
	while(!_noteQueue->isEmpty()){
		// send command
		MidiOutput::instance()->sendEnqueuedCommand(_noteQueue->dequeue());
	}
	timer->blockSignals(false);
}
void SenderThread::stop() {
	if(currentThread() != this) {
		QMetaObject::invokeMethod(this, "stop",
						Qt::QueuedConnection);
	} else {
		_noteQueue->clear();
		_eventQueue->clear();
		if (timer) {
			timer->stop();
			disconnect(timer, SIGNAL(timeout()), this, SIGNAL(sendCommands()));
		}
		quit();
	}
}
void SenderThread::enqueue(MidiEvent *event){
	if (!isRunning ()){
		start(QThread::TimeCriticalPriority);
	}
	// If it is a NoteOnEvent or an OffEvent, we put it in _noteQueue.
	if (qobject_cast <NoteOnEvent*>(event) || qobject_cast <OffEvent*>(event))
		_noteQueue->enqueue(event->save());
	// Otherwise, it goes into _eventQueue.
	else
		_eventQueue->enqueue(event->save());
}
