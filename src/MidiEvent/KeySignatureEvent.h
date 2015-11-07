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

#ifndef KEYSIGNATUREEVENT_H_
#define KEYSIGNATUREEVENT_H_

#include "MidiEvent.h"

#include <QWidget>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>

class KeySignatureEvent  : public MidiEvent {

	public:
		KeySignatureEvent(int channel, int tonality, bool minor, MidiTrack *track);
		KeySignatureEvent(KeySignatureEvent &other);

		virtual int line();

		QString toMessage();
		QByteArray save();

		virtual ProtocolEntry *copy();
		virtual void reloadState(ProtocolEntry *entry);

		QString typeString();
		void generateWidget(QWidget *widget);
		void editByWidget();

		int tonality();
		bool minor();
		void setTonality(int t);
		void setMinor(bool minor);

		private:
			int _tonality;
			bool _minor;
			static QSpinBox *_ton_box;
			static QLabel *_ton_label;
			static QCheckBox *_minor_check;
			static QWidget *_ton_widget, *_minor_widget;
};

#endif
