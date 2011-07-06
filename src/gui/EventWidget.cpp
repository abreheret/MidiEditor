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
