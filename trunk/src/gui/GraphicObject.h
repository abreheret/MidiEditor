#ifndef GRAPHICOBJECT_H_
#define GRAPHICOBJECT_H_

#include <QPainter>
#include <QColor>

class GraphicObject {

	public:
		GraphicObject();

		int x();
		int y();
		int width();
		int height();

		void setX(int x);
		void setY(int y);
		void setWidth(int w);
		void setHeight(int h);

		virtual void draw(QPainter *p, QColor c);

		bool shown();
		void setShown(bool b);

	private:
		int _x, _y, _width, _height;
		bool shownInWidget;
};

#endif
