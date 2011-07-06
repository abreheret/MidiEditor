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
