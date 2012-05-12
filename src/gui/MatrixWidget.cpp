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

#include "MatrixWidget.h"
#include "../midi/MidiFile.h"
#include "../MidiEvent/MidiEvent.h"
#include "../MidiEvent/TempoChangeEvent.h"
#include "../MidiEvent/TimeSignatureEvent.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../MidiEvent/OffEvent.h"
#include "../midi/MidiChannel.h"
#include "../tool/Tool.h"
#include "../tool/EditorTool.h"
#include "../tool/EventTool.h"
#include "../protocol/Protocol.h"
#include "../midi/MidiPlayer.h"
#include "../midi/PlayerThread.h"
#include "../midi/MidiInput.h"

#define NUM_LINES 136
#define PIXEL_PER_S 100
#define PIXEL_PER_LINE 12
#define PIXEL_PER_EVENT 15

MatrixWidget::MatrixWidget(QWidget *parent) : PaintWidget(parent) {

	screen_locked = false;

	startTimeX = 0;
	startLineY = 0;
	endTimeX = 0;
	endLineY = 0;
	file = 0;
	scaleX = 1;
	pianoEvent = new NoteOnEvent(0,100,0);
	scaleY = 1;
	lineNameWidth = 80;
	timeHeight = 40;
	currentTempoEvents = new QList<MidiEvent*>;
	currentTimeSignatureEvents = new QList<TimeSignatureEvent*>;
	msOfFirstEventInList = 0;
	objects = new QList<MidiEvent*>;
	velocityObjects = new QList<MidiEvent*>;
	EditorTool::setMatrixWidget(this);
	setMouseTracking(true);
	setFocusPolicy(Qt::ClickFocus);

	setRepaintOnMouseMove(false);
	setRepaintOnMousePress(false);
	setRepaintOnMouseRelease(false);

	connect(MidiPlayer::playerThread(), SIGNAL(timeMsChanged(int)),
			this, SLOT(timeMsChanged(int)));

	pixmap = 0;
}

void MatrixWidget::setScreenLocked(bool b){
	screen_locked = b;
}

bool MatrixWidget::screenLocked() {
	return screen_locked;
}

void MatrixWidget::timeMsChanged(int ms){

	if(!file) return;

	int x = xPosOfMs(ms);

	if(!screen_locked && (ms<startTimeX || x>width()-100)) {

		// return if the last tick is already shown
		if(file->maxTime() <= endTimeX)
		{
			repaint();
			return;
		}

		// sets the new position and repaints
		emit scrollChanged(ms, file->maxTime(), startLineY,
				NUM_LINES-(endLineY-startLineY));
	} else {
		repaint();
	}
}

void MatrixWidget::scrollXChanged(int scrollPositionX){

	if(!file) return;

	startTimeX = scrollPositionX;
	endTimeX = startTimeX + ((width()-lineNameWidth)*1000)/(PIXEL_PER_S*scaleX);

	// more space than needed: scale x
	if(endTimeX-startTimeX>file->maxTime()){
		endTimeX = file->maxTime();
		startTimeX = 0;
	} else if(startTimeX<0){
		endTimeX-=startTimeX;
		startTimeX=0;
	} else if(endTimeX>file->maxTime()){
		startTimeX+=file->maxTime()-endTimeX;
		endTimeX=file->maxTime();
	}
	registerRelayout();
	repaint();
}

void MatrixWidget::scrollYChanged(int scrollPositionY){

	if(!file) return;

	startLineY = scrollPositionY;

	double space = height()-timeHeight;
	double lineSpace = scaleY * PIXEL_PER_LINE;
	double linesInWidget = space/lineSpace;
	endLineY = startLineY + linesInWidget;

	if(endLineY>NUM_LINES){
		int d = endLineY-NUM_LINES;
		endLineY=NUM_LINES;
		startLineY-=d;
		if(startLineY<0){
			startLineY = 0;
		}
	}
	registerRelayout();
	repaint();
}

void MatrixWidget::paintEvent(QPaintEvent *event){

	if(!file) return;

	QPainter *painter = new QPainter(this);
	QFont font = painter->font();
	font.setPixelSize(12);
	painter->setFont(font);
	painter->setClipping(false);

	bool totalRepaint = !pixmap;

	if(totalRepaint){
		pixmap = new QPixmap(width(), height());
		QPainter *pixpainter = new QPainter(pixmap);

		QFont f = pixpainter->font();
		f.setPixelSize(12);
		pixpainter->setFont(f);
		pixpainter->setClipping(false);

		for(int i = 0; i<objects->length(); i++){
			objects->at(i)->setShown(false);
			OnEvent *onev = dynamic_cast<OnEvent*>(objects->at(i));
			if(onev && onev->offEvent()){
				onev->offEvent()->setShown(false);
			}
		}
		objects->clear();
		velocityObjects->clear();
		currentTempoEvents->clear();
		currentTimeSignatureEvents->clear();

		startTick = file->tick(startTimeX, endTimeX, &currentTempoEvents,
				&endTick, &msOfFirstEventInList);
		TempoChangeEvent *ev = dynamic_cast<TempoChangeEvent*>(
				currentTempoEvents->at(0));
		if(!ev){
			pixpainter->fillRect(0,0,width(),height(),Qt::red);
			delete pixpainter;
			return;
		}
		int numLines = endLineY-startLineY;
		if(numLines == 0){
			delete pixpainter;
			return;
		}

		// fill background of the line descriptions
		pixpainter->fillRect(PianoArea, QApplication::palette().window());

		// fill the pianos background white
		int pianoKeys = numLines;
		if(endLineY>128){
			pianoKeys-=(endLineY-129);
		}
		if(pianoKeys>0){
			pixpainter->fillRect(0, timeHeight,lineNameWidth,
					pianoKeys*lineHeight(),	Qt::white);
		}

		// draw lines, pianokeys and linenames
		for(int i = startLineY; i<=endLineY; i++){
			int startLine = yPosOfLine(i);
			QColor c(194,230,255);
			if(i%2==0){
				c = QColor(234,246,255);
			}
			pixpainter->fillRect(lineNameWidth, startLine, width(),
					startLine+lineHeight(), c);
		}

		// paint measures
		pixpainter->fillRect(0, 0, width(), timeHeight,
				QApplication::palette().window());

		pixpainter->drawText(2, timeHeight-2, "measure:");
		int measure = file->measure(startTick,endTick, &currentTimeSignatureEvents);
		TimeSignatureEvent *currentEvent = currentTimeSignatureEvents->at(0);
		int i = 0;
		if(!currentEvent){
			return;
		}
		int tick = currentEvent->midiTime();
		while(tick+currentEvent->ticksPerMeasure()<startTick){
			tick += currentEvent->ticksPerMeasure();
		}
		while(tick<endTick){
			if(tick>startTick){
				int x = xPosOfMs(msOfTick(tick));
				pixpainter->drawLine(x, timeHeight/2, x, height());
				pixpainter->drawText(x+2, timeHeight-2, QString::number(measure));
			}
			measure++;
			tick += currentEvent->ticksPerMeasure();
			if(i<currentTimeSignatureEvents->length()-1){
				if(currentTimeSignatureEvents->at(i+1)->midiTime()<=tick){
					currentEvent = currentTimeSignatureEvents->at(i+1);
					tick = currentEvent->midiTime();
					i++;
				}
			}
		}

		// paint numbers (ms)
		pixpainter->drawText(2, timeHeight/2-2, "time(ms):");
		int numbers = (width()-lineNameWidth)/50;
		if(numbers>0){
			int step = (endTimeX-startTimeX)/numbers;
			int realstep = 1;
			int nextfak = 2;
			int tenfak = 1;
			while(realstep<=step){
				realstep=nextfak*tenfak;
				if(nextfak==1){
					nextfak++;
					continue;
				}
				if(nextfak==2){
					nextfak=5;
					continue;
				}
				if(nextfak==5){
					nextfak=1;
					tenfak*=10;
				}
			}
			int startNumber = ((startTimeX)/realstep);
			startNumber*=realstep;
			if(startNumber<startTimeX){
				startNumber+=realstep;
			}
			pixpainter->setClipping(true);
			pixpainter->setClipRect(lineNameWidth, 0, width()-lineNameWidth,
					timeHeight);
			while(startNumber<endTimeX){
				int pos = xPosOfMs(startNumber);
				QString text = QString::number(startNumber);
				int textlength = QFontMetrics(pixpainter->font()).width(text);
				pixpainter->drawText(pos-textlength/2, timeHeight/2-2, text);
				pixpainter->fillRect(pos-1, timeHeight/2-1, 2, 3, Qt::black);
				startNumber+=realstep;
			}
			pixpainter->setClipping(false);
		}

		// line between time texts and matrixarea
		pixpainter->drawLine(0, timeHeight, width(), timeHeight);

		// line between time text and measure text
		pixpainter->drawLine(0, timeHeight/2, width(), timeHeight/2);

		// paint the events
		pixpainter->setClipping(true);
		pixpainter->setClipRect(lineNameWidth, timeHeight, width()-lineNameWidth,
				height()-timeHeight);
		for(int i = 0; i<19; i++){
			paintChannel(pixpainter, i);
		}
		pixpainter->setClipping(false);

		pixpainter->setPen(Qt::black);

		// line between piano/matrixarea
		pixpainter->drawLine(lineNameWidth, 0, lineNameWidth, height());

		delete pixpainter;
	}

	painter->drawPixmap(0,0,*pixmap);

	// draw the piano / linenames
	for(int i = startLineY; i<=endLineY; i++){
		int startLine = yPosOfLine(i);
		if(i>=0 && i<=128){
			paintPianoKey(painter, 128-i,0,startLine,
					lineNameWidth,lineHeight());
		} else {
			QString text = "";
			switch(i){
				case MidiEvent::CONTROLLER_LINE: {
					text = "control";
					break;
				}
				case MidiEvent::TEMPO_CHANGE_EVENT_LINE: {
					text = "tempo ch.";
					break;
				}
				case MidiEvent::TIME_SIGNATURE_EVENT_LINE: {
					text = "time-sig.";
					break;
				}
				case MidiEvent::PROG_CHANGE_LINE: {
					text = "prog.ch.";
					break;
				}
				case MidiEvent::KEY_PRESSURE_LINE: {
					text = "key pressure";
					break;
				}
				case MidiEvent::CHANNEL_PRESSURE_LINE: {
					text = "ch. pressure";
					break;
				}
				case MidiEvent::UNKNOWN_LINE: {
					text = "unknown";
					break;
				}
			}
			font = painter->font();
			font.setPixelSize(9);
			painter->setFont(font);
			painter->drawText(QRectF(5, startLine, lineNameWidth-5,
					lineHeight()), text);
			font = painter->font();
			font.setPixelSize(12);
			painter->setFont(font);
		}
		if(i>128){
			painter->drawLine(0, startLine, width(), startLine);
		}
	}
	if(Tool::currentTool()){
		painter->setClipping(true);
		painter->setClipRect(ToolArea);
		Tool::currentTool()->draw(painter);
		painter->setClipping(false);
	}

	if(enabled && mouseInRect(TimeLineArea)){
		painter->setPen(Qt::red);
		painter->drawLine(mouseX, 0, mouseX, height());
		painter->setPen(Qt::black);
	}

	if(MidiPlayer::isPlaying()){
		painter->setPen(Qt::red);
		int x = xPosOfMs(MidiPlayer::timeMs());
		if(x>=lineNameWidth){
			painter->drawLine(x, 0, x, height());
		}
		painter->setPen(Qt::black);
	}

	// paint the cursorTick of file
	if(midiFile()->cursorTick()>=startTick &&
			midiFile()->cursorTick()<=endTick)
	{
		int x = xPosOfMs(msOfTick(midiFile()->cursorTick()));

		 QPointF points[3] = {
		     QPointF(x-8, timeHeight/2+2),
		     QPointF(x+8, timeHeight/2+2),
		     QPointF(x, timeHeight-2),
		 };

		 painter->setBrush(QBrush(Qt::blue, Qt::SolidPattern));

		painter->drawPolygon(points, 3);
		painter->setPen(Qt::gray);
		painter->drawLine(x, 0, x, height());

	}

	// border
	painter->drawLine(0,0,width(), 0);
	painter->drawLine(0,0,0,height());
	painter->drawLine(width()-1, height()-1, 0, height()-1);
	painter->drawLine(width()-1, height()-1, width()-1, 0);

	// if the recorder is recording, show red circle
	if(MidiInput::recording()){
		painter->setBrush(Qt::red);
		painter->drawEllipse(width()-20, timeHeight+5, 15, 15);
	}
	delete painter;

	// if MouseRelease was not used, delete it
	mouseReleased = false;

	if(totalRepaint){
		emit objectListChanged();
	}
}

void MatrixWidget::paintChannel(QPainter *painter, int channel){
	if(!file->channel(channel)->visible()){
		return;
	}
	QColor cC = *file->channel(channel)->color();

	// filter events
	QMultiMap<int, MidiEvent*> *map = file->channelEvents(channel);

	QMap<int, MidiEvent*>::iterator it = map->lowerBound(startTick);
	while(it!=map->end() && it.key()<=endTick){
		MidiEvent *event = it.value();
		if(eventInWidget(event)){
			// insert all Events in objects, set their coordinates
			// Only onEvents are inserted. When there is an On
			// and an OffEvent, the OnEvent will hold the coordinates
			int line = event->line();

			OffEvent *offEvent = dynamic_cast<OffEvent*>(event);
			OnEvent *onEvent = dynamic_cast<OnEvent*>(event);

			int x, width;
			int y = yPosOfLine(line);
			int height = lineHeight();

			if(onEvent || offEvent){
				if(onEvent){
					offEvent = onEvent->offEvent();
				} else if(offEvent){
					onEvent = dynamic_cast<OnEvent*>(offEvent->onEvent());
				}

				width = xPosOfMs(msOfTick(offEvent->midiTime()))-
						xPosOfMs(msOfTick(onEvent->midiTime()));
				x = xPosOfMs(msOfTick(onEvent->midiTime()));
				event = onEvent;
				if(objects->contains(event)){
					it++;
					continue;
				}
			} else {
				width = PIXEL_PER_EVENT;
				x = xPosOfMs(msOfTick(event->midiTime()));
			}

			event->setX(x);
			event->setY(y);
			event->setWidth(width);
			event->setHeight(height);

			event->draw(painter, cC);
			objects->append(event);
		}

		// append event to velocityObjects if its not a offEvent and if it
		// is in the x-Area
		OffEvent *offEvent = dynamic_cast<OffEvent*>(event);
		if(!offEvent && event->midiTime()>=startTick &&
				event->midiTime()<=endTick &&
				!velocityObjects->contains(event))
		{
			event->setX(xPosOfMs(msOfTick(event->midiTime())));
			velocityObjects->append(event);
		}
		it++;
	}
}

void MatrixWidget::paintPianoKey(QPainter *painter, int number, int x, int y,
		int width, int height)
{

	if(number>=0 && number<=128){

		double scaleHeightBlack = 0.5;
		double scaleWidthBlack = 0.7;

		bool isBlack = false;
		bool blackOnTop = false;
		bool blackBeneath = false;
		QString name = "";

		switch(number%12){
			case 0: {
				// C
				blackOnTop = true;
				name = "";
				int i = number/12;
				if(i<4){
					name="C";{
						for(int j = 0; j<3-i; j++){
							name+="'";
						}
					}
				} else {
					name = "c";
					for(int j = 0; j<i-4; j++){
						name+="'";
					}
				}
				break;
			}
			// Cis
			case 1: { isBlack = true; break; }
			// D
			case 2: { blackOnTop = true; blackBeneath = true; break; }
			// Dis
			case 3: { isBlack = true; break; }
			// E
			case 4: { blackBeneath = true; break; }
			// F
			case 5: { blackOnTop = true; break; }
			// fis
			case 6: { isBlack = true; break; }
			// G
			case 7: { blackOnTop = true; blackBeneath = true; break; }
			// gis
			case 8: { isBlack = true; break; }
			// A
			case 9: { blackOnTop = true; blackBeneath = true; break; }
			// ais
			case 10: { isBlack = true; break; }
			// H
			case 11: { blackBeneath = true; break; }
		}


		bool inRect = false;
		if(isBlack){
			painter->drawLine(x, y+height/2, x+width, y+height/2);
			y+=(height-height*scaleHeightBlack)/2;
			QRectF playerRect;
			playerRect.setX(x);
			playerRect.setY(y);
			playerRect.setWidth(width*scaleWidthBlack);
			playerRect.setHeight(height*scaleHeightBlack);
			QColor c = Qt::black;
			if(mouseInRect(playerRect)){
				c = QColor(200,200,200);
				inRect = true;
			}
			painter->fillRect(playerRect, c);
		} else {
			if(!blackBeneath){
				painter->drawLine(x, y+height, x+width, y+height);
			}
			inRect = mouseInRect(x,y,width, height);
			if(inRect){
				painter->fillRect(x, y, width, height, QColor(200,200,200));
			}
		}
		if(name!=""){
			int textlength = QFontMetrics(painter->font()).width(name);
			painter->drawText(x+width-textlength-2, y+height-1, name);
		}
		if(inRect && enabled){
			// mark the current Line
			QColor lineColor = QColor(0, 0, 100, 40);
			painter->fillRect(x+width, yPosOfLine(128-number),
					this->width()-x-width, height, lineColor);
		}
		if(inRect && mouseReleased){
			// play note
			pianoEvent->setNote(number);
			MidiPlayer::play(pianoEvent);
			mouseReleased = false;
		}
	}
}

void MatrixWidget::setFile(MidiFile *f){

	file = f;

	scaleX = 1;
	scaleY = 1;

	startTimeX = 0;
	startLineY = 0;


	connect(file->protocol(), SIGNAL(protocolChanged()), this,
			SLOT(registerRelayout()));
	connect(file->protocol(), SIGNAL(protocolChanged()), this, SLOT(update()));

	calcSizes();
}

void MatrixWidget::calcSizes(){
	if(!file){
		return;
	}
	int time = file->maxTime();
	int timeInWidget = ((width()-lineNameWidth)*1000)/(PIXEL_PER_S*scaleX);

	ToolArea = QRectF(lineNameWidth, timeHeight, width()-lineNameWidth,
			height()-timeHeight);
	PianoArea = QRectF(0, timeHeight, lineNameWidth, height()-timeHeight);
	TimeLineArea = QRectF(lineNameWidth, 0, width()-lineNameWidth, timeHeight);


	scrollXChanged(startTimeX);
	scrollYChanged(startLineY);

	emit sizeChanged(time-timeInWidget, NUM_LINES-endLineY+startLineY, startTimeX,
			startLineY);
}

MidiFile *MatrixWidget::midiFile(){
	return file;
}

void MatrixWidget::mouseMoveEvent(QMouseEvent *event){
	PaintWidget::mouseMoveEvent(event);

	if(!MidiPlayer::isPlaying() && Tool::currentTool()){
		Tool::currentTool()->move(event->x(), event->y());
	}

	if(!MidiPlayer::isPlaying()){
		repaint();
	}
}

void MatrixWidget::resizeEvent(QResizeEvent *event){
	Q_UNUSED(event);
	calcSizes();
}

int MatrixWidget::xPosOfMs(int ms){
	return lineNameWidth+(ms-startTimeX)*(width()-lineNameWidth)/
			(endTimeX-startTimeX);
}

int MatrixWidget::yPosOfLine(int line){
	return timeHeight + (line-startLineY) * lineHeight();
}

double MatrixWidget::lineHeight(){
	if(endLineY-startLineY==0) return 0;
	return (double)(height()-timeHeight)/(double)(endLineY-startLineY);
}

void MatrixWidget::enterEvent(QEvent *event){
	PaintWidget::enterEvent(event);
	if(Tool::currentTool()){
		Tool::currentTool()->enter();
		repaint();
	}
}
void MatrixWidget::leaveEvent(QEvent *event){
	PaintWidget::leaveEvent(event);
	if(Tool::currentTool()){
		Tool::currentTool()->exit();
		repaint();
	}
}
void MatrixWidget::mousePressEvent(QMouseEvent *event){
	PaintWidget::mousePressEvent(event);
	if(!MidiPlayer::isPlaying()&&Tool::currentTool() && mouseInRect(ToolArea)){
		if(Tool::currentTool()->press()){
			repaint();
		}
	}
}
void MatrixWidget::mouseReleaseEvent(QMouseEvent *event){
	PaintWidget::mouseReleaseEvent(event);
	if(!MidiPlayer::isPlaying()&&Tool::currentTool()&&mouseInRect(ToolArea)){
		if(Tool::currentTool()->release()){
			repaint();
		}
	} else if(Tool::currentTool()){
		if(Tool::currentTool()->releaseOnly()){
			repaint();
		}
	}
}

void MatrixWidget::keyPressEvent(QKeyEvent *event){
	// Undo, redo
	if(event->matches(QKeySequence::Undo)){
		file->protocol()->undo();
		repaint();
	} else if(event->matches(QKeySequence::Redo)){
		file->protocol()->redo();
		repaint();
	}   else if(event->matches(QKeySequence::Copy)){
		EventTool::copyAction();
	}  else if(event->matches(QKeySequence::Paste)){
		EventTool::pasteAction();
	}  else if(event->key() == Qt::Key_Shift){
		EventTool::shiftPressed = true;
	} else if(event->key() == Qt::Key_Control){
		EventTool::strPressed = true;
	}  else if(event->matches(QKeySequence::SelectAll)){
		if(file){
			// Select all
			foreach(MidiEvent *event,
					*(file->eventsBetween(0, file->endTick())))
			{
				EventTool::selectEvent(event, false, true);
			}
			repaint();
		}
	} else if(Tool::currentTool()){
		// an das Werkzeug weitergeben
		if(Tool::currentTool()->pressKey(event->key())){
			repaint();
		}
	}
}

void MatrixWidget::keyReleaseEvent(QKeyEvent *event){
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

QList<MidiEvent*> *MatrixWidget::activeEvents(){
	return objects;
}

QList<MidiEvent*> *MatrixWidget::velocityEvents(){
	return velocityObjects;
}


int MatrixWidget::msOfXPos(int x){
	return startTimeX + ((x-lineNameWidth)*(endTimeX-startTimeX))/(width()-
			lineNameWidth);
}

int MatrixWidget::msOfTick(int tick){
	return file->msOfTick(tick, currentTempoEvents, msOfFirstEventInList);
}

int MatrixWidget::timeMsOfWidth(int w){
	return (w*(endTimeX-startTimeX))/(width()-lineNameWidth);
}

bool MatrixWidget::eventInWidget(MidiEvent *event){
	NoteOnEvent *on = dynamic_cast<NoteOnEvent*>(event);
	OffEvent *off = dynamic_cast<OffEvent*>(event);
	if(on){
		off=on->offEvent();
	} else if(off){
		on = dynamic_cast<NoteOnEvent*>(off->onEvent());
	}
	if(on && off){
		int line = off->line();
		int tick = off->midiTime();
		bool offIn =  line>=startLineY && line<=endLineY && tick>=startTick &&
				tick<=endTick;
		line = on->line();
		tick = on->midiTime();
		bool onIn = line>=startLineY && line<=endLineY && tick>=startTick &&
				tick<=endTick;

		off->setShown(offIn);
		on->setShown(onIn);

		return offIn || onIn;

	} else {
		int line = event->line();
		int tick = event->midiTime();
		bool shown = line>=startLineY && line<=endLineY && tick>=startTick &&
				tick<=endTick;
		event->setShown(shown);

		return shown;
	}
}

int MatrixWidget::lineAtY(int y){
	return (y-timeHeight)/lineHeight()+startLineY;
}

void MatrixWidget::zoomHorIn(){
	scaleX+=0.1;
	calcSizes();
}

void MatrixWidget::zoomHorOut(){
	if(scaleX>=0.2){
		scaleX-=0.1;
		calcSizes();
	}
}

void MatrixWidget::zoomVerIn(){
	scaleY+=0.1;
	calcSizes();
}

void MatrixWidget::zoomVerOut(){
	if(scaleY>=0.2){
		scaleY-=0.1;
		if(height()<=NUM_LINES*lineHeight()*scaleY/(scaleY+0.1)){
			calcSizes();
		} else {
			scaleY+=0.1;
		}
	}
}

void MatrixWidget::mouseDoubleClickEvent(QMouseEvent *event){
	if(mouseInRect(TimeLineArea)){
		int tick = file->tick(msOfXPos(mouseX));
		file->setCursorTick(tick);

	}
}

void MatrixWidget::registerRelayout(){
	delete pixmap;
	pixmap = 0;
}

int MatrixWidget::minVisibleMidiTime(){
	return startTick;
}

int MatrixWidget::maxVisibleMidiTime(){
	return endTick;
}
