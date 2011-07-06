/*
 * SenderThread.h
 *
 *  Created on: 08.04.2011
 *      Author: markus
 */

#ifndef SENDERTHREAD_H_
#define SENDERTHREAD_H_

#include <QQueue>
#include <QThread>

#include "MidiOutput.h"

class SenderThread : public QThread {

	public:
		SenderThread();
		void run();
		void enqueue(MidiEvent *event);

	private:
		QQueue<MidiEvent*> *_eventQueue;

};

#endif
