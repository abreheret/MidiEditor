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

#include "EventTool.h"

#include "../MidiEvent/MidiEvent.h"
#include "../MidiEvent/OnEvent.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../MidiEvent/OffEvent.h"
#include "../midi/MidiFile.h"
#include "../midi/MidiTrack.h"
#include "../midi/MidiChannel.h"
#include "../gui/MatrixWidget.h"
#include "../protocol/Protocol.h"
#include "../midi/MidiPlayer.h"
#include "../gui/MainWindow.h"
#include "../gui/EventWidget.h"

#include <QtCore/qmath.h>

QList<MidiEvent*> *EventTool::selectedEvents = new QList<MidiEvent*>;
QList<MidiEvent*> *EventTool::copiedEvents = new QList<MidiEvent*>;
int EventTool::_pasteChannel = -1;
MidiTrack *EventTool::_pasteTrack = 0;
bool EventTool::_magnet = false;

EventTool::EventTool() : EditorTool() {
	ownSelectedEvents = 0;
}

EventTool::EventTool(EventTool &other) : EditorTool(other) {
	ownSelectedEvents = new QList<MidiEvent*>;
	ownSelectedEvents->append(*selectedEvents);
}

ProtocolEntry *EventTool::copy(){
	return new EventTool(*this);
}

void EventTool::selectEvent(MidiEvent *event, bool single, bool ignoreStr){

	if(!event->file()->channel(event->channel())->visible()){
		return;
	}

	if(event->track()->hidden()){
		return;
	}

	OffEvent *offevent = dynamic_cast<OffEvent*>(event);
	if(offevent){
		return;
	}

	if(single && !QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) && (!QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) || ignoreStr)){
		selectedEvents->clear();
		NoteOnEvent *on = dynamic_cast<NoteOnEvent*>(event);
		if(on){
			MidiPlayer::play(on);
		}
	}
	if(!selectedEvents->contains(event) && (!QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) || ignoreStr)){
		selectedEvents->append(event);
	} else if(QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) && !ignoreStr){
		selectedEvents->removeAll(event);
	}
	_mainWindow->eventWidget()->setEvents(*selectedEvents);
}

void EventTool::deselectEvent(MidiEvent *event){
	selectedEvents->removeAll(event);
	if(_mainWindow->eventWidget()->events().contains(event)){
		_mainWindow->eventWidget()->removeEvent(event);
	}
}

void EventTool::clearSelection(){
	selectedEvents->clear();
	_mainWindow->eventWidget()->setEvents(*selectedEvents);
}

void EventTool::reloadState(ProtocolEntry *entry){
	EventTool *other = dynamic_cast<EventTool*>(entry);
	if(!other){
		return;
	}
	EditorTool::reloadState(entry);
	selectedEvents->clear();
	selectedEvents->append(*(other->ownSelectedEvents));
	_mainWindow->eventWidget()->setEvents(*selectedEvents);
}

bool EventTool::pressKey(int key){
	if(key == Qt::Key_Delete && selectedEvents->size()>0){
		ProtocolEntry *toCopy = copy();
		// delete all selected Events
		currentFile()->protocol()->startNewAction("Remove Event(s)");
		foreach(MidiEvent *ev, *selectedEvents){
			file()->channel(ev->channel())->removeEvent(ev);
		}
		selectedEvents->clear();
		_mainWindow->eventWidget()->setEvents(*selectedEvents);
		protocol(toCopy, this);
		currentFile()->protocol()->endAction();
		return true;
	}
	return false;
}

void EventTool::paintSelectedEvents(QPainter *painter){
	foreach(MidiEvent *event, *selectedEvents){

		bool show = event->shown();
		if(!show){
			OnEvent *ev = dynamic_cast<OnEvent*>(event);
			if(ev){
				show = ev->offEvent() && ev->offEvent()->shown();
			}
		}
		if(show){
            painter->setBrush(Qt::darkBlue);
            painter->setPen(Qt::lightGray);
            painter->drawRoundedRect(event->x(), event->y(), event->width(),
                    event->height(), 1, 1);
		}
	}
}

void EventTool::changeTick(MidiEvent* event, int shiftX){
	// TODO: falls event gezeigt ist, über matrixWidget tick erfragen (effizienter)
	//int newMs = matrixWidget->msOfXPos(event->x()-shiftX);

	int newMs = file()->msOfTick(event->midiTime())-matrixWidget->timeMsOfWidth(shiftX);
	int tick = file()->tick(newMs);
	event->setMidiTime(tick);
}

QList<MidiEvent*> *EventTool::selectedEventList(){
	return selectedEvents;
}


void EventTool::copyAction(){

	// clear old copied Events
	copiedEvents->clear();

	foreach(MidiEvent *event, *selectedEvents){

		// add the current Event
		MidiEvent *ev = dynamic_cast<MidiEvent*>(event->copy());
		if(ev){
			// do not append off event here
			OffEvent *off = dynamic_cast<OffEvent*>(ev);
			if(!off){
				copiedEvents->append(ev);
			}
		}

		// if its onEvent, add a copy of the OffEvent
		OnEvent *onEv = dynamic_cast<OnEvent*>(ev);
		if(onEv){
			OffEvent *offEv = dynamic_cast<OffEvent*>(onEv->offEvent()->copy());
			offEv->setOnEvent(onEv);
			copiedEvents->append(offEv);
		}
	}
}

void EventTool::pasteAction(){

	// TODO what happends to TempoEvents??

	// copy copied events to insert unique events
	QList<MidiEvent*> copiedCopiedEvents;
	foreach(MidiEvent *event, *copiedEvents){

		// add the current Event
		MidiEvent *ev = dynamic_cast<MidiEvent*>(event->copy());
		if(ev){
			// do not append off event here
			OffEvent *off = dynamic_cast<OffEvent*>(ev);
			if(!off){
				copiedCopiedEvents.append(ev);
			}
		}

		// if its onEvent, add a copy of the OffEvent
		OnEvent *onEv = dynamic_cast<OnEvent*>(ev);
		if(onEv){
			OffEvent *offEv = dynamic_cast<OffEvent*>(onEv->offEvent()->copy());
			offEv->setOnEvent(onEv);
			copiedCopiedEvents.append(offEv);
		}
	}

	if(copiedCopiedEvents.count()>0){

		// Begin a new ProtocolAction
		currentFile()->protocol()->startNewAction("Paste "+
				QString::number(copiedCopiedEvents.count())+" Events");

		// get first Tick of the copied events
		int firstTick = -1;
		foreach(MidiEvent *event, copiedCopiedEvents){
			if(event->midiTime()<firstTick || firstTick<0){
				firstTick = event->midiTime();
			}
		}

		if(firstTick<0) firstTick = 0;

		// calculate the difference of old/new events in MidiTicks
		int diff = currentFile()->cursorTick()-firstTick;

		// set the Positions and add the Events to the channels
		foreach(MidiEvent *event, copiedCopiedEvents){

			// get channel
			int channel = event->channel();
			if((_pasteChannel >= 0) && (channel < 16)){
				channel = _pasteChannel;
			}

			// get track
			MidiTrack *track = event->track();
			if(pasteTrack()){
				track = pasteTrack();
			} else if(event->file() != currentFile() || !currentFile()->tracks()->contains(track)){
				track = currentFile()->getPasteTrack(event->track(), event->file());
				if(!track){
					track = event->track()->copyToFile(currentFile());
				}
			}

			event->setFile(currentFile());
			event->setChannel(channel, false);
			event->setTrack(track, false);
			currentFile()->channel(channel)->insertEvent(event,
					event->midiTime()+diff);
		}
		currentFile()->protocol()->endAction();
	}
}

bool EventTool::showsSelection(){
	return false;
}

void EventTool::setPasteTrack(MidiTrack *track){
	_pasteTrack = track;
}

MidiTrack *EventTool::pasteTrack(){
	return _pasteTrack;
}

void EventTool::setPasteChannel(int channel){
	_pasteChannel = channel;
}

int EventTool::pasteChannel(){
	return _pasteChannel;
}

int EventTool::rasteredX(int x){
	if(!_magnet){
		return x;
	}
	typedef QPair<int, int> TMPPair;
	foreach(TMPPair p, matrixWidget->divs()){
		int xt = p.first;
		if(qAbs(xt-x)<=5){
			return xt;
		}
	}
	return x;
}

void EventTool::enableMagnet(bool enable){
	_magnet = enable;
}

bool EventTool::magnetEnabled(){
	return _magnet;
}
