#ifndef NEWNOTETOOL_H_
#define NEWNOTETOOL_H_

#include "EventTool.h"

class NewNoteTool : public EventTool {

	public:
		NewNoteTool();
		NewNoteTool(NewNoteTool &other);

		ProtocolEntry *copy();
		void reloadState(ProtocolEntry *entry);

		void draw(QPainter *painter);
		bool press();
		bool release();
		bool move(int mouseX, int mouseY);
		bool releaseOnly();

	private:
		bool inDrag;
		int line;
		int xPos;
};

#endif
