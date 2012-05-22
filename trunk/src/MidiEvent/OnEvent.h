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

#ifndef ONEVENT_H_
#define ONEVENT_H_

#include "MidiEvent.h"
#include <QWidget>
#include <QLabel>
#include <QSpinBox>
class OffEvent;

class OnEvent : public MidiEvent{

	public:
		OnEvent(int ch);
		OnEvent(OnEvent &other);

		void setOffEvent(OffEvent *event);
		OffEvent *offEvent();
		virtual QByteArray saveOffEvent();
		virtual QString offEventMessage();
		virtual ProtocolEntry *copy();
		virtual void reloadState(ProtocolEntry *entry);

		void generateWidget(QWidget *widget);
		void editByWidget();

		static QSpinBox *_off_tick_box;

	protected:
		OffEvent *_offEvent;

		static QWidget *_off_tick_widget;
		static QLabel *_off_tick_label;

};

#endif
