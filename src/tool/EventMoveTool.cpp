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

#include "EventMoveTool.h"

#include "../MidiEvent/MidiEvent.h"
#include "../gui/MatrixWidget.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../MidiEvent/OffEvent.h"
#include "../protocol/Protocol.h"
#include "../midi/MidiFile.h"
#include "StandardTool.h"

EventMoveTool::EventMoveTool(bool upDown, bool leftRight) : EventTool() {
	moveUpDown = upDown;
	moveLeftRight = leftRight;
	inDrag = false;
	startX = 0;
	startY = 0;
	if(moveUpDown&&moveLeftRight){
		setImage("move_up_down_left_right.png");
		setToolTipText("Move Events (all directions)");
	} else if(moveUpDown){
		setImage("move_up_down.png");
		setToolTipText("Move Events (Up and Down)");
	} else {
		setImage("move_left_right.png");
		setToolTipText("Move Events (Left and Right)");
	}
}

EventMoveTool::EventMoveTool(EventMoveTool &other) : EventTool(other){
	moveUpDown = other.moveUpDown;
	moveLeftRight = other.moveLeftRight;
	inDrag = false;
	startX = 0;
	startY = 0;
}

ProtocolEntry *EventMoveTool::copy(){
	return new EventMoveTool(*this);
}

void EventMoveTool::reloadState(ProtocolEntry *entry){
	EventTool::reloadState(entry);
	EventMoveTool *other = dynamic_cast<EventMoveTool*>(entry);
	if(!other){
		return;
	}
	moveUpDown = other->moveUpDown;
	moveLeftRight = other->moveLeftRight;

	inDrag = false;
	startX = 0;
	startY = 0;
}

void EventMoveTool::draw(QPainter *painter){
	paintSelectedEvents(painter);
	if(inDrag){
		int shiftX = startX-mouseX;
		if(!moveLeftRight){
			shiftX = 0;
		}
		int shiftY = startY - mouseY;
		if(!moveUpDown){
			shiftY = 0;
		}
		int lineHeight = matrixWidget->lineHeight();
		shiftY = shiftY/lineHeight*lineHeight;
		foreach(MidiEvent *event, *selectedEvents){
			if(event->shown()){
                painter->setPen(Qt::lightGray);
                painter->setBrush(Qt::darkBlue);
                painter->drawRoundedRect(event->x()-shiftX, event->y()-shiftY,
                        event->width(), event->height(), 1, 1);
				painter->setPen(Qt::gray);
				painter->drawLine(event->x()-shiftX, 0, event->x()-shiftX,
						matrixWidget->height());
				painter->drawLine(event->x()+event->width()-shiftX, 0,
						event->x()+event->width()-shiftX, matrixWidget->height());
				painter->setPen(Qt::black);
			}
		}
	}
}

bool EventMoveTool::press(){
	inDrag = true;
	startX = mouseX;
	startY = mouseY;
	if(selectedEvents->count()>0){
		if(moveUpDown&&moveLeftRight){
			matrixWidget->setCursor(Qt::SizeAllCursor);
		} else if(moveUpDown){
			matrixWidget->setCursor(Qt::SizeVerCursor);
		} else {
			matrixWidget->setCursor(Qt::SizeHorCursor);
		}
	}
	return true;
}

bool EventMoveTool::release(){
	inDrag = false;
	matrixWidget->setCursor(Qt::ArrowCursor);
	int shiftX = startX-mouseX;
	if(!moveLeftRight){
		shiftX = 0;
	}
	int shiftY = startY - mouseY;
	if(!moveUpDown){
		shiftY = 0;
	}
	int lineHeight = matrixWidget->lineHeight();
	int numLines = shiftY/lineHeight;

	// return when there shiftX/shiftY is too small or there are no selected
	// events
	if(selectedEvents->count()==0 ||
			(-2<=shiftX && shiftX<=2 && -2<=shiftY && shiftY<=2))
	{
		if(_standardTool){
			Tool::setCurrentTool(_standardTool);
			_standardTool->move(mouseX, mouseY);
			_standardTool->release();
		}
		return true;
	}

	currentProtocol()->startNewAction("Move Notes", image());

	// backword to hold stability
	for(int i = selectedEvents->count()-1;i>=0;i--){
		MidiEvent *event = selectedEvents->at(i);
		NoteOnEvent *ev = dynamic_cast<NoteOnEvent*>(event);
		OffEvent *off = dynamic_cast<OffEvent*>(event);
		if(ev){
			int note = ev->note()+numLines;
			if(note<0){
				note = 0;
			}
			if(note>128){
				note = 128;
			}
			ev->setNote(note);
			changeTick(ev, shiftX);
			if(ev->offEvent()){
				changeTick(ev->offEvent(), shiftX);
			}
		} else if(!off){
			changeTick(event, shiftX);
		}
	}

	currentProtocol()->endAction();
	if(_standardTool){
		Tool::setCurrentTool(_standardTool);
		_standardTool->move(mouseX, mouseY);
		_standardTool->release();
	}
	return true;
}

bool EventMoveTool::move(int mouseX, int mouseY){
	EventTool::move(mouseX, mouseY);
	return inDrag;
}

bool EventMoveTool::releaseOnly(){
	inDrag = false;
	matrixWidget->setCursor(Qt::ArrowCursor);
	startX = 0;
	startY = 0;
	return true;
}

void EventMoveTool::setDirections(bool upDown, bool leftRight){
	moveUpDown = upDown;
	moveLeftRight = leftRight;
}
