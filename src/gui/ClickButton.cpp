#include "ClickButton.h"

ClickButton::ClickButton(QString imageName, QWidget *parent):QPushButton(parent){
	setFixedSize(34, 34);
	button_mouseInButton = false;
	button_mouseClicked = false;
	setImageName(imageName);
	connect(this, SIGNAL(clicked()), this, SLOT(buttonClick()));
}

void ClickButton::paintEvent(QPaintEvent *event){

	Q_UNUSED(event);

	QPainter painter(this);

	if(button_mouseInButton){
		painter.fillRect(0,0,width(), height(), Qt::gray);
		if(button_mouseClicked){
			painter.fillRect(0,0,width(), height(), Qt::darkGray);
		}
	}

	painter.drawImage(QRectF(1,1,32,32),*(image), QRectF(0,0,32,32));
}

void ClickButton::enterEvent(QEvent *event){
	Q_UNUSED(event);
	button_mouseInButton = true;
}

void ClickButton::leaveEvent(QEvent *event){
	Q_UNUSED(event);
	button_mouseInButton = false;
	button_mouseClicked = false;
}

void ClickButton::buttonClick(){
	button_mouseClicked  = true;
	repaint();
}


void ClickButton::setImageName(QString imageName) {
	image = new QImage("graphics/tool/"+imageName);
	repaint();
}
