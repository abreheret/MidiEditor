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

#include "TrackListWidget.h"

#include "../midi/MidiFile.h"
#include "../midi/MidiTrack.h"
#include "../protocol/Protocol.h"
#include "../midi/MidiChannel.h"

#include <QPainter>
#include <QLinearGradient>
#include <QFont>
#include <QLineEdit>
#include <QInputDialog>

#define LINE_HEIGHT 50
#define BORDER 5

TrackListWidget::TrackListWidget(QWidget *parent) : PaintWidget(parent) {
	setRepaintOnMouseMove(true);
	setRepaintOnMousePress(true);
	setRepaintOnMouseRelease(true);
	file = 0;
}

void TrackListWidget::setFile(MidiFile *f){
	file = f;
	connect(file->protocol(), SIGNAL(protocolChanged()), this, SLOT(update()));
	update();
}

void TrackListWidget::paintEvent(QPaintEvent *event){

	if(!file){
		return;
	}

	setMinimumHeight(LINE_HEIGHT*file->numTracks());

	QPainter *painter = new QPainter(this);
	QFont f = painter->font();
	f.setPixelSize(12);
	painter->setFont(f);
	painter->fillRect(0, 0, width(), height(), Qt::white);

	for(int i = 0; i<file->numTracks(); i++){

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

		QColor *color = file->track(i)->color();
		painter->fillRect(5, y+5, 16, 16, *color);

		painter->drawLine(5,y+5, 21, y+5);
		painter->drawLine(5,y+5, 5, y+21);
		painter->drawLine(21, y+21, 5, y+21);
		painter->drawLine(21, y+21, 21, y+5);

		QString text = "Track "+QString::number(i);
		painter->drawText(25, y+17, text);
		text = file->tracks()->at(i)->name();
		painter->drawText(25, y+30, text);

		text = "";

		painter->drawImage(6, y+35,
				QImage("graphics/trackwidget/remove.png"));
		if(enabled && mouseInRect(6, y+35, 12, 12)){
			text = "remove track";
			painter->fillRect(6, y+35, 12, 12, QColor(0,0,0, 100));
			if(mouseReleased && enabled){
				file->protocol()->startNewAction("remove track");
				emit trackRemoveClicked(i);
				file->protocol()->endAction();
			}
		}

		painter->drawImage(22, y+35,
				QImage("graphics/trackwidget/rename.png"));

		if(mouseInRect(22, y+35, 12, 12)){
			painter->fillRect(22, y+35, 12, 12, QColor(0,0,0, 100));
			text = "rename track";
			if(mouseReleased && enabled){
				emit trackRenameClicked(i);
			}
		}


		MidiTrack *track = file->track(i);

		if(track->hidden()){
			painter->drawImage(38, y+35,
					QImage("graphics/trackwidget/hidden.png"));
			painter->fillRect(38, y+35, 12, 12, QColor(0,0,0, 60));
		} else {
			painter->drawImage(38, y+35,
					QImage("graphics/trackwidget/visible.png"));
		}
		if(enabled && mouseInRect(38, y+35, 12, 12)){
			if(track->hidden()){
				text = "make track visible";
			} else {
				text = "hide track";
			}
			painter->fillRect(38, y+35, 12, 12, QColor(0,0,0, 100));
			if(mouseReleased && enabled){
				file->protocol()->startNewAction("toggle track visibility");
				track->setHidden(!track->hidden());
				file->protocol()->endAction();
			}
		}

		if(track->muted()){
			painter->drawImage(54, y+35,
					QImage("graphics/trackwidget/mute.png"));
			painter->fillRect(54, y+35, 12, 12, QColor(0,0,0, 60));
		} else {
			painter->drawImage(54, y+35,
					QImage("graphics/trackwidget/loud.png"));
		}
		if(enabled && mouseInRect(54, y+35, 12, 12)){
			if(track->muted()){
				text = "make track audible";
			} else {
				text = "mute track";
			}
			painter->fillRect(54, y+35, 12, 12, QColor(0,0,0, 100));
			if(mouseReleased && enabled){
				file->protocol()->startNewAction("toggle track audibility");
				track->setMuted(!track->muted());
				file->protocol()->endAction();
			}
		}

		painter->drawText(70, y+46, text);
	}

	painter->drawLine(0,0,width()-1,0);
	painter->drawLine(width()-1, 0, width()-1, height()-1);
	painter->drawLine(width()-1, height()-1, 0, height()-1);
	painter->drawLine(0, height()-1, 0, 0);

	if(!enabled){
		painter->fillRect(0,0,width(), height(), QColor(110, 110, 110, 100));
	}

	if(mouseReleased && enabled){
		mouseReleased = false;
		delete painter;
		update();
		return;
	}

	delete painter;
}
