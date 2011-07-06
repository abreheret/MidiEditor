#include "ToolButton.h"
#include "Tool.h"

ToolButton::ToolButton(Tool *tool, QWidget *parent) : QPushButton(parent){
	button_tool = tool;
	tool->setButton(this);
	setToolTip(button_tool->toolTip());
	connect(this, SIGNAL(clicked()), this, SLOT(buttonClick()));
	setFixedSize(34, 34);
	button_mouseInButton = false;
	button_mouseClicked = false;
}

void ToolButton::paintEvent(QPaintEvent *event){

	Q_UNUSED(event);

	QPainter painter(this);

	if(button_tool->selected()){
		painter.fillRect(0,0,width(), height(), Qt::gray);
	}

	if(button_mouseInButton){
		painter.fillRect(0,0,width(), height(), Qt::gray);
		if(button_mouseClicked){
			painter.fillRect(0,0,width(), height(), Qt::darkGray);
		}
	}

	if(button_mouseInButton || button_tool->selected()){
		painter.drawLine(0,0,0,height()-1);
		painter.drawLine(0,0,width()-1, 0);
		painter.drawLine(width()-1, height()-1, 0, height()-1);
		painter.drawLine(width()-1, height()-1, width()-1, 0);
	}


	if(button_tool->image()){
		painter.drawImage(QRectF(1,1,32,32),*(button_tool->image()),
			QRectF(0,0,32,32));
	}
}

void ToolButton::buttonClick(){
	button_mouseClicked  = true;
	button_tool->buttonClick();
	repaint();
}

void ToolButton::enterEvent(QEvent *event){
	Q_UNUSED(event);
	button_mouseInButton = true;
}
void ToolButton::leaveEvent(QEvent *event){
	Q_UNUSED(event);
	button_mouseInButton = false;
	button_mouseClicked = false;
}

void ToolButton::releaseButton(){
	button_mouseClicked = false;
	button_tool->buttonClick();
	repaint();
}
