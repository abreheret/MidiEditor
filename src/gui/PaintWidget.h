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

#ifndef PAINTWIDGET_H
#define PAINTWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QEvent>
#include <QCursor>
#include <QPoint>
#include <QPolygonF>

// With all these classes, for some reason, QtMath
// (and subsequently, cmath) is not included.
#include <QtMath>

class PaintWidget : public QWidget {

	Q_OBJECT

	public:
		PaintWidget(QWidget *parent = 0);
		void setRepaintOnMouseMove(bool b);
		void setRepaintOnMousePress(bool b);
		void setRepaintOnMouseRelease(bool b);
		void setRepaintOnScroll(bool b);
		void setEnabled(bool b);
		QRect relativeRect();

		/**
		 * \brief These helpers will return pixel-aligned values
		 * when antialiasing is disabled and precise valeus
		 * when it isn't.
		 *
		 * This uses the functions from GraphicsObject.
		 */
		static QRectF qRectF(QRectF other);
		static QRectF qRectF(qreal x, qreal y, qreal w, qreal h);
		static QLineF qLineF(QLineF other);
		static QLineF qLineF(qreal x1, qreal y1, qreal x2, qreal y2);
		static QPolygonF qPolygonF(QPolygonF other);
		static QPointF qPointF(QPointF other);
		static QPointF qPointF(qreal x, qreal y);
	protected:
		void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
		void enterEvent(QEvent *event) Q_DECL_OVERRIDE;
		void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
		void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
		void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
		void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
		qreal movedX(){return mouseX - mouseLastX; }
		qreal movedY() { return mouseY - mouseLastY; }
		qreal draggedX();
		qreal draggedY();
		bool mouseInRect(qreal x, qreal y, qreal width, qreal height);
		bool mouseInRect(QRectF rect);
		bool mouseInWidget(PaintWidget *widget);
		bool mouseBetween(qreal x1, qreal y1, qreal x2, qreal y2);

		void setMousePinned(bool b){mousePinned = b;}

		bool mouseOver, mousePressed, mouseReleased, repaintOnMouseMove,
			 repaintOnMousePress, repaintOnMouseRelease, repaintOnScroll,
			 inDrag, mousePinned, enabled;
		qreal mouseX, mouseY, mouseLastX, mouseLastY;
};
#endif
