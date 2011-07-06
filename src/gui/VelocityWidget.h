#ifndef VELOCITYWIDGET_H_
#define VELOCITYWIDGET_H_

#include "PaintWidget.h"

class MatrixWidget;

#define SINGLE_MODE 1
#define LINE_MODE 2
#define MOUSE_MODE 3

class VelocityWidget : public PaintWidget {

	public:
		VelocityWidget(MatrixWidget *mw, QWidget *parent=0);

	protected:
		void paintEvent(QPaintEvent *event);
		void keyPressEvent(QKeyEvent* e);
		void keyReleaseEvent(QKeyEvent *event);

		void mouseReleaseEvent(QMouseEvent *event);
		void mousePressEvent(QMouseEvent *event);
		void leaveEvent(QEvent *event);

	private:
		MatrixWidget *matrixWidget;

		// Mode is SINGLE_MODE or LINE_MODE
		int mode;

		// dragY for SINGLE_MODE
		int dragY;

		// Start coordinates in the line Mode, will be -1 if no action started
		int startX, startY;

		// is true when there is a drag in the mouse Mode
		bool protocolActionStarted;
};

#endif
