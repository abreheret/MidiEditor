#include "OffEvent.h"
#include "OnEvent.h"

QMultiMap<int, OnEvent*> *OffEvent::onEvents = new QMultiMap<int, OnEvent*>();

OffEvent::OffEvent(int ch, int l) : MidiEvent(ch) {
	_line = l;
	_onEvent = 0;
	QList<OnEvent*> eventsToClose = onEvents->values(line());
	for(int i = 0; i<eventsToClose.length(); i++){
		if(eventsToClose.at(i)->channel() == channel()){
			setOnEvent(eventsToClose.at(i));

			// remove entry
			removeOnEvent(eventsToClose.at(i));
			return;
		}
	}
	qWarning("nothing found!"+QString::number(line()).toAscii());
}

void OffEvent::removeOnEvent(OnEvent *event){
	onEvents->remove(event->line(), event);
	/*
	for(int j = 0; j<eventsToClose.length(); j++){
		if(i!=j){
			onEvents->insertMulti(line(), eventsToClose.at(j));
		}
	}
	*/
}
OffEvent::OffEvent(OffEvent &other) : MidiEvent(other) {
	_onEvent = other._onEvent;
}

void OffEvent::setOnEvent(OnEvent *event){
	_onEvent = event;
	event->setOffEvent(this);
}

OnEvent *OffEvent::onEvent(){
	return _onEvent;
}

void OffEvent::enterOnEvent(OnEvent *event){
	onEvents->insertMulti(event->line(), event);
}

void OffEvent::clearOnEvents(){
	onEvents->clear();
}

void OffEvent::draw(QPainter *p, QColor c){
	if(onEvent() && !onEvent()->shown()){
		onEvent()->draw(p, c);
	}
}

ProtocolEntry *OffEvent::copy(){
	return new OffEvent(*this);
}

void OffEvent::reloadState(ProtocolEntry *entry){
	OffEvent *other = dynamic_cast<OffEvent*>(entry);
	if(!other){
		return;
	}
	MidiEvent::reloadState(entry);
	_onEvent = other->_onEvent;
}

QByteArray OffEvent::save(){
	if(onEvent()){
		return onEvent()->saveOffEvent();
	}
}

QString OffEvent::toMessage(){
	if(onEvent()){
		return onEvent()->offEventMessage();
	}
}

int OffEvent::line(){
	if(onEvent()){
		return onEvent()->line();
	}
	return _line;
}
