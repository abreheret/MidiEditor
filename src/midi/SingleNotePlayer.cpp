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

#include "SingleNotePlayer.h"

#define SINGLE_NOTE_LENGTH_MS 2000

#include <QTimer>
#include "../MidiEvent/NoteOnEvent.h"
#include "MidiOutput.h"
#include "MidiPlayer.h"

#include "../Singleton.h"

SingleNotePlayer::SingleNotePlayer() {
	playing = false;
	offMessage.clear();
	timer = new QTimer();
	timer->setInterval(SINGLE_NOTE_LENGTH_MS);
	timer->setSingleShot(true);
	connect(timer, &QTimer::timeout, this, &SingleNotePlayer::stop);
}

void SingleNotePlayer::play(NoteOnEvent *event){
	if (MidiPlayer::instance()->isPlaying()) {
		return;
	}
	if(playing){
		MidiOutput::instance()->sendCommand(offMessage);
		timer->stop();
	}
	offMessage = event->saveOffEvent();
	MidiOutput::instance()->sendCommand(event);
	playing = true;
	timer->start();
}

void SingleNotePlayer::stop(){
	MidiOutput::instance()->sendCommand(offMessage);
	timer->stop();
	playing = false;
	MidiPlayer::instance()->stop();
}
SingleNotePlayer *SingleNotePlayer::createInstance() {
	return new SingleNotePlayer();
}
SingleNotePlayer *SingleNotePlayer::instance() {
	return Singleton<SingleNotePlayer>::instance(SingleNotePlayer::createInstance);
}
bool SingleNotePlayer::isPlaying() {
	return playing;
}
