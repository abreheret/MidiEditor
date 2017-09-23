#include "Metronome.h"

#include "MidiFile.h"
#include "MidiPlayer.h"
#include "PlayerThread.h"

#include "../Singleton.h"

#include <QtCore/qmath.h>
#include <QFile>
#include <QFileInfo>
#include <QSoundEffect>
#include <QTime>

bool Metronome::_enable = false;
QSoundEffect *Metronome::clickSound = 0;

Metronome::Metronome(QObject *parent) : QObject(parent) {
	if (!clickSound){
		clickSound = new QSoundEffect();
		clickSound->setSource(QUrl::fromLocalFile(":/run_environment/metronome/metronome-01.wav"));
	}
	_file = 0;
	num = 4;
	denom = 2;
	lastMeasure = -1;
	lastPos = -1;
}

void Metronome::setFile(MidiFile *file) {
	_file = file;
}

void Metronome::measureUpdate(int measure, int tickInMeasure) {

	// compute pos
	if (!_file) {
		return;
	}

	int ticksPerClick = int((_file->ticksPerQuarter() * 4) / qPow(2, denom));
	int pos = tickInMeasure / ticksPerClick;

	if (lastMeasure < measure) {
		emit click();
		lastMeasure = measure;
		lastPos = 0;
		return;
	} else {
		if (pos > lastPos) {
			emit click();
			lastPos = pos;
			return;
		}
	}
}

void Metronome::meterChanged(int n, int d) {
	num = n;
	denom = d;
}

void Metronome::playbackStarted() {
	reset();
}

void Metronome::playbackStopped() {

}
Metronome *Metronome::createInstance() {
	return new Metronome();
}
Metronome *Metronome::instance() {
	return Singleton<Metronome>::instance(Metronome::createInstance);
}

void Metronome::reset() {
	lastPos = 0;
	lastMeasure = -1;
}

bool Metronome::enabled() {
	return _enable;
}

void Metronome::setEnabled(bool b) {
	if (b) {
		// metronome
		connect(MidiPlayer::player(),
				SIGNAL(measureChanged(int, int)), Metronome::instance(), SLOT(measureUpdate(int,
						int)));
		connect(MidiPlayer::player(),
				SIGNAL(measureUpdate(int, int)), Metronome::instance(), SLOT(measureUpdate(int,
						int)));
		connect(MidiPlayer::player(),
				SIGNAL(meterChanged(int, int)), Metronome::instance(), SLOT(meterChanged(int,
						int)));
		connect(MidiPlayer::player(),
				SIGNAL(playerStopped()), Metronome::instance(), SLOT(playbackStopped()));
		connect(MidiPlayer::player(),
				SIGNAL(playerStarted()), Metronome::instance(), SLOT(playbackStarted()));
		connect(Metronome::instance(), SIGNAL(click()),
				clickSound, SLOT(play()), Qt::DirectConnection);
	} else {
		// metronome
		disconnect(MidiPlayer::player(),
				SIGNAL(measureChanged(int, int)), Metronome::instance(), SLOT(measureUpdate(int,
						int)));
		disconnect(MidiPlayer::player(),
				SIGNAL(measureUpdate(int, int)), Metronome::instance(), SLOT(measureUpdate(int,
						int)));
		disconnect(MidiPlayer::player(),
				SIGNAL(meterChanged(int, int)), Metronome::instance(), SLOT(meterChanged(int,
						int)));
		disconnect(MidiPlayer::player(),
				SIGNAL(playerStopped()), Metronome::instance(), SLOT(playbackStopped()));
		disconnect(MidiPlayer::player(),
				SIGNAL(playerStarted()), Metronome::instance(), SLOT(playbackStarted()));
		disconnect(Metronome::instance(), SIGNAL(click()),
				   clickSound, SLOT(play()));
	}
	_enable = b;
}
