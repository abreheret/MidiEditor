#ifndef PROGCHANGEEVENT_H_
#define PROGCHANGEEVENT_H_

#include "MidiEvent.h"

class QComboBox;
class QLabel;

class ProgChangeEvent : public MidiEvent {

	public:
		ProgChangeEvent(int channel, int prog);
		ProgChangeEvent(ProgChangeEvent &other);

		virtual int line();

		QString toMessage();
		QByteArray save();

		virtual ProtocolEntry *copy();
		virtual void reloadState(ProtocolEntry *entry);

		QString typeString();
		void generateWidget(QWidget *widget);
		void editByWidget();
		int program();
		void setProgram(int prog);

	private:
		int _program;
		static QComboBox *_instr_combo;
		static QLabel *_instr_label;
};

#endif
