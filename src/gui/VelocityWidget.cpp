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

#include "VelocityWidget.h"
#include "MatrixWidget.h"
#include "../midi/MidiFile.h"
#include "../MidiEvent/MidiEvent.h"
#include "../midi/MidiChannel.h"
#include "../midi/MidiTrack.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../tool/EventTool.h"
#include "../protocol/Protocol.h"

#define WIDTH 7
#define BUTTON_SIZE 20
#define LEFT_BORDER 80

VelocityWidget::VelocityWidget(MatrixWidget *mw, QWidget *parent) :
	PaintWidget(parent)
{
	setRepaintOnMouseMove(true);
	setRepaintOnMousePress(true);
	setRepaintOnMouseRelease(true);
	matrixWidget = mw;
	mode = SINGLE_MODE;
	dragY = 0;
	startY = -1;
	startX = -1;
	protocolActionStarted = false;
	connect(matrixWidget, SIGNAL(objectListChanged()), this, SLOT(update()));
}

void VelocityWidget::paintEvent(QPaintEvent *event){

	if(!matrixWidget->midiFile()) return;

	setCursor(Qt::ArrowCursor);
	QPainter *painter = new QPainter(this);
	QFont f = painter->font();
	f.setPixelSize(9);
	painter->setFont(f);
	QColor c(194,230,255);

	// SINGLE_MODE Button
	QRectF singleModeRect = QRectF(0, 0, BUTTON_SIZE, BUTTON_SIZE);

	if(mouseInRect(singleModeRect) || mode == SINGLE_MODE){
		painter->fillRect(singleModeRect, QColor(200,200,200));
		if(mousePressed && mouseInRect(singleModeRect)){
			painter->fillRect(singleModeRect, QColor(150,150,150));
		}
		if(mode!=SINGLE_MODE){
			painter->drawText(5, 15+BUTTON_SIZE, "Single Mode");
			mousePressed = false;
		}
		if(mouseReleased && mouseInRect(singleModeRect)){
			mouseReleased = false;
			mode = SINGLE_MODE;
			startX = -1;
			startY = -1;
			update();
			return;
		}
	}
	painter->drawImage(singleModeRect, QImage("graphics/tool/vel_select.png"));
	painter->drawLine(0, 0, 0, BUTTON_SIZE);
	painter->drawLine(0,0,BUTTON_SIZE, 0);
	painter->drawLine(0,BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE);
	painter->drawLine(BUTTON_SIZE, 0, BUTTON_SIZE, BUTTON_SIZE);

	// LINE_MODE Button
	QRectF lineModeRect = QRectF(BUTTON_SIZE+5, 0, BUTTON_SIZE, BUTTON_SIZE);

	if(mouseInRect(lineModeRect) || mode == LINE_MODE){
		painter->fillRect(lineModeRect, QColor(200,200,200));
		if(mode!=LINE_MODE){
			painter->drawText(5, 15+BUTTON_SIZE, "Line Mode");
		}
		if(mousePressed && mouseInRect(lineModeRect)){
			painter->fillRect(lineModeRect, QColor(150,150,150));
			mousePressed = false;
		}
		if(mouseReleased && mouseInRect(lineModeRect)){
			mouseReleased = false;
			mode = LINE_MODE;
			startX = -1;
			startY = -1;
			update();
			return;
		}
	}
	painter->drawImage(lineModeRect, QImage("graphics/tool/vel_line.png"));
	painter->drawLine(lineModeRect.x(), lineModeRect.y(),
			lineModeRect.x()+lineModeRect.width(), lineModeRect.y());
	painter->drawLine(lineModeRect.x(), lineModeRect.y(),
			lineModeRect.x(), lineModeRect.y()+lineModeRect.height());
	painter->drawLine(lineModeRect.x()+lineModeRect.width(), lineModeRect.y()+
			lineModeRect.height(), lineModeRect.x(),
			lineModeRect.y()+lineModeRect.height());
	painter->drawLine(lineModeRect.x()+lineModeRect.width(), lineModeRect.y()+
			lineModeRect.height(), lineModeRect.x()+lineModeRect.width(),
			lineModeRect.y());

	// MOUSE_MODE Button
	QRectF mouseModeRect = QRectF(2*BUTTON_SIZE+10, 0, BUTTON_SIZE, BUTTON_SIZE);

	if(mouseInRect(mouseModeRect) || mode == MOUSE_MODE){
		painter->fillRect(mouseModeRect, QColor(200,200,200));
		if(mousePressed && mouseInRect(mouseModeRect)){
			painter->fillRect(mouseModeRect, QColor(150,150,150));
		}
		if(mode!=MOUSE_MODE){
			painter->drawText(5, 15+BUTTON_SIZE, "Mouse Mode");
			mousePressed = false;
		}
		if(mouseReleased && mouseInRect(mouseModeRect)){
			mouseReleased = false;
			mode = MOUSE_MODE;
			startX = -1;
			startY = -1;
			update();
			return;
		}
	}
	painter->drawImage(mouseModeRect, QImage("graphics/tool/vel_mouse.png"));
	painter->drawLine(mouseModeRect.x(), mouseModeRect.y(),
			mouseModeRect.x()+mouseModeRect.width(), mouseModeRect.y());
	painter->drawLine(mouseModeRect.x(), mouseModeRect.y(),
			mouseModeRect.x(), mouseModeRect.y()+mouseModeRect.height());
	painter->drawLine(mouseModeRect.x()+mouseModeRect.width(), mouseModeRect.y()
			+mouseModeRect.height(), mouseModeRect.x(),
			mouseModeRect.y()+mouseModeRect.height());
	painter->drawLine(mouseModeRect.x()+mouseModeRect.width(), mouseModeRect.y()
			+mouseModeRect.height(), mouseModeRect.x()+mouseModeRect.width(),
			mouseModeRect.y());

	// Edit Events
	bool inEvent = false;

	if(mode==SINGLE_MODE){

		if(inDrag){
			dragY += mouseY-mouseLastY;
		} else {
			if(mouseReleased){
				int vToAdd = (128 * (-dragY) / height());
				QList<MidiEvent*> *selectedEvents =
						EventTool::selectedEventList();
				if(selectedEvents->size()>0){
					matrixWidget->midiFile()->protocol()->
							startNewAction("change velocity");
					foreach(MidiEvent *ev, *selectedEvents){
						NoteOnEvent *noteOn = dynamic_cast<NoteOnEvent*>(ev);
						if(noteOn){
							noteOn->setVelocity(noteOn->velocity()+vToAdd);
						}
					}
					matrixWidget->midiFile()->protocol()->endAction();
				}
				mouseReleased = false;
			}
			dragY = 0;
		}
	}

	// Area of velocity
	painter->fillRect(LEFT_BORDER,0,width()-LEFT_BORDER,height(), c);
	painter->drawLine(LEFT_BORDER, 0, LEFT_BORDER, height());
	painter->drawLine(width()-1, 0, width()-1, height());
	painter->drawLine(LEFT_BORDER, height()-1, width()-1, height()-1);

	QList<MidiEvent*> *list = matrixWidget->velocityEvents();

	for(int i = 0; i<8; i++){
		painter->drawLine(LEFT_BORDER, (i*height())/8, width(), (i*height())/8);
	}
	foreach(MidiEvent* event, *list){

		if(!event->file()->channel(event->channel())->visible()){
			continue;
		}

		if(event->file()->track(event->track())->hidden()){
			continue;
		}

		QColor *c=matrixWidget->midiFile()->channel(event->channel())->color();
		if(!matrixWidget->colorsByChannel()){
			c = matrixWidget->midiFile()->track(event->track())->color();
		}
		int velocity = 0;
		NoteOnEvent *noteOn = dynamic_cast<NoteOnEvent*>(event);
		if(noteOn){
			velocity=noteOn->velocity();
		}
		if(velocity>0){
			int h = (height()*velocity)/128;
			painter->fillRect(event->x(), height()-h, WIDTH, h, *c);
			painter->drawLine(event->x(), height()-h, event->x()+
					WIDTH,height()-h);
			painter->drawLine(event->x(), height()-h, event->x(),height());
			painter->drawLine(event->x()+WIDTH, height()-h, event->x()+WIDTH,
					height());
		}
	}
	bool pressureReleased = false;
	bool needsUpdate = false;
	// paint selected events above all others
	foreach(MidiEvent* event, *(EventTool::selectedEventList())){

		int velocity = 0;
		NoteOnEvent *noteOn = dynamic_cast<NoteOnEvent*>(event);

		if(noteOn && noteOn->midiTime()>=matrixWidget->minVisibleMidiTime() && noteOn->midiTime()<=matrixWidget->maxVisibleMidiTime()){
			velocity=noteOn->velocity();
		}
		if(velocity>0){
			int h = (height()*velocity)/128;
			if(mode==SINGLE_MODE){
				h-=dragY;
			}
			painter->fillRect(event->x(), height()-h, WIDTH, h, Qt::black);

			if(mouseInRect(event->x(), height()-h-3, WIDTH, 6) &&
					mode==SINGLE_MODE)
			{
				setCursor(Qt::SizeVerCursor);
				if(!inDrag && mousePressed){
					EventTool::selectEvent(event, false);
					matrixWidget->update();
					pressureReleased = true;
				}
				inEvent=true;
			}
		}
	}
	// quick and dirty: selection, perhaps fill painted event in pq with selected events in the front?
	// also so store heights and positions...
	if(!pressureReleased){
		foreach(MidiEvent* event, *list){
			int velocity = 0;
			NoteOnEvent *noteOn = dynamic_cast<NoteOnEvent*>(event);
			if(noteOn){
				velocity=noteOn->velocity();
			}
			if(velocity>0){
				int h = (height()*velocity)/128;
				bool selected =  EventTool::selectedEventList()->contains(event);
				if(!selected && mouseInRect(event->x(), height()-h-3, WIDTH, 6) &&
						mode==SINGLE_MODE)
				{
					setCursor(Qt::SizeVerCursor);
					if(!inDrag && mousePressed){
						EventTool::selectEvent(event, true);
						matrixWidget->update();
						needsUpdate = true;
					}
					inEvent=true;
				}
			}
		}
	}

	int dX = draggedX();
	int dY = draggedY();
	if(mode==LINE_MODE || mode==MOUSE_MODE){
		if((mode==LINE_MODE && startX>=0 && startY >=0) ||
				(mode==MOUSE_MODE && inDrag && (dY!=0|| dX!=0)))
		{
			// draw Line from clicked point to mousePosition (only LINE_MODE)
			if(mode==LINE_MODE){
				painter->drawLine(startX, startY, mouseX, mouseY);
			}

			if((mouseReleased&&mode==LINE_MODE) || (inDrag&&mode==MOUSE_MODE)){
				int startPosX = startX;
				int startPosY = startY;
				if(mode==MOUSE_MODE){
					startPosX = mouseX-dX;
					startPosY = mouseY-dY;
				}
				// edit the events velocity
				int endX = mouseX;
				int endY = mouseY;
				if(endX<startPosX){
					int tmp = endX;
					endX = startPosX;
					startPosX = tmp;
					tmp=endY;
					endY = startPosY;
					startPosY = tmp;
				}

				// start new protocolentry in LINE_MODE
				if(mode==LINE_MODE){
					matrixWidget->midiFile()->protocol()->
							startNewAction("Edit velocity");
				}

				// Edit all Events between startPosX and endX
				foreach(MidiEvent* event, *list){
					if(EventTool::selectedEventList()->contains(event)){
						NoteOnEvent *noteOn = dynamic_cast<NoteOnEvent*>(event);
						if(noteOn){
							int x = noteOn->x();
							if(x<=endX && x>=startPosX){
								int y = (x-startPosX)*(endY-startPosY)/
										(endX-startPosX)+startPosY;
								int velocity = 128*(height()-y)/height();
								noteOn->setVelocity(velocity);
							}
						}
					}
				}

				if(mode==LINE_MODE){
					matrixWidget->midiFile()->protocol()->endAction();
					mouseReleased = false;
					startX = mouseX;
					startY = mouseY;
				}

				update();
				return;
			}
		} else if(mouseReleased && mode == LINE_MODE){
			mouseReleased = false;
			startX = mouseX;
			startY = mouseY;
			update();
			return;
		}
	}

	if(!inEvent && mousePressed && !inDrag && mode==SINGLE_MODE){
		EventTool::selectedEventList()->clear();
		matrixWidget->update();
		needsUpdate = true;
	}

	if(needsUpdate){
		update();
	}
}

void VelocityWidget::keyPressEvent(QKeyEvent *event){
	// Undo, redo abfragen
	if(event->matches(QKeySequence::Undo)){
		matrixWidget->midiFile()->protocol()->undo();
		repaint();
	} else if(event->matches(QKeySequence::Redo)){
		matrixWidget->midiFile()->protocol()->redo();
		repaint();
	}  else if(event->key() == Qt::Key_Shift){
		EventTool::shiftPressed = true;
	} else if(event->key() == Qt::Key_Control){
		EventTool::strPressed = true;
	} else if(Tool::currentTool()){
		// an das Werkzeug weitergeben
		if(Tool::currentTool()->pressKey(event->key())){
			repaint();
		}
	}
}

void VelocityWidget::keyReleaseEvent(QKeyEvent *event){
	if(event->key() == Qt::Key_Shift){
		EventTool::shiftPressed = false;
	} else if(event->key() == Qt::Key_Control){
		EventTool::strPressed = false;
	} else if(Tool::currentTool()){
		if(Tool::currentTool()->releaseKey(event->key())){
			repaint();
		}
	}
}

void VelocityWidget::mouseReleaseEvent(QMouseEvent *event){

	if(protocolActionStarted){
		matrixWidget->midiFile()->protocol()->endAction();
		protocolActionStarted = false;
		inDrag = false;
	}
	if(event->button()==Qt::LeftButton){
		PaintWidget::mouseReleaseEvent(event);
		return;
	}
	startX = -1;
	startY = -1;

}

void VelocityWidget::mousePressEvent(QMouseEvent *event){
	if(mode==MOUSE_MODE){
		matrixWidget->midiFile()->protocol()->startNewAction("Edit Velocity");
		protocolActionStarted = true;
		PaintWidget::mousePressEvent(event);
		return;
	}
	if(event->button()==Qt::LeftButton){
		PaintWidget::mousePressEvent(event);
		return;
	}
}

void VelocityWidget::leaveEvent(QEvent *event){
	if(protocolActionStarted){
		matrixWidget->midiFile()->protocol()->endAction();
		protocolActionStarted=false;
		inDrag = false;
	}
	PaintWidget::leaveEvent(event);
}
