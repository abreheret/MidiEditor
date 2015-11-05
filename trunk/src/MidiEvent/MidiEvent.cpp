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

#include "MidiEvent.h"
#include "NoteOnEvent.h"
#include "OffEvent.h"
#include "UnknownEvent.h"
#include "TempoChangeEvent.h"
#include "TimeSignatureEvent.h"
#include "ProgChangeEvent.h"
#include "ControlChangeEvent.h"
#include "ChannelPressureEvent.h"
#include "KeyPressureEvent.h"
#include "TextEvent.h"
#include "../midi/MidiFile.h"
#include "../gui/EventWidget.h"
#include "KeySignatureEvent.h"
#include "PitchBendEvent.h"
#include "SysExEvent.h"

#include <QByteArray>
#include <QSpinBox>
#include <QLabel>
#include <QLayout>
#include <QWidget>
#include <QBoxLayout>
#include "../midi/MidiChannel.h"

quint8 MidiEvent::_startByte = 0;
EventWidget *MidiEvent::_eventWidget = 0;

MidiEvent::MidiEvent(int channel) : GraphicObject(), ProtocolEntry(){
	numTrack = 0;
	numChannel = channel;
	timePos = 0;
	midiFile = 0;
}

MidiEvent::MidiEvent(MidiEvent &other) : GraphicObject(), ProtocolEntry(other) {
	numTrack = other.numTrack;
	numChannel = other.numChannel;
	timePos = other.timePos;
	midiFile = other.midiFile;
}

MidiEvent *MidiEvent::loadMidiEvent(QDataStream *content, bool *ok,
		bool *endEvent, quint8 startByte, quint8 secondByte)
{

	// brief first try to load the event. If this does not work try to use
	// old first byte as new first byte. This is implemented in the end of this
	// method using recursive calls.
	// if startByte (paramater) is not 0, this is the second call so first and
	// second byte must not be loaded from the stream but from the parameters.

	*ok = true;

	quint8 tempByte;

	quint8 prevStartByte = _startByte;

	if(!startByte){
		(*content)>>tempByte;
	} else {
		tempByte = startByte;
	}
	_startByte = tempByte;

	int channel = tempByte & 0x0F;


	switch(tempByte&0xF0){

		case 0x80: {
			// Note Off
			if(!startByte){
				(*content)>>tempByte;
			} else {
				tempByte = secondByte;
			}
			int note = tempByte;
			if(note<0 || note>127){
				*ok = false;
				return 0;
			}
			// skip byte (velocity)
			(*content)>>tempByte;

			OffEvent *event = new OffEvent(channel, 127-note);
			*ok = true;
			return event;
		}


		case 0x90: {
			// Note On
			if(!startByte){
				(*content)>>tempByte;
			} else {
				tempByte = secondByte;
			}
			int note = tempByte;
			if(note<0 || note>127){
				*ok = false;
				return 0;
			}
			(*content)>>tempByte;
			int velocity = tempByte;
			*ok = true;

			if(velocity>0){
				NoteOnEvent *event = new NoteOnEvent(note, velocity, channel);
				return event;
			} else {
				OffEvent *event = new OffEvent(channel, 127-note);
				return event;
			}
		}

		case 0xA0: {
			// Key Pressure
			if(!startByte){
				(*content)>>tempByte;
			} else {
				tempByte = secondByte;
			}
			int note = tempByte;
			if(note<0 || note>127){
				*ok = false;
				return 0;
			}
			(*content)>>tempByte;
			int value = tempByte;

			*ok = true;

			return new KeyPressureEvent(channel, value, note);
		}

		case 0xB0: {
			// Controller
			if(!startByte){
				(*content)>>tempByte;
			} else {
				tempByte = secondByte;
			}
			int control = tempByte;
			(*content)>>tempByte;
			int value = tempByte;
			*ok = true;
			return new ControlChangeEvent(channel, control, value);
		}

		case 0xC0: {
			// programm change
			if(!startByte){
				(*content)>>tempByte;
			} else {
				tempByte = secondByte;
			}
			*ok = true;
			return new ProgChangeEvent(channel, tempByte);
		}

		case 0xD0: {
			// Key Pressure
			if(!startByte){
				(*content)>>tempByte;
			} else {
				tempByte = secondByte;
			}
			int value = tempByte;

			*ok = true;

			return new ChannelPressureEvent(channel, value);
		}

		case 0xE0: {

			// Pitch Wheel
            if(!startByte){
                (*content)>>tempByte;
            } else {
                tempByte = secondByte;
            }
            quint8 first = tempByte;
            (*content)>>tempByte;
            quint8 second = tempByte;

            int value = (second << 7) | first;

            *ok = true;

            return new PitchBendEvent(channel, value);
		}

		case 0xF0: {
			// System Message
			channel = 16; // 16 is channel without number


			switch(tempByte & 0x0F){

				case 0x00: {

					// SysEx
					QByteArray array;
					while(tempByte!=0xF7){
						(*content)>>tempByte;
						if(tempByte!=0xF7){
							array.append((char)tempByte);
						}
					}
					*ok = true;
					return new	SysExEvent(channel, array);
				}

				case 0x0F: {
					// MetaEvent
					if(!startByte){
						(*content)>>tempByte;
					} else {
						tempByte = secondByte;
					}
					switch(tempByte){
						case 0x51: {
							// TempoChange
							//(*content)>>tempByte;
							//if(tempByte!=3){
							//	*ok = false;
							//	return 0;
							//}
							quint32 value;
							(*content)>>value;
							// 1te Stelle abziehen,
							value-=50331648;
							return new TempoChangeEvent(17, (int)value);
						}
						case 0x58: {
							// TimeSignature
							(*content)>>tempByte;
							if(tempByte!=4){
								*ok = false;
								return 0;
							}

							(*content)>>tempByte;
							int num = (int)tempByte;
							(*content)>>tempByte;
							int denom = (int)tempByte;
							(*content)>>tempByte;
							int metronome = (int)tempByte;
							(*content)>>tempByte;
							int num32 = (int)tempByte;
							return new TimeSignatureEvent(18, num, denom, metronome, num32);
						}
						case 0x59: {
							// keysignature
							(*content)>>tempByte;
							if(tempByte!=2){
								*ok = false;
								return 0;
							}
							qint8 t;
							(*content)>>t;
							int tonality = (int)t;
							(*content)>>tempByte;
							bool minor = true;
							if(tempByte == 0){
								minor = false;
							}
							return new KeySignatureEvent(channel, tonality, minor);
						}
						case 0x2F: {
							// end Event
							*endEvent = true;
							*ok = true;
							return 0;
						}
						default: {
							if(tempByte >= 0x01 && tempByte <=0x07){

								// textevent
								// read type
								TextEvent *textEvent = new TextEvent(channel);
								textEvent->setType(tempByte);
								int length = MidiFile::variableLengthvalue(content);
								QByteArray array;
								for(int i = 0; i<length; i++){
									(*content)>>tempByte;
									array.append((char)tempByte);
								}
								textEvent->setText(QString(array));
								*ok = true;
								return textEvent;

							} else {

								// tempByte is meta event type
								int typeByte = ((char)tempByte);

								// read length
								int length = MidiFile::variableLengthvalue(content);

								// content
								QByteArray array;
								for(int i = 0; i<length; i++){
									(*content)>>tempByte;
									array.append((char)tempByte);
								}
								*ok = true;
								return new UnknownEvent(channel, typeByte, array);
							}
						}
					}

				}
			}

		}
	}

	// if the event could not be loaded try to use old firstByte before the new
	// data.
	// To do this, pass prefFirstByte and secondByte (the current firstByte)
	// and use it recursive.
	_startByte = prevStartByte;
	return loadMidiEvent(content, ok, endEvent, _startByte, tempByte);
}

void MidiEvent::setTrack(int num, bool toProtocol){
	ProtocolEntry *toCopy = copy();
	if(num < file()->numTracks()){
		numTrack = num;
		if(toProtocol){
			protocol(toCopy, this);
		} else {
			delete toCopy;
		}
	}
	if(shownInEventWidget()){
		_track_spinBox->setValue(numTrack);
	}
}

int MidiEvent::track(){
	return numTrack;
}

void MidiEvent::setChannel(int ch, bool toProtocol){
	int oldChannel = channel();
	ProtocolEntry *toCopy = copy();
	numChannel = ch;
	if(toProtocol){
		protocol(toCopy, this);
		file()->channelEvents(oldChannel)->remove(midiTime(), this);
		// tells the new channel to add this event
		setMidiTime(midiTime(), toProtocol);
	} else {
		delete toCopy;
	}
	if(shownInEventWidget()){
		_channel_spinBox->setValue(ch);
	}
}

int MidiEvent::channel(){
	return numChannel;
}

QString MidiEvent::toMessage(){
	return "";
}

QByteArray MidiEvent::save(){
	return QByteArray();
}

void MidiEvent::setMidiTime(int t, bool toProtocol){

	// if its once TimeSig / TempoChange at 0, dont delete event
	if(toProtocol && (channel()==18 || channel()==17)){
		if(midiTime()==0 && midiFile->channel(channel())->eventMap()->count(0)
				== 1)
		{
			return;
		}
	}

	ProtocolEntry *toCopy = copy();
	file()->channelEvents(numChannel)->remove(timePos, this);
	timePos = t;
	if(timePos>file()->endTick()){
		file()->setMaxLengthMs(file()->msOfTick(timePos)+100);
	}
	if(toProtocol){
		protocol(toCopy, this);
	} else {
		delete toCopy;
	}
	file()->channelEvents(numChannel)->insert(timePos, this);

	if(shownInEventWidget()){
		_timePos_spinBox->setValue(timePos);
	}
}

int MidiEvent::midiTime(){
	return timePos;
}

void MidiEvent::setFile(MidiFile *f){
	midiFile = f;
}

MidiFile *MidiEvent::file(){
	return midiFile;
}

int MidiEvent::line(){
	return 0;
}

void MidiEvent::draw(QPainter *p, QColor c){
    p->setPen(Qt::gray);
    p->setBrush(c);
    p->drawRoundedRect(x(), y(), width(), height(), 1, 1);
}

ProtocolEntry *MidiEvent::copy(){
	return new MidiEvent(*this);
}

void MidiEvent::reloadState(ProtocolEntry *entry){

	MidiEvent *other = dynamic_cast<MidiEvent*>(entry);
	if(!other){
		return;
	}
	numTrack = other->numTrack;
	numChannel = other->numChannel;
	file()->channelEvents(numChannel)->remove(timePos, this);
	timePos = other->timePos;
	file()->channelEvents(numChannel)->insert(timePos, this);
	midiFile = other->midiFile;
}

QString MidiEvent::typeString(){
	return "Midi Event";
}

// for generating the Widget
QSpinBox *MidiEvent::_channel_spinBox = 0;
QSpinBox *MidiEvent::_track_spinBox = 0;
QSpinBox *MidiEvent::_timePos_spinBox = 0;
QLabel *MidiEvent::_channel_label = 0;
QLabel *MidiEvent::_track_label = 0;
QLabel *MidiEvent::_timePos_label = 0;
QLabel *MidiEvent::_title_label = 0;
QWidget *MidiEvent::_channel_widget = 0;
QWidget *MidiEvent::_track_widget = 0;
QWidget *MidiEvent::_timePos_widget = 0;

void MidiEvent::generateWidget(QWidget *widget){

	// first call: content has to be generated
	if(_channel_spinBox == 0) _channel_spinBox = new QSpinBox();
	if(_track_spinBox == 0) _track_spinBox = new QSpinBox();
	if(_timePos_spinBox == 0) _timePos_spinBox = new QSpinBox();
	if(_channel_label == 0) _channel_label = new QLabel();
	if(_track_label == 0) _track_label = new QLabel();
	if(_timePos_label == 0) _timePos_label = new QLabel();
	if(_title_label == 0) _title_label = new QLabel();
	if(_channel_widget == 0) _channel_widget = new QWidget();
	if(_track_widget == 0) _track_widget = new QWidget();
	if(_timePos_widget == 0) _timePos_widget = new QWidget();

	// set the new parent
	_channel_spinBox->setParent(_channel_widget);
	_track_spinBox->setParent(_track_widget);
	_timePos_spinBox->setParent(_timePos_widget);
	_channel_label->setParent(_channel_widget);
	_track_label->setParent(_track_widget);
	_timePos_label->setParent(_timePos_widget);
	_timePos_widget->setParent(widget);
	_channel_widget->setParent(widget);
	_track_widget->setParent(widget);
	_title_label->setParent(widget);

	// unhide
	_timePos_widget->setHidden(false);
	_channel_widget->setHidden(false);
	_track_widget->setHidden(false);
	_title_label->setHidden(false);

	// edit channelSpinBox
	_channel_spinBox->setMaximum(15);
	_channel_spinBox->setMinimum(0);
	_channel_spinBox->setValue(channel());
	_channel_label->setText("Channel:");

	// edit TrackSpinBox
	_track_spinBox->setMinimum(0);
	_track_spinBox->setMaximum(file()->numTracks()-1);
	_track_spinBox->setValue(track());
	_track_label->setText("Track:");

	// edit timePosSpinBox
	_timePos_spinBox->setMaximum(file()->endTick());
	_timePos_spinBox->setMinimum(0);
	_timePos_spinBox->setValue(midiTime());
	_timePos_label->setText("Midi Tick:");

	// edit the title
	_title_label->setText(typeString());

	// get the Layout
	QLayout *layout = widget->layout();

	// add the data
	layout->addWidget(_title_label);

	// time
	QLayout *timeL = _timePos_widget->layout();
	if(!timeL){
		timeL = new QBoxLayout(QBoxLayout::LeftToRight, _timePos_widget);
		_timePos_widget->setLayout(timeL);
	}
	timeL->addWidget(_timePos_label);
	timeL->addWidget(_timePos_spinBox);
	layout->addWidget(_timePos_widget);

	// Channel
	QLayout *channelL = _channel_widget->layout();
	if(!channelL){
		channelL = new QBoxLayout(QBoxLayout::LeftToRight,_channel_widget);
		_channel_widget->setLayout(channelL);
	}
	channelL->addWidget(_channel_label);

	if(channel()<16){
		channelL->addWidget(_channel_spinBox);
		_channel_spinBox->setVisible(true);
	} else {
		_channel_label->setText("Channel: General Channel");
		_channel_spinBox->hide();
	}
	layout->addWidget(_channel_widget);

	// Track
	QLayout *trackL = _track_widget->layout();
	if(!trackL){
		trackL = new QBoxLayout(QBoxLayout::LeftToRight,_track_widget);
		_track_widget->setLayout(trackL);
	}
	trackL->addWidget(_track_label);
	trackL->addWidget(_track_spinBox);

	layout->addWidget(_track_widget);
}

void MidiEvent::editByWidget(){
	if(midiTime()!=_timePos_spinBox->value()){
		setMidiTime(_timePos_spinBox->value(), true);
	}
	if(track()!=_track_spinBox->value()){
		setTrack(_track_spinBox->value());
	}
	if(channel()<16 && channel()!=_channel_spinBox->value()){
		setChannel(_channel_spinBox->value());
	}
}

void MidiEvent::setEventWidget(EventWidget *widget){
	_eventWidget = widget;
}

EventWidget *MidiEvent::eventWidget(){
	return _eventWidget;
}

bool MidiEvent::shownInEventWidget(){
	if(!_eventWidget){
		return false;
	}
	return _eventWidget->event() == this;
}

bool MidiEvent::isOnEvent(){
	return true;
}
