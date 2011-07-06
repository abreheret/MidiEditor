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

#ifndef VELOCITYWIDGET_H_
#define VELOCITYWIDGET_H_

#include "PaintWidget.h"

class MatrixWidget;

#define SINGLE_MODE 1
#define LINE_MODE 2
#define MOUSE_MODE 3

class VelocityWidget : public PaintWidget {

	public:
		VelocityWidget(MatrixWidget *mw, QWidget *parent=0);

	protected:
		void paintEvent(QPaintEvent *event);
		void keyPressEvent(QKeyEvent* e);
		void keyReleaseEvent(QKeyEvent *event);

		void mouseReleaseEvent(QMouseEvent *event);
		void mousePressEvent(QMouseEvent *event);
		void leaveEvent(QEvent *event);

	private:
		MatrixWidget *matrixWidget;

		// Mode is SINGLE_MODE or LINE_MODE
		int mode;

		// dragY for SINGLE_MODE
		int dragY;

		// Start coordinates in the line Mode, will be -1 if no action started
		int startX, startY;

		// is true when there is a drag in the mouse Mode
		bool protocolActionStarted;
};

#endif
