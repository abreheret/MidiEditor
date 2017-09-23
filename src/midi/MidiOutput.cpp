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

#include "MidiOutput.h"

#include "../MidiEvent/MidiEvent.h"
#include "../gui/MainWindow.h"

#include <QFile>
#include <QTextStream>
#include <QByteArray>

#include <vector>

#include "rtmidi/RtMidi.h"

#include "SenderThread.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../MidiEvent/OffEvent.h"

#include "../Singleton.h"

SenderThread *MidiOutput::_sender = new SenderThread();
MidiOutput::MidiOutput() : QObject() {
	_midiOut = 0;
	_outPort = "";
	playedNotes = QMap<int, QList<int> >();
	_alternativePlayer = false;

	_stdChannel = 0;
}
void MidiOutput::init() {
	// RtMidiOut constructor
	try {
		_midiOut = new RtMidiOut(RtMidi::UNSPECIFIED,
								 QString("MidiEditor output").toStdString());
	} catch (RtMidiError &error) {
		error.printMessage();
	}
	// Alert MainWindow that output is ready.
	MainWindow::getMainWindow()->ioReady(false);
}

SenderThread *MidiOutput::sender() {
	return _sender;
}

void MidiOutput::sendCommand(QByteArray array) {

	sendEnqueuedCommand(array);
}
bool MidiOutput::isAlternativePlayer() {
	return MidiOutput::instance()->_alternativePlayer;
}
void MidiOutput::setAlternativePlayer(bool enable) {
	MidiOutput::instance()->_alternativePlayer = enable;
}
void MidiOutput::sendCommand(MidiEvent *e) {
	if (!_sender->isRunning()) {
		_sender->start(QThread::TimeCriticalPriority);
	}
	if (e->channel() >= 0 && e->channel() < 16) {
		_sender->enqueue(e);

		if (isAlternativePlayer()) {
			NoteOnEvent *n = qobject_cast<NoteOnEvent *>(e);
			if (n && n->velocity() > 0) {
				playedNotes[n->channel()].append(n->note());
			} else if (n && n->velocity() == 0) {
				playedNotes[n->channel()].removeOne(n->note());
			} else {
				OffEvent *o = qobject_cast<OffEvent *>(e);
				if (o) {
					n = qobject_cast<NoteOnEvent *>(o->onEvent());
					if (n) {
						playedNotes[n->channel()].removeOne(n->note());
					}
				}
			}
		}
	}
}

QStringList MidiOutput::outputPorts() {
	QStringList ports;

	// Check outputs.
	uint nPorts = _midiOut->getPortCount();

	for (uint i = 0; i < nPorts; i++) {

		try {
			ports.append(QString::fromStdString(_midiOut->getPortName(i)));
		} catch (RtMidiError &error) {
			error.printMessage();
		}
	}
	return ports;
}

bool MidiOutput::setOutputPort(QString name) {

	// try to find the port
	uint nPorts = _midiOut->getPortCount();

	for (uint i = 0; i < nPorts; i++) {

		try {
			// if the current port has the given name, select it and close
			// current port
			if (_midiOut->getPortName(i) == name.toStdString()) {
				_midiOut->closePort();
				_midiOut->openPort(i);
				_outPort = name;
				return true;
			}

		} catch (RtMidiError &) {}

	}

	// port not found
	return false;
}

QString MidiOutput::outputPort() {
	return _outPort;
}

void MidiOutput::sendEnqueuedCommand(QByteArray array) {

	if (_outPort != "") {

		// convert data to std::vector
		std::vector<quint8> message;

		foreach (qint8 byte, array) {
			message.push_back(quint8(byte));
		}
		try {
			_midiOut->sendMessage(&message);
		} catch (RtMidiError &error) {
			error.printMessage();
		}
	}
}

void MidiOutput::setStandardChannel(int channel) {
	_stdChannel = channel;
}

int MidiOutput::standardChannel() {
	return _stdChannel;
}

void MidiOutput::sendProgram(int channel, int prog) {
	QByteArray array = QByteArray();
	array.append(0xC0 | qint8(channel));
	array.append(qint8(prog));
	sendCommand(array);
}
MidiOutput *MidiOutput::createInstance() {
	return new MidiOutput();
}
MidiOutput *MidiOutput::instance() {
	return Singleton<MidiOutput>::instance(MidiOutput::createInstance);
}
