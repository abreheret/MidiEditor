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

#include <QSpinBox>
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QLayout>

TimeSignatureEvent::TimeSignatureEvent(int channel, int num, int denom,
		int midiClocks, int num32In4, MidiTrack *track) : MidiEvent(channel, track)
{
	numerator = num;
	denominator = denom;
	midiClocksPerMetronome = midiClocks;
	num32In4th = num32In4;
}

TimeSignatureEvent::TimeSignatureEvent(TimeSignatureEvent &other):
		MidiEvent(other)
{
	numerator = other.numerator;
	denominator = other.denominator;
	midiClocksPerMetronome = other.midiClocksPerMetronome;
	num32In4th = other.num32In4th;
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
	return (4*numerator*file()->ticksPerQuarter())/pow(2, denominator);
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
	TimeSignatureEvent *other = dynamic_cast<TimeSignatureEvent*>(entry);
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
	ProtocolEntry *toCopy = copy();
	numerator = n;
	protocol(toCopy, this);
	if(shownInEventWidget()){
		_num_box->setValue(n);
	}
}

void TimeSignatureEvent::setDenominator(int d){
	ProtocolEntry *toCopy = copy();
	denominator = d;
	protocol(toCopy, this);
	if(shownInEventWidget()){
		_denom_combo->setCurrentIndex(d);
	}
}

QByteArray TimeSignatureEvent::save(){
	QByteArray array = QByteArray();
	array.append(0xFF);
	array.append(0x58);
	array.append(0x04);
	array.append(numerator);
	array.append(denominator);
	array.append(midiClocksPerMetronome);
	array.append(num32In4th);
	return array;
}

QString TimeSignatureEvent::typeString(){
	return "Time Signature Event";
}
// Widgets for EventWidget
QWidget *TimeSignatureEvent::_num_widget = 0;
QSpinBox *TimeSignatureEvent::_num_box = 0;
QLabel *TimeSignatureEvent::_num_label = 0;
QWidget *TimeSignatureEvent::_denom_widget = 0;
QComboBox *TimeSignatureEvent::_denom_combo = 0;
QLabel *TimeSignatureEvent::_denom_label = 0;

void TimeSignatureEvent::generateWidget(QWidget *widget){
	// general data
	MidiEvent::generateWidget(widget);

	// first call
	if(_num_widget == 0) _num_widget = new QWidget();
	if(_num_box == 0) _num_box = new QSpinBox();
	if(_num_label == 0) _num_label = new QLabel();
	if(_denom_widget == 0) _denom_widget = new QWidget();
	if(_denom_combo == 0) {
		_denom_combo = new QComboBox();
		for(int i = 0; i<7; i++){
			_denom_combo->addItem(QString::number(pow(2, i)));
		}
	}
	if(_denom_label == 0) _denom_label = new QLabel();

	// set Parents
	_num_widget->setParent(widget);
	_num_label->setParent(_num_widget);
	_num_box->setParent(_num_widget);
	_denom_widget->setParent(widget);
	_denom_label->setParent(_denom_widget);
	_denom_combo->setParent(_denom_widget);

	// unhide
	_num_widget->setHidden(false);
	_num_label->setHidden(false);
	_num_box->setHidden(false);
	_denom_widget->setHidden(false);
	_denom_label->setHidden(false);
	_denom_combo->setHidden(false);

	// Edit Note
	_denom_label->setText("Denominator: ");
	QLayout *denomL = _denom_widget->layout();
	if(!denomL){
		denomL = new QBoxLayout(QBoxLayout::LeftToRight, _denom_widget);
		_denom_widget->setLayout(denomL);
	}
	denomL->addWidget(_denom_label);

	// box
	_denom_combo->setCurrentIndex(denominator);
	denomL->addWidget(_denom_combo);

	// Edit value
	_num_label->setText("Numerator: ");
	QLayout *nomL = _num_widget->layout();
	if(!nomL){
		nomL = new QBoxLayout(QBoxLayout::LeftToRight, _num_widget);
		_num_widget->setLayout(nomL);
	}
	nomL->addWidget(_num_label);

	// box
	_num_box->setMaximum(30);
	_num_box->setMinimum(1);
	_num_box->setValue(numerator);
	nomL->addWidget(_num_box);

	// Add Widgets
	QLayout *layout = widget->layout();
	layout->addWidget(_num_widget);
	layout->addWidget(_denom_widget);

}

void TimeSignatureEvent::editByWidget(){
	if(denominator!=_denom_combo->currentIndex()){
		setDenominator(_denom_combo->currentIndex());
	}
	if(numerator!=_num_box->value()){
		setNumerator(_num_box->value());
	}
	MidiEvent::editByWidget();
}
