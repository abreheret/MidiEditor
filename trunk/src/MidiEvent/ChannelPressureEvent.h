#ifndef CHANNELPRESSUREEVENT_H_
#define CHANNELPRESSUREEVENT_H_

#include "MidiEvent.h"

#include <QSpinBox>
#include <QWidget>
#include <QLabel>

class ChannelPressureEvent : public MidiEvent {

	public:
		ChannelPressureEvent(int channel, int value);
		ChannelPressureEvent(ChannelPressureEvent &other);

		virtual int line();

		QString toMessage();
		QByteArray save();

		virtual ProtocolEntry *copy();
		virtual void reloadState(ProtocolEntry *entry);

		QString typeString();
		void generateWidget(QWidget *widget);
		void editByWidget();
		int value();
		void setValue(int v);

	private:
		int _value;
		static QSpinBox *_val_box;
		static QLabel *_val_label;
		static QWidget *_val_widget;
};

#endif
