#ifndef SELECTTOOL_H_
#define SELECTTOOL_H_

#include "EventTool.h"

#define SELECTION_TYPE_RIGHT 0
#define SELECTION_TYPE_LEFT 1
#define SELECTION_TYPE_BOX 2
#define SELECTION_TYPE_SINGLE 3

class MidiEvent;

class SelectTool: public EventTool {

	public:
		SelectTool(int type);
		SelectTool(SelectTool &other);

		void draw(QPainter *painter);

		/**
		 * \brief this method is called when the mouse is clicked above the
		 * Widget.
		 *
		 * Returns wether the Widget has to be repainted after the Tools
		 * action
		 */
		bool press();
		/**
		 * \brief this method is called when the mouse is released above the
		 * Widget.
		 *
		 * Returns wether the Widget has to be repainted after the Tools
		 * action
		 */
		bool release();
		/**
		 * \brief this method is called when the mouse is moved above the
		 * Widget.
		 *
		 * Returns wether the Widget has to be repainted after the Tools
		 * action
		 */
		bool releaseOnly();

		bool move(int mouseX, int mouseY);

		ProtocolEntry *copy();
		void reloadState(ProtocolEntry *entry);
		bool inRect(MidiEvent *event, int x_start, int y_start, int x_end, int y_end);

	protected:
		int stool_type;
		int x_rect, y_rect;
};

#endif
