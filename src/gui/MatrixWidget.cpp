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
#include <QSizePolicy>
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
#include "../midi/MidiTrack.h"
#include "../tool/Selection.h"

#include <QList>
#include <QPixmapCache>
#include <QtCore/qmath.h>
#include <QTime>

#define NUM_LINES 139
#define PIXEL_PER_S 100
#define PIXEL_PER_LINE 11
#define PIXEL_PER_EVENT 15
#define MAX_HORIZ_ZOOM 10
#define MAX_VERT_ZOOM 3

/*
 * TODO:
 * - Remove old variables that we don't need
 * - Make painting even more efficient:
 *     > Further decrease CPU usage.
 *     > Hardware acceleration?
 *     > Decrease update frequency
 *     > Only repaint areas that are updated.
 * - Remove unnecessary checks
 * - Make sure everything that should be
 *   changed...changes, and everything that
 *   shouldn't...shouldn't.
 *     > Divs don't update properly
 *     > MiscWidget doesn't update properly.
 * - PianoWidget interaction
 */
bool MatrixWidget::antiAliasingEnabled = true;
MatrixWidget::MatrixWidget(QWidget *parent) : PaintWidget(parent) {
	pianoKeys = QMap<int, QRectF>();
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	screen_locked = false;
	startTimeX = 0;
	setFixedHeight(NUM_LINES * PIXEL_PER_LINE);
	startLineY = NUM_LINES;
	endTimeX = 0;
	endLineY = 0;
	scrollDir = NONE;
	// Double the cache space as we use it a lot.
	QPixmapCache::setCacheLimit(20480);
	file = 0;
	scaleX = 1;
	pianoEvent = new NoteOnEvent(0, 100, 0, 0);
	scaleY = 1;
	timeHeight = 0;
	currentTempoEvents = new QList<MidiEvent *>;
	currentTimeSignatureEvents = new QList<TimeSignatureEvent *>;
	msOfFirstEventInList = 0;
	objects = new QList<MidiEvent *>;
	velocityObjects = new QList<MidiEvent *>;
	EditorTool::setMatrixWidget(this);
	setMouseTracking(true);
	setFocusPolicy(Qt::ClickFocus);

	setRepaintOnMouseMove(false);
	setRepaintOnMousePress(false);
	setRepaintOnMouseRelease(false);

	connect(MidiPlayer::player(), SIGNAL(timeMsChanged(int)),
			this, SLOT(timeMsChanged(int)));
	_div = 2;
}

void MatrixWidget::setScreenLocked(bool b) {
	screen_locked = b;
}

bool MatrixWidget::screenLocked() {
	return screen_locked;
}

void MatrixWidget::timeMsChanged(int ms, bool ignoreLocked) {
	if (!file) {
		return;
	}
	int x = xPosOfMs(ms);
	int startTimeXTemp = msOfXPos(relativeRect().left());
	int endTimeXTemp = msOfXPos(relativeRect().right());

	if ((!screen_locked || ignoreLocked) && (x < 0 || ms < startTimeXTemp
			|| ms > endTimeXTemp || x > width() - 100)) {

		// return if the last tick is already shown
		if (file->maxTime() <= endTimeXTemp && ms >= startTimeXTemp) {
			update();
			return;
		}

		// sets the new position and repaints
		emit scrollChanged(relativeRect().right(), relativeRect().top());
	} else {
		update();
	}
}

/*
 * FIXME: Divs don't update after undo/redo
 */
void MatrixWidget::paintEvent(QPaintEvent *event) {
	// Q_UNUSED(event)
	if (!file) {
		return;
	}
	if (paintingActive()) {
		qWarning("MatrixWidget::paintEvent: Painting active!");
		return;
	}
	QPainter painter(this);
	QFont font = painter.font();
	font.setPixelSize(12);
	painter.setFont(font);

	/*
	 * Check if we are calling a flat-out repaint
	 * or just updating part of the widget.
	 *
	 * TODO: Make further use of this, especially
	 * when scrolling.
	 */
	int updatemode = 0;
	if (!event->region().isNull() && !event->region().isEmpty()) {
		updatemode = 1;
		painter.setClipping(true);
		painter.setClipRegion(event->region());
	} else if (!event->rect().isNull()) {
		updatemode = 2;
		painter.setClipping(true);
		painter.setClipRect(event->rect());
	} else {
		painter.setClipping(true);
		painter.setClipRect(relativeRect());
	}
	// This complex QString serves as the ID of the events pixmap.
	// It stores the zoom level, measure division, and the UUID of the current ProtocolStep.
	QString pixmapId = "ProtocolStep_"
			+ QString::number(scaleX, 'f', 2) + "_"
			+ QString::number(scaleY, 'f', 2) + "_"
			+ QString::number(div()) + "_"
			+ file->protocol()->currentStepId();
	QPixmap notesPixmap;
	bool totalRepaint = !QPixmapCache::find(pixmapId, notesPixmap);

	if (totalRepaint) {
		QPixmap linesTexture;
		if (!QPixmapCache::find("MatrixWidget_" + QString::number(scaleY, 'f', 2), linesTexture)) {
				linesTexture = QPixmap(1, height());
				//linesTexture.fill(Qt::transparent);

				QPainter linesPainter(&linesTexture);
				if (antiAliasingEnabled) {
					linesPainter.setRenderHint(QPainter::Antialiasing);
				}
				for (int i = qFloor(startLineY); i <= qFloor(endLineY); i++) {
					int startLine = yPosOfLine(i);
					QColor c(194, 230, 255);
					if (i % 2 == 0) {
						c = QColor(234, 246, 255);
					}
					if (i > 127) {
						c = QColor(194, 194, 194);
						if (i % 2 == 1) {
							c = QColor(234, 246, 255);
						}
					}
					linesPainter.fillRect(qRectF(0, startLine, 1,
												  startLine + lineHeight()), c);

				}
				linesPainter.end();
				QPixmapCache::insert("MatrixWidget_" + QString::number(scaleY, 'f', 2), linesTexture);
			}
		this->pianoKeys.clear();
		notesPixmap = QPixmap(width(), height());

		if (notesPixmap.paintingActive()) {
			return;
		}
		notesPixmap.fill(Qt::transparent);
		QPainter pixpainter(&notesPixmap);
		pixpainter.setBrush(Qt::transparent);
		if (antiAliasingEnabled) {
			pixpainter.setRenderHint(QPainter::Antialiasing);
		}
		// fill background
		pixpainter.fillRect(qRectF(0, 0, width(), height()), QBrush(linesTexture));

		QFont f = pixpainter.font();
		f.setPixelSize(12);
		pixpainter.setFont(f);
		pixpainter.setClipping(false);

		for (int i = 0; i < objects->length(); i++) {
			objects->at(i)->setShown(false);
			OnEvent *onev = qobject_cast<OnEvent *>(objects->at(i));
			if (onev && onev->offEvent()) {
				onev->offEvent()->setShown(false);
			}
		}
		objects->clear();
		velocityObjects->clear();
		currentTempoEvents->clear();
		currentTimeSignatureEvents->clear();
		currentDivs.clear();

		startTick = file->tick(startTimeX, endTimeX, &currentTempoEvents,
							   &endTick, &msOfFirstEventInList);

		TempoChangeEvent *ev = qobject_cast<TempoChangeEvent *>(currentTempoEvents->at(0));
		if (!ev) {
			return;
		}

		// paint measures and timeline
		currentTimeSignatureEvents = new QList<TimeSignatureEvent *>;
		int measure = file->measure(0, file->endTick(), &currentTimeSignatureEvents);

		TimeSignatureEvent *currentEvent = currentTimeSignatureEvents->at(0);
		int i = 0;
		if (!currentEvent) {
			return;
		}
		int tick = currentEvent->midiTime();
		while (tick + currentEvent->ticksPerMeasure() <= 0) {
			tick += currentEvent->ticksPerMeasure();
		}
		qreal xfrom,xDiv;
		int measureStartTick, metronomeDiv, ticksPerDiv, startTickDiv, divTick;
		QPen oldPen, dashPen;
		while (tick < file->endTick()) {
			TimeSignatureEvent *measureEvent = currentTimeSignatureEvents->at(i);
			xfrom = xPosOfMs(msOfTick(tick));
			currentDivs.append(QPair<qreal, int>(xfrom, tick));
			measure++;
			measureStartTick = tick;
			tick += currentEvent->ticksPerMeasure();
			if (i < currentTimeSignatureEvents->length() - 1) {
				if (currentTimeSignatureEvents->at(i + 1)->midiTime() <= tick) {
					currentEvent = currentTimeSignatureEvents->at(i + 1);
					tick = currentEvent->midiTime();
					i++;
				}
			}
			// draw measures
			if (_div >= 0) {
				metronomeDiv = qRound(4 / qPow(2, _div));
				ticksPerDiv = metronomeDiv * file->ticksPerQuarter();
				startTickDiv = ticksPerDiv;
				oldPen = pixpainter.pen();
				dashPen = QPen(Qt::lightGray, 1, Qt::DashLine);
				pixpainter.setPen(dashPen);
				while (startTickDiv <= measureEvent->ticksPerMeasure()) {
					divTick = startTickDiv + measureStartTick;
					xDiv = xPosOfMs(msOfTick(divTick));
					currentDivs.append(QPair<qreal, int>(xDiv, divTick));
					pixpainter.drawLine(qLineF(xDiv, 0, xDiv, height()));
					startTickDiv += ticksPerDiv;
				}
				pixpainter.setPen(oldPen);
			}
		}

		// line between time texts and matrixarea
		pixpainter.setPen(Qt::gray);
		pixpainter.drawLine(qLineF(0, 0, width(), 0));
		pixpainter.drawLine(qLineF(0, 0, 0,
									height()));

		pixpainter.setPen(Qt::black);

		// paint the events
		pixpainter.setClipping(true);
		pixpainter.setClipRect(qRectF(0, 0, width(), height()));
		for (int i = 0; i < 19; i++) {
			paintChannel(&pixpainter, i);
		}
		pixpainter.setClipping(false);
		pixpainter.setPen(Qt::black);
		pixpainter.end();

		QPixmapCache::insert(pixmapId, notesPixmap);
		emit objectListChanged();

		// Set the background of the widget to prevent further
		// repaints.
		QPalette palette = this->palette();
		palette.setBrush(backgroundRole(), QBrush(notesPixmap));
		setPalette(palette);
	}
	// TODO: I don't think we need this.
	/*switch (updatemode) {
		case 1: {
			QPainter::PixmapFragment frags[event->region().rectCount()];
			QVector<QRect> rects = event->region().rects();
			for(int i = 0; i < event->region().rectCount(); i++) {
				QRect rect = rects.at(i);
				frags[i] = QPainter::PixmapFragment::create(rect.center(), rect, 1, 1, 0, 1);
			}
			painter.drawPixmapFragments(frags, event->region().rectCount(), notesPixmap);
			break;
		}
		case 2: {
			QPainter::PixmapFragment frag = QPainter::PixmapFragment::create(event->rect().center(), event->rect());
			painter.drawPixmapFragments(&frag, 1, notesPixmap);
			break;
		}
		default:
			QPainter::PixmapFragment frag = QPainter::PixmapFragment::create(relativeRect().center(), relativeRect());

			painter.drawPixmapFragments(&frag, 1, notesPixmap);
	}*/

	if (!QApplication::arguments().contains("--no-antialiasing")
			&& antiAliasingEnabled) {
		painter.setRenderHint(QPainter::Antialiasing);
	}
	// draw the piano / linenames
	if (Tool::currentTool()) {
		painter.setClipping(true);
		painter.setClipRect(relativeRect());
		Tool::currentTool()->draw(&painter);
		painter.setClipping(false);
	}

	int timelinePos = timelineWidget->mousePosition();
	if (enabled && timelinePos >= 0) {
		painter.setPen(Qt::red);
		painter.drawLine(qLineF(timelinePos, 0, timelinePos, height()));
		painter.setPen(Qt::black);
	}

	if (MidiPlayer::instance()->isPlaying()) {
		painter.setPen(Qt::red);
		int x = xPosOfMs(MidiPlayer::instance()->timeMs());
		if (x >= 0) {
			painter.drawLine(qLineF(x, 0, x, height()));
		}
		painter.setPen(Qt::black);
	}

	// border
	painter.setPen(Qt::gray);
	painter.drawLine(qLineF(width() - 1, height() - 1, 0,
							 height() - 1));
	painter.drawLine(qLineF(width() - 1, height() - 1, width() - 1, 2));

	// if the recorder is recording, show red circle
	if (MidiInput::instance()->recording()) {
		painter.setBrush(Qt::red);
		painter.drawEllipse(qPointF(width() - 20, timeHeight + 5), 15, 15);
	}

	// if MouseRelease was not used, delete it
	mouseReleased = false;

	//if (totalRepaint) {
	//	emit objectListChanged();
	//}
}

void MatrixWidget::paintChannel(QPainter *painter, int channel) {
	if (!file->channel(channel)->visible()) {
		return;
	}
	QColor cC = file->channel(channel)->color();

	// filter events
	QMultiMap<int, MidiEvent *> *map = file->channelEvents(channel);

	QMap<int, MidiEvent *>::iterator it = map->lowerBound(startTick);
	while (it != map->end() && it.key() <= endTick) {
		MidiEvent *event = it.value();
		if (eventInWidget(event)) {
			// insert all Events in objects, set their coordinates
			// Only onEvents are inserted. When there is an On
			// and an OffEvent, the OnEvent will hold the coordinates
			int line = event->line();

			OffEvent *offEvent = qobject_cast<OffEvent *>(event);
			OnEvent *onEvent = qobject_cast<OnEvent *>(event);

			qreal x, width;
			qreal y = yPosOfLine(line);
			double height = lineHeight();

			if (onEvent || offEvent) {
				if (onEvent) {
					offEvent = onEvent->offEvent();
				} else if (offEvent) {
					onEvent = qobject_cast<OnEvent *>(offEvent->onEvent());
				}

				width = xPosOfMs(msOfTick(offEvent->midiTime())) -
						xPosOfMs(msOfTick(onEvent->midiTime()));
				x = xPosOfMs(msOfTick(onEvent->midiTime()));
				event = onEvent;
				if (objects->contains(event)) {
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

			if (!(event->track()->hidden())) {
				if (!_colorsByChannels) {
					cC = *event->track()->color();
				}
				event->draw(painter, cC);


				if (Selection::instance()->selectedEvents().contains(event)) {
					painter->setPen(Qt::gray);
					painter->drawLine(qLineF(0, y, this->width(), y));
					painter->drawLine(qLineF(0, y + height, this->width(), y + height));
					painter->setPen(Qt::black);

				}
				objects->prepend(event);
			}
		}

		if (!(event->track()->hidden())) {
			// append event to velocityObjects if its not a offEvent and if it
			// is in the x-Area
			OffEvent *offEvent = qobject_cast<OffEvent *>(event);
			if (!offEvent && event->midiTime() >= startTick &&
					event->midiTime() <= endTick &&
					!velocityObjects->contains(event)) {
				qreal mX = xPosOfMs(msOfTick(event->midiTime()));
				event->setX(mX);

				velocityObjects->prepend(event);
			}
		}
		it++;
	}
}


void MatrixWidget::setFile(MidiFile *f) {

	file = f;

	scaleX = 1;
	scaleY = 1;

	startTimeX = 0;
	// Roughly vertically center on Middle C.
	startLineY = 0;
	int widthNew = qRound((endTimeX / 1000) * (PIXEL_PER_S * scaleX));
	if (parentWidget()->width()
			&& widthNew < parentWidget()->width()
			&& parentWidget()->width()> 0) {
		if (widthNew != 0)
			scaleX =parentWidget()->width() / widthNew;
		widthNew = parentWidget()->width();
	}
	endLineY = NUM_LINES;
	setFixedHeight(qRound(NUM_LINES * PIXEL_PER_LINE * scaleY));
	endTimeX = file->maxTime();

	setFixedWidth(widthNew);
	connect(file->protocol(), SIGNAL(actionFinished()), this,
			SLOT(update()));

	calcSizes();

	// scroll down to see events
	int maxNote = -1;
	for (int channel = 0; channel < 16; channel++) {

		QMultiMap<int, MidiEvent *> *map = file->channelEvents(channel);

		QMap<int, MidiEvent *>::iterator it = map->lowerBound(0);
		while (it != map->end()) {
			NoteOnEvent *onev = qobject_cast<NoteOnEvent *>(it.value());
			if (onev && eventInWidget(onev)) {
				if (onev->line() < maxNote || maxNote < 0) {
					maxNote = onev->line();
				}
			}
			it++;
		}
	}

	if (maxNote - 5 > 0) {
		// startLineY = maxNote - 5.0;
	}
	update();
}

void MatrixWidget::setPianoWidget(PianoWidget *widget) {
	pianoWidget = widget;
}

void MatrixWidget::setTimelineWidget(TimelineWidget *widget) {
	timelineWidget = widget;
}

void MatrixWidget::calcSizes() {
	if (!file) {
		return;
	}

	int widthOld = width();
	int heightOld = height();
	int widthNew = qRound((endTimeX / 1000) * (PIXEL_PER_S * scaleX));
	if (parentWidget()->width ()
			&& widthNew < parentWidget()->width ()
			&& parentWidget()->width () > 0) {
		if (widthNew != 0)
			scaleX = parentWidget()->width () / widthNew;
		widthNew = parentWidget()->width ();
	}
	setFixedHeight(qRound(NUM_LINES * PIXEL_PER_LINE * scaleY));
	setFixedWidth(widthNew);
	ToolArea = QRectF(parentWidget()->geometry());
	PianoArea = qRectF(0, timeHeight, lineNameWidth, height() - timeHeight);
	TimeLineArea = qRectF(lineNameWidth, 0, width() - lineNameWidth, timeHeight);


	if (widthOld != widthNew || heightOld != height()) {
		update();
		emit sizeChanged(0, NUM_LINES - (endLineY + startLineY),
						 startTimeX,
						 startLineY);
	}
}
QSize MatrixWidget::sizeHint() const {
	return QSize(1500, 1500);
}

MidiFile *MatrixWidget::midiFile() {
	return file;
}

void MatrixWidget::mouseMoveEvent(QMouseEvent *event) {
	PaintWidget::mouseMoveEvent(event);

	if (!enabled) {
		return;
	}

	if (!MidiPlayer::instance()->isPlaying() && Tool::currentTool()) {
		Tool::currentTool()->move(event->localPos().x(), event->localPos().y());
		if (mousePressed) {
			update(relativeRect());
		}
	}
}

void MatrixWidget::resizeEvent(QResizeEvent *event) {
	Q_UNUSED(event);
	calcSizes();
}

qreal MatrixWidget::xPosOfMs(qreal ms) {
	return ms * width() / (endTimeX - startTimeX);
}

qreal MatrixWidget::yPosOfLine(int line) {
	return (line - startLineY) * lineHeight();
}

qreal MatrixWidget::lineHeight() {
	if (qFuzzyIsNull(endLineY - startLineY)) {
		return 0;
	}
	return (height() - timeHeight) / (endLineY - startLineY);
}

void MatrixWidget::enterEvent(QEvent *event) {
	PaintWidget::enterEvent(event);
	if (Tool::currentTool()) {
		Tool::currentTool()->enter();
		if (enabled) {
			update();
		}
	}
}
void MatrixWidget::leaveEvent(QEvent *event) {
	PaintWidget::leaveEvent(event);
	if (Tool::currentTool()) {
		Tool::currentTool()->exit();
		if (enabled) {
			update();
		}
	}
}
void MatrixWidget::mousePressEvent(QMouseEvent *event) {
	PaintWidget::mousePressEvent(event);
	if (!MidiPlayer::instance()->isPlaying() && Tool::currentTool()) {
		if (Tool::currentTool()->press(event->buttons() == Qt::LeftButton)) {
			if (enabled) {
				update();
			}
		}
	} /*else if (enabled && (!MidiPlayer::instance()->isPlaying()) && (mouseInRect(PianoArea))) {
		foreach (int key, pianoKeys.keys()) {
			bool inRect = mouseInRect(pianoKeys.value(key));
			if (inRect) {
				// play note
				pianoEvent->setNote(key);
				MidiPlayer::instance()->play(pianoEvent);
			}
		}
	}*/

}
void MatrixWidget::mouseReleaseEvent(QMouseEvent *event) {
	PaintWidget::mouseReleaseEvent(event);

	if (!MidiPlayer::instance()->isPlaying() && Tool::currentTool()) {
		if (Tool::currentTool()->release()) {
			if (enabled) {
				update();
			}
		}
	} else if (Tool::currentTool()) {
		if (Tool::currentTool()->releaseOnly()) {
			if (enabled) {
				update();
			}
		}
	}
	//emit objectListChanged();

}

void MatrixWidget::takeKeyPressEvent(QKeyEvent *event) {
	if (Tool::currentTool()) {
		if (Tool::currentTool()->pressKey(event->key())) {
			update();
		}
	}
}

void MatrixWidget::takeKeyReleaseEvent(QKeyEvent *event) {
	if (Tool::currentTool()) {
		if (Tool::currentTool()->releaseKey(event->key())) {
			update();
		}
	}
	emit objectListChanged();

}

QList<MidiEvent *> *MatrixWidget::activeEvents() {
	return objects;
}

QList<MidiEvent *> *MatrixWidget::velocityEvents() {
	return velocityObjects;
}


int MatrixWidget::msOfXPos(qreal x) {
	return qRound(startTimeX + (x * (endTimeX - startTimeX)) / width());
}

int MatrixWidget::msOfTick(int tick) {
	return file->msOfTick(tick, currentTempoEvents, msOfFirstEventInList);
}

int MatrixWidget::timeMsOfWidth(int w) {
	return (w * (endTimeX - startTimeX)) / width() ;
}

bool MatrixWidget::eventInWidget(MidiEvent *event) {
	NoteOnEvent *on = qobject_cast<NoteOnEvent *>(event);
	OffEvent *off = qobject_cast<OffEvent *>(event);
	if (on) {
		off = on->offEvent();
	} else if (off) {
		on = qobject_cast<NoteOnEvent *>(off->onEvent());
	}
	if (on && off) {
		int line = off->line();
		int tick = off->midiTime();
		bool offIn =  line >= startLineY && line <= endLineY && tick >= startTick &&
					  tick <= endTick;
		line = on->line();
		tick = on->midiTime();
		bool onIn = line >= startLineY && line <= endLineY && tick >= startTick &&
					tick <= endTick;

		off->setShown(offIn);
		on->setShown(onIn);

		return offIn || onIn;

	} else {
		int line = event->line();
		int tick = event->midiTime();
		bool shown = line >= startLineY && line <= endLineY && tick >= startTick &&
					 tick <= endTick;
		event->setShown(shown);

		return shown;
	}
}

int MatrixWidget::lineAtY(qreal y) {
	return qRound(y / lineHeight() + startLineY);
}

void MatrixWidget::zoomStd() {
	scaleX = 1;
	scaleY = 1;
	calcSizes();
}

void MatrixWidget::zoomHorIn() {
	if (scaleX < MAX_HORIZ_ZOOM) {
		scaleX += 0.1;
		calcSizes();
	}
}

void MatrixWidget::zoomHorOut() {
	if (width() * (scaleX - 0.1) >= parentWidget()->width () && scaleX >= 0.2) {
		scaleX -= 0.1;
		calcSizes();
	}
}

void MatrixWidget::zoomVerIn() {
	if (scaleY < MAX_VERT_ZOOM) {
		scaleY += 0.1;
		calcSizes();
	}
}

void MatrixWidget::zoomVerOut() {
	if (height() * (scaleY - 0.1) >= parentWidget()->width () && scaleY >= 0.2) {
		scaleY -= 0.1;
			calcSizes();

	}
}

int MatrixWidget::minVisibleMidiTime() {
	return startTick;
}

int MatrixWidget::maxVisibleMidiTime() {
	return endTick;
}
void MatrixWidget::keyPressEvent(QKeyEvent *event) {
	takeKeyPressEvent(event);
}

void MatrixWidget::keyReleaseEvent(QKeyEvent *event) {
	takeKeyReleaseEvent(event);
}

void MatrixWidget::setColorsByChannel() {
	_colorsByChannels = true;
}
void MatrixWidget::setColorsByTracks() {
	_colorsByChannels = false;
}

bool MatrixWidget::colorsByChannel() {
	return _colorsByChannels;
}

void MatrixWidget::setDiv(int div) {
	_div = div;
	update();
}

QList<QPair<qreal, int> > MatrixWidget::divs() {
	return currentDivs;
}

int MatrixWidget::div() {
	return _div;
}
