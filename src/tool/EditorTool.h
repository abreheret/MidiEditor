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

#ifndef EDITORTOOL_H_
#define EDITORTOOL_H_


#include <QPainter>

#include "Tool.h"

class MatrixWidget;
class MainWindow;

/**
 * \class EditorTool
 *
 * \brief EditorTool is a Tool which can be connected to a Widget to act on.
 *
 * An EditorTool can be selected by clicking the Button of the Tool.
 * Clicking on another EditorTools Button will deselect the currently selected
 * and select the new EditorTool.
 * The active EditorTool has to paint itself above a special Widget, defined
 * in the subclasses of EditorWidget. It will register all actions released
 * on this Widget.
 */
class EditorTool : public Tool {

	Q_OBJECT

	public:

		/**
		 * \brief creates a new EditorTool.
		 */
		EditorTool();

		/**
		 * \brief creates a new EditorTool copying &other.
		 */
		EditorTool(EditorTool &other);

		Tool::ToolType type() const Q_DECL_OVERRIDE;

		/**
		 * \brief draws the EditorTools data to painter.
		 */
		virtual void draw(QPainter *painter);

		/**
		 * \brief this method is called when the mouse is clicked above the
		 * Widget.
		 *
		 * Returns wether the Widget has to be repainted after the Tools
		 * action
		 */
		virtual bool press(bool leftClick);

		/**
		 * \brief this method is called when a key is pressed while the
		 * Widget is focused.
		 *
		 * Returns wether the Widget has to be repainted after the Tools
		 * action
		 */
		virtual bool pressKey(int key);

		/**
		 * \brief this method is called when a key is released while the
		 * Widget is focused.
		 *
		 * Returns wether the Widget has to be repainted after the Tools
		 * action
		 */
		virtual bool releaseKey(int key);

		/**
		 * \brief this method is called when the mouse is released above the
		 * Widget.
		 *
		 * Returns wether the Widget has to be repainted after the Tools
		 * action
		 */
		virtual bool release();

		/**
		 * \brief this method is called when the mouse is released above the
		 * Widget and the releaseAction should not be called.
		 *
		 * Returns wether the Widget has to be repainted after the Tools
		 * action
		 */
		virtual bool releaseOnly();

		/**
		 * \brief this method is called when the mouse is moved above the
		 * Widget.
		 *
		 * Returns wether the Widget has to be repainted after the Tools
		 * action
		 */
		virtual bool move(qreal mouseX, qreal mouseY);

		/**
		 * \brief this method is called when the mouse has exited the Widget.
		 */
		virtual void exit();

		/**
		 * \brief this method is called when the mouse has entered the Widget.
		 */
		virtual void enter();

		/**
		 * \brief deselects this Tool.
		 *
		 * Repaints the Tools Button (if existing)
		 */
		void deselect();

		/**
		 * \brief selects this Tool.
		 *
		 * Repaints the Tools Button (if existing)
		 */
		void select();

		bool selected() Q_DECL_OVERRIDE;

		virtual void buttonClick() Q_DECL_OVERRIDE;

		virtual ProtocolEntry *copy() Q_DECL_OVERRIDE;

		virtual void reloadState(ProtocolEntry *entry) Q_DECL_OVERRIDE;
		static void setMatrixWidget(MatrixWidget *w);
		static void setMainWindow(MainWindow *mw);

		bool pointInRect(qreal x, qreal y,  qreal x_start, qreal y_start, qreal x_end, qreal y_end);

	protected:
		bool etool_selected;
		qreal mouseX, mouseY;
		bool mouseIn;
		static MatrixWidget *matrixWidget;
		static MainWindow *_mainWindow;
};

#endif
