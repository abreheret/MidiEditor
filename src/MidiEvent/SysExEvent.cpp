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

#include "SysExEvent.h"

#include <QLabel>
#include <QLayout>

SysExEvent::SysExEvent(int channel, QByteArray data) : MidiEvent(channel){
	_data = data;
}

QByteArray SysExEvent::data(){
	return _data;
}

int SysExEvent::line(){
	return SYSEX_LINE;
}

QByteArray SysExEvent::save(){
	QByteArray s;
	s.append(0xF0);
	s.append(_data);
	s.append(0xF7);
	return s;
}

void SysExEvent::generateWidget(QWidget *widget){
	MidiEvent::generateWidget(widget);
	QLayout *layout = widget->layout();
	int i = 0;
	foreach(unsigned char b,_data){
		QLabel *l = new QLabel("0x"+QString::number(i, 16)+"   0x"+QString::number(b, 16));
		layout->addWidget(l);
		i++;
	}
}

QString SysExEvent::typeString(){
	return "System Exclusive Message (SysEx)";
}
