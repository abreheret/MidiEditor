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

/**
 * \file tool/tool.h
 *
 * \brief Headerfile.
 *
 * Contains the descripton of the class ToolButton
 */
#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <QPushButton>
#include <QFont>
#include <QPainter>

class Tool;

/**
 * \class ToolButton
 *
 * \brief ToolButton represents a Tool with a Button.
 *
 * Every ToolButton is connected to one Tool and paints itself using some
 * parameters of the Tool.
 * A click on the ToolButton will be sent to the Tool.
 */
class ToolButton  : public QPushButton {

	Q_OBJECT
	
	public:
		/**
		 * \brief creates a ToolButton and connects it with tool.
		 */
		ToolButton(Tool *tool, QWidget *parent = 0);
	
	public slots:
		/**
		 * \brief called, when the button is clicked.
		 *
		 * Sends the command to the Tool.
		 */
		void buttonClick();
		/**
		 * \brief called, when the button is released.
		 */
		void releaseButton();
		
	protected:
		void paintEvent(QPaintEvent *event);
		void enterEvent(QEvent *event);
		void leaveEvent(QEvent *event);

	private:
		Tool *button_tool;
		bool button_mouseInButton, button_mouseClicked;
};
#endif
