#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include "PaintWidget.h"
#include "MatrixWidget.h"
#include "MainWindow.h"
#include "../midi/MidiFile.h"
#include "../MidiEvent/TimeSignatureEvent.h"

#include <QObject>
class PaintWidget;
class MidiFile;

class TimelineWidget : public PaintWidget
{
	Q_OBJECT

	public:
		TimelineWidget(QWidget *parent = 0);
		void setMatrixWidget(MatrixWidget *widget);

		void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
		void setFile(MidiFile *midiFile);
		QSize sizeHint() const Q_DECL_OVERRIDE;
		qreal mousePosition();
	protected:
		void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
		void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	private:
		MatrixWidget *matrixWidget;
		qreal timeHeight;
		MidiFile *file;
		QList<TimeSignatureEvent*> *currentTimeSignatureEvents;
};

#endif // TIMELINEWIDGET_H
