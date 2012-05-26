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

#include "NewNoteTool.h"

#include "../gui/MatrixWidget.h"
#include "../midi/MidiChannel.h"
#include "../midi/MidiFile.h"
#include "../protocol/Protocol.h"
#include "../midi/MidiPlayer.h"
#include "../MidiEvent/MidiEvent.h"
#include "../MidiEvent/ProgChangeEvent.h"
#include "../MidiEvent/TimeSignatureEvent.h"
#include "../MidiEvent/TempoChangeEvent.h"
#include "../MidiEvent/ChannelPressureEvent.h"
#include "../MidiEvent/KeyPressureEvent.h"
#include "../MidiEvent/ControlChangeEvent.h"
#include "../MidiEvent/TextEvent.h"

NewNoteTool::NewNoteTool() : EventTool() {
	inDrag = false;
	line = 0;
	xPos = 0;
	setImage("newnote.png");
	setToolTipText("Create new Events (Adds the Events to the edited Channel)");
}

NewNoteTool::NewNoteTool(NewNoteTool &other) : EventTool(other){
	return;
}

ProtocolEntry *NewNoteTool::copy(){
	return new NewNoteTool(*this);
}

void NewNoteTool::reloadState(ProtocolEntry *entry){
	NewNoteTool *other = dynamic_cast<NewNoteTool*>(entry);
	if(!other){
		return;
	}
	EventTool::reloadState(entry);
}

void NewNoteTool::draw(QPainter *painter){
	if(inDrag){
		if(line<=128) {
			int y = matrixWidget->yPosOfLine(line);
			painter->fillRect(xPos, y, mouseX-xPos, matrixWidget->lineHeight(), Qt::black);
			painter->setPen(Qt::gray);
			painter->drawLine(xPos, 0, xPos, matrixWidget->height());
			painter->drawLine(mouseX, 0, mouseX, matrixWidget->height());
			painter->setPen(Qt::black);
		} else {
			int y = matrixWidget->yPosOfLine(line);
			painter->fillRect(mouseX, y, 15, matrixWidget->lineHeight(), Qt::black);
			painter->setPen(Qt::gray);
			painter->drawLine(mouseX, 0, mouseX, matrixWidget->height());
			painter->drawLine(mouseX+15, 0, mouseX+15, matrixWidget->height());
			painter->setPen(Qt::black);
		}
	}
}

bool NewNoteTool::press(){
	inDrag = true;
	line = matrixWidget->lineAtY(mouseY);
	xPos = mouseX;
	return true;
}

bool NewNoteTool::release(){
	inDrag = false;
	if(mouseX<xPos || line>128){
		int temp = mouseX;
		mouseX = xPos;
		xPos = temp;
	}
	if(mouseX-xPos>2 || line>128){

		int channel = file()->editedChannel();

		// note
		if(line>=0 && line<128){
			currentProtocol()->startNewAction("Create Note", image());


			int startMs = matrixWidget->msOfXPos(xPos);
			int startTick = file()->tick(startMs);

			int endMs = matrixWidget->msOfXPos(mouseX);
			int endTick = file()->tick(endMs);

			file()->channel(channel)->insertNote(128-line,
					startTick, endTick, 100);
			currentProtocol()->endAction();

			return true;
		} else {

			MidiEvent *event;
			// prog
			if(line == MidiEvent::PROG_CHANGE_LINE){

				currentProtocol()->startNewAction("Create Program Change Event",
						image());
				event = new ProgChangeEvent(channel, 0);
				int startMs = matrixWidget->msOfXPos(xPos);
				int startTick = file()->tick(startMs);
				file()->channel(channel)->insertEvent(event, startTick);
				currentProtocol()->endAction();

			} else if(line == MidiEvent::TIME_SIGNATURE_EVENT_LINE){
				currentProtocol()->startNewAction("Create Time Signature Event",
						image());

				// 4/4
				event = new TimeSignatureEvent(18, 4, 2, 24, 8);
				int startMs = matrixWidget->msOfXPos(xPos);
				int startTick = file()->tick(startMs);
				file()->channel(18)->insertEvent(event, startTick);
				currentProtocol()->endAction();
			} else if(line == MidiEvent::TEMPO_CHANGE_EVENT_LINE){
				currentProtocol()->startNewAction("Create Tempo Change Event",
						image());
				// quarter = 120
				event = new TempoChangeEvent(17, 500000);
				int startMs = matrixWidget->msOfXPos(xPos);
				int startTick = file()->tick(startMs);
				file()->channel(17)->insertEvent(event, startTick);
				currentProtocol()->endAction();
			}  else if(line == MidiEvent::CONTROLLER_LINE){
				currentProtocol()->startNewAction("Create Control Change Event",
						image());
				event = new ControlChangeEvent(channel, 0, 0);
				int startMs = matrixWidget->msOfXPos(xPos);
				int startTick = file()->tick(startMs);
				file()->channel(channel)->insertEvent(event, startTick);
				currentProtocol()->endAction();
			}  else if(line == MidiEvent::KEY_PRESSURE_LINE){
				currentProtocol()->startNewAction("Create Key Pressure Event",
						image());
				event = new KeyPressureEvent(channel, 127, 100);
				int startMs = matrixWidget->msOfXPos(xPos);
				int startTick = file()->tick(startMs);
				file()->channel(channel)->insertEvent(event, startTick);
				currentProtocol()->endAction();
			}  else if(line == MidiEvent::CHANNEL_PRESSURE_LINE){
				currentProtocol()->startNewAction(
						"Create Channel Pressure Event", image());
				event = new ChannelPressureEvent(channel, 100);
				int startMs = matrixWidget->msOfXPos(xPos);
				int startTick = file()->tick(startMs);
				file()->channel(channel)->insertEvent(event, startTick);
				currentProtocol()->endAction();
			} else if(line == MidiEvent::TEXT_EVENT_LINE){
				currentProtocol()->startNewAction(
						"Create Text Event", image());
				event = new TextEvent(16);
				TextEvent *textEvent = (TextEvent*)event;
				textEvent->setText("New Text Event");
				textEvent->setType(TextEvent::TEXT);
				int startMs = matrixWidget->msOfXPos(xPos);
				int startTick = file()->tick(startMs);
				file()->channel(channel)->insertEvent(event, startTick);
				currentProtocol()->endAction();
			} else {
				return true;
			}
		}
	}
	return true;
}

bool NewNoteTool::move(int mouseX, int mouseY){
	EventTool::move(mouseX, mouseY);
	return inDrag;
}

bool NewNoteTool::releaseOnly(){
	inDrag = false;
	xPos = 0;
	return true;
}
