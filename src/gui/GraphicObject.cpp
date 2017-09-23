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

#include "GraphicObject.h"

#include "MatrixWidget.h"

GraphicObject::GraphicObject() {
	_x = 0;
	_y = 0;
	_width = 0;
	_height = 0;
}

qreal GraphicObject::x() {
	return _x;
}

qreal GraphicObject::y() {
	return _y;
}

qreal GraphicObject::width() {
	return _width;
}

qreal GraphicObject::height() {
	return _height;
}

void GraphicObject::setX(qreal x) {
	_x = x;
}

void GraphicObject::setY(qreal y) {
	_y = y;
}

void GraphicObject::setWidth(qreal w) {
	_width = w;
}

void GraphicObject::setHeight(qreal h) {
	_height = h;
}

void GraphicObject::draw(QPainter *p, QColor c) {
	Q_UNUSED(p) Q_UNUSED(c)
	return;
}

bool GraphicObject::shown() {
	return shownInWidget;
}

void GraphicObject::setShown(bool b) {
	shownInWidget = b;
}
QRectF GraphicObject::qRectF(QRectF other) {
	if (MatrixWidget::antiAliasingEnabled) {
		return other;
	} else {
		return QRectF(other.toRect());
	}
}
QRectF GraphicObject::qRectF(qreal x, qreal y, qreal w, qreal h) {
	return qRectF(QRectF(x, y, w, h));
}
QLineF GraphicObject::qLineF(QLineF other) {
	if (MatrixWidget::antiAliasingEnabled) {
		return other;
	} else {
		return QLineF(other.toLine());
	}
}
QLineF GraphicObject::qLineF(qreal x1, qreal y1, qreal x2, qreal y2) {
	return qLineF(QLineF(x1, y1, x2, y2));
}
QPolygonF GraphicObject::qPolygonF(QPolygonF other) {
	if (MatrixWidget::antiAliasingEnabled) {
		return other;
	} else {
		return QPolygonF(other.toPolygon());
	}
}
QPointF GraphicObject::qPointF(QPointF  other) {
	if (MatrixWidget::antiAliasingEnabled) {
		return other;
	} else {
		return QPointF(other.toPoint());
	}
}
QPointF GraphicObject::qPointF(qreal x, qreal y) {
	return qPointF(QPointF(x, y));
}
