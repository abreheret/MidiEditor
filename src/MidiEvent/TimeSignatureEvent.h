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

#ifndef TIMESIGNATUREEVENT_H_
#define TIMESIGNATUREEVENT_H_

#include "MidiEvent.h"

class TimeSignatureEvent : public MidiEvent {

	Q_OBJECT

	public:
		TimeSignatureEvent(int channel, int num, int denom, int midiClocks,
				int num32In4, MidiTrack *track);
		TimeSignatureEvent(const TimeSignatureEvent &other);
		MidiEvent::EventType type() const Q_DECL_OVERRIDE;

		int num();
		int denom();
		int midiClocks();
		int num32In4();
		int measures(int tick, int *ticksLeft = 0);
		int ticksPerMeasure();

		virtual ProtocolEntry *copy() Q_DECL_OVERRIDE;
		virtual void reloadState(ProtocolEntry *entry) Q_DECL_OVERRIDE;
		int line() Q_DECL_OVERRIDE;
		QByteArray save() Q_DECL_OVERRIDE;

		void setDenominator(int d);
		void setNumerator(int n);

		QString typeString() Q_DECL_OVERRIDE;

	private:
		int numerator, denominator, midiClocksPerMetronome, num32In4th;
};

#endif
