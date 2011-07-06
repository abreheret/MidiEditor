#include "PlayerThread.h"
#include "MidiOutput.h"
#include <QMultiMap>
#include "MidiFile.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../MidiEvent/OffEvent.h"
#include "MidiInput.h"

#define INTERVAL_TIME 15
#define TIMEOUTS_PER_SIGNAL 1

PlayerThread::PlayerThread() : QThread() {
	file = 0;
	timer = new QTimer();
	timeoutSinceLastSignal = 0;
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

	events = file->playerData();

	position = file->msOfTick(file->cursorTick());

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

	if(exec() == 0){
		emit playerStopped();
	}
}

void PlayerThread::timeout(){
	disconnect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
	if(stopped){
		timer->stop();
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

		QMultiMap<int, MidiEvent*>::iterator it = events->lowerBound(position);
		while(it!=events->end() && it.key()<position+interval){
			MidiOutput::sendCommand(it.value());
			it++;
		}

		// end if it was last event, but only if not recording
		if(it == events->end() && !MidiInput::recording()){
			stop();
		}
		position += interval;
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
