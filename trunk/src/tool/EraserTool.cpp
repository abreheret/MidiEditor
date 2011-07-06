#include "EraserTool.h"

#include "../gui/MatrixWidget.h"
#include "../midi/MidiChannel.h"
#include "../midi/MidiFile.h"
#include "../MidiEvent/MidiEvent.h"
#include "../protocol/Protocol.h"

EraserTool::EraserTool()  : EventTool(){
	setImage("eraser.png");
	setToolTipText("Eraser (deletes Events)");
}

EraserTool::EraserTool(EraserTool &other) : EventTool(other){
	return;
}

ProtocolEntry *EraserTool::copy(){
	return new EraserTool(*this);
}

void EraserTool::reloadState(ProtocolEntry *entry){
	EraserTool *other = dynamic_cast<EraserTool*>(entry);
	if(!other){
		return;
	}
	EventTool::reloadState(entry);
}

void EraserTool::draw(QPainter *painter){
	foreach(MidiEvent *ev, *(matrixWidget->activeEvents())){
		if(pointInRect(mouseX, mouseY, ev->x(), ev->y(), ev->x()+ev->width(),
				ev->y()+ev->height()))
		{
			painter->fillRect(ev->x(), ev->y(), ev->width(), ev->height(), Qt::black);
		}
	}
}

bool EraserTool::move(int mouseX, int mouseY){
	EventTool::move(mouseX, mouseY);
	return true;
}

bool EraserTool::release(){
	currentProtocol()->startNewAction("Remove Event(s)", image());
	foreach(MidiEvent *ev, *(matrixWidget->activeEvents())){
		if(pointInRect(mouseX, mouseY, ev->x(), ev->y(), ev->x()+ev->width(),
				ev->y()+ev->height()))
		{
			file()->channel(ev->channel())->removeEvent(ev);
		}
	}
	currentProtocol()->endAction();
	return true;
}

