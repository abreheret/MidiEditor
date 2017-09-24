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

#include "MidiPlayer.h"

#include "MidiFile.h"
#include "PlayerThread.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "SingleNotePlayer.h"
#include "MidiOutput.h"
#include "SenderThread.h"

#include "Metronome.h"

#include "../Singleton.h"

PlayerThread *MidiPlayer::_playerThread = new PlayerThread();

MidiPlayer::MidiPlayer() : QObject() {
	playing = false;
	_speed = 1;

}
void MidiPlayer::play(MidiFile *file) {
	if (isPlaying() || SingleNotePlayer::instance()->isPlaying()) {
		stop();
	}
// I don't know why we are reinstancing this.
#ifdef Q_OS_WIN32
	delete _playerThread;
	_playerThread = new PlayerThread();

	connect(_playerThread,
			SIGNAL(measureChanged(int, int)), Metronome::instance(), SLOT(measureUpdate(int,
					int)));
	connect(_playerThread,
			SIGNAL(measureUpdate(int, int)), Metronome::instance(), SLOT(measureUpdate(int,
					int)));
	connect(_playerThread,
			SIGNAL(meterChanged(int, int)), Metronome::instance(), SLOT(meterChanged(int,
					int)));
	connect(_playerThread,
			SIGNAL(playerStopped()), Metronome::instance(), SLOT(playbackStopped()));
	connect(_playerThread,
			SIGNAL(playerStarted()), Metronome::instance(), SLOT(playbackStarted()));
#endif

	int tickFrom = file->cursorTick();
	if (file->pauseTick() >= 0) {
		tickFrom = file->pauseTick();
	}
	file->preparePlayerData(tickFrom);
	if (!MidiOutput::sender()->isRunning()) {
		MidiOutput::sender()->start(QThread::TimeCriticalPriority);
	}
	_playerThread->setFile(file);
	_playerThread->start(QThread::TimeCriticalPriority);
	playing = true;
}

void MidiPlayer::play(NoteOnEvent *event) {
	SingleNotePlayer::instance()->play(event);
}

void MidiPlayer::stop() {
	playing = false;
	if (SingleNotePlayer::instance()->isPlaying()) {
		SingleNotePlayer::instance()->stop();
	}
	if (MidiOutput::sender()->isRunning()) {
		MidiOutput::sender()->stop();
	}
	if (_playerThread->isRunning()) {
		_playerThread->stop();
	}
}

PlayerThread *MidiPlayer::player() {
	return _playerThread;
}

bool MidiPlayer::isPlaying() {
	return playing;
}

int MidiPlayer::timeMs() {
	return _playerThread->timeMs();
}

MidiPlayer *MidiPlayer::createInstance() {
	return new MidiPlayer();
}

MidiPlayer *MidiPlayer::instance() {
	return Singleton<MidiPlayer>::instance(MidiPlayer::createInstance);
}

void MidiPlayer::panic() {
	if (isPlaying()){
		stop();
	}
	// set all channels note off / sounds off
	for (int i = 0; i < 16; i++) {
		// value (third number) should be 0, but doesn't work
		QByteArray array;
		array.append(qint8(0xB0 | i));
		array.append(qint8(123));
		array.append(qint8(127));

		MidiOutput::instance()->sendCommand(array);

		array.clear();
		array.append(qint8(0xB0 | i));
		array.append(qint8(120));
		array.append(char(0));
		MidiOutput::instance()->sendCommand(array);
	}
	if (MidiOutput::isAlternativePlayer()) {
		foreach (int channel, MidiOutput::instance()->playedNotes.keys()) {
			foreach (int note, MidiOutput::instance()->playedNotes.value(channel)) {
				QByteArray array;
				array.append(qint8(0x80 | channel));
				array.append(qint8(note));
				array.append(char(0));
				MidiOutput::instance()->sendCommand(array);
			}
		}
	}
}

double MidiPlayer::speedScale() {
	return _speed;
}

void MidiPlayer::setSpeedScale(double d) {
	_speed = d;
}
