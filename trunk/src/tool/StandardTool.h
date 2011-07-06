#ifndef STANDARDTOOL_H_
#define STANDARDTOOL_H_

#include "EventTool.h"

class EventMoveTool;
class SelectTool;
class SizeChangeTool;

class StandardTool :public EventTool {

	public:
		StandardTool();
		StandardTool(StandardTool &other);

		void draw(QPainter *painter);
		bool press();
		bool move(int mouseX, int mouseY);
		bool release();

		ProtocolEntry *copy();
		void reloadState(ProtocolEntry *entry);

	private:
		EventMoveTool *moveTool;
		SelectTool *selectTool;
		SizeChangeTool *sizeChangeTool;
};
#endif
