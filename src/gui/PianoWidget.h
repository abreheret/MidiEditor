#ifndef PIANOWIDGET_H
#define PIANOWIDGET_H

#include "PaintWidget.h"
#include <QPaintEvent>
#include <QObject>
#include "MatrixWidget.h"
#include <QSize>
#include "../MidiEvent/MidiEvent.h"

class PaintWidget;
class MatrixWidget;

class PianoWidget : public PaintWidget
{
	Q_OBJECT

	public:
		PianoWidget(QWidget *parent);
		void paintPianoKey(QPainter *painter, int number, qreal x, qreal y, qreal width, qreal height);
		void setMatrixWidget(MatrixWidget *widget);
		QSize sizeHint() const Q_DECL_OVERRIDE;
		void setFile(MidiFile *file);
		enum PianoKey {
			C = 0,
			C_Sharp,
			D,
			D_Sharp,
			E,
			F,
			F_Sharp,
			G,
			G_Sharp,
			A,
			A_Sharp,
			B
		};
		enum PianoKeyShape {
			Black,
			WhiteBelowBlack,
			WhiteAboveBlack,
			WhiteBetweenBlack,
			WhiteOnly
		};
	public slots:
		void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
	private:
		MatrixWidget *matrixWidget;
		//QCache<PianoKey, QPainterPath> keyCache;
};

#endif // PIANOWIDGET_H
