#ifndef CLICKBUTTON_H
#define CLICKBUTTON_H

#include <QPushButton>
#include <QFont>
#include <QPainter>
#include <QImage>

class ClickButton  : public QPushButton {

	Q_OBJECT

	public:
		ClickButton(QString imageName, QWidget *parent = 0);
		void setImageName(QString imageName);

	public slots:
		void buttonClick();

	protected:
		void paintEvent(QPaintEvent *event);
		void enterEvent(QEvent *event);
		void leaveEvent(QEvent *event);

	private:
		bool button_mouseInButton, button_mouseClicked;
		QImage *image;
};
#endif
