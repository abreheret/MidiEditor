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
#include "../MidiEvent/KeySignatureEvent.h"
#include "../MidiEvent/PitchBendEvent.h"
#include "../MidiEvent/TextEvent.h"
#include "../MidiEvent/UnknownEvent.h"
#include "../MidiEvent/SysExEvent.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "StandardTool.h"

#include <QStyleOption>
#include <QToolTip>

int NewNoteTool::_channel = 0;
int NewNoteTool::_track = 0;
bool NewNoteTool::enableVelocityDragging = false;

NewNoteTool::NewNoteTool() : EventTool() {
	inDrag = false;
	velocity = 100;
	line = 0;
	xPos = 0;
	_channel = 0;
	_track = 0;
	setImage(":/run_environment/graphics/tool/newnote.png");
	setToolTipText("Create new Events");
}

NewNoteTool::NewNoteTool(NewNoteTool &other) : EventTool(other){
	return;
}

Tool::ToolType NewNoteTool::type() const {
	return Tool::NewNote;
}

ProtocolEntry *NewNoteTool::copy(){
	return new NewNoteTool(*this);
}

void NewNoteTool::reloadState(ProtocolEntry *entry){
	NewNoteTool *other = qobject_cast<NewNoteTool*>(entry);
	if(!other){
		return;
	}
	EventTool::reloadState(entry);
}

void NewNoteTool::draw(QPainter *painter){
	qreal currentX = rasteredX(mouseX);
	if(inDrag){
		if(line<=127) {
			int y = matrixWidget->yPosOfLine(line);
			painter->fillRect(qRectF(xPos, y, currentX-xPos, matrixWidget->lineHeight()), Qt::black);
			painter->setPen(Qt::gray);
			painter->drawLine(qLineF(xPos, 0, xPos, matrixWidget->height()));
			painter->drawLine(qLineF(currentX, 0, currentX, matrixWidget->height()));
			if (enableVelocityDragging) {
				// Show a fake ToolTip that allows the velocity to be changed vertically.
				// Hooray for overcomplicated-ness.

				// Check the velocity
				int vel = (-0.5 * (mouseY - y - matrixWidget->lineHeight() / 2)) + 100;
				if (vel > 127)
					vel = 127;
				if (vel < 0)
					vel = 0;
				velocity = vel;

				// Get the dimensions and the font.
				QFont font = QFont(QToolTip::font());
				QString velocityTxt = "Velocity: " + QString::number(vel);
				QFontMetricsF *metrics = new QFontMetricsF(font);
				qreal velocityTxtWidth = metrics->width(velocityTxt);
				qreal velocityTxtHeight = metrics->height();
				qreal velocityTxtX = mouseX;
				qreal velocityTxtY = y - velocityTxtHeight - 8;

				// Reposition the fake ToolTip near the edge of the MatrixWidget.
				// I'm sure that there is a better way to do this.
				if (mouseX + velocityTxtWidth + 10 > matrixWidget->width())
					velocityTxtX = matrixWidget->width() - velocityTxtWidth - 10;
				else if (mouseX < 10)
					velocityTxtX = 10;
				if (velocityTxtY - matrixWidget->startLineY < 10)
					velocityTxtY += (matrixWidget->lineHeight() + velocityTxtHeight + 16);

				// Create the QRect for the ToolTip dimensions (too lazy to read from a QStyle)
				QRect tooltip = QRectF(velocityTxtX - 4, velocityTxtY, velocityTxtWidth + 8,
									   velocityTxtHeight + 8).toRect();

				// Get the right colors
				QPalette palette = matrixWidget->palette();
				QColor brush = palette.color(QPalette::Inactive, QPalette::ToolTipText);
				painter->setPen(brush);

				// Make the ToolTip background using drawPrimitive
				QStyle *style = matrixWidget->style();
				QStyleOption options;
				options.initFrom(matrixWidget);
				options.rect = tooltip;
				style->drawPrimitive(QStyle::PE_PanelTipLabel, &options, painter);

				// Finally, draw the text.
				painter->drawText(options.rect, Qt::AlignCenter, velocityTxt);
			}
			painter->setPen(Qt::black);
		} else {
			qreal y = matrixWidget->yPosOfLine(line);
			painter->fillRect(qRectF(currentX, y, 15, matrixWidget->lineHeight()), Qt::black);
			painter->setPen(Qt::gray);
			painter->drawLine(qLineF(currentX, 0, currentX, matrixWidget->height()));
			painter->drawLine(qLineF(currentX+15, 0, currentX+15, matrixWidget->height()));
			painter->setPen(Qt::black);
		}
	}
}

bool NewNoteTool::press(bool leftClick){
	Q_UNUSED(leftClick);
	inDrag = true;
	line = matrixWidget->lineAtY(mouseY);
	xPos = rasteredX(mouseX);
	return true;
}

bool NewNoteTool::release(){
	int startTick, endTick;
	int currentX = rasteredX(mouseX);
	inDrag = false;
	if(currentX<xPos || line>127){
		qreal temp = currentX;
		currentX = xPos;
		xPos = temp;
	}

	// get start/end tick if magnet
	rasteredX(currentX, &endTick);
	rasteredX(xPos, &startTick);

	MidiTrack *track = file()->track(_track);
	// TODO: Clean up this mess.
	if(currentX-xPos>2 || line>127){

		// note
		if(line>=0 && line<=127){
			currentProtocol()->startNewAction("Create note", image());

			if(startTick == -1){
				int startMs = matrixWidget->msOfXPos(xPos);
				startTick = file()->tick(startMs);
			}
			if(endTick == -1){
				int endMs = matrixWidget->msOfXPos(currentX);
				endTick = file()->tick(endMs);
			}

			NoteOnEvent *on = file()->channel(_channel)->insertNote(127-line,
					startTick, endTick, velocity, track);

			selectEvent(on, true, true);
			currentProtocol()->endAction();

			if(_standardTool){
				Tool::setCurrentTool(_standardTool);
				_standardTool->move(mouseX, mouseY);
				return _standardTool->release();
			}

			return true;
		} else {

			MidiTrack *generalTrack = file()->track(0);

			MidiEvent *event;
			// prog
			int startMs = matrixWidget->msOfXPos(xPos);
			int startTick = file()->tick(startMs);
			if(line == MidiEvent::PROG_CHANGE_LINE){

				currentProtocol()->startNewAction("Create Program Change Event",
						image());
				event = new ProgChangeEvent(_channel, 0, track);
				file()->insertEventInChannel(_channel, event, startTick);
				selectEvent(event, true, true);
				currentProtocol()->endAction();

			} else if(line == MidiEvent::TIME_SIGNATURE_EVENT_LINE){
				currentProtocol()->startNewAction("Create Time Signature Event",
						image());

				// 4/4
				event = new TimeSignatureEvent(18, 4, 2, 24, 8, generalTrack);
				file()->insertEventInChannel(18, event, startTick);
				selectEvent(event, true, true);
				currentProtocol()->endAction();
			} else if(line == MidiEvent::TEMPO_CHANGE_EVENT_LINE){
				currentProtocol()->startNewAction("Create Tempo Change Event",
						image());
				// quarter = 120
				event = new TempoChangeEvent(17, 500000, generalTrack);
				file()->insertEventInChannel(17, event, startTick);
				selectEvent(event, true, true);
				currentProtocol()->endAction();
			}  else if(line == MidiEvent::KEY_SIGNATURE_EVENT_LINE){
				currentProtocol()->startNewAction("Create Key Signature Event",
						image());
				event = new KeySignatureEvent(16, 0, false, generalTrack);
				file()->insertEventInChannel(16, event, startTick);
				selectEvent(event, true, true);
				currentProtocol()->endAction();
			} else if(line == MidiEvent::CONTROLLER_LINE){
				currentProtocol()->startNewAction("Create Control Change Event",
						image());
				event = new ControlChangeEvent(_channel, 0, 0, track);
				file()->insertEventInChannel(_channel, event, startTick);
				selectEvent(event, true, true);
				currentProtocol()->endAction();
			}  else if(line == MidiEvent::KEY_PRESSURE_LINE){
				currentProtocol()->startNewAction("Create Key Pressure Event",
						image());
				event = new KeyPressureEvent(_channel, 127, 100, track);
				file()->insertEventInChannel(_channel, event, startTick);
				selectEvent(event, true, true);
				currentProtocol()->endAction();
			}  else if(line == MidiEvent::CHANNEL_PRESSURE_LINE){
				currentProtocol()->startNewAction(
						"Create Channel Pressure Event", image());
				event = new ChannelPressureEvent(_channel, 100, track);
				file()->insertEventInChannel(_channel, event, startTick);
				selectEvent(event, true, true);
				currentProtocol()->endAction();
			} else if(line == MidiEvent::PITCH_BEND_LINE){
				currentProtocol()->startNewAction(
						"Create Pitch Bend Event", image());
				event = new PitchBendEvent(_channel, 8192, track);
				file()->insertEventInChannel(_channel, event, startTick);
				selectEvent(event, true, true);
				currentProtocol()->endAction();
			} else if(line == MidiEvent::TEXT_EVENT_LINE){
				currentProtocol()->startNewAction(
						"Create Text Event", image());
				event = new TextEvent(16, track);
				TextEvent *textEvent = (TextEvent*)event;
				textEvent->setText("New Text Event");
				textEvent->setTextType(TextEvent::TextTextEventType);
				file()->insertEventInChannel(16, event, startTick);
				selectEvent(event, true, true);
				currentProtocol()->endAction();
			} else if(line == MidiEvent::UNKNOWN_LINE){
				currentProtocol()->startNewAction(
						"Create Unknown Event", image());
				event = new UnknownEvent(16, 0x52, QByteArray(), track);
				file()->insertEventInChannel(16, event, startTick);
				selectEvent(event, true, true);
				currentProtocol()->endAction();
			} else if(line == MidiEvent::SYSEX_LINE){
				currentProtocol()->startNewAction(
						"Create SysEx Event", image());
				event = new SysExEvent(16, QByteArray(), track);
				file()->insertEventInChannel(16, event, startTick);
				selectEvent(event, true, true);
				currentProtocol()->endAction();
			}  else {
				if(_standardTool){
					Tool::setCurrentTool(_standardTool);
					_standardTool->move(mouseX, mouseY);
					_standardTool->release();
				}
				return true;
			}
		}
	}
	if(_standardTool){
		Tool::setCurrentTool(_standardTool);
		_standardTool->move(mouseX, mouseY);
		return _standardTool->release();
	}
	return true;
}

bool NewNoteTool::move(qreal mouseX, qreal mouseY){
	EventTool::move(mouseX, mouseY);
	return inDrag;
}

bool NewNoteTool::releaseOnly(){
	inDrag = false;
	xPos = 0;
	return true;
}

int NewNoteTool::editTrack(){
	return _track;
}

int NewNoteTool::editChannel(){
	return _channel;
}

void NewNoteTool::setEditTrack(int i){
	_track = i;
}

void NewNoteTool::setEditChannel(int i){
	_channel = i;
}
