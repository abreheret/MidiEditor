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

#include "ProtocolWidget.h"
#include "../midi/MidiFile.h"
#include "../midi/MidiChannel.h"
#include "../protocol/Protocol.h"
#include "../protocol/ProtocolStep.h"

#include <QPainter>
#include <QLinearGradient>

#define LINE_HEIGHT 20
#define BORDER 2

ProtocolWidget::ProtocolWidget(QWidget *parent) : PaintWidget(parent) {
	setRepaintOnMouseMove(true);
	setRepaintOnMousePress(true);
	setRepaintOnMouseRelease(true);

	file = 0;
}

void ProtocolWidget::setFile(MidiFile *f){
	file = f;
	connect(file->protocol(), SIGNAL(protocolChanged()), this, SLOT(protocolChanged()));
	update();

}

void ProtocolWidget::protocolChanged(){
	int num = file->protocol()->stepsBack()+file->protocol()->stepsForward();
	setMinimumHeight(LINE_HEIGHT*num);
	update();
}

void ProtocolWidget::paintEvent(QPaintEvent *event){
	if(!file){
		return;
	}

	QPainter *painter = new QPainter(this);
	QFont f = painter->font();
	f.setPixelSize(12);
	painter->setFont(f);
	painter->fillRect(0, 0, width(), height(), Qt::white);

	int num = file->protocol()->stepsBack()+file->protocol()->stepsForward();

	ProtocolStep *toGo = 0;
	for(int i = 0; i<num; i++){

		ProtocolStep *step;
		QColor color = QColor(0,0,0,0);
		if(i<file->protocol()->stepsBack()){
			step = file->protocol()->undoStep(i);
		} else {
			color = QColor(0, 0, 0, 60);
			step = file->protocol()->redoStep(file->protocol()->
					stepsForward()-i+file->protocol()->stepsBack()-1);
		}
		bool mouseIn = mouseInRect(0, LINE_HEIGHT*i, width(), LINE_HEIGHT);
		if(enabled && mouseIn){
			painter->fillRect(0, LINE_HEIGHT*i, width(), LINE_HEIGHT,
					QColor(234,246,255));
			toGo = step;
		} else {
			painter->fillRect(0, LINE_HEIGHT*i, width(), LINE_HEIGHT,
					QColor(194,230,255));
		}
		painter->drawLine(0,LINE_HEIGHT*i, width(), LINE_HEIGHT*i);
		painter->drawLine(0,LINE_HEIGHT*(i+1), width(), LINE_HEIGHT*(i+1));
		if(step->image()){
			painter->drawImage(QRectF(BORDER,LINE_HEIGHT*i+BORDER,
					LINE_HEIGHT-2*BORDER, LINE_HEIGHT-2*BORDER), *step->image(),
					QRectF(0,0,32,32));
		}
		painter->fillRect(BORDER,LINE_HEIGHT*i+BORDER, LINE_HEIGHT-2*BORDER, LINE_HEIGHT-2*BORDER, color);
		painter->drawLine(BORDER,LINE_HEIGHT*i+BORDER, LINE_HEIGHT-BORDER, LINE_HEIGHT*i+BORDER);
		painter->drawLine(BORDER,LINE_HEIGHT*(i+1)-BORDER, LINE_HEIGHT-BORDER, LINE_HEIGHT*(i+1)-BORDER);
		painter->drawLine(BORDER,LINE_HEIGHT*i+BORDER, BORDER, LINE_HEIGHT*(i+1)-BORDER);
		painter->drawLine(LINE_HEIGHT-BORDER,LINE_HEIGHT*i+BORDER, LINE_HEIGHT-BORDER, LINE_HEIGHT*(i+1)-BORDER);

		painter->drawText(LINE_HEIGHT+BORDER, LINE_HEIGHT*(i+1)-5, step->description());
	}

	if(mouseReleased && enabled){
		mouseReleased = false;
		delete painter;
		if(toGo){
			file->protocol()->goTo(toGo);
		} else {
			update();
		}
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
