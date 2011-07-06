#ifndef ERASERTOOL_H_
#define ERASERTOOL_H_

#include "EventTool.h"

class EraserTool : public EventTool {

	public:
		EraserTool();
		EraserTool(EraserTool &other);

		ProtocolEntry *copy();
		void reloadState(ProtocolEntry *entry);
		bool move(int mouseX, int mouseY);
		void draw(QPainter *painter);
		bool release();
};

#endif
