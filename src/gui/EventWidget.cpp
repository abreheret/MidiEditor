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

#include "EventWidget.h"

#include <QBoxLayout>
#include <QPushButton>
#include "../MidiEvent/MidiEvent.h"
#include "../protocol/Protocol.h"
#include "../midi/MidiFile.h"

EventWidget::EventWidget(QWidget *parent) : QWidget(parent) {
	_event = 0;
	QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	setLayout(layout);
}

void EventWidget::setEvent(MidiEvent *event){
	_event = event;
	reload();
	emit eventSelected(event);
}

MidiEvent *EventWidget::event(){
	return _event;
}

void EventWidget::reload(){
	QBoxLayout *l = dynamic_cast<QBoxLayout*>(layout());
	int size = l->count();
	for (int i = 0; i<size; i++){
		QWidget *w = l->itemAt(i)->widget();
		if(w){
			w->hide();
		}
	}
	l->update();
	for(int i = 0; i<size; i++){
		l->removeItem(l->itemAt(0));
	}
	if(_event){
		_event->generateWidget(this);
		QPushButton *submit = new QPushButton("Change values");
		connect(submit, SIGNAL(clicked()), this, SLOT(submit()));
		l->addWidget(submit);
		l->addStretch();
	}
}

void EventWidget::submit(){
	_event->file()->protocol()->startNewAction("edit "+_event->typeString());
	_event->editByWidget();
	_event->file()->protocol()->endAction();
}
