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

#ifndef MIDIINPUT_H_
#define MIDIINPUT_H_

#include <QObject>
#include <QProcess>
#include <QList>
#include <QMultiMap>

#include <vector>

class MidiEvent;
class RtMidiIn;
class RtMidiOut;
class QStringList;
class MidiTrack;

class MidiInput : public QObject {

	Q_OBJECT

	public:
		static MidiInput *instance();

		void sendCommand(QByteArray array);
		void sendCommand(MidiEvent *e);

		QStringList inputPorts();
		bool setInputPort(QString name);
		QString inputPort();

		void startInput();
		QMultiMap<int, MidiEvent*> endInput(MidiTrack *track);

		static void receiveMessage(double deltatime,
					std::vector<quint8> *message, void *userData = 0);

		void setTime(int ms);

		bool recording();
		void setThruEnabled(bool b);
		bool thru();

	public slots:
		void init();

	private:
		QString _inPort;
		RtMidiIn *_midiIn;
		QMultiMap<int, std::vector<quint8> > *_messages;
		int _currentTime;
		bool _recording;
		QList<int> toUnique(QList<int> in);
		bool _thru;

		MidiInput();
		static MidiInput *createInstance();
};

#endif
