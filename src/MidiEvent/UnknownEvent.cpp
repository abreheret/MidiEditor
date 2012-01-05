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

UnknownEvent::UnknownEvent(int channel, QByteArray data) : MidiEvent(channel){
	_data = data;
}

QByteArray UnknownEvent::data(){
	return _data;
}

int UnknownEvent::line(){
	return UNKNOWN_LINE;
}

QByteArray UnknownEvent::save(){
	return data();
}

void UnknownEvent::generateWidget(QWidget *widget){
	MidiEvent::generateWidget(widget);
	QLayout *layout = widget->layout();
	int i = 0;
	foreach(char b,_data){
		QLabel *l = new QLabel("0x"+QString::number(i, 16)+"   0x"+QString::number(b, 16));
		layout->addWidget(l);
		i++;
	}
}
