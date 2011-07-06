#ifndef PAINTWIDGET_H
#define PAINTWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QEvent>
#include <QCursor>
#include <QPoint>

class PaintWidget : public QWidget {

	public:
		PaintWidget(QWidget *parent = 0);
		void setRepaintOnMouseMove(bool b);
		void setRepaintOnMousePress(bool b);
		void setRepaintOnMouseRelease(bool b);
		void setEnabled(bool b);

	protected:
		void mouseMoveEvent(QMouseEvent *event);
		void enterEvent(QEvent *event);
		void leaveEvent(QEvent *event);
		void mousePressEvent(QMouseEvent *event);
		void mouseReleaseEvent(QMouseEvent *event);

		int movedX(){return mouseX - mouseLastX; }
		int movedY() { return mouseY - mouseLastY; }
		int draggedX();
		int draggedY();
		bool mouseInRect(int x, int y, int width, int height);
		bool mouseInRect(QRectF rect);
		bool mouseBetween(int x1, int y1, int x2, int y2);

		void setMousePinned(bool b){mousePinned = b;}

		bool mouseOver, mousePressed, mouseReleased, repaintOnMouseMove,
			 repaintOnMousePress, repaintOnMouseRelease, inDrag, mousePinned,
			 enabled;
		int mouseX, mouseY, mouseLastX, mouseLastY;

};
#endif
