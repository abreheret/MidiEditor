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

#include "NoteOnEvent.h"

#include "OffEvent.h"
#include <QBoxLayout>

NoteOnEvent::NoteOnEvent(int note, int velocity, int ch) : OnEvent(ch){
	_note = note;
	_velocity = velocity;
	// has to be done here because the line is not known in OnEvents constructor
	// before
	OffEvent::enterOnEvent(this);
}

NoteOnEvent::NoteOnEvent(NoteOnEvent &other) : OnEvent(other){
	_note = other._note;
	_velocity = other._velocity;
}

int NoteOnEvent::note(){
	return _note;
}

int NoteOnEvent::velocity(){
	return _velocity;
}

void NoteOnEvent::setVelocity(int v){
	ProtocolEntry *toCopy = copy();
	if(v<0){
		v = 0;
	}
	if(v>127){
		v = 127;
	}
	_velocity = v;
	protocol(toCopy, this);
	if(shownInEventWidget()){
		_velocity_box->setValue(v);
	}
}

int NoteOnEvent::line(){
	return 127-_note;
}

void NoteOnEvent::setNote(int n){
	ProtocolEntry *toCopy = copy();
	_note = n;
	protocol(toCopy, this);
	if(shownInEventWidget()){
		_note_box->setValue(n);
	}
}

ProtocolEntry *NoteOnEvent::copy(){
	return new NoteOnEvent(*this);
}

void NoteOnEvent::reloadState(ProtocolEntry *entry){
	NoteOnEvent *other = dynamic_cast<NoteOnEvent*>(entry);
	if(!other){
		return;
	}
	OnEvent::reloadState(entry);

	_note = other->_note;
	_velocity = other->_velocity;
}


QString NoteOnEvent::toMessage(){
	return "noteon "+QString::number(channel())+" "+
			QString::number(note())+" "+QString::number(velocity());
}

QString NoteOnEvent::offEventMessage(){
	return "noteoff "+QString::number(channel())+" "+QString::number(note());
}

QByteArray NoteOnEvent::save(){
	QByteArray array = QByteArray();
	array.append(0x90 | channel());
	array.append(note());
	array.append(velocity());
	return array;
}

QByteArray NoteOnEvent::saveOffEvent(){
	QByteArray array = QByteArray();
	array.append(0x80 | channel());
	array.append(note());
	array.append((char)0x0);
	return array;
}

QString NoteOnEvent::typeString(){
	return "Note On Event";
}

QWidget *NoteOnEvent::_note_widget = 0;
QSpinBox *NoteOnEvent::_note_box = 0;
QLabel *NoteOnEvent::_note_label = 0;
QWidget *NoteOnEvent::_velocity_widget = 0;
QSpinBox *NoteOnEvent::_velocity_box = 0;
QLabel *NoteOnEvent::_velocity_label = 0;

void NoteOnEvent::generateWidget(QWidget *widget){

	// general data
	OnEvent::generateWidget(widget);

	// first call
	if(_note_widget == 0) _note_widget = new QWidget();
	if(_note_box == 0) _note_box = new QSpinBox();
	if(_note_label == 0) _note_label = new QLabel();
	if(_velocity_widget == 0) _velocity_widget = new QWidget();
	if(_velocity_box == 0) _velocity_box = new QSpinBox();
	if(_velocity_label == 0) _velocity_label = new QLabel();

	// set Parents
	_note_widget->setParent(widget);
	_note_label->setParent(_note_widget);
	_note_box->setParent(_note_widget);
	_velocity_widget->setParent(widget);
	_velocity_label->setParent(_velocity_widget);
	_velocity_box->setParent(_velocity_widget);

	// unhide
	_note_widget->setHidden(false);
	_note_label->setHidden(false);
	_note_box->setHidden(false);
	_velocity_widget->setHidden(false);
	_velocity_label->setHidden(false);
	_velocity_box->setHidden(false);


	// Edit note
	_note_label->setText("Note value: ");
	QLayout *noteL = _note_widget->layout();
	if(!noteL){
		noteL = new QBoxLayout(QBoxLayout::LeftToRight, _note_widget);
		_note_widget->setLayout(noteL);
	}
	noteL->addWidget(_note_label);

	// box
	_note_box->setMaximum(127);
	_note_box->setMinimum(0);
	_note_box->setValue(note());
	noteL->addWidget(_note_box);

	// Edit velocity
	_velocity_label->setText("Velocity: ");
	QLayout *velocityL = _velocity_widget->layout();
	if(!velocityL){
		velocityL = new QBoxLayout(QBoxLayout::LeftToRight, _velocity_widget);
		_velocity_widget->setLayout(velocityL);
	}
	velocityL->addWidget(_velocity_label);

	// box
	_velocity_box->setMaximum(127);
	_velocity_box->setMinimum(0);
	_velocity_box->setValue(velocity());
	velocityL->addWidget(_velocity_box);

	// Add Widgets
	QLayout *layout = widget->layout();
	layout->addWidget(_note_widget);
	layout->addWidget(_velocity_widget);
}

void NoteOnEvent::editByWidget(){
	if(velocity()!=_velocity_box->value()){
		setVelocity(_velocity_box->value());
	}
	if(note()!=_note_box->value()){
		setNote(_note_box->value());
	}
	OnEvent::editByWidget();
}
