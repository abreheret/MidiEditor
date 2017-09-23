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
#include "PaintWidget.h"

#include <QScrollArea>

PaintWidget::PaintWidget(QWidget *parent) : QWidget(parent) {
	this->setMouseTracking(true);
	this->mouseOver = false;
	this->mousePressed = false;
	this->mouseReleased = false;
	this->repaintOnMouseMove = false;
	this->repaintOnMousePress = false;
	this->repaintOnMouseRelease = false;
	this->repaintOnScroll = true;
	this->inDrag = false;
	this->mousePinned = false;
	this->mouseX = 0;
	this->mouseY = 0;
	this->mouseLastY = 0;
	this->mouseLastX = 0;
	this->enabled = true;
}

void PaintWidget::mouseMoveEvent(QMouseEvent *event){

	this->mouseOver = true;

	if(mousePinned){
		// do not change mousePosition but lastMousePosition to get the
		// correct move distance
		QCursor::setPos(mapToGlobal(QPointF(mouseX, mouseY).toPoint()));
		mouseLastX = 2*mouseX-qPointF(event->localPos()).x();
		mouseLastY = 2*mouseY-qPointF(event->localPos()).y();
	} else {
		this->mouseLastX = this->mouseX;
		this->mouseLastY = this->mouseY;
		this->mouseX = qPointF(event->localPos()).x();
		this->mouseY = qPointF(event->localPos()).y();
	}
	if(mousePressed){
		inDrag = true;
	}

	if(!enabled){
		return;
	}

	if(this->repaintOnMouseMove){
		this->update();
	}
}

void PaintWidget::enterEvent(QEvent *event){
	Q_UNUSED(event)

	this->mouseOver = true;

	if(!enabled){
		return;
	}

	update();
}

void PaintWidget::wheelEvent(QWheelEvent *event) {
	if (repaintOnScroll)
		update();
}

void PaintWidget::leaveEvent(QEvent *event){
	Q_UNUSED(event)

	this->mouseOver = false;

	if(!enabled){
		return;
	}
	update();
}

void PaintWidget::mousePressEvent(QMouseEvent *event){
	Q_UNUSED(event)

	this->mousePressed = true;
	this->mouseReleased = false;

	if(!enabled){
		return;
	}

	if(this->repaintOnMousePress){
		this->update();
	}
}

void PaintWidget::mouseReleaseEvent(QMouseEvent *event){
	Q_UNUSED(event)

	this->inDrag = false;
	this->mouseReleased = true;
	this->mousePressed = false;

	if(!enabled){
		return;
	}

	if(this->repaintOnMouseRelease){
		this->update();
	}
}

bool PaintWidget::mouseInRect(qreal x, qreal y, qreal width, qreal height){
	return mouseBetween(x, y, x+width, y+height);
}

bool PaintWidget::mouseInRect(QRectF rect){
	return mouseInRect(rect.x(), rect.y(), rect.width(), rect.height());
}

bool PaintWidget::mouseInWidget(PaintWidget *widget) {
	if (qobject_cast<QScrollArea*>(widget->parentWidget()->parentWidget())) {
		QRect rect = widget->parentWidget()->rect();
		return widget->mouseInRect(QRect(widget->mapFromParent(rect.topLeft()),widget->mapFromParent(rect.bottomRight())));
	}
	return false;
}
QRect PaintWidget::relativeRect() {
	QScrollArea *scrollArea = qobject_cast<QScrollArea*>(parentWidget()->parentWidget());
	if (scrollArea) {
		return QRect(mapFrom(parentWidget(), parentWidget()->frameGeometry().topLeft()),
			   mapFrom(parentWidget(), parentWidget()->frameGeometry().bottomRight()));
	}
	return rect();
}

bool PaintWidget::mouseBetween(qreal x1, qreal y1, qreal x2, qreal y2){
	qreal temp;
	if(x1>x2){
		temp = x1;
		x1 = x2;
		x2 = temp;
	}
	if(y1>y2){
		temp = y1;
		y1 = y2;
		y2 = temp;
	}
	return mouseOver && mouseX>=x1 && mouseX<=x2 && mouseY>=y1 && mouseY<=y2;
}


qreal PaintWidget::draggedX(){
	if(!inDrag){
		return 0;
	}
	qreal i = mouseX - mouseLastX;
	mouseLastX = mouseX;
	return i;
}

qreal PaintWidget::draggedY(){
	if(!inDrag){
		return 0;
	}
	qreal i = mouseY - mouseLastY;
	mouseLastY = mouseY;
	return i;
}

void PaintWidget::setRepaintOnMouseMove(bool b){
	repaintOnMouseMove = b;
}

void PaintWidget::setRepaintOnMousePress(bool b){
	repaintOnMousePress = b;
}

void PaintWidget::setRepaintOnMouseRelease(bool b){
	repaintOnMouseRelease = b;
}

void PaintWidget::setRepaintOnScroll(bool b) {
	repaintOnScroll = b;
}

void PaintWidget::setEnabled(bool b){
	enabled = b;
	setMouseTracking(enabled);
	update();
}
QRectF PaintWidget::qRectF(QRectF other) {
	return GraphicObject::qRectF(other);
}
QRectF PaintWidget::qRectF(qreal x, qreal y, qreal w, qreal h) {
	return GraphicObject::qRectF(x, y, w, h);
}
QLineF PaintWidget::qLineF(QLineF other) {
	return GraphicObject::qLineF(other);
}
QLineF PaintWidget::qLineF(qreal x1, qreal y1, qreal x2, qreal y2) {
	return GraphicObject::qLineF(x1, y1, x2, y2);
}
QPointF PaintWidget::qPointF(QPointF other) {
	return GraphicObject::qPointF(other);
}
QPointF PaintWidget::qPointF(qreal x, qreal y) {
	return GraphicObject::qPointF(x, y);
}
QPolygonF PaintWidget::qPolygonF(QPolygonF other) {
	return GraphicObject::qPolygonF(other);
}
