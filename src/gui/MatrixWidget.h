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

#ifndef MATRIXWIDGET_H_
#define MATRIXWIDGET_H_

#include "PaintWidget.h"
#include "TimelineWidget.h"
#include "PianoWidget.h"

#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QMap>
#include <QColor>
#include <QPixmap>
#include <QApplication>
#include <QPalette>
#include <QPixmapCache>

class MidiFile;
class TempoChangeEvent;
class TimeSignatureEvent;
class MidiEvent;
class GraphicObject;
class NoteOnEvent;
class TimelineWidget;
class PianoWidget;

class MatrixWidget : public PaintWidget {

	Q_OBJECT

	public:
		MatrixWidget(QWidget *parent = 0);
		void setFile(MidiFile *file);
		MidiFile *midiFile();
		QList<MidiEvent*> *activeEvents();
		QList<MidiEvent*> *velocityEvents();

		qreal lineHeight();
		int lineAtY(qreal y);
		int lineNameWidth;
		qreal startLineY;
		qreal scaleX, scaleY, endLineY;
		int msOfXPos(qreal x);
		int timeMsOfWidth(int w);
		bool eventInWidget(MidiEvent *event);
		qreal yPosOfLine(int line);
		void setScreenLocked(bool b);
		bool screenLocked();
		int minVisibleMidiTime();
		int maxVisibleMidiTime();

		void setColorsByChannel();
		void setColorsByTracks();
		bool colorsByChannel();

		QList<QPair<qreal, int> > currentDivs;

		int msOfTick(int tick);
		qreal xPosOfMs(qreal ms);
		QList<QPair<qreal, int> > divs();

		static bool antiAliasingEnabled;
		QMap<int, QRectF> pianoKeys;
		QSize sizeHint() const  Q_DECL_OVERRIDE;

		void setPianoWidget(PianoWidget *widget);
		void setTimelineWidget(TimelineWidget *widget);
	public slots:
		void zoomHorIn();
		void zoomHorOut();
		void zoomVerIn();
		void zoomVerOut();
		void zoomStd();
		void timeMsChanged(int ms, bool ignoreLocked=false);
		void calcSizes();
		void takeKeyPressEvent(QKeyEvent *event);
		void takeKeyReleaseEvent(QKeyEvent *event);
		void setDiv(int div);
		int div();
	signals:
		void sizeChanged(int maxScrollTime, double maxScrollLine, int valueX,
				double valueY);
		void objectListChanged();
		void scrollChanged(int x, int y);

	protected:
		void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
		void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
		void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
		void enterEvent(QEvent *event) Q_DECL_OVERRIDE;
		void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
		void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
		void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
		void keyPressEvent(QKeyEvent* e) Q_DECL_OVERRIDE;
		void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

	private:
		void paintChannel(QPainter *painter, int channel);
		void paintPianoKey(QPainter *painter, int number, qreal x, qreal y,
				qreal width, qreal height);

		int startTick, endTick, startTimeX, endTimeX, timeHeight,
				msOfFirstEventInList, visibleStartTick, visibleEndTick;

		enum VerticalScrollDir {NONE, UP, DOWN};
		VerticalScrollDir scrollDir = NONE;
		MidiFile *file;

		QRectF ToolArea, PianoArea, TimeLineArea;
		bool screen_locked;

		// pixmap is the painted widget (without tools and cursorLines).
		// it will be zero if it needs to be repainted
		QPixmap *pixmap;

		// saves all TempoEvents from one before the first shown tick to the
		// last in the window
		QList<MidiEvent*> *currentTempoEvents;
		QList<TimeSignatureEvent*> *currentTimeSignatureEvents;

		// All Events to show in the velocityWidget are saved in velocityObjects
		QList<MidiEvent*> *objects, *velocityObjects;

		// To play the pianokeys, there is one NoteOnEvent
		NoteOnEvent *pianoEvent;

		bool _colorsByChannels;
		int _div;

		TimelineWidget *timelineWidget;
		PianoWidget *pianoWidget;
};

#endif
