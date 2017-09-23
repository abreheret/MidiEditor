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

#include "TimeSignatureEvent.h"
#include "math.h"
#include "../midi/MidiFile.h"

TimeSignatureEvent::TimeSignatureEvent(int channel, int num, int denom,
		int midiClocks, int num32In4, MidiTrack *track) : MidiEvent(channel, track)
{
	numerator = num;
	denominator = denom;
	midiClocksPerMetronome = midiClocks;
	num32In4th = num32In4;
}

TimeSignatureEvent::TimeSignatureEvent(const TimeSignatureEvent &other):
		MidiEvent(other)
{
	numerator = other.numerator;
	denominator = other.denominator;
	midiClocksPerMetronome = other.midiClocksPerMetronome;
	num32In4th = other.num32In4th;
}

MidiEvent::EventType TimeSignatureEvent::type() const {
	return TimeSignatureEventType;
}
int TimeSignatureEvent::num(){
	return numerator;
}

int TimeSignatureEvent::denom(){
	return denominator;
}

int TimeSignatureEvent::midiClocks(){
	return midiClocksPerMetronome;
}

int TimeSignatureEvent::num32In4(){
	return num32In4th;
}

int TimeSignatureEvent::ticksPerMeasure(){
	return (4*numerator*file()->ticksPerQuarter())/powf(2, denominator);
}

int TimeSignatureEvent::measures(int ticks, int *ticksLeft){
	//int numTicks = tick-midiTime();
	if(ticksLeft){
		*ticksLeft = ticks % ticksPerMeasure();
	}
	return ticks/ticksPerMeasure();
}

ProtocolEntry *TimeSignatureEvent::copy(){
	return new TimeSignatureEvent(*this);
}

void TimeSignatureEvent::reloadState(ProtocolEntry *entry){
	TimeSignatureEvent *other = qobject_cast<TimeSignatureEvent*>(entry);
	if(!other){
		return;
	}
	MidiEvent::reloadState(entry);
	numerator = other->numerator;
	denominator = other->denominator;
	midiClocksPerMetronome = other->midiClocksPerMetronome;
	num32In4th = other->num32In4th;
}
int TimeSignatureEvent::line(){
	return MidiEvent::TIME_SIGNATURE_EVENT_LINE;
}

void TimeSignatureEvent::setNumerator(int n){
	numerator = n;
	protocol(copy(), this);
}

void TimeSignatureEvent::setDenominator(int d){
	denominator = d;
	protocol(copy(), this);
}

QByteArray TimeSignatureEvent::save(){
	QByteArray array = QByteArray();
	array.append(qint8(0xFF));
	array.append(qint8(0x58));
	array.append(qint8(0x04));
	array.append(qint8(numerator));
	array.append(qint8(denominator));
	array.append(qint8(midiClocksPerMetronome));
	array.append(qint8(num32In4th));
	return array;
}

QString TimeSignatureEvent::typeString(){
	return "Time Signature Event";
}
