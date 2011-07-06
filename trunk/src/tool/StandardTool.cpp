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

#include "StandardTool.h"
#include "EventMoveTool.h"
#include "SizeChangeTool.h"
#include "SelectTool.h"

#include "../MidiEvent/MidiEvent.h"
#include "../gui/MatrixWidget.h"
#include "../midi/MidiFile.h"
#include "../protocol/Protocol.h"

StandardTool::StandardTool() : EventTool() {
	setImage("select.png");
	moveTool = new EventMoveTool(true, true);
	moveTool->setStandardTool(this);
	sizeChangeTool = new SizeChangeTool();
	sizeChangeTool->setStandardTool(this);
	selectTool = new SelectTool(SELECTION_TYPE_BOX);
	selectTool->setStandardTool(this);
	setToolTipText("Standard Tool");
}

StandardTool::StandardTool(StandardTool &other) : EventTool(other){
	sizeChangeTool = other.sizeChangeTool;
	moveTool = other.moveTool;
	selectTool = other.selectTool;
}

void StandardTool::draw(QPainter *painter){
	paintSelectedEvents(painter);
}

bool StandardTool::press(){

	foreach(MidiEvent *ev, *(matrixWidget->activeEvents())){

		// left/right side means SizeChangeTool
		if(pointInRect(mouseX, mouseY, ev->x()-2, ev->y(), ev->x()+2,
				ev->y()+ev->height()) ||
				pointInRect(mouseX, mouseY, ev->x()+ev->width()-2, ev->y(),
						ev->x()+ev->width()+2, ev->y()+ev->height()))
		{
			file()->protocol()->startNewAction("Selection changed", image());
			ProtocolEntry* toCopy = copy();
			EventTool::selectEvent(ev,!EventTool::selectedEvents->contains(ev));
			protocol(toCopy, this);
			file()->protocol()->endAction();

			Tool::setCurrentTool(sizeChangeTool);
			sizeChangeTool->move(mouseX, mouseY);
			sizeChangeTool->press();
			return false;
		}

		// in the event means EventMoveTool
		if(pointInRect(mouseX, mouseY, ev->x(), ev->y(), ev->x()+ev->width(),
				ev->y()+ev->height()))
		{

			file()->protocol()->startNewAction("Selection changed", image());
			ProtocolEntry* toCopy = copy();
			EventTool::selectEvent(ev,!EventTool::selectedEvents->contains(ev));
			protocol(toCopy, this);
			file()->protocol()->endAction();

			Tool::setCurrentTool(moveTool);
			moveTool->move(mouseX, mouseY);
			moveTool->press();
			return false;
		}
	}

	// no event means SelectTool
	Tool::setCurrentTool(selectTool);
	selectTool->move(mouseX, mouseY);
	selectTool->press();
	return true;
}

bool StandardTool::move(int mouseX, int mouseY){
	EventTool::move(mouseX, mouseY);
	foreach(MidiEvent *ev, *(matrixWidget->activeEvents())){
		// left/right side means SizeChangeTool
		if(pointInRect(mouseX, mouseY, ev->x()-2, ev->y(), ev->x()+2,
				ev->y()+ev->height()) ||
				pointInRect(mouseX, mouseY, ev->x()+ev->width()-2, ev->y(),
						ev->x()+ev->width()+2, ev->y()+ev->height()))
		{
			matrixWidget->setCursor(Qt::SplitHCursor);
			return false;
		}
	}
	matrixWidget->setCursor(Qt::ArrowCursor);
	return false;
}

ProtocolEntry *StandardTool::copy(){
	return new StandardTool(*this);
}

void StandardTool::reloadState(ProtocolEntry *entry){
	StandardTool *other = dynamic_cast<StandardTool*>(entry);
	if(!other){
		return;
	}
	EventTool::reloadState(entry);
	sizeChangeTool = other->sizeChangeTool;
	moveTool = other->moveTool;
	selectTool = other->selectTool;
}

bool StandardTool::release(){
	matrixWidget->setCursor(Qt::ArrowCursor);
	return true;
}
