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

#include "ChannelListWidget.h"

#include "../midi/MidiFile.h"
#include "../midi/MidiChannel.h"
#include "../protocol/Protocol.h"

#include <QPainter>
#include <QLinearGradient>
#include <QFont>

#define LINE_HEIGHT 50
#define BORDER 5

ChannelListWidget::ChannelListWidget(QWidget *parent) : PaintWidget(parent) {
	setMinimumHeight(LINE_HEIGHT*17);
	setRepaintOnMouseMove(true);
	setRepaintOnMousePress(true);
	setRepaintOnMouseRelease(true);
	file = 0;
}

void ChannelListWidget::setFile(MidiFile *f){
	file = f;
	connect(file->protocol(), SIGNAL(protocolChanged()), this, SLOT(update()));
	update();
}

void ChannelListWidget::paintEvent(QPaintEvent *event){

	if(!file){
		return;
	}

	QPainter *painter = new QPainter(this);
	QFont f = painter->font();
	f.setPixelSize(12);
	painter->setFont(f);
	painter->fillRect(0, 0, width(), height(), Qt::white);

	for(int i = 0; i<17; i++){

		bool mouseIn = mouseInRect(0, LINE_HEIGHT*i, width(), LINE_HEIGHT);
		if(mouseIn && enabled){
			painter->fillRect(0, LINE_HEIGHT*i, width(), LINE_HEIGHT,
					QColor(234,246,255));
		} else {
			painter->fillRect(0, LINE_HEIGHT*i, width(), LINE_HEIGHT,
					QColor(194,230,255));
		}
		painter->drawLine(0,LINE_HEIGHT*i, width(), LINE_HEIGHT*i);

		int y = LINE_HEIGHT*i;
		painter->fillRect(5, y+5, 16, 16, *(file->channel(i)->color()));

		painter->drawLine(5,y+5, 21, y+5);
		painter->drawLine(5,y+5, 5, y+21);
		painter->drawLine(21, y+21, 5, y+21);
		painter->drawLine(21, y+21, 21, y+5);

		QString text = "Channel "+QString::number(i);
		if(i==16){
			text = "General Events";
		}
		painter->drawText(25, y+17, text);
		text = MidiFile::instrumentName(file->channel(i)->
				progAtTick(file->cursorTick()));
		if(i==16){
			text = "Events affecting all channels";
		}
		painter->drawText(25, y+30, text);

		text = "";

		if(file->channel(i)->visible()){
			painter->drawImage(6, y+35,
					QImage("graphics/channelwidget/visible.png"));
		} else {
			painter->drawImage(6, y+35,
					QImage("graphics/channelwidget/hidden.png"));
			painter->fillRect(6, y+35, 12, 12, QColor(0,0,0, 60));
		}
		if(enabled && mouseInRect(6, y+35, 12, 12)){
			painter->fillRect(6, y+35, 12, 12, QColor(0,0,0, 100));
			if(file->channel(i)->visible()){
				text = "hide channel";
			} else {
				text = "set visible";
			}
			if(mouseReleased && enabled){
				file->protocol()->startNewAction("change visibility");
				file->channel(i)->setVisible(!file->channel(i)->visible());
				file->protocol()->endAction();
			}
		}

		if(!file->channel(i)->mute()){
			painter->drawImage(22, y+35,
					QImage("graphics/channelwidget/loud.png"));
		} else {
			painter->drawImage(22, y+35,
					QImage("graphics/channelwidget/mute.png"));
			painter->fillRect(22, y+35, 12, 12, QColor(0,0,0, 60));
		}
		if(mouseInRect(22, y+35, 12, 12)){
			painter->fillRect(22, y+35, 12, 12, QColor(0,0,0, 100));
			if(file->channel(i)->mute()){
				text = "make audible";
			} else {
				text = "mute channel";
			}
			if(mouseReleased && enabled){
				file->protocol()->startNewAction(text);
				file->channel(i)->setMute(!file->channel(i)->mute());
				file->protocol()->endAction();
			}
		}
		painter->drawImage(54, y+35, QImage("graphics/channelwidget/solo.png"));
		if(!file->channel(i)->solo()){
			painter->fillRect(54, y+35, 12, 12, QColor(0,0,0, 60));
		}
		if(enabled && mouseInRect(54, y+35, 12, 12)){
			painter->fillRect(54, y+35, 12, 12, QColor(0,0,0, 100));
			if(!file->channel(i)->solo()){
				text = "enter solomode";
			} else {
				text = "exit solomode";
			}
			if(mouseReleased && enabled){
				file->protocol()->startNewAction(text);
				for(int c = 0; c<17; c++){
					file->channel(c)->setSolo(c==i&&!file->channel(c)->solo());
				}
				file->protocol()->endAction();
			}
		}

		if(file->channel(i)->edit()){
			painter->drawImage(38, y+35,
					QImage("graphics/channelwidget/edit.png"));
		} else {
			painter->drawImage(38, y+35,
					QImage("graphics/channelwidget/noedit.png"));
			painter->fillRect(38, y+35, 12, 12, QColor(0,0,0, 60));
		}
		if(enabled && mouseInRect(38, y+35, 12, 12)){
			painter->fillRect(38, y+35, 12, 12, QColor(0,0,0, 100));
			if(!file->channel(i)->edit()){
				text = "enter editmode";
				if(mouseReleased){
					file->protocol()->startNewAction(text);
					for(int c = 0; c<17; c++){
						file->channel(c)->setEdit(c==i);
					}
					file->protocol()->endAction();
				}
			}
		}
		painter->drawText(68, y+46, text);
	}
	if(mouseReleased && enabled){
		emit(channelStateChanged());
		mouseReleased = false;
		delete painter;
		update();
		return;
	}

	painter->drawLine(0,0,width()-1,0);
	painter->drawLine(width()-1, 0, width()-1, height()-1);
	painter->drawLine(width()-1, height()-1, 0, height()-1);
	painter->drawLine(0, height()-1, 0, 0);

	if(!enabled){
		painter->fillRect(0,0,width(), height(), QColor(110, 110, 110, 100));
	}

	delete painter;
}
