#include "SingleNotePlayer.h"

#define SINGLE_NOTE_LENGTH_MS 2000

#include <QTimer>
#include "../MidiEvent/NoteOnEvent.h"
#include "MidiOutput.h"

SingleNotePlayer::SingleNotePlayer() {
	playing = false;
	offMessage.clear();
	timer = new QTimer();
	timer->setInterval(SINGLE_NOTE_LENGTH_MS);
	timer->setSingleShot(true);
	connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

void SingleNotePlayer::play(NoteOnEvent *event){
	if(playing){
		MidiOutput::sendCommand(offMessage);
		timer->stop();
	}
	offMessage = event->saveOffEvent();
	MidiOutput::sendCommand(event);
	playing = true;
	timer->start();
}

void SingleNotePlayer::timeout(){
	MidiOutput::sendCommand(offMessage);
	timer->stop();
	playing = false;
}
