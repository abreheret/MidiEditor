/*
 * MidiEditor
 * Copyright (C) 2010  Markus Schwenk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SELECTTOOL_H_
#define SELECTTOOL_H_

#include "EventTool.h"

#define SELECTION_TYPE_RIGHT 0
#define SELECTION_TYPE_LEFT 1
#define SELECTION_TYPE_BOX 2
#define SELECTION_TYPE_SINGLE 3

class MidiEvent;

class SelectTool: public EventTool {

	Q_OBJECT

	public:
		SelectTool(int type);
		SelectTool(SelectTool &other);
		Tool::ToolType type() const Q_DECL_OVERRIDE;

		void draw(QPainter *painter) Q_DECL_OVERRIDE;

		bool press(bool leftClick) Q_DECL_OVERRIDE;
		bool release() Q_DECL_OVERRIDE;
		bool releaseOnly() Q_DECL_OVERRIDE;

		bool move(qreal mouseX, qreal mouseY) Q_DECL_OVERRIDE;

		ProtocolEntry *copy() Q_DECL_OVERRIDE;
		void reloadState(ProtocolEntry *entry) Q_DECL_OVERRIDE;
		bool inRect(MidiEvent *event, qreal x_start, qreal y_start, qreal x_end, qreal y_end);

		bool showsSelection() Q_DECL_OVERRIDE;

	protected:
		int stool_type;
		qreal x_rect, y_rect;
};

#endif
