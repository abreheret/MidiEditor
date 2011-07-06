#include "GraphicObject.h"

GraphicObject::GraphicObject() {
	_x = 0;
	_y = 0;
	_width = 0;
	_height = 0;
}

int GraphicObject::x(){
	return _x;
}

int GraphicObject::y(){
	return _y;
}

int GraphicObject::width(){
	return _width;
}

int GraphicObject::height(){
	return _height;
}

void GraphicObject::setX(int x){
	_x = x;
}

void GraphicObject::setY(int y){
	_y = y;
}

void GraphicObject::setWidth(int w){
	_width = w;
}

void GraphicObject::setHeight(int h){
	_height = h;
}

void GraphicObject::draw(QPainter *p, QColor c){
	return;
}

bool GraphicObject::shown(){
	return shownInWidget;
}

void GraphicObject::setShown(bool b){
	shownInWidget = b;
}
