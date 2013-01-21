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

#ifndef MIDIEVENT_H_
#define MIDIEVENT_H_

#include <QDataStream>
#include <QColor>
#include "../gui/GraphicObject.h"
#include "../protocol/ProtocolEntry.h"
#include <QWidget>

class MidiFile;
class QSpinBox;
class QLabel;
class QWidget;
class EventWidget;

class MidiEvent : public ProtocolEntry, public GraphicObject{

	public:
		MidiEvent(int channel);
		MidiEvent(MidiEvent &other);

		static MidiEvent *loadMidiEvent(QDataStream *content,
				bool *ok, bool *endEvent, quint8 startByte = 0,
				quint8 secondByte = 0);

		static EventWidget *eventWidget();
		static void setEventWidget(EventWidget *widget);

		enum {
			TEMPO_CHANGE_EVENT_LINE = 129,
			TIME_SIGNATURE_EVENT_LINE,
			PROG_CHANGE_LINE,
			CONTROLLER_LINE,
			KEY_PRESSURE_LINE,
			CHANNEL_PRESSURE_LINE,
			TEXT_EVENT_LINE,
			UNKNOWN_LINE
		};
		void setTrack(int num, bool toProtocol=true);
		int track();
		void setChannel(int channel, bool toProtocol=true);
		int channel();
		virtual void setMidiTime(int t, bool toProtocol = true);
		int midiTime();
		void setFile(MidiFile *f);
		MidiFile *file();
		bool shownInEventWidget();

		virtual int line();
		virtual QString toMessage();
		virtual QByteArray save();
		virtual void draw(QPainter *p, QColor c);

		virtual ProtocolEntry *copy();
		virtual void reloadState(ProtocolEntry *entry);

		virtual QString typeString();
		virtual void generateWidget(QWidget *widget);
		virtual void editByWidget();

		virtual bool isOnEvent();

	protected:
		int numTrack, numChannel, timePos;
		static QSpinBox *_channel_spinBox, *_track_spinBox, *_timePos_spinBox;
		static QLabel *_channel_label, *_track_label, *_timePos_label,
			*_title_label;
		static QWidget *_channel_widget, *_track_widget, *_timePos_widget;
		MidiFile *midiFile;
		static quint8 _startByte;
		static EventWidget *_eventWidget;

};

#endif
