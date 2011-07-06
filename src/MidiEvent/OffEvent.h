#ifndef OFFEVENT_H_
#define OFFEVENT_H_

#include "MidiEvent.h"
#include <QMultiMap>

class OnEvent;

class OffEvent : public MidiEvent{

	public:
		OffEvent(int ch, int line);
		OffEvent(OffEvent &other);

		void setOnEvent(OnEvent *event);
		OnEvent *onEvent();

		static void enterOnEvent(OnEvent *event);
		static void clearOnEvents();
		static void removeOnEvent(OnEvent *event);
		void draw(QPainter *p, QColor c);
		int line();
		QByteArray save();
		QString toMessage();

		ProtocolEntry *copy();
		void reloadState(ProtocolEntry *entry);

	protected:
		OnEvent *_onEvent;

		// Saves all openes and not closed onEvents. When an offEvent is created,
		// it searches his onEvent in onEvents and removes it from onEvents.
		static QMultiMap<int, OnEvent*> *onEvents;

		// needs to save the line, because offEvents are bound to their onEvents.
		// Setting the line is necessary to find the onEvent in the QMap
		int _line;
};

#endif
