#include "EditorTool.h"
#include "ToolButton.h"
#include "../gui/MatrixWidget.h"
#include "../gui/MainWindow.h"

MatrixWidget *EditorTool::matrixWidget = 0;
MainWindow *EditorTool::_mainWindow = 0;

EditorTool::EditorTool():Tool(){
	etool_selected = false;
	mouseIn = false;
}

EditorTool::EditorTool(EditorTool &other):Tool(other){
	etool_selected = other.etool_selected;
	mouseIn = other.mouseIn;
}

void EditorTool::draw(QPainter *painter){
	Q_UNUSED(painter);
	return;
}

bool EditorTool::press(){
	return false;
}

bool EditorTool::pressKey(int key){
	Q_UNUSED(key);
	return false;
}

bool EditorTool::releaseKey(int key){
	Q_UNUSED(key);
	return false;
}

bool EditorTool::release(){
	return false;
}

bool EditorTool::move(int mouseX, int mouseY){
	this->mouseX = mouseX;
	this->mouseY = mouseY;
	return false;
}

void EditorTool::exit(){
	mouseIn = false;
	return;
}

void EditorTool::enter(){
	mouseIn = true;
	return;
}

void EditorTool::deselect(){
	etool_selected = false;
	if(button()){
		button()->repaint();
	}
}

void EditorTool::select(){
	etool_selected = true;
	if(button()){
		button()->repaint();
	}
}

bool EditorTool::selected(){
	return etool_selected;
}

void EditorTool::buttonClick(){
	if(_currentTool){
		_currentTool->deselect();
	}
	_currentTool = this;
	select();
}

ProtocolEntry *EditorTool::copy(){
	return new EditorTool(*this);
}

void EditorTool::reloadState(ProtocolEntry *entry){

	Tool::reloadState(entry);

	EditorTool *other = dynamic_cast<EditorTool*>(entry);
	if(!other){
		return;
	}
	etool_selected = other->etool_selected;
	mouseIn = other->mouseIn;
}

void EditorTool::setMatrixWidget(MatrixWidget *w){
	matrixWidget = w;
}

void EditorTool::setMainWindow(MainWindow *mw){
	_mainWindow = mw;
}

bool EditorTool::pointInRect(int x, int y,  int x_start, int y_start, int x_end, int y_end){
	return x>=x_start && x<x_end && y>=y_start && y<=y_end;
}

bool EditorTool::releaseOnly(){
	return false;
}
