#ifndef EVENTMOVETOOL_H_
#define EVENTMOVETOOL_H_

#include "EventTool.h"

class MidiEvent;

class EventMoveTool : public EventTool {

	public:
		EventMoveTool(bool upDown, bool leftRight);
		EventMoveTool(EventMoveTool &other);

		ProtocolEntry *copy();
		void reloadState(ProtocolEntry *entry);

		void draw(QPainter *painter);
		bool press();
		bool release();
		bool move(int mouseX, int mouseY);
		bool releaseOnly();

	protected:
		bool moveUpDown, moveLeftRight, inDrag;
		int startX, startY;
};

#endif
