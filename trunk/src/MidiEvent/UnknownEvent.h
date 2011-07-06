#ifndef UNKNOWNEVENT_H_
#define UNKNOWNEVENT_H_

#include "MidiEvent.h"
#include <QByteArray>
class QWdiget;
class UnknownEvent : public MidiEvent{

	public:
		UnknownEvent(int channel, QByteArray data);
		QByteArray data();
		int line();
		QByteArray save();

		void generateWidget(QWidget *widget);

	private:
		QByteArray _data;

};

#endif
