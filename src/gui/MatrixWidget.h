#ifndef MATRIXWIDGET_H_
#define MATRIXWIDGET_H_

#include "PaintWidget.h"

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

class MidiFile;
class TempoChangeEvent;
class TimeSignatureEvent;
class MidiEvent;
class GraphicObject;
class NoteOnEvent;

class MatrixWidget : public PaintWidget {

	Q_OBJECT

	public:
		MatrixWidget(QWidget *parent = 0);
		void setFile(MidiFile *file);
		MidiFile *midiFile();
		QList<MidiEvent*> *activeEvents();
		QList<MidiEvent*> *velocityEvents();
		double lineHeight();
		int lineAtY(int y);
		int msOfXPos(int x);
		int timeMsOfWidth(int w);
		bool eventInWidget(MidiEvent *event);
		int yPosOfLine(int line);
		void setScreenLocked(bool b);
		bool screenLocked();

	public slots:
		void scrollXChanged(int scrollPositionX);
		void scrollYChanged(int scrollPositionY);
		void zoomHorIn();
		void zoomHorOut();
		void zoomVerIn();
		void zoomVerOut();
		void timeMsChanged(int ms);
		void registerRelayout();
		void calcSizes();

	signals:
		void sizeChanged(int maxScrollTime, int maxScrollLine, int valueX,
				int valueY);
		void objectListChanged();
		void scrollChanged(int startMs,int maxMs,int startLine,int maxLine);

	protected:
		void paintEvent(QPaintEvent *event);
		void mouseMoveEvent(QMouseEvent *event);
		void resizeEvent(QResizeEvent *event);
		void enterEvent(QEvent *event);
		void leaveEvent(QEvent *event);
		void mousePressEvent(QMouseEvent *event);
		void mouseDoubleClickEvent(QMouseEvent *event);
		void mouseReleaseEvent(QMouseEvent *event);
		void keyPressEvent(QKeyEvent* e);
		void keyReleaseEvent(QKeyEvent *event);

	private:
		void paintChannel(QPainter *painter, int channel);
		void paintPianoKey(QPainter *painter, int number, int x, int y,
				int width, int height);

		int startTick, endTick, startTimeX, endTimeX, startLineY, endLineY,
			lineNameWidth, timeHeight, msOfFirstEventInList;
		double scaleX, scaleY;
		MidiFile *file;
		int msOfTick(int tick);
		int xPosOfMs(int ms);
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
};

#endif
