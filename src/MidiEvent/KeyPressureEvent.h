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

#ifndef KEYPRESSUREEVENT_H_
#define KEYPRESSUREEVENT_H_

#include "MidiEvent.h"

#include <QSpinBox>
#include <QWidget>
#include <QLabel>

class KeyPressureEvent : public MidiEvent {

	Q_OBJECT

	public:
		KeyPressureEvent(int channel, int value, int note, MidiTrack *track);
		KeyPressureEvent(const KeyPressureEvent &other);
		MidiEvent::EventType type() const Q_DECL_OVERRIDE;

		virtual int line() Q_DECL_OVERRIDE;

		QString toMessage() Q_DECL_OVERRIDE;
		QByteArray save() Q_DECL_OVERRIDE;

		virtual ProtocolEntry *copy() Q_DECL_OVERRIDE;
		virtual void reloadState(ProtocolEntry *entry) Q_DECL_OVERRIDE;

		QString typeString() Q_DECL_OVERRIDE;

		int value();
		int note();
		void setValue(int v);
		void setNote(int n);

	private:
		int _value, _note;
};

#endif
