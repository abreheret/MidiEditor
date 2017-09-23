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

#ifndef NOTEONEVENT_H_
#define NOTEONEVENT_H_

#include "OnEvent.h"

class OffEvent;

class NoteOnEvent : public OnEvent{

	Q_OBJECT

	public:
		NoteOnEvent(int note, int velocity, int ch, MidiTrack *track);
		NoteOnEvent(const NoteOnEvent &other);
		MidiEvent::EventType type() const Q_DECL_OVERRIDE;

		int note();
		int velocity();
		int line() Q_DECL_OVERRIDE;

		void setNote(int n);
		void setVelocity(int v);
		virtual ProtocolEntry *copy() Q_DECL_OVERRIDE;
		virtual void reloadState(ProtocolEntry *entry) Q_DECL_OVERRIDE;
		QString toMessage() Q_DECL_OVERRIDE;
		QString offEventMessage() Q_DECL_OVERRIDE;
		QByteArray save() Q_DECL_OVERRIDE;
		QByteArray saveOffEvent() Q_DECL_OVERRIDE;

		QString typeString() Q_DECL_OVERRIDE;

	protected:
		int _note, _velocity;
};

#endif
