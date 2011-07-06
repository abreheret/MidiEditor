#ifndef SIZECHANGETOOL_H_
#define SIZECHANGETOOL_H_

#include "EventTool.h"

class SizeChangeTool : public EventTool {

	public:
		SizeChangeTool();
		SizeChangeTool(SizeChangeTool &other);

		ProtocolEntry *copy();
		void reloadState(ProtocolEntry *entry);

		void draw(QPainter *painter);
		bool press();
		bool release();
		bool move(int mouseX, int mouseY);
		bool releaseOnly();

	private:
		bool inDrag, dragsOnEvent;
		int xPos;
};

#endif
