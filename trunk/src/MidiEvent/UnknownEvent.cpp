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

#include "UnknownEvent.h"

#include <QLabel>
#include <QLayout>

#include "../midi/MidiFile.h"

UnknownEvent::UnknownEvent(int channel, int type, QByteArray data, MidiTrack *track) : MidiEvent(channel, track){
	_data = data;
	_type = type;
}

QByteArray UnknownEvent::data(){
	return _data;
}

int UnknownEvent::line(){
	return UNKNOWN_LINE;
}

QByteArray UnknownEvent::save(){
	QByteArray s;
	s.append(0xFF);
	s.append(_type);
	s.append(MidiFile::writeVariableLengthValue(_data.length()));
	s.append(_data);
	return s;
}

void UnknownEvent::generateWidget(QWidget *widget){
	MidiEvent::generateWidget(widget);
	QLayout *layout = widget->layout();
	QLabel *t = new QLabel("Type: 0x"+QString::number(_type, 16));
	layout->addWidget(t);
	t = new QLabel("Data:");
	layout->addWidget(t);
	int i = 0;
	foreach(unsigned char b,_data){
		QLabel *l = new QLabel("0x"+QString::number(i, 16)+"   0x"+QString::number(b, 16));
		layout->addWidget(l);
		i++;
	}
}
