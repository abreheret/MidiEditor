#include "EventTool.h"

#include "../MidiEvent/MidiEvent.h"
#include "../MidiEvent/OnEvent.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../MidiEvent/OffEvent.h"
#include "../midi/MidiFile.h"
#include "../midi/MidiChannel.h"
#include "../gui/MatrixWidget.h"
#include "../protocol/Protocol.h"
#include "../midi/MidiPlayer.h"
#include "../gui/MainWindow.h"
#include "../gui/EventWidget.h"

QList<MidiEvent*> *EventTool::selectedEvents = new QList<MidiEvent*>;
QList<MidiEvent*> *EventTool::copiedEvents = new QList<MidiEvent*>;
bool EventTool::shiftPressed = false;
bool EventTool::strPressed = false;

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

	if(single && !shiftPressed && (!strPressed || ignoreStr)){
		selectedEvents->clear();
		NoteOnEvent *on = dynamic_cast<NoteOnEvent*>(event);
		if(on){
			MidiPlayer::play(on);
		}
	}
	if(!selectedEvents->contains(event) && (!strPressed || ignoreStr)){
		selectedEvents->append(event);
	} else if(strPressed && !ignoreStr){
		selectedEvents->removeAll(event);
	}
	if(single){
		_mainWindow->eventWidget()->setEvent(event);
	}
	if(selectedEvents->count()>1){
		_mainWindow->eventWidget()->setEvent(0);
	}
}

void EventTool::deselectEvent(MidiEvent *event){
	selectedEvents->removeAll(event);
	if(selectedEvents->count()==0){
		_mainWindow->eventWidget()->setEvent(0);
	}
}

void EventTool::clearSelection(){
	selectedEvents->clear();
	_mainWindow->eventWidget()->setEvent(0);
}

void EventTool::reloadState(ProtocolEntry *entry){
	EventTool *other = dynamic_cast<EventTool*>(entry);
	if(!other){
		return;
	}
	EditorTool::reloadState(entry);
	selectedEvents->clear();
	selectedEvents->append(*(other->ownSelectedEvents));
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
			painter->fillRect(event->x(), event->y(), event->width(),
					event->height(), Qt::black);
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
			copiedEvents->append(ev);
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

	if(copiedEvents->count()>0){

		// Begin a new ProtocolAction
		currentFile()->protocol()->startNewAction("Paste "+
				QString::number(copiedEvents->count())+" Events");

		// get first Tick of the copied events
		int firstTick = -1;
		foreach(MidiEvent *event, *copiedEvents){
			if(event->midiTime()<firstTick || firstTick<0){
				firstTick = event->midiTime();
			}
		}
		if(firstTick<0) firstTick = 0;

		// calculate the difference of old/new events in MidiTicks
		int diff = currentFile()->cursorTick()-firstTick;

		// set the Positions and add the Events to the channels, select Events
		selectedEvents->clear();
		foreach(MidiEvent *event, *copiedEvents){
			currentFile()->channel(event->channel())->insertEvent(event,
					event->midiTime()+diff);
			selectedEvents->append(event);
		}
		currentFile()->protocol()->endAction();

		// copy the selected Events so the user can call paste() more than
		// one time
		copyAction();
	}
}
