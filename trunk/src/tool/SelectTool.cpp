#include "SelectTool.h"
#include "../MidiEvent/MidiEvent.h"
#include "../midi/MidiFile.h"
#include "../protocol/Protocol.h"
#include "../gui/MatrixWidget.h"
#include "StandardTool.h"
#include "../MidiEvent/NoteOnEvent.h"

SelectTool::SelectTool(int type) : EventTool() {
	stool_type = type;
	x_rect = 0;
	y_rect = 0;
	switch(stool_type){
		case SELECTION_TYPE_BOX: {
			setImage("select_box.png");
			setToolTipText("Select Events (Box)");
			break;
		}
		case SELECTION_TYPE_SINGLE: {
			setImage("select_single.png");
			setToolTipText("Select single Events");
			break;
		}
		case SELECTION_TYPE_LEFT: {
			setImage("select_left.png");
			setToolTipText("Select all Events on the left side");
			break;
		}
		case SELECTION_TYPE_RIGHT: {
			setImage("select_right.png");
			setToolTipText("Select all Events on the right side");
			break;
		}
	}
}

SelectTool::SelectTool(SelectTool &other) : EventTool(other){
	stool_type = other.stool_type;
	x_rect = 0;
	y_rect = 0;
}

void SelectTool::draw(QPainter *painter){
	paintSelectedEvents(painter);
	if(SELECTION_TYPE_BOX && (x_rect || y_rect)){
		painter->drawLine(x_rect, y_rect, mouseX, y_rect);
		painter->drawLine(x_rect, y_rect, x_rect, mouseY);
		painter->drawLine(mouseX, y_rect, mouseX, mouseY);
		painter->drawLine(x_rect, mouseY, mouseX, mouseY);
	} else if(stool_type == SELECTION_TYPE_RIGHT ||
			stool_type == SELECTION_TYPE_LEFT)
	{
		if(mouseIn){
			painter->setPen(Qt::red);
			painter->drawLine(mouseX, 0, mouseX, matrixWidget->height());
			int fac = 1;
			if(stool_type == SELECTION_TYPE_LEFT){
				fac = -1;
			}
			// draw arrow
			painter->setPen(Qt::black);
			int height = 3;
			int width = 10;
			int arrowHeight = 5; // up and down
			painter->fillRect(mouseX, mouseY-(height/2),fac*width,height,Qt::black);
			painter->drawLine(mouseX+fac*width, mouseY+arrowHeight, mouseX+fac*(width+5), mouseY);
			painter->drawLine(mouseX+fac*width, mouseY-arrowHeight, mouseX+fac*(width+5), mouseY);
			painter->drawLine(mouseX+fac*width, mouseY-arrowHeight, mouseX+fac*width, mouseY+arrowHeight);
		}
	}
}

bool SelectTool::press(){
	if(stool_type == SELECTION_TYPE_BOX){
		y_rect = mouseY;
		x_rect = mouseX;
	}
	return true;
}

bool SelectTool::release(){

	if(!file()){
		return false;

	}
	file()->protocol()->startNewAction("Selection changed", image());
	ProtocolEntry* toCopy = copy();

	if(!shiftPressed && !strPressed){
		clearSelection();
	}

	if(stool_type==SELECTION_TYPE_BOX || stool_type == SELECTION_TYPE_SINGLE){
		int x_start, y_start, x_end, y_end;
		if(stool_type == SELECTION_TYPE_BOX) {
			x_start = x_rect;
			y_start = y_rect;
			x_end = mouseX;
			y_end = mouseY;
			if(x_start>x_end){
				int tmp = x_start;
				x_start = x_end;
				x_end = tmp;
			}
			if(y_start>y_end){
				int tmp = y_start;
				y_start = y_end;
				y_end = tmp;
			}
		} else if(stool_type == SELECTION_TYPE_SINGLE) {
			x_start = mouseX;
			y_start = mouseY;
			x_end = mouseX+1;
			y_end = mouseY+1;
		}
		foreach(MidiEvent* event, *(matrixWidget->activeEvents())){
			if(inRect(event, x_start, y_start, x_end, y_end)){
				selectEvent(event, false);
			}
		}
	} else if(stool_type == SELECTION_TYPE_RIGHT ||
			stool_type == SELECTION_TYPE_LEFT)
	{
		int tick = file()->tick(matrixWidget->msOfXPos(mouseX));
		int start, end;
		if(stool_type == SELECTION_TYPE_LEFT){
			start = 0;
			end = tick;
		} else if(stool_type == SELECTION_TYPE_RIGHT){
			end = file()->endTick();
			start = tick;
		}
		foreach(MidiEvent *event, *(file()->eventsBetween(start, end))){
			selectEvent(event, false);
		}
	}

	x_rect = 0;
	y_rect = 0;

	protocol(toCopy, this);
	file()->protocol()->endAction();
	if(_standardTool){
		Tool::setCurrentTool(_standardTool);
		_standardTool->move(mouseX, mouseY);
		_standardTool->release();
	}
	return true;
}

bool SelectTool::inRect(MidiEvent *event, int x_start, int y_start, int x_end, int y_end){
	return 	pointInRect(event->x(), event->y(), x_start, y_start, x_end, y_end) ||
			pointInRect(event->x(), event->y()+event->height(), x_start, y_start, x_end, y_end) ||
			pointInRect(event->x()+event->width(), event->y(), x_start, y_start, x_end, y_end) ||
			pointInRect(event->x()+event->width(), event->y()+event->height(), x_start, y_start, x_end, y_end) ||
			pointInRect(x_start, y_start, event->x(), event->y(), event->x()+event->width(), event->y()+event->height());
}

bool SelectTool::move(int mouseX, int mouseY){
	EditorTool::move(mouseX, mouseY);
	return true;
}

ProtocolEntry *SelectTool::copy(){
	return new SelectTool(*this);
}

void SelectTool::reloadState(ProtocolEntry *entry){
	SelectTool *other = dynamic_cast<SelectTool*>(entry);
	if(!other){
		return;
	}
	EventTool::reloadState(entry);
	x_rect = 0;
	y_rect = 0;
	stool_type = other->stool_type;
}

bool SelectTool::releaseOnly(){
	return release();
}
