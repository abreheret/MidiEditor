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

#ifndef CONTROLCHANGEEVENT_H_
#define CONTROLCHANGEEVENT_H_

#include "MidiEvent.h"

class QComboBox;
class QLabel;
class QWidget;
class QSpinBox;

class ControlChangeEvent : public MidiEvent {

	public:
		ControlChangeEvent(int channel, int contr, int val);
		ControlChangeEvent(ControlChangeEvent &other);

		virtual int line();
		int control();
		int value();
		void setValue(int v);
		void setControl(int c);

		QString toMessage();
		QByteArray save();

		virtual ProtocolEntry *copy();
		virtual void reloadState(ProtocolEntry *entry);

		QString typeString();
		void generateWidget(QWidget *widget);
		void editByWidget();

	private:
		int _control, _value;
		static QComboBox *_control_combo;
		static QSpinBox *_value_box;
		static QLabel *_control_label, *_value_label;
		static QWidget *_value_widget;
};

#endif
